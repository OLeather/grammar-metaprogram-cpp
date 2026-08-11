#pragma once
#include "source/grammar_new.h"

namespace example::syntax {

using namespace language;

// 1. Declare target struct
struct Expr;

// 2. Pure Grammar Definition
using ExprGrammar =
    grammar::Or<grammar::Regex<"a">,
                grammar::Seq<grammar::Regex<"(">, grammar::Eval<Expr>,
                             grammar::Regex<")">>>;

// 3. Define target struct (NOW COMPLETE)
struct Expr {
  using Grammar = ExprGrammar;
  using ReturnType = grammar::ReturnTypeOf<ExprGrammar>::type;
  ReturnType value;
};

// struct Expr;

// // using Whitespace = Repeated<Regex<"[ \\t\\r\\n]+">>;
// using Number     = Regex<"[0-9]+">;
// using Plus       = Regex<"\\+">;
// using Star       = Regex<"\\*">;
// using LParen     = Regex<"\\(">;
// using RParen     = Regex<"\\)">;

// // using Parenthesized = Seq<LParen, Plus, RParen>;
// using Parenthesized = Seq<LParen, Eval<Expr>, RParen>;

// struct Expr : Or<Number, Parenthesized>{};

// using Parenthesized = Sequence<LParen, Eval<Expr>, RParen>;

// // using Pluses = Repeated<Plus>;
// // struct Expr : Or<Number, Parenthesized>{};

// using Factor = Or<Number, Parenthesized>;

// using Term = Sequence<Factor, Repeated<Sequence<Star, Factor>>>;

// struct Expr : Or<Sequence<Term, Repeated<Sequence<Plus, Term>>>, Term> {};

// using Grammar = Sequence<Expr, EndOfFile>;

} // namespace example::syntax