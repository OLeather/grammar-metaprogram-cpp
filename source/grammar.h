#pragma once

#include "source/regex_helpers.h"
#include <algorithm>
#include <boost/variant/recursive_wrapper.hpp>
#include <concepts>
#include <cstddef>
#include <memory>
#include <optional>
#include <regex>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>
#include <iostream>

namespace language {

/*
Rule Tags
*/

template <FixedString Pattern> struct Regex {
  static constexpr auto pattern = Pattern;
  std::string match;
  Regex(const std::string &match) : match(match) {};
};

template <typename... Rules> struct Or {
  using IsOr = void;
};
template <typename... Rules> struct Seq {
  using IsSeq = void;
};

template <typename Rule> struct Repeated {
  using IsRepeated = void;
};

template <typename TargetRule> struct Eval {
  using Target = TargetRule;
};

template <typename Condition, typename Action> struct Conditional {
  using IsConditional = void;
};

struct EndOfFile {
  using IsEOF = void;
};

/*
Return Types
*/

template <typename Rule> struct ReturnTypeOf;

// TODO (owen): Remove duplicates in the variant
template <typename... Rules> struct ReturnTypeOf<Or<Rules...>> {
  using type = std::variant<typename ReturnTypeOf<Rules>::type...>;
};

template <typename... Rules> struct ReturnTypeOf<Seq<Rules...>> {
  using type = std::tuple<typename ReturnTypeOf<Rules>::type...>;
};

template <typename TargetRule> struct ReturnTypeOf<Eval<TargetRule>> {
  using type = boost::recursive_wrapper<TargetRule>;
};

template <FixedString Pattern> struct ReturnTypeOf<Regex<Pattern>> {
  using type = Regex<Pattern>;
};

template <typename Rule> struct ReturnTypeOf<Repeated<Rule>> {
  using type = std::vector<typename ReturnTypeOf<Rule>::type>;
};

template <typename Condition, typename Action>
struct ReturnTypeOf<Conditional<Condition, Action>> {
  using type = std::optional<typename ReturnTypeOf<Action>::type>;
};

template <> struct ReturnTypeOf<EndOfFile> {
  using type = std::monostate;
};

/*
Matchers
*/

template <typename Rule> struct Matcher;

template <FixedString Pattern> struct Matcher<Regex<Pattern>> {
  using ReturnType = typename ReturnTypeOf<Regex<Pattern>>::type;

  static std::optional<Result<ReturnType>> Match(Context ctx) {
    static const std::regex re{"^(" + std::string(Pattern.value) + ")", std::regex::optimize};
    return MatchRegex<ReturnType>(re, ctx);
  }
};

template <typename Head> struct Matcher<Or<Head>> {
  using VariantType = typename ReturnTypeOf<Or<Head>>::type;

  static std::optional<Result<VariantType>> Match(Context ctx) {
    if (auto res = Matcher<Head>::Match(ctx)) {
      return Result<VariantType>{
          .ctx = res->ctx,
          .value = VariantType(std::move(res->value))
      };
    }
    return std::nullopt;
  }
};

template <typename Head, typename... Tail> struct Matcher<Or<Head, Tail...>> {
  using VariantType = typename ReturnTypeOf<Or<Head, Tail...>>::type;

  static std::optional<Result<VariantType>> Match(Context ctx) {
    auto head = Matcher<Head>::Match(ctx);
    if (head) {
      return Result<VariantType>{.ctx = head->ctx,
                                 .value = VariantType(std::move(head->value))};
    }

    if constexpr (sizeof...(Tail) > 0) {
      auto tail = Matcher<Or<Tail...>>::Match(ctx);
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

template <typename Head, typename... Tail> struct Matcher<Seq<Head, Tail...>> {
  using TupleType = typename ReturnTypeOf<Seq<Head, Tail...>>::type;

  static std::optional<Result<TupleType>> Match(Context ctx) {
    auto head_res = Matcher<Head>::Match(ctx);
    if (!head_res)
      return std::nullopt;

    if constexpr (sizeof...(Tail) == 0) {
      return Result<TupleType>{.ctx = head_res->ctx,
                               .value =
                                   std::make_tuple(std::move(head_res->value))};
    } else {
      auto tail_res = Matcher<Seq<Tail...>>::Match(head_res->ctx);
      if (!tail_res)
        return std::nullopt;

      return Result<TupleType>{
          .ctx = tail_res->ctx,
          .value = std::tuple_cat(std::make_tuple(std::move(head_res->value)),
                                  std::move(tail_res->value))};
    }
  }
};

template <typename Target> struct Matcher<Eval<Target>> {
  using ReturnType = boost::recursive_wrapper<Target>;

  static std::optional<Result<ReturnType>> Match(Context ctx) {
    auto res = Matcher<typename Target::Grammar>::Match(ctx);
    if (!res)
      return std::nullopt;

    return Result<ReturnType>{
        .ctx = res->ctx, .value = ReturnType(Target{std::move(res->value)})};
  }
};

template <typename Rule> struct Matcher<Repeated<Rule>> {
  using VecType = typename ReturnTypeOf<Repeated<Rule>>::type;

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

template <typename Condition, typename Action>
struct Matcher<Conditional<Condition, Action>> {
  using OptType = typename ReturnTypeOf<Conditional<Condition, Action>>::type;

  static std::optional<Result<OptType>> Match(Context ctx) {
    if (auto cond_res = Matcher<Condition>::Match(ctx)) {
      if (auto action_res = Matcher<Action>::Match(ctx)) {
        return Result<OptType>{
            .ctx = action_res->ctx,
            .value = OptType(std::move(action_res->value))
        };
      }
      return std::nullopt;
    }

    return Result<OptType>{
        .ctx = ctx,
        .value = std::nullopt
    };
  }
};

template <> struct Matcher<EndOfFile> {
  static std::optional<Result<std::monostate>> Match(Context ctx) {
    if (ctx.input.empty()) {
      return Result<std::monostate>{.ctx = ctx, .value = std::monostate{}};
    }
    return std::nullopt;
  }
};

} // namespace language