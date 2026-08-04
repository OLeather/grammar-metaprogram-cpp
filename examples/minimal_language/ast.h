#pragma once

#include "syntax.h"
#include <iostream>
#include <memory>
#include <string>
#include <variant>

namespace example::ast {

// Forward Declarations of Domain Nodes
struct NumberExpr;
struct BinaryExpr;

using ASTExpr = std::variant<
    NumberExpr,
    std::unique_ptr<BinaryExpr>
>;

struct NumberExpr {
    int value;
};

enum class Operator {
    Add,
    Multiply
};

struct BinaryExpr {
    Operator op;
    ASTExpr left;
    ASTExpr right;
};

// Visitor Transformer class using tag-dispatching
struct ASTBuilder {
    template <typename Rule>
    ASTExpr build(const language::Node& node) const {
        return (*this)(language::Tag<Rule>{}, node);
    }

    // Convert Number
    ASTExpr operator()(language::Tag<syntax::Number>, const language::Node& node) const {
        return NumberExpr{ std::stoi(std::string(node.match_text)) };
    }

    // Convert Parenthesized: Index 1 contains the inner Expr
    ASTExpr operator()(language::Tag<syntax::Parenthesized>, const language::Node& node) const {
        return build<syntax::Expr>(node.children[1]);
    }

    // Convert Factor
    ASTExpr operator()(language::Tag<syntax::Factor>, const language::Node& node) const {
        // A Factor is Or<Number, Parenthesized>. 
        // Number produces a Regex node with NO children, 
        // whereas Parenthesized produces a Sequence node WITH children.
        if (node.children.empty()) { 
            return build<syntax::Number>(node);
        }
        return build<syntax::Parenthesized>(node);
    }

    // Convert Term (Handles left-associative sequence of multiplications)
    ASTExpr operator()(language::Tag<syntax::Term>, const language::Node& node) const {
        ASTExpr current = build<syntax::Factor>(node.children[0]);
        const auto& repeated_tail = node.children[1]; // Repeated<Sequence<Star, Factor>>

        for (const auto& seq_node : repeated_tail.children) {
            ASTExpr rhs = build<syntax::Factor>(seq_node.children[1]);
            current = std::make_unique<BinaryExpr>(BinaryExpr{
                .op = Operator::Multiply,
                .left = std::move(current),
                .right = std::move(rhs)
            });
        }
        return current;
    }

    // Convert Expr
    ASTExpr operator()(language::Tag<syntax::Expr>, const language::Node& node) const {
        // Expr is Or<Sequence<Term, Repeated<...>>, Term>
        // If the top sequence matched, children[1] holds the Repeated tail.
        if (node.children.size() > 1) {
            ASTExpr current = build<syntax::Term>(node.children[0]);
            const auto& repeated_tail = node.children[1];

            for (const auto& seq_node : repeated_tail.children) {
                ASTExpr rhs = build<syntax::Term>(seq_node.children[1]);
                current = std::make_unique<BinaryExpr>(BinaryExpr{
                    .op = Operator::Add,
                    .left = std::move(current),
                    .right = std::move(rhs)
                });
            }
            return current;
        }

        // Direct Term fallback
        return build<syntax::Term>(node);
    }
};

// Printable AST Evaluator Helper
inline int evaluate(const ASTExpr& expr) {
    return std::visit(
        [](auto&& arg) -> int {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, NumberExpr>) {
                return arg.value;
            } else if constexpr (std::is_same_v<T, std::unique_ptr<BinaryExpr>>) {
                int left = evaluate(arg->left);
                int right = evaluate(arg->right);
                return (arg->op == Operator::Add) ? (left + right) : (left * right);
            }
        },
        expr
    );
}

} // namespace example::ast