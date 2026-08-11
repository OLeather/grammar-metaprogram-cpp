#pragma once
#include "source/grammar_new.h"

namespace example::syntax {

using namespace language;
struct Expr;

using Whitespace = Repeated<Regex<"[ \\t\\r\\n]+">>;
using Number = Regex<"[0-9]+">;
using Plus = Regex<"\\+">;
using Star = Regex<"\\*">;
using LParen = Regex<"\\(">;
using RParen = Regex<"\\)">;

using Parenthesized = Seq<LParen, Eval<Expr>, RParen>;

using Factor = Or<Number, Parenthesized>;

using Term = Seq<Factor, Repeated<Seq<Star, Factor>>>;

struct Expr : Or<Seq<Term, Repeated<Seq<Plus, Term>>>, Term> {
  using Grammar = Or<Seq<Term, Repeated<Seq<Plus, Term>>>, Term>;
  using ReturnType = ReturnTypeOf<Grammar>::type;
  ReturnType value;
  Expr(ReturnType t) : value(t) {};
};

using Grammar = Seq<Eval<Expr>, Regex<"E">>;

} // namespace example::syntax