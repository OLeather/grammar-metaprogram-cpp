#pragma once

#include "source/grammar.h"

namespace language {

void PrintIndent(const size_t &indent) {
  for (size_t i = 0; i < indent * 2; i++) {
    std::cout << " ";
  };
}

bool IsWhitespaceOnly(const std::string &str) {
  return std::all_of(str.begin(), str.end(),
                     [](unsigned char ch) { return std::isspace(ch); });
}

struct NodePrinter {
  size_t indent;

  template <typename... T> void operator()(const std::tuple<T...> &tup) {
    std::apply(
        [this](const auto &...element) {
          (NodePrinter{indent + 1}(element), ...);
        },
        tup);
  }

  template <typename T> void operator()(const boost::recursive_wrapper<T> &t) {
    auto value = t.get().value;
    NodePrinter{indent}(value);
  }

  template <typename... T> void operator()(const std::variant<T...> &v) {
    std::visit(NodePrinter{indent}, v);
  }

  template <typename T> void operator()(const std::vector<T> &v) {
    for (const auto &e : v) {
      NodePrinter{indent + 1}(e);
    }
  }

  template <FixedString Str> void operator()(const Regex<Str> &r) {
    if (IsWhitespaceOnly(r.match)) {
      return;
    }

    PrintIndent(indent);
    std::cout << "Regex(\"" << r.match << "\")" << std::endl;
  }

  template <typename T> void operator()(const std::optional<T> &opt) {
    if (opt.has_value()) {
      NodePrinter{indent}(opt.value());
    }
  }
  void operator()(const std::monostate& m) {}
  void operator()(const EndOfFile& m) {}
};
} // namespace language