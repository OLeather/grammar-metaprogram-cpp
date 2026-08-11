#pragma once
#include "source/grammar_new.h"

namespace example::syntax {

using namespace language;
struct Expr;

// // using Whitespace = Repeated<Regex<"[ \\t\\r\\n]+">>;
using Number = Regex<"[0-9]+">;
using Plus = Regex<"\\+">;
using Star = Regex<"\\*">;
using LParen = Regex<"\\(">;
using RParen = Regex<"\\)">;

// using Parenthesized = Seq<LParen, Operator, RParen>;
// using Parenthesized = Seq<LParen, Eval<Expr>, RParen>;

// struct Expr : Or<Number, Parenthesized> {
//   using Grammar = Or<Number, Parenthesized>;
//   using ReturnType = ReturnTypeOf<Grammar>::type;
//   ReturnType value;
//   Expr(ReturnType t) : value(t){};
// };

// using Test = Repeated<Plus>;

// struct Expr : Or<Number, Parenthesized>{};

using Parenthesized = Seq<LParen, Eval<Expr>, RParen>;

// // using Pluses = Repeated<Plus>;
// // struct Expr : Or<Number, Parenthesized>{};

using Factor = Or<Number, Parenthesized>;

using Term = Seq<Factor, Repeated<Seq<Star, Factor>>>;

// using ExprGrammar = Or<Seq<Term, Repeated<Seq<Plus, Term>>>, Term>;
struct Expr : Or<Seq<Term, Repeated<Seq<Plus, Term>>>, Term> {
  using Grammar = Or<Seq<Term, Repeated<Seq<Plus, Term>>>, Term>;
  using ReturnType = ReturnTypeOf<Grammar>::type;
  ReturnType value;
  Expr(ReturnType t) : value(t) {};
};

using Grammar = Seq<Eval<Expr>, Regex<"E">>;

} // namespace example::syntax