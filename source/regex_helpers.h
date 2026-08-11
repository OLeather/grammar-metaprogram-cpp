#pragma once
#include "source/result.h"
#include <optional>
#include <regex>
namespace language {

template <std::size_t N> struct FixedString {
  char value[N]{};
  constexpr FixedString(const char (&str)[N]) { std::copy_n(str, N, value); }
  constexpr auto operator<=>(const FixedString &) const = default;
};

template <typename T>
std::optional<Result<T>> MatchRegex(const std::regex &regex, Context ctx) {
  const auto match_type = std::regex_constants::match_continuous;

  std::match_results<std::string_view::const_iterator> match;
  const bool search_result = std::regex_search(
      ctx.input.begin(), ctx.input.end(), match, regex, match_type);

  const bool success = search_result && !match.empty();
  if (!success)
    return std::nullopt;

  const size_t match_length = match.length(0);
  const Context new_ctx{ctx.input.substr(match_length)};
  const auto consumed = ctx.input.substr(0, match_length);

  return Result<T>{.ctx = new_ctx, .value = T(std::string(consumed))};
};

} // namespace language
