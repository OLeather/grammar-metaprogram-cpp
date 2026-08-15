#pragma once

#include "source/regex_helpers.h"
#include <algorithm>
#include <boost/variant/recursive_wrapper.hpp>
#include <concepts>
#include <cstddef>
#include <iostream>
#include <memory>
#include <optional>
#include <regex>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>

namespace language {

/*
Rule Tags
*/

template <FixedString Pattern> struct Regex {
  static constexpr auto pattern = Pattern;
  std::string match;
  Regex(const std::string &match) : match(match) {};
};

template <typename Rule> struct Not {
  using IsNot = void;
};

template <typename Rule> struct And {
  using IsAnd = void;
};

struct EndOfFile {
  using IsEOF = void;
};

/*
Return Types
*/

template <typename Rule> struct ReturnTypeOf;

// TODO (owen): Remove duplicates in the variant
template <typename... Rules> struct ReturnTypeOf<std::variant<Rules...>> {
  using type = std::variant<typename ReturnTypeOf<Rules>::type...>;
};

template <typename... Rules> struct ReturnTypeOf<std::tuple<Rules...>> {
  using type = std::tuple<typename ReturnTypeOf<Rules>::type...>;
};

template <typename TargetRule>
struct ReturnTypeOf<boost::recursive_wrapper<TargetRule>> {
  using type = boost::recursive_wrapper<TargetRule>;
};

template <FixedString Pattern> struct ReturnTypeOf<Regex<Pattern>> {
  using type = Regex<Pattern>;
};

template <typename Rule> struct ReturnTypeOf<std::vector<Rule>> {
  using type = std::vector<typename ReturnTypeOf<Rule>::type>;
};

template <typename Condition> struct ReturnTypeOf<std::optional<Condition>> {
  using type = std::optional<typename ReturnTypeOf<Condition>::type>;
};

template <typename Rule> struct ReturnTypeOf<Not<Rule>> {
  using type = std::monostate;
};

template <typename Rule> struct ReturnTypeOf<And<Rule>> {
  using type = std::monostate;
};

template <> struct ReturnTypeOf<EndOfFile> {
  using type = EndOfFile;
};

/*
Recursive Definition
*/
template <typename GrammarT> struct Def {
  using Grammar = GrammarT;
  using ReturnType = ReturnTypeOf<Grammar>::type;
  ReturnType value;
  Def(ReturnType t) : value(t) {};
};

/*
Matchers
*/

template <typename Rule> struct Matcher;

template <FixedString Pattern> struct Matcher<Regex<Pattern>> {
  using ReturnType = typename ReturnTypeOf<Regex<Pattern>>::type;

  static std::optional<Result<ReturnType>> Match(Context ctx) {
    static const std::regex re{"^(" + std::string(Pattern.value) + ")",
                               std::regex::optimize};
    return MatchRegex<ReturnType>(re, ctx);
  }
};

template <typename Head> struct Matcher<std::variant<Head>> {
  using VariantType = typename ReturnTypeOf<std::variant<Head>>::type;

  static std::optional<Result<VariantType>> Match(Context ctx) {
    if (auto res = Matcher<Head>::Match(ctx)) {
      return Result<VariantType>{.ctx = res->ctx,
                                 .value = VariantType(std::move(res->value))};
    }
    return std::nullopt;
  }
};

template <typename Head, typename... Tail>
struct Matcher<std::variant<Head, Tail...>> {
  using VariantType = typename ReturnTypeOf<std::variant<Head, Tail...>>::type;

  static std::optional<Result<VariantType>> Match(Context ctx) {
    auto head = Matcher<Head>::Match(ctx);
    if (head) {
      return Result<VariantType>{.ctx = head->ctx,
                                 .value = VariantType(std::move(head->value))};
    }

    if constexpr (sizeof...(Tail) > 0) {
      auto tail = Matcher<std::variant<Tail...>>::Match(ctx);
      if (tail) {
        VariantType parent_variant = std::visit(
            [](auto &&val) -> VariantType {
              return VariantType(std::forward<decltype(val)>(val));
            },
            tail->value);

        return Result<VariantType>{.ctx = tail->ctx,
                                   .value = std::move(parent_variant)};
      }
    }

    return std::nullopt;
  }
};

template <typename Head, typename... Tail>
struct Matcher<std::tuple<Head, Tail...>> {
  using TupleType = typename ReturnTypeOf<std::tuple<Head, Tail...>>::type;

  static std::optional<Result<TupleType>> Match(Context ctx) {
    auto head_res = Matcher<Head>::Match(ctx);
    if (!head_res)
      return std::nullopt;

    if constexpr (sizeof...(Tail) == 0) {
      return Result<TupleType>{.ctx = head_res->ctx,
                               .value =
                                   std::make_tuple(std::move(head_res->value))};
    } else {
      auto tail_res = Matcher<std::tuple<Tail...>>::Match(head_res->ctx);
      if (!tail_res)
        return std::nullopt;

      return Result<TupleType>{
          .ctx = tail_res->ctx,
          .value = std::tuple_cat(std::make_tuple(std::move(head_res->value)),
                                  std::move(tail_res->value))};
    }
  }
};

template <typename Target> struct Matcher<boost::recursive_wrapper<Target>> {
  using ReturnType = boost::recursive_wrapper<Target>;

  static std::optional<Result<ReturnType>> Match(Context ctx) {
    auto res = Matcher<typename Target::Grammar>::Match(ctx);
    if (!res)
      return std::nullopt;

    return Result<ReturnType>{
        .ctx = res->ctx, .value = ReturnType(Target{std::move(res->value)})};
  }
};

template <typename Rule> struct Matcher<std::vector<Rule>> {
  using VecType = typename ReturnTypeOf<std::vector<Rule>>::type;

  static std::optional<Result<VecType>> Match(Context ctx) {
    Context current = ctx;
    VecType children;

    while (auto res = Matcher<Rule>::Match(current)) {
      if (res->ctx.input.size() == current.input.size())
        break;
      children.push_back(std::move(res->value));
      current = res->ctx;
    }

    return Result<VecType>{.ctx = current, .value = std::move(children)};
  }
};

template <typename Rule> struct Matcher<std::optional<Rule>> {
  using OptType = typename ReturnTypeOf<std::optional<Rule>>::type;

  static std::optional<Result<OptType>> Match(Context ctx) {
    if (auto res = Matcher<Rule>::Match(ctx)) {
      return Result<OptType>{.ctx = res->ctx,
                             .value = OptType(std::move(res->value))};
    }

    return Result<OptType>{.ctx = ctx, .value = std::nullopt};
  }
};

template <typename Rule> struct Matcher<Not<Rule>> {
  using ReturnType = std::monostate;

  static std::optional<Result<ReturnType>> Match(Context ctx) {
    if (auto res = Matcher<Rule>::Match(ctx)) {
      return std::nullopt;
    }
    return Result{.ctx = ctx, .value = std::monostate()};
  }
};

template <typename Rule> struct Matcher<And<Rule>> {
  using ReturnType = std::monostate;

  static std::optional<Result<ReturnType>> Match(Context ctx) {
    if (auto res = Matcher<Rule>::Match(ctx)) {
      return Result{.ctx = ctx, .value = std::monostate()};
    }
    return std::nullopt;
  }
};

template <> struct Matcher<EndOfFile> {
  static std::optional<Result<EndOfFile>> Match(Context ctx) {
    if (ctx.input.empty()) {
      return Result<EndOfFile>{.ctx = ctx, .value = EndOfFile{}};
    }
    return std::nullopt;
  }
};

} // namespace language