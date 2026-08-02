#pragma once

#include <algorithm>
#include <memory>
#include <optional>
#include <regex>
#include <string>
#include <type_traits>

namespace language {

/*
Core
*/

struct Context {
  std::string_view input;
};

template <typename T> struct Result {
  Context ctx;
  T value;
};

template <typename Rule> auto match_rule(Context ctx, const bool consume) {
  return Rule::Match(ctx, consume);
}

/*
AST
*/

struct ASTNode {
  virtual ~ASTNode() = default;
};

using ASTPtr = std::shared_ptr<ASTNode>;

/*
Regex
*/

template <std::size_t N> struct FixedString {
  constexpr FixedString(const char (&str)[N]) { std::copy_n(str, N, value); }
  char value[N];
};

template <FixedString RegexStr, typename Converter> struct Regex {
  using ReturnType = ASTPtr;

  static std::optional<Result<ReturnType>> Match(Context ctx, const bool consume) {
    const std::regex regex{RegexStr.value};

    const auto match_type = std::regex_constants::match_continuous;

    std::match_results<std::string_view::const_iterator> match;
    const bool search_result = std::regex_search(
        ctx.input.begin(), ctx.input.end(), match, regex, match_type);

    if (search_result && !match.empty()) {
      const size_t match_length = match.length(0);
      const auto new_input = consume ? ctx.input.substr(match_length) : ctx.input; 
      const Context new_ctx{new_input};
      const auto consumed = ctx.input.substr(0, match_length);
      const auto make_value = [&]() -> ReturnType {
        if constexpr (std::is_constructible_v<Converter, std::string>) {
          return std::make_shared<Converter>(std::string(consumed));
        } else if constexpr (std::is_default_constructible_v<Converter>) {
          return std::make_shared<Converter>();
        } else {
          static_assert(
              std::is_constructible_v<Converter, std::string> ||
                  std::is_default_constructible_v<Converter>,
              "Converter must be constructible from std::string or default-constructible");
          return std::make_shared<Converter>();
        }
      };

      return Result<ReturnType>{.ctx = new_ctx, .value = make_value()};
    }
    return std::nullopt;
  }
};

/*
Or
*/

template <typename... Definitions> struct Or;

template <> struct Or<> {
  using ReturnType = ASTPtr;

  static std::optional<Result<ReturnType>> Match(Context, const bool consume) {
    return std::nullopt;
  }
};

template <typename Head, typename... Tail> struct Or<Head, Tail...> {
  using ReturnType = ASTPtr;

  static std::optional<Result<ReturnType>> Match(Context ctx, const bool consume) {
    if (auto next = match_rule<Head>(ctx, consume)) {
      return Result<ReturnType>{next->ctx, next->value};
    }
    if constexpr (sizeof...(Tail) > 0) {
      if (auto next = match_rule<Or<Tail...>>(ctx, consume)) {
        return Result<ReturnType>{next->ctx, next->value};
      }
    }
    return std::nullopt;
  }
};

/*
Sequence
*/

template <typename... Definitions> struct Sequence;

template <> struct Sequence<> {
  using ReturnType = std::tuple<>;

  static std::optional<Result<ReturnType>> Match(Context ctx, const bool consume) {
    return Result<ReturnType>{ctx, {}};
  }
};

template <typename Head, typename... Tail> struct Sequence<Head, Tail...> {
  using ReturnType = decltype(std::tuple_cat(
      std::declval<std::tuple<typename Head::ReturnType>>(),
      std::declval<typename Sequence<Tail...>::ReturnType>()));

  static std::optional<Result<ReturnType>> Match(Context ctx, const bool consume) {
    auto head_res = match_rule<Head>(ctx, consume);
    if (!head_res)
      return std::nullopt;

    auto tail_res = match_rule<Sequence<Tail...>>(head_res->ctx, consume);
    if (!tail_res)
      return std::nullopt;

    return Result<ReturnType>{
        tail_res->ctx,
        std::tuple_cat(std::make_tuple(head_res->value), tail_res->value)};
  }
};

/*
Sequence Transform
*/

template <typename LayoutT, typename Converter> struct SequenceTransform {
  using ReturnType = ASTPtr;

  static std::optional<Result<ReturnType>> Match(Context ctx, const bool consume) {
    auto res = match_rule<LayoutT>(ctx, consume);
    if (!res)
      return std::nullopt;

    return Result<ReturnType>{res->ctx,
                              std::make_shared<Converter>(res->value)};
  }
};

/*
Repeated
*/

template <typename Definition> struct Repeated;

template <typename Definition> struct Repeated {
  using ReturnType = std::vector<ASTPtr>;

  static std::optional<Result<ReturnType>> Match(Context ctx, const bool consume) {
    Context current = ctx;
    ReturnType values;

    while (auto next = match_rule<Definition>(current, consume)) {
      if (next->ctx.input.size() == current.input.size())
        break;
      values.push_back(next->value);
      current = next->ctx;
    }
    return Result<ReturnType>{current, values};
  }
};

/*
End Of File
*/

struct EndOfFile {
  using ReturnType = ASTPtr;
  static std::optional<Result<ReturnType>> Match(Context ctx, const bool consume) {
    if (ctx.input.empty())
      return Result<ReturnType>{.ctx = ctx, .value = nullptr};
    return std::nullopt;
  }
};

/*
Conditionals
*/

template <typename Condition>
struct Not{
  using ReturnType = ASTPtr;

  static std::optional<Result<ReturnType>> Match(Context ctx, const bool consume) {
    if (auto next = match_rule<Condition>(ctx, consume)) {
      return std::nullopt;
    }

    return Result<ReturnType>{ctx, nullptr};
  }
};

template <typename Condition, typename Action>
struct Conditional {
  using ReturnType = ASTPtr;

  static std::optional<Result<ReturnType>> Match(Context ctx, const bool consume) {
    constexpr auto kConsume{true};
    constexpr auto kDontConsume{false};
    
    if (match_rule<Condition>(ctx, kDontConsume)) {
      return match_rule<Action>(ctx, kConsume);
    }

    return std::nullopt;
  }
};

} // namespace language