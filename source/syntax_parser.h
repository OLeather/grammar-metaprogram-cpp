#pragma once

#include <cstddef>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <tuple>
#include <utility>
#include <variant>
#include <vector>

#include "source/grammar/grammar.h"
#include "source/number.h"
#include <iostream>

namespace language {

/*
AST Node Definition
*/

struct Keyword : ASTNode {
  Keyword(const std::string &input) {};
};

struct Spacing : Keyword {
  using Keyword::Keyword;
};

struct LeftParen : Keyword {
  using Keyword::Keyword;
};

struct RightParen : Keyword {
  using Keyword::Keyword;
};

struct Plus : Keyword {
  using Keyword::Keyword;
};

struct Minus : Keyword {
  using Keyword::Keyword;
};

struct Asterisk : Keyword {
  using Keyword::Keyword;
};

struct Slash : Keyword {
  using Keyword::Keyword;
};

struct Equal : Keyword {
  using Keyword::Keyword;
};

struct SemiColon : Keyword {
  using Keyword::Keyword;
};

struct FloatKeyword : Keyword {
  using Keyword::Keyword;
};

struct Identifier : ASTNode {
  std::string name;
  Identifier(const std::string &input) : name(input) {};
};

struct Number : ASTNode {
  number val;
  Number(const std::string &input) : val(NumberFromString(input)) {
    std::cout << " Calling number converter" << std::endl;
  };
};

/*
Grammar Rules
*/

// clang-format off


using SpacingR = Regex<R"([ \t\n\r]*)", Spacing>;
using LeftParenR = Regex<R"(\()", LeftParen>;
using RightParenR = Regex<R"(\))", RightParen>;
using PlusR = Regex<R"(\+)", Plus>;
using MinusR = Regex<R"(\-)", Minus>;
using AsteriskR = Regex<R"(\*)", Asterisk>;
using SlashR = Regex<R"(\/)", Slash>;
using EqualR = Regex<R"(\=)", Equal>;
using SemiColonR = Regex<R"(\;)", SemiColon>;
using FloatKeywordR = Regex<R"(float)", FloatKeyword>;

using IdentifierR = Regex<R"([a-zA-Z_][a-zA-Z0-9_]*)", Identifier>;

using DecimalR = Regex<R"([-+]?(?:\d+(?:\.\d*)?|\.\d+)(?:[eE][-+]?\d+)?[fFdD]?)", Number>;
using IntegerR = Regex<R"([0-9]+)", Number>;
using NumberR = Or<DecimalR, IntegerR>;



/*
# Expr handles + and - via right-recursion
Expr    <- Term '+' Expr
         / Term '-' Expr
         / Term

# Term handles * and / via right-recursion
Term    <- Factor '*' Term
         / Factor '/' Term
         / Factor

# Factor handles parentheses and primary values
Factor  <- '(' Expr ')'
         / Number
*/


struct BinaryExpr : ASTNode {
  ASTPtr left;
  ASTPtr right;
  ASTPtr op;

