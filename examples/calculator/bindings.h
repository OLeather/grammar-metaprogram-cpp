#include "examples/calculator/syntax.h"
#include <cmath>

namespace example::calculator::bindings {
using namespace example::calculator::syntax;

struct NumberVisitor {
  double operator()(const Float &f) { return std::stod(f.match); }
  double operator()(const Integer &i) { return std::stoi(i.match); }
};

struct Bindings {
  double operator()(const Calculation &c) {
    const auto expr_eval = std::get<1>(c);
    return Bindings{}(expr_eval);
  }

  double operator()(const boost::recursive_wrapper<Expression> &e) {
    const auto expr = e.get().value;
    const auto left = std::get<0>(expr);
    const auto right_vec = std::get<1>(expr);
    double sum = Bindings{}(left);
    for (const auto &r : right_vec) {
      const bool plus = std::holds_alternative<Plus>(std::get<0>(r));
      const double val = Bindings{}(std::get<1>(r));
      if (plus)
        sum += val;
      else
        sum -= val;
    }
    return sum;
  }

  double operator()(const Term &t) {
    const auto left = std::get<0>(t);
    const auto right_vec = std::get<1>(t);
    double sum = Bindings{}(left);
    for (const auto &r : right_vec) {
      const bool mult = std::holds_alternative<Star>(std::get<0>(r));
      const double val = Bindings{}(std::get<1>(r));
      if (mult)
        sum *= val;
      else
        sum /= val;
    }
    return sum;
  }

  double operator()(const boost::recursive_wrapper<Factor> &f) {
    const auto factor = f.get().value;
    const auto left = std::get<0>(factor);
    const auto exponent_opt = std::get<1>(factor);
    double res = Bindings{}(left);
    if (exponent_opt) {
      res = std::pow(res, Bindings{}(std::get<1>(exponent_opt.value())));
    }
    return res;
  }

  double operator()(const ExpFactor &e) { return Bindings{}(std::get<1>(e)); }

  double operator()(const Primary &p) {
    struct PrimaryVisitor {
      double operator()(const std::variant_alternative_t<0, Primary> &t) {
        return Bindings{}(std::get<1>(t));
      }
      double operator()(const std::variant_alternative_t<1, Primary> &n) {
        return std::visit(NumberVisitor{}, std::get<0>(n));
      }
    };
    return std::visit(PrimaryVisitor{}, p);
  }
};
} // namespace example::calculator::bindings