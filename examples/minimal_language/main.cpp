#include "examples/minimal_language/ast.h"
#include "examples/minimal_language/node_printer.h"
#include <iostream>

int main() {
    std::string input = "2+3*4";
    language::Context ctx{.input = input};

    if (auto parse_result = language::match_rule<example::syntax::Grammar>(ctx)) {
      language::print_ast(parse_result->value);  
      example::ast::ASTBuilder builder;
        
        // Root sequence children: [0] = Expr, [1] = EndOfFile
        auto ast = builder.build<example::syntax::Expr>(parse_result->value.children[0]);
        
        std::cout << "Input: " << input << "\n";
        std::cout << "Evaluated Output: " << example::ast::evaluate(ast) << "\n"; // Output: 14
    } else {
        std::cerr << "Parse failed!\n";
    }
}