#pragma once

#include "source/grammar.h"
#include "source/number.h"
#include <iostream>
#include <memory>
#include <stdexcept>

namespace syntax {
using namespace language;

/*
Rule Definitions
*/

// clang-format off
using SpacingR = Regex<R"([ \t\n\r]*)">;
using LeftParenR = Regex<R"(\()">;
using RightParenR = Regex<R"(\))">;
using PlusR = Regex<R"(\+)">;
using MinusR = Regex<R"(\-)">;
using AsteriskR = Regex<R"(\*)">;
using SlashR = Regex<R"(\/)">;
using EqualR = Regex<R"(\=)">;
using SemiColonR = Regex<R"(\;)">;
using FloatKeywordR = Regex<R"(float)">;

using IdentifierR = Regex<R"([a-zA-Z_][a-zA-Z0-9_]*)">;

using DecimalR = Regex<R"([-+]?(?:\d+(?:\.\d*)?|\.\d+)(?:[eE][-+]?\d+)?[fFdD]?)">;
using IntegerR = Regex<R"([0-9]+)">;
using NumberR = Or<DecimalR, IntegerR>;

struct Expression;
using ExpressionPtr = std::shared_ptr<Expression>;

struct ExpressionR;
struct TermR;
struct GroupR;
struct FactorR;

using GroupL = Sequence<LeftParenR, SpacingR, Eval<ExpressionR>, SpacingR, RightParenR>;
using FactorL = Or<NumberR, IdentifierR, Eval<GroupR>>;

using TermL = Sequence<Eval<FactorR>, Repeated<Sequence<SpacingR, Or<AsteriskR, SlashR>, SpacingR, Eval<FactorR>>>>;
using ExpressionL = Sequence<Eval<TermR>, Repeated<Sequence<SpacingR, Or<PlusR, MinusR>, SpacingR, Eval<TermR>>>>;

template <typename In, typename Out>
struct ExpressionConverter {
  Out operator()(In::ReturnType t) { return nullptr; };
};

struct ExpressionR : Converter<ExpressionL, ExpressionConverter<ExpressionL, ExpressionPtr>, ExpressionPtr> {};
struct GroupR : Converter<GroupL, ExpressionConverter<GroupL, ExpressionPtr>, ExpressionPtr> {};
struct TermR : Converter<TermL, ExpressionConverter<TermL, ExpressionPtr>, ExpressionPtr> {};
struct FactorR : Converter<FactorL, ExpressionConverter<FactorL, ExpressionPtr>, ExpressionPtr> {};

using Operator = std::variant<PlusR, MinusR, SlashR, AsteriskR>;
using FactorT = FactorL::ReturnType;

using Statement = Or<ExpressionR>;

// clang-format on

/*
AST Definitions
*/

struct Expression {
  virtual ~Expression() = default;
};

struct LeafExpression : Expression {
  std::variant<NumberR::ReturnType, IdentifierR> leaf;
  LeafExpression(auto leaf_) : leaf(leaf_) {};
};

struct BinaryExpression : Expression {
  ExpressionPtr left;
  ExpressionPtr right;
  Operator op;
  BinaryExpression(auto left, auto right, auto op)
      : left(left), right(right), op(op) {};
};


template <>
inline ExpressionPtr ExpressionConverter<FactorL, ExpressionPtr>::operator()(
    FactorL::ReturnType in) {
  struct Visitor {
    ExpressionPtr operator()(NumberR::ReturnType t) {
      return std::make_shared<LeafExpression>(LeafExpression(t));
    };
    ExpressionPtr operator()(IdentifierR t) {
      return std::make_shared<LeafExpression>(LeafExpression(t));
    };
    ExpressionPtr operator()(ExpressionPtr t) { return t; };
  };

  return std::visit(Visitor{}, in);
}

template <>
inline ExpressionPtr
ExpressionConverter<GroupL, ExpressionPtr>::operator()(GroupL::ReturnType in) {
  return std::get<2>(in);
}

template <typename T> ExpressionPtr ExpressionTransformer(const T &in) {
  const auto left = std::get<0>(in);
  const auto right_exprs = std::get<1>(in);
  struct Visitor {
    Operator operator()(const AsteriskR &t) { return t; }
    Operator operator()(const SlashR &t) { return t; }
    Operator operator()(const PlusR &t) { return t; }
    Operator operator()(const MinusR &t) { return t; }
  };

  ExpressionPtr current = left;
  for (const auto &right_seq : right_exprs) {
    const auto op = std::get<1>(right_seq);
    const auto right = std::get<3>(right_seq);
    current = std::make_shared<BinaryExpression>(
        BinaryExpression(current, right, std::visit(Visitor{}, op)));
  }
  return current;
}

template <>
inline ExpressionPtr
ExpressionConverter<TermL, ExpressionPtr>::operator()(TermL::ReturnType in) {
  return ExpressionTransformer<TermL::ReturnType>(in);
}

template <>
inline ExpressionPtr
ExpressionConverter<ExpressionL, ExpressionPtr>::operator()(
    ExpressionL::ReturnType in) {
  return ExpressionTransformer<ExpressionL::ReturnType>(in);
}

} // namespace syntax