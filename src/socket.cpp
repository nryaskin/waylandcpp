#include "waylandcpp/wire/socket.hpp"

#include <array>
#include <cassert>
#include <cstring>
#include <iostream>
#include <unistd.h>
#include <cerrno>
#include <format>


namespace waylandcpp::wire {
    inline void assert_err(int ret) {
        std::cout << "ret: " << ret << "; errno: " << errno << std::endl;
        assert(ret != -1);
    }

    std::string WAYLAND_DISPLAY("/run/user/1000/wayland-0");

    WLSocket::WLSocket() {
        int ret;
        fd = socket(AF_UNIX, SOCK_STREAM | SOCK_NONBLOCK, 0);        
        if (fd < 0) {
            throw std::runtime_error("Cannot open socket");
        }
        addr.sun_family = AF_UNIX;
        std::strncpy(addr.sun_path, WAYLAND_DISPLAY.data(), WAYLAND_DISPLAY.size() + 1);
        ret = connect(fd, reinterpret_cast<struct sockaddr*>(&addr), sizeof(decltype(addr))); 
        if (ret == -1) {
            throw std::runtime_error("Cannot create socket: ");
        }
    }

    void WLSocket::sendfd(uint32_t fd_, const uint8_t* data, uint32_t size) {
        struct msghdr header = {0};
        struct cmsghdr *cmsg;
        struct iovec io = {
            .iov_base = const_cast<uint8_t*>(data),
            .iov_len = size
        };

        union {         /* Ancillary data buffer, wrapped in a union
                           in order to ensure it is suitably aligned */
            char buf[CMSG_SPACE(sizeof(fd_))];
            struct cmsghdr align;
        } u;

        header.msg_name = nullptr;
        header.msg_namelen = 0;
        header.msg_iov = &io;
        header.msg_iovlen = 1;
        header.msg_control = u.buf;
        header.msg_controllen = sizeof(u.buf);
        header.msg_flags = 0;

        cmsg = CMSG_FIRSTHDR(&header);
        cmsg->cmsg_level = SOL_SOCKET;
        cmsg->cmsg_type = SCM_RIGHTS;
        cmsg->cmsg_len = CMSG_LEN(sizeof(fd_));
        memcpy(CMSG_DATA(cmsg), &fd_, sizeof(fd_));

        int ret = ::sendmsg(fd, &header, MSG_NOSIGNAL | MSG_DONTWAIT);

        if (ret < 0) {
            throw std::runtime_error(std::format("Error while write: {:x}, {}", errno, std::strerror(errno)));
        }
    }

    void WLSocket::write(const uint8_t* data, uint32_t size) {
        int ret = ::write(fd, data, size);
        if (ret < 0) {
            throw std::runtime_error(std::format("Error while write: {:x}, {}", errno, std::strerror(errno)));
        }
    }

    std::vector<uint8_t> WLSocket::read() {
        std::vector<uint8_t> buffer;
        std::array<uint8_t, 1024> buf;
        int ret = buf.size();

        // Q: Can it go on forever if there is too much data in socket?
        while(ret == buf.size()) {
            ret = ::read(fd, buf.data(), buf.size());
            if (ret == -1) {
                if (errno == EAGAIN) {
                    break;
                } else {
                    throw std::runtime_error(std::format("Cannot read from socket {:x}, {}", errno, std::strerror(errno)));
                }
            }
            buffer.insert(buffer.end(), buf.begin(), buf.begin() + ret);
        }
        assert(buffer.size() % 4 == 0);
        return std::move(buffer);
    }

    WLSocket::~WLSocket() {
        close(fd);
    }
}
