#include "waylandcpp/wire/buffer.hpp"
#include <cassert>
#include <cstring>

namespace waylandcpp::wire {
    WireBuffer::WireBuffer(int size) {
        buffer.resize(size);
    }

    WireBuffer::WireBuffer(const std::vector<uint8_t>& data) {
        assert(data.size() % 4 == 0);
        buffer.resize(data.size() / 4);
        std::memcpy(buffer.data(), data.data(), data.size());
    }

    WireBuffer::WireBuffer(const WireBuffer& wb, uint32_t start, uint32_t size) {
        assert(wb.size() >= start + size);
        buffer.insert(buffer.begin(), wb.buffer.begin() + start, wb.buffer.begin() + start + size);
    }

    WireBuffer::WireBuffer(WireBuffer&& w) {
        *this = std::move(w);
    }

    WireBuffer& WireBuffer::operator=(WireBuffer&& w) {
        std::swap(this->buffer, w.buffer); 
        return *this;
    }

    uint32_t WireBuffer::size() const { return buffer.size(); }

    void WireBuffer::resize(uint32_t size) {
        buffer.resize(size);
    }

    const uint8_t* WireBuffer::const_data() const { return reinterpret_cast<const uint8_t*>(buffer.data()); }
    int WireBuffer::raw_size() const { return buffer.size() * sizeof(uint32_t); }

    uint32_t& WireBuffer::operator[](int index) {
        return buffer[index];
    }
}
