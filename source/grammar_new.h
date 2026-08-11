#pragma once

// #include "source/regex_helpers.h"
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

namespace grammar {


// Grammar types contain NO code—only structural type aliases
template <typename... Rules> struct Or { using IsOr = void; };
template <typename... Rules> struct Seq { using IsSeq = void; };

template <typename TargetRule> struct Eval {
  using Target = TargetRule;
};

// Return type traits computed purely from structural types
template <typename Rule> struct ReturnTypeOf;

template <typename... Rules>
struct ReturnTypeOf<Or<Rules...>> {
  using type = std::variant<typename ReturnTypeOf<Rules>::type...>;
};

template <typename... Rules>
struct ReturnTypeOf<Seq<Rules...>> {
  using type = std::tuple<typename ReturnTypeOf<Rules>::type...>;
};

template <typename TargetRule>
struct ReturnTypeOf<Eval<TargetRule>> {
  // Free recursive wrapper since TargetRule is a full struct/type alias
  using type = boost::recursive_wrapper<TargetRule>;
};


} // namespace grammar

namespace grammar {

// Fixed-size string helper for C++ structural type templates
template <std::size_t N>
struct FixedString {
  char buf[N]{};
  constexpr FixedString(const char (&str)[N]) {
    std::copy_n(str, N, buf);
  }
};

// Pure declarative type—contains NO match code
template <FixedString Pattern>
struct Regex {
  static constexpr auto pattern = Pattern;
};

// Return type trait mapping for Regex
template <FixedString Pattern>
struct ReturnTypeOf<Regex<Pattern>> {
  using type = std::string_view; // Matched substring
};

} // namespace grammar


namespace language {
  
struct Context {
  std::string_view input;
  std::size_t offset = 0;

  // View of the unparsed text remaining
  [[nodiscard]] std::string_view remaining_text() const {
    return input.substr(offset);
  }

  // Returns a new Context advanced by 'count' characters
  [[nodiscard]] Context advance(std::size_t count) const {
    return Context{input, offset + count};
  }

  [[nodiscard]] bool is_at_end() const {
    return offset >= input.size();
  }
};

template <typename ValueType>
struct Result {
  Context ctx;     // The NEW context (offset advanced by the length of the match)
  ValueType value; // The parsed value / AST node
};

template <typename Rule>
struct Matcher;

// --- Regex Matcher ---
template <grammar::FixedString Pattern>
struct Matcher<grammar::Regex<Pattern>> {
  using ReturnType = typename grammar::ReturnTypeOf<grammar::Regex<Pattern>>::type;

  static std::optional<Result<ReturnType>> Match(Context ctx, bool consume) {
    std::string_view pat(Pattern.buf);
    if (ctx.remaining_text().starts_with(pat)) {
      Context next_ctx = consume ? ctx.advance(pat.length()) : ctx;
      return Result<ReturnType>{.ctx = next_ctx, .value = pat};
    }
    return std::nullopt;
  }
};

// --- Eval Matcher ---
template <typename Target>
struct Matcher<grammar::Eval<Target>> {
  using ReturnType = boost::recursive_wrapper<Target>;

  static std::optional<Result<ReturnType>> Match(Context ctx, bool consume) {
    auto res = Matcher<typename Target::Grammar>::Match(ctx, consume);
    if (!res) return std::nullopt;

    return Result<ReturnType>{
        .ctx = res->ctx,
        .value = ReturnType(Target{std::move(res->value)})
    };
  }
};

// --- Seq Matcher (MUST BE BEFORE OR) ---
template <typename Head, typename... Tail>
struct Matcher<grammar::Seq<Head, Tail...>> {
  using SeqRule = grammar::Seq<Head, Tail...>;
  using TupleType = typename grammar::ReturnTypeOf<SeqRule>::type;

  static std::optional<Result<TupleType>> Match(Context ctx, bool consume) {
    auto head_res = Matcher<Head>::Match(ctx, consume);
    if (!head_res) return std::nullopt;

    if constexpr (sizeof...(Tail) == 0) {
      return Result<TupleType>{
          .ctx = head_res->ctx,
          .value = std::make_tuple(std::move(head_res->value))
      };
    } else {
      auto tail_res = Matcher<grammar::Seq<Tail...>>::Match(head_res->ctx, consume);
      if (!tail_res) return std::nullopt;

      return Result<TupleType>{
          .ctx = tail_res->ctx,
          .value = std::tuple_cat(std::make_tuple(std::move(head_res->value)),
                                  std::move(tail_res->value))
      };
    }
  }
};

// --- Or Matcher ---
template <typename Head, typename... Tail>
struct Matcher<grammar::Or<Head, Tail...>> {
  using OrRule = grammar::Or<Head, Tail...>;
  using VariantType = typename grammar::ReturnTypeOf<OrRule>::type;

  static std::optional<Result<VariantType>> Match(Context ctx, bool consume) {
    if (auto res = Matcher<Head>::Match(ctx, consume)) {
      return Result<VariantType>{
          .ctx = res->ctx,
          .value = VariantType(std::move(res->value))
      };
    }

    if constexpr (sizeof...(Tail) > 0) {
      if (auto tail_res = Matcher<grammar::Or<Tail...>>::Match(ctx, consume)) {
        VariantType parent_val = std::visit(
            [](auto&& val) -> VariantType {
              return VariantType(std::forward<decltype(val)>(val));
            },
            tail_res->value);

        return Result<VariantType>{
            .ctx = tail_res->ctx,
            .value = std::move(parent_val)
        };
      }
    }

    return std::nullopt;
  }
};


}