#include "waylandcpp/wire/buffer_parser.hpp"
#include <string>

namespace waylandcpp::wire {

    WireBufferParser::WireBufferParser(WireBuffer& buffer) : buffer_(buffer) {}

    Event WireBufferParser::event() {
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

    wire_object_id_t WireBufferParser::object_id() {
        wire_object_id_t id = buffer_[index++];
        return id;
    }

    wire_uint_t WireBufferParser::uint() {
        wire_uint_t uint = buffer_[index++];
        return uint;
    }

    wire_int_t WireBufferParser::int_() {
        wire_int_t int_v = buffer_[index++];
        return int_v;
    }

    // TODO: It is UTF-8 so I shall decode it but I am lazy, will do it properly when needed
    wire_string_t WireBufferParser::string() {
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

    bool WireBufferParser::isEnd() {
        return buffer_.size() == index;
    }
}
