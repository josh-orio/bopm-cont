#include "options.hpp"
#include <cmath>
#include <iostream>

std::string type_str(Type t) {
  if (t == Type::European) {
    return "European";

  } else if (t == Type::American) {
    return "American";

  } else if (t == Type::Asian) {
    return "Asian";

  } else if (t == Type::Undefined) {
    return "-";

  } else {
    return "?";
  }
}

std::string side_str(Side s) {
  if (s == Side::Call) {
    return "Call";

  } else if (s == Side::Put) {
    return "Put";

  } else if (s == Side::Undefined) {
    return "-";

  } else {
    return "?";
  }
}

std::string payoff_type_str(PayoffType pt) {
  if (pt == PayoffType::Floating) {
    return "Floating";

  } else if (pt == PayoffType::Fixed) {
    return "Fixed";

  } else if (pt == PayoffType::Undefined) {
    return "-";

  } else {
    return "?";
  }
}

Option::Option()
    : underlying(""), currency(""), spot(std::nanf(0)), strike(std::nanf(0)), expiration(std::nanf(0)), type(Type::Undefined), side(Side::Undefined) {}

Option::Option(Type t) : underlying(""), currency(""), spot(std::nanf(0)), strike(std::nanf(0)), expiration(std::nanf(0)), type(t), side(Side::Undefined) {}

float Option::payout(float spot) {
  if (side == Side::Call) {
    return std::max(spot - strike, 0.f);
  } else /* Put */ {
    return std::max(strike - spot, 0.f);
  }
}
