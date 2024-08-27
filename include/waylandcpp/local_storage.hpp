#pragma once

#include <tuple>
#include <variant>
#include <unordered_map>
#include <stdexcept>

#include "waylandcpp/wire/types.hpp"
#include "waylandcpp/wire/buffer_parser.hpp"

namespace waylandcpp {

    template<typename id_t, typename ...wl_object_t>
    class local_storage_t {
    public:
        using object_t = std::variant<std::monostate, wl_object_t...>;

        local_storage_t() {}

        template<typename wayland_type>
        wayland_type& create(auto& socket, int id, auto&& ...callbacks) {
            wayland_type object(socket, id, std::forward<decltype(callbacks)>(callbacks)...);
            auto [it, inserted] = objects.emplace(std::make_pair(id, object));
            if (inserted) {
                return std::get<wayland_type>(it->second);
            }
            else throw std::runtime_error("Cannot insert new wayland object");
        }

        template<typename expected_t>
        expected_t& get(id_t id) {
            return std::get<expected_t>(objects[id]);
        }

        auto contains(id_t id) {
            return objects.contains(id);
        }

        void dispatch(id_t id, wire::Event& ev) {
            if (auto it = objects.find(id); it != objects.end()) {
                std::visit([&](auto&& wl_obj) {
                    using Type = std::decay_t<decltype(wl_obj)>;
                    if constexpr (std::is_same_v<std::monostate, Type>) {
                        throw std::runtime_error("We shall not be here!");
                    } else {
                        wire::WireBufferParser body(ev.buffer);
                        wl_obj.dispatch(ev.op_code, body);
                    }
                }, it->second);
            }
            else throw std::runtime_error("No such object!");
        }

    private:
        std::unordered_map<id_t, object_t> objects;
    };
}
