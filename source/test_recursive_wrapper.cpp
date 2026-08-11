#include <iostream>
#include <variant>
#include <boost/variant/recursive_wrapper.hpp>

// Helper for inline overload pattern
template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

struct A;

// B can contain an int or a wrapped A
using B = std::variant<int, boost::recursive_wrapper<A>>;

struct A {
    using T = std::variant<int, B>;
    T data;
};

void process_B(const B& b);

void process_T(const A::T& t) {
    std::visit(overloaded {
        [](int val) {
            std::cout << "Found int in T: " << val << '\n';
        },
        [](const B& b) {
            std::cout << "Found B in T... down the rabbit hole!\n";
            process_B(b); // Recurse into B
        }
    }, t);
}

void process_B(const B& b) {
    std::visit(overloaded {
        [](int val) {
            std::cout << "Found int in B: " << val << '\n';
        },
        [](const boost::recursive_wrapper<A>& wrapper) {
            const A& a = wrapper.get(); // Access A explicitly
            std::cout << "Found A inside B!\n";
            process_T(a.data);
        }
    }, b);
}

int main() {
    // Constructing: T -> B -> A -> T -> int
    A nested_a{ A::T{ 42 } };
    B b_with_a{ nested_a };
    A::T root_t{ b_with_a };

    process_T(root_t);
}