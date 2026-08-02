#pragma once
#include "source/grammar/grammar.h"

namespace peg {
using namespace language;

struct Keyword : ASTNode {};
struct Dot : Keyword {};
struct Close : Keyword {};
struct Open : Keyword {};
struct Plus : Keyword {};
struct Star : Keyword {};
struct Question : Keyword {};
struct Not : Keyword {};
struct And : Keyword {};
struct Slash : Keyword {};
struct LeftArrow : Keyword {};

// clang-format off


using SpacingR = Regex<R"([ \t\n\r]*)", Keyword>;
using EndOfLineR = Or<Regex<"\r\n", Keyword>, Regex<"\n", Keyword>, Regex<"\r", Keyword>>;
using SpaceR = Or<Regex<" ", Keyword>, Regex<"\t", Keyword>, EndOfLineR>;
using AnyChar = Regex<R"(.)", Keyword>;
using CommentTerminatorR = Or<EndOfLineR, EndOfFile>;
using CommentR = Sequence<Regex<"#", Keyword>, Repeated<Conditional<language::Not<CommentTerminatorR>, AnyChar>>, CommentTerminatorR>;

using DotR = Sequence<Regex<".", Dot>, SpacingR>;
using CloseR = Sequence<Regex<")", Close>, SpacingR>;
using OpenR = Sequence<Regex<"(", Open>, SpacingR>;
using PlusR = Sequence<Regex<"+", Plus>, SpacingR>;
using StarR = Sequence<Regex<"*", Star>, SpacingR>;
using QuestionR = Sequence<Regex<"?", Question>, SpacingR>;
using NotR = Sequence<Regex<"!", Not>, SpacingR>;
using AndR = Sequence<Regex<"&", And>, SpacingR>;
using SlashR = Sequence<Regex<"/", Slash>, SpacingR>;
using LeftArrowR = Sequence<Regex<"<-", LeftArrow>, SpacingR>;

using Char = Or<
Sequence<Regex<R"(\\)", ASTNode>, Or<
  Regex<"n", ASTNode>, Regex<"r", ASTNode>, Regex<"t", ASTNode>, Regex<"'", ASTNode>, Regex<R"(\")", ASTNode>, Regex<R"(\\)", ASTNode>> 
  // TODO (owen): Digits
  // TODO (owen): Unescaped regular char
>>;

using Range = Or<Sequence<Char, Regex<"-", ASTPtr>, Char>, Char>;


// clang-format on

} // namespace peg