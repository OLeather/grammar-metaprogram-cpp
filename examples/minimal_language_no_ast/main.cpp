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

  void operator()(const Matcher<Parenthesized>::TupleType &p) {
    const auto expr_or_num_ref = std::get<1>(p);
    const auto expr_or_num = expr_or_num_ref.get();
    std::cout << "Parentesized: " << std::endl;
    std::visit(Bindings{}, expr_or_num.value);
  }
};

int main() {
  std::string input = "((5))";
  language::Context ctx{.input = input};

  constexpr bool kConsume{true};
  if (auto result = Matcher<Parenthesized>::Match(ctx, kConsume)) {
    std::cout << "Parse success!\n";
    Visitor{}(result->value);

  } else {
    std::cerr << "Parse failed!\n";
  }
}