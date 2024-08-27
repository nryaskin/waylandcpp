#pragma once

#include "waylandcpp/wire/types.hpp"
#include "waylandcpp/local_storage.hpp"

namespace waylandcpp {

    template<typename socket_t, typename ...wl_object_t>
    class context_t {
    public:
        using native_id = decltype(wire::wire_object_id_t::value);
        // This way I cannot change socket but I guess socket equal session so when socket closes there is nothing can be done then restarting everything.
        context_t(socket_t& socket) : socket(socket) {}

        template<typename wayland_type>
        wayland_type& create(auto&& remote_init, auto&& ...callbacks) {
            auto& obj = create_local<wayland_type>(std::forward<decltype(callbacks)>(callbacks)...);
            remote_init(obj);
            return obj;
        }

        template<typename wayland_type>
        wayland_type& create_local(auto&& ...callbacks) {
            auto id = next_id++;
            auto& obj = local_storage.template create<wayland_type>(socket, id, std::forward<decltype(callbacks)>(callbacks)...);
            return obj;
        }

        bool contains(id_t id) {
            return local_storage.contains(id);
        }

        template<typename wayland_type>
        wayland_type& get(id_t id) {
            return local_storage.template get<wayland_type>(id);
        }

        void dispatch(id_t id, wire::Event& ev) { return local_storage.dispatch(id, ev); }

    private:
        socket_t& socket;
        local_storage_t<native_id, wl_object_t...> local_storage;
        native_id next_id = 1;
    };
}
