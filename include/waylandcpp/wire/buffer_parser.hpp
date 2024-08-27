#ifndef WAYLANDCPP_WIRE_BUFFER_PARSER_H
#define WAYLANDCPP_WIRE_BUFFER_PARSER_H

#include "waylandcpp/wire/types.hpp"

namespace waylandcpp::wire {
    struct Event {
        wire_object_id_t id;
        wire_op_t        op_code;
        WireBuffer       buffer;
    };

    class WireBufferParser {
    public:
        WireBufferParser(WireBuffer& buffer);

        Event event();

        wire_object_id_t object_id();
        wire_uint_t uint();
        wire_int_t int_();
        wire_string_t string();

        template<typename ...Types>
        std::tuple<Types...> parse() {
            return { get<Types>()...};
        }

        template<typename wire_type>
        auto get() {
            if constexpr (std::is_same_v<wire_type, wire_object_id_t>) {
                return object_id();
            }
            if constexpr (std::is_same_v<wire_type, wire_uint_t>) {
                return uint();
            }
            if constexpr (std::is_same_v<wire_type, wire_int_t>) {
                return int_();
            }
            if constexpr (std::is_same_v<wire_type, wire_string_t>) {
                return string();
            }
        };

        bool isEnd();

    private:
        WireBuffer& buffer_;
        uint32_t index = 0;
    };

}

#endif /* WAYLANDCPP_WIRE_BUFFER_PARSER_H */
