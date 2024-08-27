/**
 * https://zig.news/leroycep/wayland-from-the-wire-part-1-12a1
 */

// I am going to use unix domain socket and c api for this one, but later will use boost asio socket, I guess.
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <cassert>
#include <cstring>
#include <string>
#include <iostream>
#include <vector>
#include <cstdint>
#include <array>
#include <iomanip>
#include <format>
#include <variant>
#include <map>
#include <source_location>

std::string WAYLAND_DISPLAY("/run/user/1000/wayland-0");

inline void assert_err(int ret) {
    std::cout << "ret: " << ret << "; errno: " << errno << std::endl;
    assert(ret != -1);
}

using wire_word_t = uint32_t;
using wire_int_t = int32_t;
using wire_uint_t = uint32_t;
using wire_fixed_t = uint32_t;
using wire_object_id_t = uint32_t;
using wire_new_id_t = uint32_t;
using wire_enum_t = uint32_t;
using wire_op_t = uint16_t;
using wire_size_t = uint16_t;

//class WireBuffer;
//
//template<typename T>
//struct wire_t {
//    static size_t size(const T t);
//    static void   put(WireBuffer& wb, T t);
//};
//
//template<typename T>
//concept WireType = requires (T t, WireBuffer& wb) {
//    { t.size() } -> std::same_as<size_t>;
//    { t.put(wb) } -> std::same_as<void>;
//};
//
//
//struct new_id_t {
//    uint32_t value;
//};
//
//
// template<WireType T>
// void add(T t) {
//      resize(wb_.size() + t.size());
//      t.put(wb_);
// }
//
// template<typename ...T>
// void add(T ...t) {
//   resize(wb_.size() + (t.size() + ...));
//   (t.put(wb_)...);
// }
//

//    void add_new_id(wire_new_id_t new_id) {
//        resize(wb_.size() + (sizeof(decltype(new_id)) / 4));
//        wb_[index++] = new_id;
//    }

class WireBuffer {
public:
    WireBuffer(int size = 0) {
        buffer.resize(size);
    }

    WireBuffer(const std::vector<uint8_t>& data) {
        assert(data.size() % 4 == 0);
        buffer.resize(data.size() / 4);
        std::memcpy(buffer.data(), data.data(), data.size());
    }

    WireBuffer(const WireBuffer& wb, uint32_t start, uint32_t size) {
        assert(wb.size() >= start + size);
        buffer.insert(buffer.begin(), wb.buffer.begin() + start, wb.buffer.begin() + start + size);
    }

    WireBuffer(const WireBuffer&) = delete;
    WireBuffer& operator=(const WireBuffer&) = delete;

    WireBuffer(WireBuffer&& w) {
        *this = std::move(w);
    }

    WireBuffer& operator=(WireBuffer&& w) {
        std::swap(this->buffer, w.buffer); 
        return *this;
    }

    uint32_t size() const { return buffer.size(); }

    void resize(uint32_t size) {
        buffer.resize(size);
    }

    const uint8_t* const_data() const { return reinterpret_cast<const uint8_t*>(buffer.data()); }
    int raw_size() const { return buffer.size() * sizeof(uint32_t); }

    uint32_t& operator[](int index) {
        return buffer[index];
    }

    void print() {
        int i = 0;
        while (i < buffer.size()) {
            std::cout << "HEADER: [\n";
            std::cout << "\tobject_id: " << std::hex << buffer[i] << std::dec << "\n";
            uint32_t message_size = buffer[i + 1] >> 16;
            std::cout << "\tmessage_size: " << message_size << ", op_code: " << ( buffer[i + 1] & 0xFFFF) << "\n";
            std::cout << "]\n";
            std::cout << "Body: [\n";
            std::cout << "Raw:\n";
            for (int j = 2; j < message_size / 4; j++) {
                uint8_t* p = reinterpret_cast<uint8_t*>(&buffer[i + j]);
                std::cout << p[0] << p[1] << p[2] << p[3];
            }
            std::cout <<"\n";
            std::cout << "Bytes:\n";
            for (int j = 0; j < message_size / 4; j++) {
                std::cout << "|" << std::setfill('0') << std::setw(8) << std::hex <<  buffer[i + j] << "|" << std::dec;
            }
            std::cout <<"\n";
            std::cout << "]" << std::endl;
            i += message_size / 4;
            assert(message_size != 0);
        }
    }
private:
    std::vector<uint32_t> buffer;
};

