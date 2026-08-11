#pragma once

#include <algorithm>
#include <concepts>
#include <cstddef>
#include <memory>
#include <optional>
#include <regex>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>
#include <variant>
#include <boost/variant/recursive_wrapper.hpp>

namespace language {

// Helper tag for compile-time overload dispatching
template <typename T>
struct Tag {
    using type = T;
};

// FixedString wrapper to allow string literals as non-type template parameters (C++20)
template <std::size_t N>
struct FixedString {
    char value[N]{};
    constexpr FixedString(const char (&str)[N]) {
        std::copy_n(str, N, value);
    }
    constexpr operator std::string_view() const {
        return {value, N - 1};
    }
};


// Lightweight input cursor
struct Context {
    std::string_view input;
};

template <typename T>
struct Result {
    Context ctx;
    T value;
};

// Core rule execution caller
template <typename Rule>
auto match_rule(Context ctx, const bool consume = true) {
    return Rule::Match(ctx, consume);
}

/*
Regex Rule
*/
template <FixedString RegexStr>
struct Regex {
    using ReturnType = Regex<RegexStr>;

    std::string match;
    Regex(const std::string& match) : match(match){};

    static std::optional<Result<ReturnType>> Match(Context ctx, const bool consume) {
        static const std::regex re{RegexStr.value, std::regex::optimize};
        std::cmatch match;

        if (std::regex_search(ctx.input.data(), ctx.input.data() + ctx.input.size(),
                              match, re, std::regex_constants::match_continuous)) {
            std::size_t len = match.length(0);
            std::string_view matched_slice = ctx.input.substr(0, len);
            Context next_ctx = ctx;
            if (consume) {
                next_ctx.input.remove_prefix(len);
            }
            return Result<ReturnType>{
                .ctx = next_ctx,
                .value = ReturnType(std::string(matched_slice))
            };
        }
        return std::nullopt;
    }
};


// Generic parse tree node returned by all rules
struct Node {
    std::string_view rule_name;
    std::string_view match_text;
    std::vector<Node> children;
};



/*
Sequence Rule
*/


template <typename... Rules>
struct Sequence {
    using ReturnType = std::tuple<typename Rules::ReturnType...>;

    static std::optional<Result<ReturnType>> Match(Context ctx, const bool consume) {
        Context current = ctx;
        bool matched = true;

        // Extract each rule sequentially and move into the tuple constructor
        auto parse_all = [&]<std::size_t... Is>(std::index_sequence<Is...>) -> std::optional<ReturnType> {
            // Temporary storage for results to avoid needing default constructors
            std::tuple<std::optional<typename Rules::ReturnType>...> results;

            auto match_one = [&]<std::size_t I, typename Rule>() {
                if (!matched) return;
                auto res = match_rule<Rule>(current, consume);
                if (!res) {
                    matched = false;
                    return;
                }
                current = res->ctx;
                std::get<I>(results).emplace(std::move(res->value));
            };

            // Force sequential left-to-right evaluation
            (match_one.template operator()<Is, Rules>(), ...);

            if (!matched) return std::nullopt;

            // Move values out of optionals into the final ReturnType tuple
            return ReturnType{ std::move(*std::get<Is>(results))... };
        };

        auto values = parse_all(std::make_index_sequence<sizeof...(Rules)>{});
        if (!values) return std::nullopt;

        return Result<ReturnType>{
            .ctx = current,
            .value = std::move(*values)
        };
    }
};

/*
Or (Choice) Rule
*/


template <typename... Rules>
struct Or {
    // 1. Define ReturnType as a variant of each Rule's ReturnType
    using ReturnType = std::variant<typename Rules::ReturnType...>;

    static std::optional<Result<ReturnType>> Match(Context ctx, const bool consume) {
        std::optional<Result<ReturnType>> result = std::nullopt;

        // 2. Short-circuiting fold expression over logical OR (||)
        (... || [&]() {
            if (auto res = match_rule<Rules>(ctx, consume)) {
                result = Result<ReturnType>{
                    .ctx = res->ctx,
                    // Implicitly constructs std::variant<typename Rules::ReturnType...> 
                    // holding the value of the active Rule
                    .value = std::move(res->value) 
                };
                return true; // Stop evaluating subsequent rules
            }
            return false;
        }());

        return result;
    }
};

/*
Repeated Rule (Zero or More)
*/
template <typename Rule>
struct Repeated {
    using ReturnType = std::vector<typename Rule::ReturnType>;

    static std::optional<Result<ReturnType>> Match(Context ctx, const bool consume) {
        Context current = ctx;
        ReturnType children; // std::vector<typename Rule::ReturnType>

        while (auto res = match_rule<Rule>(current, consume)) {
            if (res->ctx.input.size() == current.input.size()) break; // Prevent zero-width infinite loops
            children.push_back(std::move(res->value));
            current = res->ctx;
        }

        return Result<ReturnType>{
            .ctx = current,
            .value = std::move(children)
        };
    }
};

/*
Eval (Deferred Rule Evaluation for Recursion)
*/
template <typename Rule>
struct Eval {
    using ReturnType = std::shared_ptr<typename Rule::ReturnType>;
    static std::optional<Result<ReturnType>> Match(Context ctx, const bool consume) {
        if(auto res = Rule::Match(ctx, consume)){
            return Result{
                .ctx = res->ctx,
                .value = std::make_shared<typename Rule::ReturnType>(res->value)
            };
        }
        return std::nullopt;
    }
};

/*
End Of File
*/
struct EndOfFile {
    using ReturnType = std::monostate;
    static std::optional<Result<ReturnType>> Match(Context ctx, const bool /*consume*/) {
        if (ctx.input.empty()) {
            return Result<ReturnType>{
                .ctx = ctx,
                .value = std::monostate()
            };
        }
        return std::nullopt;
    }
};

/*
Conditionals & Lookaheads
*/
template <typename Condition>
struct Not {
    static std::optional<Result<Node>> Match(Context ctx, const bool /*consume*/) {
        if (match_rule<Condition>(ctx, false)) {
            return std::nullopt;
        }
        return Result<Node>{
            .ctx = ctx,
            .value = Node{.rule_name = "Not", .match_text = {}, .children = {}}
        };
    }
};

template <typename Condition, typename Action>
struct Conditional {
    static std::optional<Result<Node>> Match(Context ctx, const bool consume) {
        if (auto cond_res = match_rule<Condition>(ctx, false)) {
            return match_rule<Action>(cond_res->ctx, consume);
        }
        return std::nullopt;
    }
};

} // namespace language