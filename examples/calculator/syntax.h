#pragma once
#include "source/grammar.h"
#include <boost/variant/recursive_wrapper.hpp>

namespace example::calculator::syntax {
using namespace language;

template <typename T>
using wrap = boost::recursive_wrapper<T>;

struct Expression;
struct Factor;

using Spacing = Regex<"[ \\t\\r\\n]*">;
using Float   = Regex<"[0-9]+\\.[0-9]+">;
using Integer = Regex<"[0-9]+">;

// Number <- (Float / Integer) _
using Number  = std::tuple<std::variant<Float, Integer>, Spacing>;

using Plus          = std::tuple<Regex<"\\+">, Spacing>;
using Minus         = std::tuple<Regex<"\\-">, Spacing>;
using Star          = std::tuple<Regex<"\\*">, Spacing>;
using Slash         = std::tuple<Regex<"\\/">, Spacing>;
using ExponentialOp = std::tuple<Regex<"\\^">, Spacing>;

using AdditiveOp       = std::variant<Plus, Minus>;
using MultiplicativeOp = std::variant<Star, Slash>;

using LParen = std::tuple<Regex<"\\(">, Spacing>;
using RParen = std::tuple<Regex<"\\)">, Spacing>;

// Primary <- '(' _ Expression ')' _ / Number
using Primary = std::variant<std::tuple<LParen, wrap<Expression>, RParen>, Number>;

// Factor <- Primary ('^' Factor)?
using ExpFactor = std::tuple<ExponentialOp, wrap<Factor>>;
struct Factor : Def<std::tuple<Primary, std::optional<ExpFactor>>> {};

// Term <- Factor (MultiplicativeOp Factor)*
using Term = std::tuple<wrap<Factor>, std::vector<std::tuple<MultiplicativeOp, wrap<Factor>>>>;

// Expression <- Term (AdditiveOp Term)*
struct Expression : Def<std::tuple<Term, std::vector<std::tuple<AdditiveOp, Term>>>> {};

// Calculation <- _ Expression !_
using Calculation = std::tuple<Spacing, wrap<Expression>, Spacing, EndOfFile>;

} // namespace example::calculator::syntax