class WireObjectBuilder {
public:
    WireObjectBuilder(wire_object_id_t id = 0,
                      wire_op_t op_code = 0)
    : index(0),
      wb_(2) {
        wb_[index++] = id;
        wb_[index++] = (wb_.size() << 16) | op_code;
    }
    WireObjectBuilder(WireObjectBuilder&&) = delete;
    WireObjectBuilder& operator=(WireObjectBuilder&&) = delete;

    WireObjectBuilder(const WireObjectBuilder&) = delete;
    WireObjectBuilder& operator=(const WireObjectBuilder&) = delete;

    void set_opcode(wire_op_t op_code) {
        wb_[1] &= 0xFFFF0000;
        wb_[1] |= op_code;
    }

    void add_new_id(wire_new_id_t new_id) {
        resize(wb_.size() + (sizeof(decltype(new_id)) / 4));
        wb_[index++] = new_id;
    }

    template<typename T>
    void add(T t) {
        
    }

    void print() {
        wb_.print();
    }

    const uint8_t* data() const {
        return wb_.const_data();
    }

    uint8_t size() {
        return wb_.raw_size();
    }

private:
    void resize(wire_size_t size) {
        wb_.resize(size);
        wb_[1] &= 0x0000FFFF;
        wb_[1] |= ((wb_.size() * 4) << 16);
    }

    uint32_t index;
    WireBuffer wb_;
};

class WireBufferParser {
public:
    struct Event {
        wire_object_id_t id;
        wire_op_t        op_code;
        WireBuffer       buffer;
    };

    WireBufferParser(WireBuffer& buffer) : buffer_(buffer) {}

    Event event() {
        Event ev;
        ev.id = buffer_[index++];
        wire_word_t size_op = buffer_[index++];
        ev.op_code = size_op & 0x0000FFFF;
        wire_size_t size = size_op >> 16;
        auto body_size = (size / 4 - 2);
        ev.buffer = WireBuffer(buffer_, index, body_size);
        index += body_size;
        return std::move(ev);
    }

    wire_object_id_t object_id() {
        wire_object_id_t id = buffer_[index++];
        return id;
    }

    wire_uint_t uint() {
        wire_uint_t uint = buffer_[index++];
        return uint;
    }

    // TODO: It is UTF-8 so I shall decode it but I am lazy, will do it properly when needed
    std::string string() {
        std::string result = "";
        auto size = buffer_[index++];
        auto buffer_size = (size + 3) / 4;
        int i = 0;
        while (i < buffer_size) {
            uint32_t word = buffer_[index + i++];
            std::vector<char> values = {
                static_cast<char>(word & 0xff),
                static_cast<char>(word >> 8 & 0xff),
                static_cast<char>(word >> 16 & 0xff),
                static_cast<char>(word >> 24)
            };
            for (auto& c: values) {
                if (c != '\0') {
                    result.push_back(c);
                }
                else {
                    break;
                }
            }
        }
        index += buffer_size;
        return result;
    }

    bool isEnd() {
        return buffer_.size() == index;
    }

private:
    WireBuffer& buffer_;
    uint32_t index = 0;
};

class WLSocket {
public:
    WLSocket() {
        int ret;
        fd = socket(AF_UNIX, SOCK_STREAM, 0);        
        assert(fd >= 0);
        addr.sun_family = AF_UNIX;
        std::strncpy(addr.sun_path, WAYLAND_DISPLAY.c_str(), WAYLAND_DISPLAY.size());
        ret = connect(fd, reinterpret_cast<struct sockaddr*>(&addr), sizeof(decltype(addr))); 
        assert_err(ret);
    }

    void write(const uint8_t* data, uint32_t size) {
        int ret = ::write(fd, data, size);
        assert_err(ret);
    }

    std::vector<uint8_t> read() {
        std::vector<uint8_t> buffer;
        std::array<uint8_t, 1024> buf;
        int ret = buf.size();

        // Q: Can it go on forever if there is too much data in socket?
        while(ret == buf.size()) {
            ret = ::read(fd, buf.data(), buf.size());
            assert_err(ret);
            buffer.insert(buffer.end(), buf.begin(), buf.begin() + ret);
        }
        assert(buffer.size() % 4 == 0);
        return std::move(buffer);
    }

    ~WLSocket() {
        close(fd);
    }

private:
    int fd = -1;
    struct sockaddr_un addr;
};

