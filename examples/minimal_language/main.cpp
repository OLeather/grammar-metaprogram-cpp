#include "examples/minimal_language/syntax.h"
#include <iostream>
#include <stdexcept>

using namespace syntax;

struct OperatorVisitor {
  void operator()(const PlusR &op) {
    std::cout << "Operator(" + op.value + ")" << std::endl;
  }

  void operator()(const MinusR &op) {
    std::cout << "Operator(" + op.value + ")" << std::endl;
  }
  void operator()(const AsteriskR &op) {
    std::cout << "Operator(" + op.value + ")" << std::endl;
  }
  void operator()(const SlashR &op) {
    std::cout << "Operator(" + op.value + ")" << std::endl;
  }
};

struct LeafVisitor {
  void operator()(const DecimalR &num) {
    std::cout << "Leaf(" + num.value + ")" << std::endl;
  }
  void operator()(const IntegerR &num) {
    std::cout << "Leaf(" + num.value + ")" << std::endl;
  }
  void operator()(const NumberR::ReturnType &num) {
    std::visit(*this, num);
  }
  void operator()(const IdentifierR &id) {
    std::cout << "Leaf(" + id.value + ")" << std::endl;
  }
};

void PrintIndent(int indent) {
  for (int i = 0; i < indent; i++) {
    std::cout << " ";
  }
}

void PrintExpression(ExpressionPtr expr, int indent = 0) {
  if (!expr) return;
  std::shared_ptr<BinaryExpression> binary_expr =
      std::dynamic_pointer_cast<BinaryExpression>(expr);
  if (binary_expr) {
    const auto op = binary_expr->op;
    PrintExpression(binary_expr->left, indent+1);
    PrintIndent(indent);
    std::visit(OperatorVisitor{}, op);
    PrintExpression(binary_expr->right, indent+1);
    return;
  }
  std::shared_ptr<LeafExpression> leaf_expr =
      std::dynamic_pointer_cast<LeafExpression>(expr);
  if (leaf_expr) {
    PrintIndent(indent-1);
    std::visit(LeafVisitor{}, leaf_expr->leaf);
    return;
  }
}

int main() {
  const std::string kTestCode = "a+(b + c) * 3.4f / 2 * 4 + 3";
  language::Context ctx{kTestCode};
  auto res = language::match_rule<Statement>(ctx, true);
  if (!res)
    throw std::runtime_error("Could not parse code");
  std::cout << "Successfully parsed expression" << std::endl;
  ExpressionPtr expr = res->value;
  if (!expr)
    throw std::runtime_error("Could not convert to ExpressionPtr");
  PrintExpression(expr);
}