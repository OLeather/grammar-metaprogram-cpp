#pragma once

#include "source/grammar.h"

/*
PEG defined in PEG
Figure 1. of https://bford.info/pub/lang/peg.pdf

# Hierarchical syntax
Grammar    <- Spacing Definition+ EndOfFile
Definition <- Identifier LEFTARROW Expression

Expression <- Sequence (SLASH Sequence)*
Sequence   <- Prefix*
Prefix     <- (AND / NOT)? Suffix
Suffix     <- Primary (QUESTION / STAR / PLUS)?
Primary    <- Identifier !LEFTARROW
            / OPEN Expression CLOSE
            / Literal / Class / DOT

# Lexical syntax
Identifier <- IdentStart IdentCont* Spacing
IdentStart <- [a-zA-Z_]
IdentCont  <- IdentStart / [0-9]

Literal    <- [’] (![’] Char)* [’] Spacing
            / ["] (!["] Char)* ["] Spacing

Class      <- ’[’ (!’]’ Range)* ’]’ Spacing
Range      <- Char ’-’ Char / Char
Char       <- ’\\’ [nrt’"\[\]\\]
            / ’\\’ [0-2][0-7][0-7]
            / ’\\’ [0-7][0-7]?
            / !’\\’ .

LEFTARROW  <- ’<-’ Spacing
SLASH      <- ’/’ Spacing
AND        <- ’&’ Spacing
NOT        <- ’!’ Spacing
QUESTION   <- ’?’ Spacing
STAR       <- ’*’ Spacing
PLUS       <- ’+’ Spacing
OPEN       <- ’(’ Spacing
CLOSE      <- ’)’ Spacing
DOT        <- ’.’ Spacing

Spacing    <- (Space / Comment)*
Comment    <- ’#’ (!EndOfLine .)* EndOfLine
Space      <- ’ ’ / ’\t’ / EndOfLine
EndOfLine  <- ’\r\n’ / ’\n’ / ’\r’
EndOfFile  <- !.
*/

namespace peg {
using namespace language;

// clang-format off

/*
Lexical Tokens & Operators
*/
using EndOfLine       = Or<Regex<R"(\r\n)">, Regex<R"(\n)">, Regex<R"(\r)">>;
using Space           = Or<Regex<" ">, Regex<R"(\t)">, EndOfLine>;
using EndOfLineOrFile = Or<EndOfLine, EndOfFile>;

// Comment <- '#' (!EndOfLineOrFile .)* EndOfLineOrFile
using Comment         = Sequence<Regex<R"(#)">, Repeated<Conditional<Not<EndOfLineOrFile>, Regex<R"(.)">>>, EndOfLineOrFile>;
using Spacing         = Repeated<Or<Space, Comment>>;

using LEFTARROW       = Sequence<Regex<R"(<-)">, Spacing>;
using SLASH           = Sequence<Regex<R"(/)">, Spacing>;
using AND             = Sequence<Regex<R"(&)">, Spacing>;
using NOT             = Sequence<Regex<R"(!)">, Spacing>;
using QUESTION        = Sequence<Regex<R"(\?)">, Spacing>;
using STAR            = Sequence<Regex<R"(\*)">, Spacing>;
using PLUS            = Sequence<Regex<R"(\+)">, Spacing>;
using OPEN            = Sequence<Regex<R"(\()">, Spacing>;
using CLOSE           = Sequence<Regex<R"(\))">, Spacing>;
using DOT             = Sequence<Regex<R"(\.)">, Spacing>;

/*
Character Classes & Literals
*/

using Char = Or<
  Regex<R"(\\[nrt'"\[\]\\])">,
  Regex<R"(\\[0-2][0-7][0-7])">,
  Regex<R"(\\[0-7][0-7]?)">,
  Sequence<Not<Regex<R"(\\)">>, Regex<R"(.)">>
>;

using Range = Or<
  Sequence<Char, Regex<R"(-)">, Char>,
  Char
>;

using Class = Sequence<
  Regex<R"(\[)">,
  Repeated<Sequence<Not<Regex<R"(\])">>, Range>>,
  Regex<R"(\])">,
  Spacing
>;

using Literal = Or<
  Sequence<
    Regex<R"(')">,
    Repeated<Sequence<Not<Regex<R"(')">>, Char>>,
    Regex<R"(')">,
    Spacing
  >,
  Sequence<
    Regex<R"(")">,
    Repeated<Sequence<Not<Regex<R"(")">>, Char>>,
    Regex<R"(")">,
    Spacing
  >
>;

/*
Identifiers
*/
using IdentStart = Regex<R"([a-zA-Z_])">;
using IdentCont  = Or<IdentStart, Regex<R"([0-9])">>;
using Identifier = Sequence<IdentStart, Repeated<IdentCont>, Spacing>;

/*
Hierarchical Grammar Rules
*/
struct Expression;

// Primary <- Identifier !LEFTARROW / OPEN Expression CLOSE / Literal / Class / DOT
using Primary = Or<
  Sequence<Identifier, Not<LEFTARROW>>,
  Sequence<OPEN, Eval<Expression>, CLOSE>,
  Literal,
  Class,
  DOT
>;

// Suffix <- Primary (QUESTION / STAR / PLUS)?
using Suffix = Or<
  Sequence<Primary, Or<QUESTION, STAR, PLUS>>,
  Primary
>;

// Prefix <- (AND / NOT)? Suffix
using Prefix = Or<
  Sequence<Or<AND, NOT>, Suffix>,
  Suffix
>;

// Sequence <- Prefix*
using SequenceRule = Repeated<Prefix>;

// Expression <- Sequence (SLASH Sequence)*
using ExpressionImpl = Sequence<
  SequenceRule,
  Repeated<Sequence<SLASH, SequenceRule>>
>;

struct Expression : ExpressionImpl {};

// Definition <- Identifier LEFTARROW Expression
using Definition = Sequence<
  Identifier,
  LEFTARROW,
  Expression
>;

// Grammar <- Spacing Definition+ EndOfFile
using Grammar = Sequence<
  Spacing,
  Repeated<Definition>,
  EndOfFile
>;

// clang-format on

} // namespace peg