class WLDisplay {
    wire_object_id_t id = 1;
    wire_op_t sync_op = 0x0000;
    wire_op_t get_registry_op = 0x0001;
public:
    enum class error : wire_uint_t {
        INVALID_OBJECT = 0,
        INVALID_METHOD = 1,
        NO_MEMORY = 2,
        IMPLEMENTATION = 3
    };

    WLDisplay(WLSocket& s, std::string&& name = "") : s_(s) {}

    // Methods
    void get_registry(wire_new_id_t registry_id) {
        WireObjectBuilder builder(id, get_registry_op);
        builder.add_new_id(registry_id);
        builder.print();
        s_.write(builder.data(), builder.size());
    }

    void sync(wire_new_id_t callback_id) {
        WireObjectBuilder builder(id, sync_op);
        builder.add_new_id(callback_id);
        builder.print();
        s_.write(builder.data(), builder.size());
    }

    // Events
    void error(wire_object_id_t id, wire_uint_t code, std::string& msg) {
        std::cout << std::format("{}> id: {}, code: {}, message: {}", std::source_location::current().function_name(), id, code, msg) << std::endl; 
    }

    void delete_id(wire_object_id_t id) {
        std::cout << std::format("{}> {}", std::source_location::current().function_name(), id) << std::endl;
    }

private:
    WLSocket& s_;
};

class WLRegistry {
public:
    WLRegistry(WLSocket& s, wire_object_id_t id) : s_(s), id_(id) {}

    // Requests
    void bind(wire_uint_t name, wire_new_id_t id) {

    }

    // Events
    void global(wire_uint_t name, const std::string& interface, wire_uint_t version) {
        std::cout << std::format("{}> name: {}, interface: {}, version: {}", std::source_location::current().function_name(), name, interface, version) << std::endl;
    }

    void global_remove(wire_uint_t name) {
        std::cout << std::format("{}> name: {}", std::source_location::current().function_name(), name) << std::endl;
    }

private:
    wire_object_id_t id_;
    WLSocket& s_;
};

class WLCallback {
public:
    WLCallback(WLSocket& s, wire_object_id_t id) : s_(s), id_(id) {}
    // Events
    void done(wire_uint_t callback_data) {
        std::cout << std::format("{}> callback_data: {}", std::source_location::current().function_name(), callback_data) << std::endl;
    }
private:
    wire_object_id_t id_;
    WLSocket& s_;
};

using wl_object_t = std::variant<std::monostate, WLDisplay, WLRegistry, WLCallback>;

int main() {
    WLSocket socket;
    std::map<wire_object_id_t, wl_object_t> objects;
    WLDisplay display(socket);
    objects.insert({ 1, display});
    display.get_registry(2);
    WLRegistry registry(socket, 2);
    objects.insert({ 2, registry});
    display.sync(3);
    WLCallback callback(socket, 3);
    objects.insert({3, callback});

    auto&& buffer = socket.read();

    WireBuffer wb(buffer);
    wb.print();
    auto parser = WireBufferParser(wb);
    while (!parser.isEnd()) {
        auto&& ev = parser.event();
        std::visit([e = std::move(ev)](auto &&object) mutable {
            using T = std::decay_t<decltype(object)>;
            if constexpr (std::is_same_v<T, WLDisplay>) {
                WireBufferParser body(e.buffer);
                switch (e.op_code) {
                    case 0:
                    {
                        auto object_id = body.object_id();
                        auto code = body.uint();
                        auto msg = body.string();
                        object.error(object_id, code, msg);
                    }
                    break;
                    case 1:
                    {
                        auto object_id = body.object_id();
                        object.delete_id(object_id);
                    }
                    break;
                    default:
                        assert(false);
                }
            }
            else if constexpr(std::is_same_v<T, WLRegistry>) {
                WireBufferParser body(e.buffer);
                switch(e.op_code) {
                    case 0:
                    {
                        auto&& name = body.uint();
                        auto&& interface = body.string();
                        auto&& version = body.uint();
                        object.global(name, interface, version);
                    }
                    break;
                    case 1:
                    {
                        auto&& name = body.uint();
                        object.global_remove(name);
                    }
                    break;
                    default:
                        assert(false);
                }
            }
            else if constexpr(std::is_same_v<T, WLCallback>) {
                if (e.op_code != 0) {
                    assert(false);
                }
                WireBufferParser body(e.buffer);
                object.done(body.uint());
            }
        }, objects[ev.id]);
    }
    
    return 0;
}
