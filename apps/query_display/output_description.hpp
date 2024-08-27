#pragma once

#include <cstdint>
#include <string>
#include <format>

#include "waylandcpp/wl_output.hpp"

namespace wi = waylandcpp::interface;

namespace bicycle_engine {
    class OutputDescription {
    public:
        using subpixel_t = wi::wl_output::subpixel_e;
        using transform_t = wi::wl_output::transform_e;
        using mode_t = wi::wl_output::mode_e;
        friend std::ostream& operator<<(std::ostream& os, const OutputDescription& od);

        std::string subpixel_description() const {
            switch (subpixel) {
                case subpixel_t::unknown:
                    return "Unknown";
                case subpixel_t::none:
                    return "None";
                case subpixel_t::horizontal_rgb:
                    return "Horizontal RGB";
                case subpixel_t::horizontal_bgr:
                    return "Horizontal BGR";
                case subpixel_t::vertical_rgb:
                    return "Vertical RGB";
                case subpixel_t::vertical_bgr:
                    return "Vertical BGR";
                default:
                    return "Unhandled format";
            }
        }

        std::string transform_description() const {
            switch (transform) {
                case transform_t::normal:
                    return "no transform";
                case transform_t::_90:
                    return "1-90 degrees counter-clockwise";
                case transform_t::_180:
                    return "2-180 degrees counter-clockwise";
                case transform_t::_270:
                    return "3-270 degrees counter-clockwise";
                case transform_t::flipped:
                    return "4 - 180 degree flip around a vertical axis";
                case transform_t::flipped_90:
                    return "5 - flip and rotate 90 degrees counter-clockwise";
                case transform_t::flipped_180:
                    return "6 - flip and rotate 180 degrees counter-clockwise";
                case transform_t::flipped_270:
                    return "7 - flip and rotate 270 degrees counter-clockwise";
                default:
                    return "Unhandled transform";
            }
        }

        std::string mode_description() const {
            switch (flags) {
                case mode_t::current:
                    return "current";
                case mode_t::preferred:
                    return "prefferd";
                default:
                    return "Unhandled mode";
            }
        }

        void geometry_cb(int32_t x,
                         int32_t y,
                         int32_t physical_width,
                         int32_t physical_height,
                         int32_t subpixel,
                         const std::string& make,
                         const std::string& model,
                         int32_t transform) {
            this->x = x;
            this->y = y;
            this->physical_width = physical_width;
            this->physical_height = physical_height;
            this->subpixel = static_cast<subpixel_t>(subpixel);
            this->make = make;
            this->model = model;
            this->transform = static_cast<transform_t>(transform);
        }

        void mode_cb(uint32_t flags,
                     int32_t  width,
                     int32_t  height,
                     int32_t  refresh) {
            this->flags = static_cast<mode_t>(flags);
            this->width = width;
            this->height = height;
            this->refresh = refresh;
        }

        void scale_cb(int32_t factor) {
            this->factor = factor;
        }

        void name_cb(const std::string& name) {
            this->name = name;
        }

        void description_cb(const std::string& description) {
            this->description = description;
        }

    private:
        int32_t     x;
        int32_t     y;
        int32_t     physical_width;
        int32_t     physical_height;
        subpixel_t  subpixel;
        std::string make;
        std::string model;
        transform_t transform;
        mode_t      flags;
        int32_t     width;
        int32_t     height;
        int32_t     refresh;
        int32_t     factor;
        std::string name;
        std::string description;
    };

    inline std::ostream& operator<<(std::ostream& os, const OutputDescription& od) {
        os << std::format("Manufacturer : {}\n", od.make);
        os << std::format("Model: {}\n", od.model);
        os << std::format("Name: {}\n", od.name);
        os << std::format("Description: {}\n", od.description);
        os << std::format("Position within the global compositor space: [{},{}]\n", od.x, od.y);
        os << std::format("Physical size: {} mm x {} mm\n", od.physical_width, od.physical_height);
        os << std::format("Subpixel: {}\n", od.subpixel_description());
        os << std::format("Transform: {}\n", od.transform_description());
        os << std::format("Resolution: {} x {}, refresh rate: {} mHZ, mode: {}\n", od.width, od.height, od.refresh, od.mode_description());
        os << std::format("Scale: {}\n", od.factor);
        return os;
    }
}
