/*
Re-implementation of the following PEG definition of a simple calculator:

# Top-level Entry Point
Calculation <- _ Expression !_

# Precedence Levels (Lowest to Highest)
Expression  <- Term (AdditiveOp Term)*
Term        <- Factor (MultiplicativeOp Factor)*
Factor      <- Primary ('^' Factor)?
Primary     <- '(' _ Expression ')' _ / Number

# Operators
AdditiveOp       <- ('+' / '-') _
MultiplicativeOp <- ('*' / '/') _

# Lexical Tokens
Number      <- (Float / Integer) _
Float       <- [0-9]+ '.' [0-9]+
Integer     <- [0-9]+
_           <- [ \t\n\r]*
*/

#pragma once
#include "source/grammar.h"

namespace example::calculator::syntax {
using namespace language;

struct Expression;
struct Factor;

using Spacing = Regex<"[ \\t\\r\\n]*">;
using Float   = Regex<"[0-9]+\\.[0-9]+">;
using Integer = Regex<"[0-9]+">;

// Number <- (Float / Integer) _
using Number  = Seq<Or<Float, Integer>, Spacing>;

using Plus          = Seq<Regex<"\\+">, Spacing>;
using Minus         = Seq<Regex<"\\-">, Spacing>;
using Star          = Seq<Regex<"\\*">, Spacing>;
using Slash         = Seq<Regex<"\\/">, Spacing>;
using ExponentialOp = Seq<Regex<"\\^">, Spacing>;

using AdditiveOp       = Or<Plus, Minus>;
using MultiplicativeOp = Or<Star, Slash>;

using LParen = Seq<Regex<"\\(">, Spacing>;
using RParen = Seq<Regex<"\\)">, Spacing>;

// Primary <- '(' _ Expression ')' _ / Number
using Primary = Or<Seq<LParen, Eval<Expression>, RParen>, Number>;

// Factor <- Primary ('^' Factor)?
using ExpFactor = Seq<ExponentialOp, Number>;

struct Factor : Seq<Primary, Conditional<ExpFactor, ExpFactor>> {
  using Grammar    = Seq<Primary, Conditional<ExpFactor, ExpFactor>>;
  using ReturnType = ReturnTypeOf<Grammar>::type;
  ReturnType value;
  Factor(ReturnType t) : value(t) {};
};

// Term <- Factor (MultiplicativeOp Factor)*
using Term = Seq<Eval<Factor>, Repeated<Seq<MultiplicativeOp, Eval<Factor>>>>;

// Expression <- Term (AdditiveOp Term)*
struct Expression : Seq<Term, Repeated<Seq<AdditiveOp, Term>>> {
  using Grammar    = Seq<Term, Repeated<Seq<AdditiveOp, Term>>>;
  using ReturnType = ReturnTypeOf<Grammar>::type;
  ReturnType value;
  Expression(ReturnType t) : value(t) {};
};

// Calculation <- _ Expression !_
using Calculation = Seq<Spacing, Eval<Expression>, Spacing, EndOfFile>;

} // namespace example::calculator::syntax