#pragma once
#include "source/grammar_new.h"

namespace example::syntax {

using namespace language;

// // using Whitespace = Repeated<Regex<"[ \\t\\r\\n]+">>;
using Number = Regex<"[0-9]+">;
using Plus = Regex<"\\+">;
using Star = Regex<"\\*">;
using LParen = Regex<"\\(">;
using RParen = Regex<"\\)">;

struct Expr;
// using Parenthesized = Seq<LParen, Operator, RParen>;
using Parenthesized = Seq<LParen, Eval<Expr>, RParen>;

struct Expr : Or<Number, Parenthesized> {
  using Grammar = Or<Number, Parenthesized>;
  using ReturnType = ReturnTypeOf<Grammar>::type;
  ReturnType value;
  Expr(ReturnType t) : value(t){};
};

// struct Expr : Or<Number, Parenthesized>{};

// using Parenthesized = Sequence<LParen, Eval<Expr>, RParen>;

// // using Pluses = Repeated<Plus>;
// // struct Expr : Or<Number, Parenthesized>{};

// using Factor = Or<Number, Parenthesized>;

// using Term = Sequence<Factor, Repeated<Sequence<Star, Factor>>>;

// struct Expr : Or<Sequence<Term, Repeated<Sequence<Plus, Term>>>, Term> {};

// using Grammar = Sequence<Expr, EndOfFile>;

} // namespace example::syntax