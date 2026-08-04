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

// Generic parse tree node returned by all rules
struct Node {
    std::string_view rule_name;
    std::string_view match_text;
    std::vector<Node> children;
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
    static std::optional<Result<Node>> Match(Context ctx, const bool consume) {
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
            return Result<Node>{
                .ctx = next_ctx,
                .value = Node{
                    .rule_name = "Regex",
                    .match_text = matched_slice,
                    .children = {}
                }
            };
        }
        return std::nullopt;
    }
};

/*
Sequence Rule
*/
template <typename... Rules>
struct Sequence {
    static std::optional<Result<Node>> Match(Context ctx, const bool consume) {
        Context current = ctx;
        std::vector<Node> children;
        children.reserve(sizeof...(Rules));

        bool matched = (... && [&]() {
            auto res = match_rule<Rules>(current, consume);
            if (!res) return false;
            current = res->ctx;
            children.push_back(std::move(res->value));
            return true;
        }());

        if (!matched) return std::nullopt;

        std::size_t len = ctx.input.size() - current.input.size();
        return Result<Node>{
            .ctx = current,
            .value = Node{
                .rule_name = "Sequence",
                .match_text = ctx.input.substr(0, len),
                .children = std::move(children)
            }
        };
    }
};

/*
Or (Choice) Rule
*/
template <typename... Rules>
struct Or {
    static std::optional<Result<Node>> Match(Context ctx, const bool consume) {
        std::optional<Result<Node>> result = std::nullopt;

        (... || [&]() {
            if (auto res = match_rule<Rules>(ctx, consume)) {
                result = std::move(res);
                return true;
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
    static std::optional<Result<Node>> Match(Context ctx, const bool consume) {
        Context current = ctx;
        std::vector<Node> children;

        while (auto res = match_rule<Rule>(current, consume)) {
            if (res->ctx.input.size() == current.input.size()) break; // Prevent zero-width infinite loops
            children.push_back(std::move(res->value));
            current = res->ctx;
        }

        std::size_t len = ctx.input.size() - current.input.size();
        return Result<Node>{
            .ctx = current,
            .value = Node{
                .rule_name = "Repeated",
                .match_text = ctx.input.substr(0, len),
                .children = std::move(children)
            }
        };
    }
};

/*
Eval (Deferred Rule Evaluation for Recursion)
*/
template <typename Rule>
struct Eval {
    static std::optional<Result<Node>> Match(Context ctx, const bool consume) {
        return Rule::Match(ctx, consume);
    }
};

/*
End Of File
*/
struct EndOfFile {
    static std::optional<Result<Node>> Match(Context ctx, const bool /*consume*/) {
        if (ctx.input.empty()) {
            return Result<Node>{
                .ctx = ctx,
                .value = Node{.rule_name = "EndOfFile", .match_text = {}, .children = {}}
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