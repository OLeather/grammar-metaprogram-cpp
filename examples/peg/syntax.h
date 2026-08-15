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
using EndOfLine = std::variant<Regex<"\r\n">, Regex<"\n">, Regex<"\r">>;
using Space      = std::variant<Regex<" ">, Regex<"\t">, EndOfLine>;

using Comment = std::tuple<
    Regex<"#">,
    std::vector<std::tuple<Not<EndOfLine>, Regex<".">>>,
    EndOfLine
>;

using Spacing   = std::vector<std::variant<Space, Comment>>;

using LEFTARROW = std::tuple<Regex<"<-">, Spacing>;
using SLASH     = std::tuple<Regex<"/">, Spacing>;
using AND       = std::tuple<Regex<"&">, Spacing>;
using NOT       = std::tuple<Regex<"!">, Spacing>;
using QUESTION  = std::tuple<Regex<"\\?">, Spacing>;
using STAR      = std::tuple<Regex<"\\*">, Spacing>;
using PLUS      = std::tuple<Regex<"\\+">, Spacing>;
using OPEN      = std::tuple<Regex<"\\(">, Spacing>;
using CLOSE     = std::tuple<Regex<"\\)">, Spacing>;
using DOT       = std::tuple<Regex<"\\.">, Spacing>;

using IdentStart = Regex<"[a-zA-Z_]">;
using IdentCont  = std::variant<IdentStart, Regex<"[0-9]">>;
using Identifier = std::tuple<IdentStart, std::vector<IdentCont>, Spacing>;

using Char = std::variant<
    Regex<"\\\\([nrt'\"\\[\\]\\\\])">,
    Regex<"\\\\([0-2][0-7][0-7])">,
    Regex<"\\\\([0-7]{1,2})">, 
    std::tuple<Not<Regex<"\\\\">>, Regex<".">>
>;

using Range = std::variant<
    std::tuple<Char, Regex<"-">, Char>,
    Char
>;

using Class = std::tuple<
    Regex<"\\[">,
    std::vector<std::tuple<Not<Regex<"\\]">>, Range>>,
    Regex<"\\]">,
    Spacing
>;

using Literal = std::variant<
    std::tuple<Regex<"'">, std::vector<std::tuple<Not<Regex<"'">>, Char>>, Regex<"'">, Spacing>,
    std::tuple<Regex<"\"">, std::vector<std::tuple<Not<Regex<"\"">>, Char>>, Regex<"\"">, Spacing>
>;

using Primary = std::variant<
    std::tuple<Identifier, Not<LEFTARROW>>,
    std::tuple<OPEN, boost::recursive_wrapper<ExpressionDef>, CLOSE>,
    Literal,
    Class,
    DOT
>;

using Suffix   = std::tuple<Primary, std::optional<std::variant<QUESTION, STAR, PLUS>>>;
using Prefix   = std::tuple<std::optional<std::variant<AND, NOT>>, Suffix>;
using Sequence = std::vector<Prefix>;

struct ExpressionDef : Def<std::tuple<Sequence, std::vector<std::tuple<SLASH, Sequence>>>> {};

using Definition = std::tuple<Identifier, LEFTARROW, boost::recursive_wrapper<ExpressionDef>>;
using Grammar    = std::tuple<Spacing, std::vector<Definition>, EndOfFile>;
//clang-format on

} // namespace peg
