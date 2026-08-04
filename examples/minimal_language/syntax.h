#pragma once
#include "source/grammar.h"

namespace example::syntax {

using namespace language;

struct Expr;

using Whitespace = Repeated<Regex<"[ \\t\\r\\n]+">>;
using Number     = Regex<"[0-9]+">;
using Plus       = Regex<"\\+">;
using Star       = Regex<"\\*">;
using LParen     = Regex<"\\(">;
using RParen     = Regex<"\\)">;

using Parenthesized = Sequence<LParen, Eval<Expr>, RParen>;

using Factor = Or<Number, Parenthesized>;

using Term = Sequence<Factor, Repeated<Sequence<Star, Factor>>>;

struct Expr : Or<Sequence<Term, Repeated<Sequence<Plus, Term>>>, Term> {};

using Grammar = Sequence<Expr, EndOfFile>;

} // namespace example::syntax