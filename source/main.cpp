#include "source/syntax_parser.h"
#include <iostream>
#include <stdexcept>

using namespace language;

const std::string kTestCode = "4.3+(5.7+3.4)*2";

void PrintIndent(int indent){
  for (int i = 0; i < indent; i++) {
    std::cout << " ";

  }
}

void PrintBinaryExpr(BinaryExpr* in, int indent) {
  if (in->op) {
    Plus* plus = dynamic_cast<Plus*>(in->op.get());
    if (plus){
      std::cout << "+" << std::endl;
    }
    Asterisk* asterisk = dynamic_cast<Asterisk*>(in->op.get());
    if (asterisk){
      std::cout << "*" << std::endl;
    }
  }
  if (in->left) {
    BinaryExpr* expr = dynamic_cast<BinaryExpr*>(in->left.get());
    if (expr) {
      PrintBinaryExpr(expr, indent + 1);
    }
    Number* number = dynamic_cast<Number*>(in->left.get());
    if (number) {
      PrintIndent(indent);
      std::visit([](const auto& t){std::cout << t << std::endl;}, number->val);
    }
  }

  if (in->right) {
    BinaryExpr* expr = dynamic_cast<BinaryExpr*>(in->right.get());
    if (expr) {
      PrintBinaryExpr(expr, indent + 1);
    }
    Number* number = dynamic_cast<Number*>(in->right.get());
    if (number) {
            PrintIndent(indent);
      std::visit([](const auto& t){std::cout << t << std::endl;}, number->val);
    }
  }
}

int main() {
  Context input{kTestCode};
  auto result = ExprR::Match(input, true);
  if (!result.has_value()) {
    throw std::runtime_error("Result did not compile");
  }

  if (result) {
    ASTPtr res = result->value;
    if (!res) throw std::runtime_error("Result is not present");
    BinaryExpr* expr = dynamic_cast<BinaryExpr*>(res.get());
    if(!expr) throw std::runtime_error("Result is not binary expr");
    PrintBinaryExpr(expr, 0);
    std::cout << "Parsed successfully" << std::endl;
  }
}