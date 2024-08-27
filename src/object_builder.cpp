#include "waylandcpp/wire/object_builder.hpp"

namespace waylandcpp::wire {

    WireObjectBuilder::WireObjectBuilder(wire_new_id_t id,
                                         wire_op_t op_code)
    : index(0),
      wb_(2) {
        id.put(wb_, index);
        index+=id.size();
        wb_[index++] = ((wb_.size() * 4) << 16) | op_code;
    }

    WireObjectBuilder::WireObjectBuilder(wire_object_id_t id,
                                         wire_op_t op_code)
    : index(0),
      wb_(2) {
        id.put(wb_, index);
        index+=id.size();
        wb_[index++] = (wb_.size() * 4 << 16) | op_code;
    }

    void WireObjectBuilder::set_opcode(wire_op_t op_code) {
        wb_[1] &= 0xFFFF0000;
        wb_[1] |= op_code;
    }

    const uint8_t* WireObjectBuilder::data() const {
        return wb_.const_data();
    }

    uint8_t WireObjectBuilder::size() {
        return wb_.raw_size();
    }

    void WireObjectBuilder::resize(wire_size_t size) {
        wb_.resize(size);
        wb_[1] &= 0x0000FFFF;
        wb_[1] |= ((wb_.size() * 4) << 16);
    }
}
