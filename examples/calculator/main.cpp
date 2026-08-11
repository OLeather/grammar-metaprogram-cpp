#include "examples/calculator/syntax.h"
#include "examples/calculator/bindings.h"
#include <iostream>

using namespace example::calculator::syntax;
using namespace example::calculator::bindings;

int main() {
  std::string input = "4.3 + (7 + 3.4^5) * 6";
  language::Context ctx{.input = input};
  const auto actual_value = 4.3 + (7 + std::pow(3.4,5)) * 6;

  constexpr bool kConsume{true};
  if (auto result = Matcher<Calculation>::Match(ctx, kConsume)) {
    std::cout << "Parse success!\n";
    const auto parsed_value = Bindings{}(result->value);
    std::cout << "Parsed expression value: " << parsed_value << std::endl;
    std::cout << "Actual value: " << actual_value << std::endl;
    if (actual_value == parsed_value) {
      std::cout << "Parsed value equals actual value!" << std::endl;
    }
  } else {
    std::cerr << "Parse failed!\n";
  }
}