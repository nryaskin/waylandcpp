#ifndef WAYLANDCPP_WIRE_SOCKET_H
#define WAYLANDCPP_WIRE_SOCKET_H

#include <cstdint>
#include <vector>
#include <sys/socket.h>
#include <sys/un.h>

namespace waylandcpp::wire {
    class WLSocket {
    public:
        WLSocket();
        void write(const uint8_t* data, uint32_t size);
        void sendfd(uint32_t fd_, const uint8_t* data, uint32_t size);
        std::vector<uint8_t> read();
        ~WLSocket();

    private:
        int fd = -1;
        struct sockaddr_un addr;
    };
}

#endif /* WAYLANDCPP_WIRE_SOCKET_H */
