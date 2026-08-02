#include "source/number.h"

namespace language {
number NumberFromString(const std::string& input) {
  if (input.empty()) {
    throw std::invalid_argument("Cannot parse empty numeric string");
  }

  std::string_view sv(input);

  // 1. Check for float suffix ('f' or 'F')
  if (sv.back() == 'f' || sv.back() == 'F') {
    sv.remove_suffix(1);
    float v;
    auto [ptr, ec] = std::from_chars(sv.data(), sv.data() + sv.size(), v);
    if (ec == std::errc{}) {
      return v;
    }
    throw std::invalid_argument("Invalid float literal: " + input);
  }

  // 2. Check for long suffix ('l' or 'L')
  if (sv.back() == 'l' || sv.back() == 'L') {
    sv.remove_suffix(1);
    long v;
    auto [ptr, ec] = std::from_chars(sv.data(), sv.data() + sv.size(), v);
    if (ec == std::errc{}) {
      return v;
    }
    throw std::invalid_argument("Invalid long literal: " + input);
  }

  // 3. Check if it's a floating-point number (contains '.' or 'e'/'E')
  bool is_floating = sv.find('.') != std::string_view::npos ||
                     sv.find('e') != std::string_view::npos ||
                     sv.find('E') != std::string_view::npos;

  if (is_floating) {
    double v;
    auto [ptr, ec] = std::from_chars(sv.data(), sv.data() + sv.size(), v);
    if (ec == std::errc{}) {
      return v;
    }
    throw std::invalid_argument("Invalid double literal: " + input);
  }

  // 4. Default Integer: Attempt to fit into 'int', fallback to 'long'
  int i_val;
  auto [ptr_i, ec_i] = std::from_chars(sv.data(), sv.data() + sv.size(), i_val);
  if (ec_i == std::errc{}) {
    return i_val;
  }

  long l_val;
  auto [ptr_l, ec_l] = std::from_chars(sv.data(), sv.data() + sv.size(), l_val);
  if (ec_l == std::errc{}) {
    return l_val;
  }

  throw std::invalid_argument("Number out of range or malformed: " + input);
}

}