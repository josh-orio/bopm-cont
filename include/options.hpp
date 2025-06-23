#ifndef OPTIONS_HPP
#define OPTIONS_HPP

#include "model.hpp"
#include "nlohmann/json.hpp"
#include <string>
#include <vector>

enum class Side { Call, Put };
enum class Type { European, American, Asian };
enum class PayoffType { Fixed, Floating };

std::string side_str(Side s);
std::string type_str(Type t);
std::string payoff_type_str(PayoffType pt);

namespace Option {
class Base {
public:
  std::string underlying, currency; // optional
  float spot, strike;
  float expiration; // time until expiration
  Type type;
  Side side;
  PayoffType payoff_type;

  Model model;

  virtual float price();
  virtual float
  payout(float spot); // returns payout of an option given a spot price

  // derivatives
  virtual std::vector<std::vector<float>> delta();
  virtual std::vector<std::vector<float>> theta();
  virtual std::vector<std::vector<float>> vega();

  nlohmann::json to_json();
  void from_json(nlohmann::json j);
};

class European : public Base {
public:
  European(){};
  European(std::string u, std::string c, float sp, float sk, float exp, Side s);

  float price() override;

  std::vector<std::vector<float>> delta() override;
  std::vector<std::vector<float>> theta() override;
  // std::vector<std::vector<float>> vega() override;
};

class American : public Base {
public:
  American(){};
  American(std::string u, std::string c, float sp, float sk, float exp, Side s);

  float price() override;

  std::vector<std::vector<float>> delta() override;
  std::vector<std::vector<float>> theta() override;
  // std::vector<std::vector<float>> vega() override;
};

class Asian : public Base {
public:
  Asian(){};
  Asian(std::string u, std::string c, float sp, float sk, float exp, Side s,
        PayoffType pt);

  float payout(std::vector<float>
                   intervals); // payouts for asian options are dependent on
                               // asset price throughout the option lifetime
  float price() override;

  std::vector<std::vector<float>> delta() override;
  std::vector<std::vector<float>> theta() override;
  // std::vector<std::vector<float>> vega() override;
};

} // namespace Option

#endif