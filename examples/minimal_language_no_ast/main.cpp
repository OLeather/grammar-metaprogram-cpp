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

int main() {
    std::string input = "((a))";
    language::Context ctx{.input = input};

    if (auto result = Matcher<ExprGrammar>::Match(ctx, /*consume=*/true)) {
        std::cout << "Parse success!\n";
    } else {
        std::cerr << "Parse failed!\n";
    }
}