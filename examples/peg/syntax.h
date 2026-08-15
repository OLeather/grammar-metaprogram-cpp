/*
Figure 1. of https://bford.info/pub/lang/peg.pdf
PEG formally describing its own ASCII syntax

# Hierarchical syntax
Grammar <- Spacing Definition+ EndOfFile
Definition <- Identifier LEFTARROW Expression

Expression <- Sequence (SLASH Sequence)*
Sequence <- Prefix*
Prefix <- (AND / NOT)? Suffix
Suffix <- Primary (QUESTION / STAR / PLUS)?
Primary <- Identifier !LEFTARROW
/ OPEN Expression CLOSE
/ Literal / Class / DOT

# Lexical syntax
Identifier <- IdentStart IdentCont* Spacing
IdentStart <- [a-zA-Z_]
IdentCont <- IdentStart / [0-9]

Literal <- [’] (![’] Char)* [’] Spacing
/ ["] (!["] Char)* ["] Spacing
Class <- ’[’ (!’]’ Range)* ’]’ Spacing
Range <- Char ’-’ Char / Char
Char <- ’\\’ [nrt’"\[\]\\]
/ ’\\’ [0-2][0-7][0-7]
/ ’\\’ [0-7][0-7]?
/ !’\\’ .

LEFTARROW <- ’<-’ Spacing
SLASH <- ’/’ Spacing
AND <- ’&’ Spacing
NOT <- ’!’ Spacing
QUESTION <- ’?’ Spacing
STAR <- ’*’ Spacing
PLUS <- ’+’ Spacing
OPEN <- ’(’ Spacing
CLOSE <- ’)’ Spacing
DOT <- ’.’ Spacing

Spacing <- (Space / Comment)*
Comment <- ’#’ (!EndOfLine .)* EndOfLine
Space <- ’ ’ / ’\t’ / EndOfLine
EndOfLine <- ’\r\n’ / ’\n’ / ’\r’
EndOfFile <- !.
*/

#pragma once

#include "source/grammar.h"

namespace peg {

using namespace language;

struct ExpressionDef;

// clang-format off
using EndOfLine = Or<Regex<"\r\n">, Regex<"\n">, Regex<"\r">>;
using Space      = Or<Regex<" ">, Regex<"\t">, EndOfLine>;

using Comment = Seq<
    Regex<"#">,
    Repeated<Conditional<Not<EndOfLine>, Regex<".">>>,
    EndOfLine
>;

using Spacing   = Repeated<Or<Space, Comment>>;

using LEFTARROW = Seq<Regex<"<-">, Spacing>;
using SLASH     = Seq<Regex<"/">, Spacing>;
using AND       = Seq<Regex<"&">, Spacing>;
using NOT       = Seq<Regex<"!">, Spacing>;
using QUESTION  = Seq<Regex<"\\?">, Spacing>;
using STAR      = Seq<Regex<"\\*">, Spacing>;
using PLUS      = Seq<Regex<"\\+">, Spacing>;
using OPEN      = Seq<Regex<"\\(">, Spacing>;
using CLOSE     = Seq<Regex<"\\)">, Spacing>;
using DOT       = Seq<Regex<"\\.">, Spacing>;

using IdentStart = Regex<"[a-zA-Z_]">;
using IdentCont  = Or<IdentStart, Regex<"[0-9]">>;
using Identifier = Seq<IdentStart, Repeated<IdentCont>, Spacing>;

using Char = Or<
    Regex<"\\\\([nrt'\"\\[\\]\\\\])">,
    Regex<"\\\\([0-2][0-7][0-7])">,
    Regex<"\\\\([0-7]{1,2})">, 
    Seq<Not<Regex<"\\\\">>, Regex<".">>
>;

using Range = Or<
    Seq<Char, Regex<"-">, Char>,
    Char
>;

using Class = Seq<
    Regex<"\\[">,
    Repeated<Conditional<Not<Regex<"\\]">>, Range>>,
    Regex<"\\]">,
    Spacing
>;

using Literal = Or<
    Seq<Regex<"'">, Repeated<Conditional<Not<Regex<"'">>, Char>>, Regex<"'">, Spacing>,
    Seq<Regex<"\"">, Repeated<Conditional<Not<Regex<"\"">>, Char>>, Regex<"\"">, Spacing>
>;


using Primary = Or<
    Seq<Identifier, Not<LEFTARROW>>,
    Seq<OPEN, Eval<ExpressionDef>, CLOSE>,
    Literal,
    Class,
    DOT
>;

using Suffix   = Seq<Primary, Optional<Or<QUESTION, STAR, PLUS>>>;
using Prefix   = Seq<Optional<Or<AND, NOT>>, Suffix>;
using Sequence = Repeated<Prefix>;

struct ExpressionDef : Def<Seq<Sequence, Repeated<Seq<SLASH, Sequence>>>> {};

using Definition = Seq<Identifier, LEFTARROW, Eval<ExpressionDef>>;
using Grammar    = Seq<Spacing, Repeated<Definition>, EndOfFile>;
//clang-format on

} // namespace peg
