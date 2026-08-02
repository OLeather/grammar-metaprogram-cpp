#pragma once

#include <charconv>
#include <optional>
#include <stdexcept>
#include <string_view>
#include <variant>

namespace language {
using number = std::variant<float, double, int, long>;

number NumberFromString(const std::string &input);

}; // namespace language