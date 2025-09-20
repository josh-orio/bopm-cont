#ifndef OPTIONS_HPP
#define OPTIONS_HPP

#include "nlohmann/json.hpp"
#include <string>
#include <vector>

enum class Type { Undefined = -1, European = 0, American = 1, Asian = 2 };
enum class Side { Undefined = -1, Call = 0, Put = 1 };
enum class PayoffType { Undefined = -1, Fixed = 0, Floating = 1 };

std::string type_str(Type t);
std::string side_str(Side s);
std::string payoff_type_str(PayoffType pt);

class Option {
public:
  std::string underlying, currency; // optional
  float spot, strike;
  float expiration; // time until expiration (yrs)
  Type type;
  Side side;

  Option(); // all members will be init'd as NaN or Undef, then filled in using interface

  float payout(float spot); // returns payout of an option given a spot price

protected:
  Option(Type t); // used by derived classes only
};
class EuropeanOption : public Option {
public:
  EuropeanOption();
};

class AmericanOption : public Option {
public:
  AmericanOption();
};

class AsianOption : public Option {
public:
  PayoffType payoff_type;

  AsianOption(); // payoff type init'd as Undef

  float payout(std::vector<float> intervals); // payouts for asian options are dependent on asset price throughout the option lifetime
};

#endif