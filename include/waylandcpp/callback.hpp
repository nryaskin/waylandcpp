#pragma once

#include <functional>

namespace waylandcpp {
    template<typename ...Types>
    using callback_t = std::function<void(Types...)>;
}
