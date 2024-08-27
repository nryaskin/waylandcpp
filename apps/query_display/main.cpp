#include <iostream>
#include <format>

#include "output_description.hpp"

#include "waylandcpp/wl_display.hpp"
#include "waylandcpp/wl_registry.hpp"
#include "waylandcpp/wl_output.hpp"
#include "waylandcpp/wl_callback.hpp"
#include "waylandcpp/wire/socket.hpp"
#include "waylandcpp/context.hpp"
#include "waylandcpp/wire/buffer_parser.hpp"
#include "waylandcpp/wire/types.hpp"

namespace wi = waylandcpp::interface;
namespace ww = waylandcpp::wire;
using wl_context_t = waylandcpp::context_t<ww::WLSocket, wi::wl_display, wi::wl_registry, wi::wl_callback, wi::wl_output>;

class UIContext {
public:
    UIContext() : ctx(socket) {
        auto& display = ctx.create_local<wi::wl_display>([&](auto id, auto code, auto message) {
                                                       std::cout << std::format("ERROR! ID: {}, CODE: {}, MESSAGE: {}\n", id.value, code.value, message.value);
                                                       stop = true;
                                                   },
                                                   [](auto id) {});
        auto& registry = ctx.create<wi::wl_registry>([&display](auto& registry) { display.get_registry(registry.id().value); },
                                                     [&](auto name, auto interface, auto version) {
                                                         if (interface.value == wi::wl_output::interface) {
                                                             output_received(name.value, interface.value, version.value);
                                                         }
                                                     }, [](auto name) {
                                                     });
        auto& cb = ctx.create<wi::wl_callback>([&display](auto& callback) { display.sync(callback.id().value); });
    }

    void run() {
        while (!stop) {
            auto&& buffer = socket.read();
            ww::WireBuffer wb(buffer);
            auto parser = ww::WireBufferParser(wb);
            while (!parser.isEnd()) {
                auto&& ev = parser.event();
                ctx.dispatch(ev.id.value, ev);
            }
        }
    }

    ~UIContext() {}
private:
    void output_received(uint32_t name, const std::string& interface, uint32_t version) {
        auto& output = ctx.create<wi::wl_output>(
            [&](auto& output) {
                ww::wire_new_id_t output_new_id (output.id().value, "wl_output", version);
                ctx.get<wi::wl_registry>(2).bind(name, output_new_id);
            },
            [&](auto ...args) { geometry_cb(args...); },
            [&](auto ...args) { mode_cb(args...); },
            [&](auto ...args) { done_cb(args...); },
            [&](auto ...args) { scale_cb(args...); },
            [&](auto ...args) { name_cb(args...); },
            [&](auto ...args) { description_cb(args...); }
        );
    }

    void geometry_cb(int32_t x,
                     int32_t y,
                     int32_t physical_width,
                     int32_t physical_height,
                     int32_t subpixel,
                     const std::string& make,
                     const std::string& model,
                     int32_t transform) {
        output_description.geometry_cb(x, y, physical_width, physical_height, subpixel, make, model, transform);
    }

    void mode_cb(uint32_t flags,
                 int32_t  width,
                 int32_t  height,
                 int32_t  refresh) {
        output_description.mode_cb(flags, width, height, refresh);
    }

    void done_cb() {
        stop = true;
        std::cout << output_description;
    }

    void scale_cb(int32_t factor) {
        output_description.scale_cb(factor);
    }

    void name_cb(const std::string& name) {
        output_description.name_cb(name);
    }

    void description_cb(const std::string& description) {
        output_description.description_cb(description);
    }

    waylandcpp::wire::WLSocket socket;
    wl_context_t ctx;
    bicycle_engine::OutputDescription output_description;
    bool stop = false;
};

int main() {
    try {
        UIContext ui_ctx;
        ui_ctx.run();
    }
    catch (...) {
        throw;
    }
    return 0;
}
