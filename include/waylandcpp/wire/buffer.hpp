#ifndef WAYLANDCPP_WIRE_BUFFER_H
#define WAYLANDCPP_WIRE_BUFFER_H

#include <cstdint>
#include <vector>

namespace waylandcpp::wire {
    class WireBuffer {
    public:
        WireBuffer(int size = 0);

        WireBuffer(const std::vector<uint8_t>& data);

        WireBuffer(const WireBuffer& wb, uint32_t start, uint32_t size);

        WireBuffer(const WireBuffer&) = delete;
        WireBuffer& operator=(const WireBuffer&) = delete;

        WireBuffer(WireBuffer&& w);

        WireBuffer& operator=(WireBuffer&& w);

        uint32_t size() const;

        void resize(uint32_t size);

        const uint8_t* const_data() const;
        int raw_size() const;

        uint32_t& operator[](int index);

    private:
        std::vector<uint32_t> buffer;
    };
}

#endif /* WAYLANDCPP_WIRE_BUFFER_H */
