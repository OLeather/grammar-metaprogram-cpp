#pragma once
#include "source/regex_helpers.h"
#include "source/result.h"

#include <algorithm>
#include <memory>
#include <optional>
#include <regex>
#include <string>
#include <type_traits>
#include <variant>

namespace language {

/*
Core
*/

template <typename Rule> auto match_rule(Context ctx, const bool consume) {
  return Rule::Match(ctx, consume);
}

/*
Regex
*/

template <FixedString RegexStr> struct Regex {
  using ReturnType = Regex<RegexStr>;
  std::string value;

  Regex() = default;
  Regex(const std::string &input) : value(input) {};

  static std::optional<Result<ReturnType>> Match(Context ctx,
                                                 const bool consume) {
    static const std::regex regex{RegexStr.value};
    return MatchRegex<ReturnType>(regex, ctx, consume);
  }
};

/*
Or
*/

template <typename T> struct is_variant : std::false_type {};
template <typename... Ts>
struct is_variant<std::variant<Ts...>> : std::true_type {};
template <typename T>
inline constexpr bool is_variant_v = is_variant<std::decay_t<T>>::value;

template <typename Target, typename Source>
Target convert_to_return_type(Source &&src) {
  if constexpr (std::is_same_v<std::decay_t<Target>, std::decay_t<Source>>) {
    return std::forward<Source>(src);
  } else if constexpr (is_variant_v<Source>) {
    return std::visit(
        [](auto &&val) -> Target {
          return Target(std::forward<decltype(val)>(val));
        },
        std::forward<Source>(src));
  } else {
    return Target(std::forward<Source>(src));
  }
}

template <typename... Definitions>
using VariantOfReturnTypes = std::variant<typename Definitions::ReturnType...>;

template <typename... Definitions> struct Or;

template <> struct Or<> {
  using ReturnType = std::monostate;
  static std::optional<Result<ReturnType>> Match(Context, const bool) {
    return std::nullopt;
  }
};

template <typename Head, typename... Tail> struct Or<Head, Tail...> {
  using ReturnType = std::conditional_t<
      (std::is_same_v<typename Head::ReturnType, typename Tail::ReturnType> &&
       ...),
      typename Head::ReturnType, VariantOfReturnTypes<Head, Tail...>>;

  static std::optional<Result<ReturnType>> Match(Context ctx,
                                                 const bool consume) {
    if (auto next = Head::Match(ctx, consume)) {
      return Result<ReturnType>{
          .ctx = next->ctx,
          .value = convert_to_return_type<ReturnType>(std::move(next->value))};
    }

    if constexpr (sizeof...(Tail) > 0) {
      if (auto next = Or<Tail...>::Match(ctx, consume)) {
        return Result<ReturnType>{.ctx = next->ctx,
                                  .value = convert_to_return_type<ReturnType>(
                                      std::move(next->value))};
      }
    }

    return std::nullopt;
  }
};

/*
Converter
*/
template <typename LayoutT, typename ConverterT, typename ReturnT>
struct Converter {
  using ReturnType = ReturnT;

  static std::optional<Result<ReturnType>> Match(Context ctx,
                                                 const bool consume) {
    auto res = match_rule<LayoutT>(ctx, consume);
    if (!res)
      return std::nullopt;

    return Result<ReturnType>{res->ctx, ConverterT{}(res->value)};
  }
};

/*
Sequence
*/

template <typename... Definitions> struct Sequence;

template <> struct Sequence<> {
  using ReturnType = std::tuple<>;

  static std::optional<Result<ReturnType>> Match(Context ctx,
                                                 const bool consume) {
    return Result<ReturnType>{ctx, {}};
  }
};

template <typename T>
concept HasFailOkay = requires { typename T::FailOkay; };

template <typename Head, typename... Tail> struct Sequence<Head, Tail...> {
  using ReturnType = decltype(std::tuple_cat(
      std::declval<std::tuple<typename Head::ReturnType>>(),
      std::declval<typename Sequence<Tail...>::ReturnType>()));

  static std::optional<Result<ReturnType>> Match(Context ctx,
                                                 const bool consume) {
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
Repeated
*/

template <typename Definition> struct Repeated;

template <typename Definition> struct Repeated {
  using ReturnType = std::vector<typename Definition::ReturnType>;

  static std::optional<Result<ReturnType>> Match(Context ctx,
                                                 const bool consume) {
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
Eval (Recursion Wrapper)
*/

// TODO (owen): Figure out the implications of returning a monostate here. There
// is ideally some better way to wrap the return type in some pointer to allow
// recursive definitions.
template <typename Rule> struct Eval {
  using ReturnType = std::monostate;

  static std::optional<Result<ReturnType>> Match(Context ctx,
                                                 const bool consume) {
    const auto res = Rule::Match(ctx, consume);
    return Result{.ctx = res->ctx, .value = std::monostate()};
  }
};

/*
End Of File
*/

struct EndOfFile {
  using ReturnType = std::monostate;
  static std::optional<Result<ReturnType>> Match(Context ctx,
                                                 const bool consume) {
    if (ctx.input.empty())
      return Result<ReturnType>{.ctx = ctx, .value = std::monostate()};
    return std::nullopt;
  }
};

/*
Conditionals
*/

template <typename Condition> struct Not {
  using ReturnType = std::monostate;

  static std::optional<Result<ReturnType>> Match(Context ctx,
                                                 const bool /*consume*/) {
    if (match_rule<Condition>(ctx, false)) {
      return std::nullopt;
    }
    return Result<ReturnType>{.ctx = ctx, .value = std::monostate{}};
  }
};

// TODO (owen): There is a bug with Conditional where if I have
// Sequence<Conditional<...>, ...>, the Conditional will return nullopt and the
// sequence exist. In reality, I want some way to indicate the conditional
// failed but its okay because it doesnt have to succeed, and then we continue
// with the rest of the sequence.
template <typename Condition, typename Action> struct Conditional {
  using ReturnType = typename Action::ReturnType;

  static std::optional<Result<ReturnType>> Match(Context ctx,
                                                 const bool consume) {
    if (auto cond_res = match_rule<Condition>(ctx, false)) {
      return match_rule<Action>(cond_res->ctx, consume);
    }
    return std::nullopt;
  }
};
} // namespace language