  BinaryExpr(const std::tuple<ASTPtr, ASTPtr, ASTPtr>& input) {
    left = std::get<0>(input);
    right = std::get<2>(input);
    op = std::get<1>(input);
    LeftParen* left_paren = dynamic_cast<LeftParen*>(left.get());
    RightParen* right_paren = dynamic_cast<RightParen*>(right.get());
    BinaryExpr* expr = dynamic_cast<BinaryExpr*>(op.get());
    if (left_paren && right_paren && expr) {
      left = expr->left;
      right = expr->right;
      op = expr->op;
    }
  }
};

struct ExprR;
struct TermR;
struct FactorR;

struct ExprR : Or<
  SequenceTransform<Sequence<TermR, PlusR, ExprR>, BinaryExpr>,
  SequenceTransform<Sequence<TermR, MinusR, ExprR>, BinaryExpr>,
  TermR>
{};

struct TermR : Or<
  SequenceTransform<Sequence<FactorR, AsteriskR, TermR>, BinaryExpr>,
  SequenceTransform<Sequence<FactorR, SlashR, TermR>, BinaryExpr>,
  FactorR>
{};

struct FactorR : Or<
  SequenceTransform<Sequence<LeftParenR, ExprR, RightParenR>, BinaryExpr>,
  NumberR>
{};

// clang-format on

// TODO (owen): It would be nice to separate rules from nodes. So, start with
// rules. Then, build transformers that transform rules into nodes.

/*
# Expr handles + and - via right-recursion
Expr    <- Term '+' Expr
         / Term '-' Expr
         / Term

# Term handles * and / via right-recursion
Term    <- Factor '*' Term
         / Factor '/' Term
         / Factor

# Factor handles parentheses and primary values
Factor  <- '(' Expr ')'
         / Number
*/

// struct ExprR;
// struct TermR;
// struct FactorR;

// template <> struct rule_layout<ExprR> {
//   using type =
//       Or<Sequence<TermR, Plus, ExprR>, Sequence<TermR, Minus, ExprR>, TermR>;
// };

// template <> struct rule_layout<TermR> {
//   using type = Or<Sequence<FactorR, Asterisk, TermR>,
//                   Sequence<FactorR, Slash, TermR>, FactorR>;
// };

// template <> struct rule_layout<FactorR> {
//   using type = Or<Sequence<LeftParen, ExprR, RightParen>, Numerical>;
// };

// struct ExprR {
//   using Layout = get_layout_t<ExprR>;
// };
// struct TermR {
//   using Layout = get_layout_t<TermR>;
// };
// struct FactorR {
//   using Layout = get_layout_t<FactorR>;
// };

// struct Expr {
//   ASTPtr left;
//   ASTPtr right;
//   std::variant<Plus, Minus, Asterisk, Slash> op;
// };

// struct ToExpr {
//   Expr operator()(const std::variant_alternative_t<0, ExprR::Layout> &v) {
//     const Expr left = ToExpr{}(std::get<0>(v));
//     const auto op = std::get<1>(v);
//     const Expr right = ToExpr{}(std::get<2>(v));
//     return {.left = std::make_shared<ASTNode>(left),
//             .right = std::make_shared<ASTNode>(right),
//             .op = op};
//   };

//   Expr operator()(const std::variant_alternative_t<1, ExprR::ReturnType> &v)
//   {

//   };
//   Expr operator()(const std::variant_alternative_t<2, ExprR::ReturnType> &v)
//   {

//   };
//   Expr operator()(const std::variant_alternative_t<0, TermR::ReturnType> &v)
//   {

//   };
//   Expr operator()(const std::variant_alternative_t<1, TermR::ReturnType> &v)
//   {

//   };
//   Expr operator()(const std::variant_alternative_t<0, FactorR::ReturnType>
//   &v) {

//   };
//   Expr operator()(const std::variant_alternative_t<1, FactorR::ReturnType>
//   &v) {

//   };
// };

// using Operator = std::variant<Plus, Minus, Asterisk, Slash>;

// struct ExprLayoutT;
// using GroupLayoutT = Sequence<LeftParen, Eval<ExprLayoutT>, RightParen>;

// struct Expr {
//   ASTPtr left;
//   Operator op;
//   ASTPtr right;
//    Expr(const GroupLayoutT::ReturnType& group) {

//   }
// };

// struct Expr;

// using Group = Sequence<LeftParen, Eval<Expr>, RightParen>;
// using Factor = Or<Group, Numerical>;

// struct Term;
// struct Term : Or<Sequence<Factor, Asterisk, Eval<Term>>, Sequence<Factor,
// Slash, Eval<Term>>, Factor>{};

// struct ExprVisitor {
//   template <typename T> std::tuple<ASTPtr, ASTPtr, ASTPtr> operator()(const
//   T& t) {

//   }
// };

// struct Expr;
// struct Expr : ASTNode {
//   using LayoutT = Or<Sequence<Term, Plus, Eval<Expr>>, Sequence<Term, Minus,
//   Eval<Expr>, Term>>; using Layout = SequenceTransform<Expr, LayoutT>;
//   explicit Expr(const LayoutT::ReturnType &input) {

//     using FirstType = std::variant_alternative_t<0, LayoutT::ReturnType>;
//     using SecondType = std::variant_alternative_t<1, LayoutT::ReturnType>;

//   };
// };

// struct Factor : public ASTNode{
//   using Layout = Or<Numerical, Identifier, GroupExprLayout>;
// };

// using Expression = Sequence<Numerical, Plus, Numerical>;

// clang-format off
// using ParenthesizedExpr = Sequence<LeftParen, Spacing, Eval<Expression>, Spacing, RightParen>;
// using Decimal = Regex<R"([-+]?(?:\d+(?:\.\d*)?|\.\d+)(?:[eE][-+]?\d+)?[fFdD]?)", NumberNode>;
// using Integer = Regex<R"([0-9]+)", NumberNode>;
// using Number = Or<Decimal, Integer>;

// using Identifier = Regex<R"([a-zA-Z_][a-zA-Z0-9_]*)", IdentifierNode>;

// struct Expression;
// struct Term;

// struct ExpressionNode : public ASTNode {
//   ASTPtr left;
//   ASTPtr right;
//   std::string op;

// };

// struct ParenthesesTransformer {
//   using Layout =
//       std::tuple<LeftParen, Spacing, Eval<Expression>, Spacing, RightParen>;
  
//   static ASTPtr Apply(Layout &&tup) { return std::make_shared<ExpressionNode>(std::move(std::get<2>(tup))); }
// };

// using ParenthesizedExpr =
//     Sequence<LeftParen, Spacing, Eval<Expression>, Spacing, RightParen>;

// using Factor = Or<Number, Identifier, ParenthesizedExpr>;

// using TermLayout = Sequence<Factor, Repeated<Sequence<Spacing, Or<Asterisk, Slash>, Spacing, Factor>>>;
// struct Term : public TermLayout {
//   using ReturnType = TermLayout::ReturnType;
// };

// using ExpressionLayout = Sequence<Term, Repeated<Sequence<Spacing, Or<Plus, Minus>, Spacing, Term>>>;
// struct Expression : public ExpressionLayout {
//   using ReturnType = ExpressionLayout::ReturnType;
// };

// using Statement = Or<Expression>;

// using Program = Sequence<Spacing, Repeated<Sequence<Statement, Spacing>>, End>;

// clang-format on

} // namespace language

