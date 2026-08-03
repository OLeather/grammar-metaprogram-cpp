#pragma once

#include <string>
#include <variant>

namespace language {
using number = std::variant<float, double, int, long>;

number NumberFromString(const std::string &input);

}; // namespace language