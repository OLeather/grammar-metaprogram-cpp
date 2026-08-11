#include "examples/calculator/syntax.h"
#include <cmath>

namespace example::calculator::bindings {
using namespace example::calculator::syntax;

struct NumberVisitor {
  double operator()(const Matcher<Float>::ReturnType &f) {
    return std::stod(f.match);
  }
  double operator()(const Matcher<Integer>::ReturnType &i) {
    return std::stoi(i.match);
  }
};

struct Bindings {
  double operator()(const Matcher<Calculation>::TupleType &c) {
    const auto expr_eval = std::get<1>(c);
    return Bindings{}(expr_eval);
  }

  double operator()(const Matcher<Eval<Expression>>::ReturnType &e) {
    const auto expr = e.get().value;
    const auto left = std::get<0>(expr);
    const auto right_vec = std::get<1>(expr);
    double sum = Bindings{}(left);
    for (const auto &r : right_vec) {
      const bool plus =
          std::holds_alternative<Matcher<Plus>::TupleType>(std::get<0>(r));
      const double val = Bindings{}(std::get<1>(r));
      if (plus)
        sum += val;
      else
        sum -= val;
    }
    return sum;
  }

  double operator()(const Matcher<Term>::TupleType &t) {
    const auto left = std::get<0>(t);
    const auto right_vec = std::get<1>(t);
    double sum = Bindings{}(left);
    for (const auto &r : right_vec) {
      const bool mult =
          std::holds_alternative<Matcher<Star>::TupleType>(std::get<0>(r));
      const double val = Bindings{}(std::get<1>(r));
      if (mult)
        sum *= val;
      else
        sum /= val;
    }
    return sum;
  }

  double operator()(const Matcher<Eval<Factor>>::ReturnType &f) {
    const auto factor = f.get().value;
    const auto left = std::get<0>(factor);
    const auto exponent_opt = std::get<1>(factor);
    double res = Bindings{}(left);
    if (exponent_opt) {
      res = std::pow(res, Bindings{}(std::get<1>(exponent_opt.value())));
    }
    return res;
  }

  double operator()(const Matcher<ExpFactor>::TupleType &e) {
    return Bindings{}(std::get<1>(e));
  }

  double operator()(const Matcher<Primary>::VariantType &p) {
    struct PrimaryVisitor {
      double operator()(
          const Matcher<Seq<LParen, Eval<Expression>, RParen>>::TupleType &t) {
        return Bindings{}(std::get<1>(t));
      }
      double operator()(const Matcher<Number>::TupleType &n) {
        return std::visit(NumberVisitor{}, std::get<0>(n));
      }
    };
    return std::visit(PrimaryVisitor{}, p);
  }
};
} // namespace example::calculator::bindings