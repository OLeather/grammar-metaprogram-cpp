#pragma once
#include <string_view>

namespace language {

struct Context {
  std::string_view input;
};

template <typename T> struct Result {
  Context ctx;
  T value;
};

} // namespace language