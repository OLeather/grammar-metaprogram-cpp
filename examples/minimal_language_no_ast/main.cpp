#include "examples/minimal_language_no_ast/syntax.h"
#include <iostream>

using namespace example::syntax;

// struct Visitor{
//     void operator()(const Plus& star) {
//         std::cout << "+" << std::endl;
//     }
//     void operator()(const Star& star) {
//         std::cout << "*" << std::endl;
//     }
// };

struct Bindings {
  void operator()(const Matcher<Number>::ReturnType &num) {
    std::cout << "Number: " << num.match << std::endl;
  }

  void operator()(const Matcher<Term>::TupleType &t) {
    const auto left = std::get<0>(t);
    const auto right_vec = std::get<1>(t);
    Bindings{}(left);
    for (const auto& r : right_vec) {
        Bindings{}(r);
    }
  }

  void operator()(const Matcher<Factor>::VariantType &t) {
    std::visit(Bindings{}, t);
  }

  void operator()(const Matcher<Seq<Plus, Term>>::TupleType& t) {
    std::cout << "+" << std::endl;
    Bindings{}(std::get<1>(t));
  }

  void operator()(const Matcher<Seq<Star, Factor>>::TupleType& t) {
    std::cout << "*" << std::endl;
    Bindings{}(std::get<1>(t));
  }

  void operator()(const Matcher<Seq<Term, Repeated<Seq<Plus, Term>>>>::TupleType& t) {
    const auto left = std::get<0>(t);
    const auto right_vec = std::get<1>(t);
    Bindings{}(left);
    for (const auto& r : right_vec) {
        Bindings{}(r);
    }
  }

  void operator()(const Matcher<Eval<Expr>>::ReturnType &t) {
    std::cout << "Eval<Expr>: " << std::endl;
    const auto res = t.get().value;
    std::visit(Bindings{}, res);
  }

  void operator()(const Matcher<Parenthesized>::TupleType &p) {
    const auto expr_or_num_ref = std::get<1>(p);
    const auto expr_or_num = expr_or_num_ref.get();
    std::cout << "Parentesized: " << std::endl;
    std::visit(Bindings{}, expr_or_num.value);
  }

  void operator()(const Matcher<Grammar>::TupleType& t) {
    const auto g = std::get<0>(t);
    std::cout << "Grammar" << std::endl;
    Bindings{}(g);
  }
};

int main() {
  std::string input = "5+(3+2)*4E";
  language::Context ctx{.input = input};

  constexpr bool kConsume{true};
  if (auto result = Matcher<Grammar>::Match(ctx, kConsume)) {
    std::cout << "Parse success!\n";
    Bindings{}(result->value);

  } else {
    std::cerr << "Parse failed!\n";
  }
}