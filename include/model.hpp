#ifndef MODEL_HPP
#define MODEL_HPP

#include "nlohmann/json.hpp"
#include <numbers>
#include <vector>

struct Branch {
  float uProb, dProb; // probability associated with asset price movement
  float uFac, dFac;   // factor of asset price movement

  Branch(){};

  Branch(float uP, float uF, float dP, float dF) {
    uProb = uP;
    uFac = uF;
    dProb = dP;
    dFac = dF;
  }
};

class Model {
public:
  int steps;
  float dt; // time between steps (yrs)

  std::vector<float> rates,
      vols; // rate and volatility at each step

  std::vector<std::vector<Branch>>
      branches; // uses Branch objects to build a tree of probabilities and
                // factors

  /*
  s - steps
  e - expiration (of option)
  r - risk free rate
  v - volatility
  */

  Model();
  Model(int s, float e, float r, float v);
  Model(int s, float e, std::vector<float> r,
        std::vector<float> v); // r and v vary each step
  Model(int s, float e, std::vector<float> r, std::vector<float> v,
        std::vector<std::vector<Branch>> m); // customised tree

  void update_branches();

  nlohmann::json to_json();
  void from_json(nlohmann::json j);
};

#endif