// using AssignLayout = Sequence<Identifier, Spacing, Equal, Spacing,
// Expression, Spacing, SemiColon>; using Assignment = Transform<AssignLayout,
// AssignmentTransformer>;

// using ExpressionStatementLayout = Sequence<Expression, Spacing, SemiColon>;
// using ExpressionStatement = Transform<ExpressionStatementLayout,
// ExpressionStatementTransformer>;

// using DeclarationWithoutAssignmentLayout = Sequence<FloatKeyword, Spacing,
// Identifier, Spacing, SemiColon>; using DeclarationWithoutAssignment =
// Transform<DeclarationWithoutAssignmentLayout,
// DeclarationTransformer<DeclarationWithoutAssignmentLayout>>;

// using DeclarationWithAssignmentLayout = Sequence<FloatKeyword, Spacing,
// Identifier, Spacing, Equal, Spacing, Expression, Spacing, SemiColon>; using
// DeclarationWithAssignment = Transform<DeclarationWithAssignmentLayout,
// DeclarationTransformer<DeclarationWithAssignmentLayout>>;

// using Declaration = Or<DeclarationWithoutAssignment,
// DeclarationWithAssignment>;

// using Statement = Or<Assignment, ExpressionStatement, Declaration>;

// struct ParenthesesTransformer {
//   using TupleType =
//       std::tuple<std::string, std::string, ASTPtr, std::string, std::string>;

//   static ASTPtr Apply(TupleType &&tup) { return std::move(std::get<2>(tup));
//   }
// };

// template <typename Layout> struct LeftAssociativeTransformer {
//   using MatchType = typename Layout::ReturnType;
//   static ASTPtr Apply(MatchType &&value) {
//     auto &[head_node, tail_vec] = value;
//     ASTPtr current_tree = std::move(head_node);

//     for (auto &element : tail_vec) {
//       auto &[space1, op, space2, next_node] = element;
//       current_tree = std::make_unique<BinaryExprNode>(
//           op, std::move(current_tree), std::move(next_node));
//     }
//     return current_tree;
//   }
// };

// struct AssignmentTransformer {
//   using TupType = std::tuple<ASTPtr, std::string, std::string, std::string,
//                              ASTPtr, std::string, std::string>;
//   static ASTPtr Apply(TupType &&value) {
//     auto &[id, space1, eq, space2, expr_node, space3, semi] = value;
//     return std::make_unique<AssignmentNode>(std::move(id),
//                                             std::move(expr_node));
//   }
// };

// struct ExpressionStatementTransformer {
//   using TupType = std::tuple<ASTPtr, std::string, std::string>;
//   static ASTPtr Apply(TupType &&value) {
//     auto &[expr_node, space1, semi] = value;
//     return std::move(expr_node);
//   }
// };

// struct ProgramTransformer {
//   using TupType =
//       std::tuple<std::string, std::vector<std::tuple<ASTPtr, std::string>>,
//                  std::string>;
//   static ASTPtr Apply(TupType &&value) {
//     auto &[initial_space, statement_tuples, eof] = value;
//     auto prog_node = std::make_unique<ProgramNode>();

//     for (auto &stmt_tuple : statement_tuples) {
//       prog_node->statements.push_back(std::move(std::get<0>(stmt_tuple)));
//     }

//     return prog_node;
//   }
// };

// template <typename Layout> struct DeclarationTransformer {
//   using TupType = Layout::ReturnType;
//   static ASTPtr Apply(TupType &&value) {
//     const auto &keyword = std::get<0>(value);
//     auto &id = std::get<2>(value);
//     ASTPtr assignment = nullptr;
//     constexpr auto kSizeWithoutAssignment = 5;
//     if constexpr (std::tuple_size_v<TupType> > kSizeWithoutAssignment) {
//       auto &expr_node = std::get<6>(value);
//       const auto id_casted = dynamic_cast<IdentifierNode *>(id.get());
//       if (!id_casted) {
//         throw std::runtime_error("Expected IdentifierNode for variable
//         name.");
//       }

//       ASTPtr id_copy =
//           std::make_unique<IdentifierNode>(id_casted->name);
//       assignment = std::make_unique<AssignmentNode>(std::move(id_copy),
//                                                     std::move(expr_node));
//     }

//     return std::make_unique<DeclarationNode>(keyword, std::move(id),
//                                              std::move(assignment));
//   }
// };
