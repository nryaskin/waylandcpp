#ifndef WAYLANDCPP_WIRE_OBJECT_BUILDER_H
#define WAYLANDCPP_WIRE_OBJECT_BUILDER_H

#include "waylandcpp/wire/types.hpp"
#include "waylandcpp/wire/buffer.hpp"

namespace waylandcpp::wire {
    class WireObjectBuilder {
    public:

        WireObjectBuilder(wire_new_id_t id = 0,
                          wire_op_t op_code = 0);

        WireObjectBuilder(wire_object_id_t id = 0,
                          wire_op_t op_code = 0);

        WireObjectBuilder(WireObjectBuilder&&) = delete;
        WireObjectBuilder& operator=(WireObjectBuilder&&) = delete;

        WireObjectBuilder(const WireObjectBuilder&) = delete;
        WireObjectBuilder& operator=(const WireObjectBuilder&) = delete;

        void set_opcode(wire_op_t op_code);

        template<WireType T>
        void add(T t) {
             resize(wb_.size() + t.size());
             t.put(wb_, index);
             index += t.size();
        }
        
        template<WireType ...T>
        void add(T ...t) {
          resize(wb_.size() + (t.size() + ...));
          auto incr = [&](auto size) { auto prev = index; index += size; return prev; };
          (t.put(wb_, incr(t.size())),...);
          //index += (t.size() + ...);
        }

        void append(uint32_t data);

        const uint8_t* data() const;
        uint8_t size();
    private:
        void resize(wire_size_t size);
        uint32_t index;
        WireBuffer wb_;
    };
}

#endif /* WAYLANDCPP_WIRE_OBJECT_BUILDER_H */
