#include "options.hpp"
#include "model.hpp"
#include <algorithm>
#include <numbers>
#include <string>

std::string side_str(Side s) {
  switch (s) {
  case Side::Call:
    return "Call";
  case Side::Put:
    return "Put";
  }
}
std::string type_str(Type t) {
  switch (t) {
  case Type::European:
    return "European";
  case Type::American:
    return "American";
  case Type::Asian:
    return "Asian";
  }
}
std::string payoff_type_str(PayoffType pt) {
  switch (pt) {
  case PayoffType::Fixed:
    return "Fixed";
  case PayoffType::Floating:
    return "Floating";
  }
}

float Option::Base::price() { return -1; }

float Option::Base::payout(float s /* momentary spot price */) {
  if (side == Side::Call) {
    return std::max(s - strike, 0.f);
  } else /* side == Put */ {
    return std::max(strike - s, 0.f);
  }
}

// base class can just return placeholders for derivatives
std::vector<std::vector<float>> Option::Base::delta() { return {{-1}}; }
std::vector<std::vector<float>> Option::Base::theta() { return {{-1}}; }
std::vector<std::vector<float>> Option::Base::vega() { return {{-1}}; }

nlohmann::json Option::Base::to_json() {
  nlohmann::json data;
  data["underlying"] = underlying;
  data["currency"] = currency;
  data["spot"] = spot;
  data["strike"] = strike;
  data["expiration"] = expiration;
  data["type"] = type_str(type);
  data["side"] = side_str(side);
  if (type == Type::Asian) {
    data["payoff_type"] = payoff_type_str(payoff_type);
  }

  return data;
}

void Option::Base::from_json(nlohmann::json data) {
  underlying = data["underlying"];
  currency = data["currency"];
  spot = data["spot"];
  strike = data["strike"];
  expiration = data["expiration"];

  std::string tmp = data["type"];
  if (tmp == "European") {
    type = Type::European;
  } else if (tmp == "American") {
    type = Type::American;
  } else if (tmp == "Asian") {
    type = Type::Asian;
  }

  tmp = data["side"];
  if (tmp == "Call") {
    side = Side::Call;
  } else if (tmp == "Put") {
    side = Side::Put;
  }

  if (type == Type::Asian) {
    tmp = data["payoff_type"];
    if (tmp == "Fixed") {
      payoff_type = PayoffType::Fixed;
    } else if (tmp == "Floating") {
      payoff_type = PayoffType::Floating;
    }
  }
}

Option::European::European(std::string u, std::string c, float sp, float sk,
                           float exp, Side s) {
  underlying = u;
  currency = c;
  spot = sp;
  strike = sk;
  expiration = exp;
  type = Type::European;
  side = s;
}

float Option::European::price() {
  std::vector<std::vector<float>> stock_tree{
      {spot}}; // holds possible values for the underlying asset at each time
               // step
  std::vector<float> tmp;
  for (int i = 0; i < model.steps; i++) {
    tmp = {};
    for (int ii = 0; ii < model.branches[i].size(); ii++) {
      tmp.push_back(stock_tree.back()[ii] * model.branches[i][ii].uFac);

      tmp.push_back(stock_tree.back()[ii] * model.branches[i][ii].dFac);
    }
    stock_tree.push_back(tmp);
  }

  std::vector<std::vector<float>> payout_tree =
      stock_tree; // holds the possible payout values of the option at each time
                  // step
  for (int i = 0; i < payout_tree.size(); i++) {
    for (int ii = 0; ii < payout_tree[i].size(); ii++) {
      payout_tree[i][ii] = payout(payout_tree[i][ii]);
    }
  }

  std::vector<std::vector<float>> option_tree =
      payout_tree; // holds the value of the option at each time step
  for (int i = option_tree.size() - 2; i >= 0; i--) {
    for (int ii = 0; ii < option_tree[i].size(); ii++) {
      option_tree[i][ii] =
          std::pow(std::numbers::e, -1 * model.rates[i] * model.dt) *
          ((model.branches[i][ii].uProb * option_tree[i + 1][(2 * ii)]) +
           (model.branches[i][ii].dProb * option_tree[i + 1][(2 * ii) + 1]));
    }
  }
  return option_tree[0][0];
}

std::vector<std::vector<float>> Option::European::delta() {
  // delta measures an options sensitivity to changes in the price of the
  // underlying asset

  /*
  caculated as:

  D = (Vu - Vd) / (Su - Sd)

  D = delta
  Vu = value of the option after an upward movement
  Vd = value of the option after a downward movement
  Su = price of asset after an upward movement
  Sd = price of asset after a downward movement

  */
  std::vector<std::vector<float>> stock_tree{
      {spot}}; // holds possible values for the underlying asset at each time
               // step
  std::vector<float> tmp;
  for (int i = 0; i < model.steps; i++) {
    tmp = {};
    for (int ii = 0; ii < model.branches[i].size(); ii++) {
      tmp.push_back(stock_tree.back()[ii] * model.branches[i][ii].uFac);

      tmp.push_back(stock_tree.back()[ii] * model.branches[i][ii].dFac);
    }
    stock_tree.push_back(tmp);
  }

  std::vector<std::vector<float>> payout_tree =
      stock_tree; // holds the possible payout values of the option at each time
                  // step
  for (int i = 0; i < payout_tree.size(); i++) {
    for (int ii = 0; ii < payout_tree[i].size(); ii++) {
      payout_tree[i][ii] = payout(payout_tree[i][ii]);
    }
  }

  std::vector<std::vector<float>> option_tree =
      payout_tree; // holds the value of the option at each time step
  for (int i = option_tree.size() - 2; i >= 0; i--) {
    for (int ii = 0; ii < option_tree[i].size(); ii++) {
      option_tree[i][ii] =
          std::pow(std::numbers::e, -1 * model.rates[i] * model.dt) *
          ((model.branches[i][ii].uProb * option_tree[i + 1][(2 * ii)]) +
           (model.branches[i][ii].dProb * option_tree[i + 1][(2 * ii) + 1]));
    }
  }

  std::vector<std::vector<float>> delta_tree(
      model.steps, std::vector<float>{}); // holds the possible range of delta
                                          // values for each time step
  float delta, Vu, Vd, Su, Sd;
  for (int i = 1; i < stock_tree.size(); i++) {
    for (int ii = 0; ii < stock_tree[i].size(); ii += 2) {
      Su = stock_tree[i][ii];
      Sd = stock_tree[i][ii + 1];

      Vu = option_tree[i][ii];
      Vd = option_tree[i][ii + 1];

      delta = (Vu - Vd) / (Su - Sd);

      delta_tree[i - 1].push_back(delta);
    }
  }

  return delta_tree;
}

std::vector<std::vector<float>> Option::European::theta() {
  // theta represents the rate of option value decay over time

  /*
  caculated as:

  T = (Vf - Vn) / 2dt

  T = theta
  Vf = future option value - calculated by discount an upward and downward
  movement Vn = current option calue t = time delta between two nodes

  */
  std::vector<std::vector<float>> stock_tree{
      {spot}}; // holds possible values for the underlying asset at each time
               // step
  std::vector<float> tmp;
  for (int i = 0; i < model.steps; i++) {
    tmp = {};
    for (int ii = 0; ii < model.branches[i].size(); ii++) {
      tmp.push_back(stock_tree.back()[ii] * model.branches[i][ii].uFac);

      tmp.push_back(stock_tree.back()[ii] * model.branches[i][ii].dFac);
    }
    stock_tree.push_back(tmp);
  }

  std::vector<std::vector<float>> payout_tree =
      stock_tree; // holds the possible payout values of the option at each time
                  // step
  for (int i = 0; i < payout_tree.size(); i++) {
    for (int ii = 0; ii < payout_tree[i].size(); ii++) {
      payout_tree[i][ii] = payout(payout_tree[i][ii]);
    }
  }

  std::vector<std::vector<float>> option_tree =
      payout_tree; // holds the value of the option at each time step
  for (int i = option_tree.size() - 2; i >= 0; i--) {
    for (int ii = 0; ii < option_tree[i].size(); ii++) {
      option_tree[i][ii] =
          std::pow(std::numbers::e, -1 * model.rates[i] * model.dt) *
          ((model.branches[i][ii].uProb * option_tree[i + 1][(2 * ii)]) +
           (model.branches[i][ii].dProb * option_tree[i + 1][(2 * ii) + 1]));
    }
  }

  std::vector<std::vector<float>> theta_tree(
      model.steps, std::vector<float>{}); // holds the possible range of theta
                                          // values for each time step
  for (int i = 0; i < option_tree.size() - 1; i++) {
    float theta, Vfuture, Vnow;
    for (int ii = 0; ii < option_tree[i].size(); ii++) {
      Vnow = option_tree[i][ii];
      Vfuture =
          std::pow(std::numbers::e, -1 * model.rates[i] * model.dt) *
          ((model.branches[i][ii].uProb * option_tree[i + 1][(2 * ii)]) +
           (model.branches[i][ii].dProb * option_tree[i + 1][(2 * ii) + 1]));

      theta = (Vfuture - Vnow) / model.dt;

      theta_tree[i].push_back(theta);
    }
  }

  return theta_tree;
}

Option::American::American(std::string u, std::string c, float sp, float sk,
                           float exp, Side s) {
  underlying = u;
  currency = c;
  spot = sp;
  strike = sk;
  expiration = exp;
  type = Type::American;
  side = s;
}

float Option::American::price() {
  // this implementation accounts for the early exercise feature of american
  // american options should exit if exercise value > hold value at any time
  // step

  std::vector<std::vector<float>> stock_tree{
      {spot}}; // holds possible values for the underlying asset at each time
               // step
  std::vector<float> tmp;
  for (int i = 0; i < model.steps; i++) {
    tmp = {};
    for (int ii = 0; ii < model.branches[i].size(); ii++) {
      tmp.push_back(stock_tree.back()[ii] * model.branches[i][ii].uFac);

      tmp.push_back(stock_tree.back()[ii] * model.branches[i][ii].dFac);
    }
    stock_tree.push_back(tmp);
  }

  std::vector<std::vector<float>> payout_tree =
      stock_tree; // holds the possible payout values of the option at each time
                  // step
  for (int i = 0; i < payout_tree.size(); i++) {
    for (int ii = 0; ii < payout_tree[i].size(); ii++) {
      payout_tree[i][ii] = payout(payout_tree[i][ii]);
    }
  }

  std::vector<std::vector<float>> option_tree =
      payout_tree; // holds the value of the option at each time step
  for (int i = option_tree.size() - 2; i >= 0; i--) {
    for (int ii = 0; ii < option_tree[i].size(); ii++) {
      float intrinsic_value =
          std::pow(std::numbers::e, -1 * model.rates[i] * model.dt) *
          ((model.branches[i][ii].uProb * option_tree[i + 1][(2 * ii)]) +
           (model.branches[i][ii].dProb * option_tree[i + 1][(2 * ii) + 1]));

      float payout_value = payout_tree[i][ii];

      option_tree[i][ii] = std::max(intrinsic_value, payout_value);
    }
  }
  return option_tree[0][0];
}

std::vector<std::vector<float>> Option::American::delta() {
  std::vector<std::vector<float>> stock_tree{
      {spot}}; // holds possible values for the underlying asset at each time
               // step
  std::vector<float> tmp;
  for (int i = 0; i < model.steps; i++) {
    tmp = {};
    for (int ii = 0; ii < model.branches[i].size(); ii++) {
      tmp.push_back(stock_tree.back()[ii] * model.branches[i][ii].uFac);

      tmp.push_back(stock_tree.back()[ii] * model.branches[i][ii].dFac);
    }
    stock_tree.push_back(tmp);
  }

  std::vector<std::vector<float>> payout_tree =
      stock_tree; // holds the possible payout values of the option at each time
                  // step
  for (int i = 0; i < payout_tree.size(); i++) {
    for (int ii = 0; ii < payout_tree[i].size(); ii++) {
      payout_tree[i][ii] = payout(payout_tree[i][ii]);
    }
  }

  std::vector<std::vector<float>> option_tree =
      payout_tree; // holds the value of the option at each time step
  for (int i = option_tree.size() - 2; i >= 0; i--) {
    for (int ii = 0; ii < option_tree[i].size(); ii++) {
      float intrinsic_value =
          std::pow(std::numbers::e, -1 * model.rates[i] * model.dt) *
          ((model.branches[i][ii].uProb * option_tree[i + 1][(2 * ii)]) +
           (model.branches[i][ii].dProb * option_tree[i + 1][(2 * ii) + 1]));

      float payout_value = payout_tree[i][ii];

      option_tree[i][ii] = std::max(intrinsic_value, payout_value);
    }
  }

  std::vector<std::vector<float>> delta_tree(
      model.steps, std::vector<float>{}); // holds the possible range of delta
                                          // values for each time step
  float delta, Vu, Vd, Su, Sd;
  for (int i = 1; i < stock_tree.size(); i++) {
    for (int ii = 0; ii < stock_tree[i].size(); ii += 2) {
      Su = stock_tree[i][ii];
      Sd = stock_tree[i][ii + 1];

      Vu = option_tree[i][ii];
      Vd = option_tree[i][ii + 1];

      delta = (Vu - Vd) / (Su - Sd);

      delta_tree[i - 1].push_back(delta);
    }
  }
  return delta_tree;
}

std::vector<std::vector<float>> Option::American::theta() {
  std::vector<std::vector<float>> stock_tree{
      {spot}}; // holds possible values for the underlying asset at each time
               // step
  std::vector<float> tmp;
  for (int i = 0; i < model.steps; i++) {
    tmp = {};
    for (int ii = 0; ii < model.branches[i].size(); ii++) {
      tmp.push_back(stock_tree.back()[ii] * model.branches[i][ii].uFac);

      tmp.push_back(stock_tree.back()[ii] * model.branches[i][ii].dFac);
    }
    stock_tree.push_back(tmp);
  }

  std::vector<std::vector<float>> payout_tree =
      stock_tree; // holds the possible payout values of the option at each time
                  // step
  for (int i = 0; i < payout_tree.size(); i++) {
    for (int ii = 0; ii < payout_tree[i].size(); ii++) {
      payout_tree[i][ii] = payout(payout_tree[i][ii]);
    }
  }

  std::vector<std::vector<float>> option_tree =
      payout_tree; // holds the value of the option at each time step
  for (int i = option_tree.size() - 2; i >= 0; i--) {
    for (int ii = 0; ii < option_tree[i].size(); ii++) {
      float intrinsic_value =
          std::pow(std::numbers::e, -1 * model.rates[i] * model.dt) *
          ((model.branches[i][ii].uProb * option_tree[i + 1][(2 * ii)]) +
           (model.branches[i][ii].dProb * option_tree[i + 1][(2 * ii) + 1]));

      float payout_value = payout_tree[i][ii];

      option_tree[i][ii] = std::max(intrinsic_value, payout_value);
    }
  }

  std::vector<std::vector<float>> theta_tree(
      model.steps, std::vector<float>{}); // holds the possible range of theta
                                          // values for each time step
  for (int i = 0; i < option_tree.size() - 1; i++) {
    float theta, Vfuture, Vnow;
    for (int ii = 0; ii < option_tree[i].size(); ii++) {
      Vnow = option_tree[i][ii];

      Vfuture =
          std::pow(std::numbers::e, -1 * model.rates[i] * model.dt) *
          ((model.branches[i][ii].uProb * option_tree[i + 1][(2 * ii)]) +
           (model.branches[i][ii].dProb * option_tree[i + 1][(2 * ii) + 1]));

      theta = (Vfuture - Vnow) / model.dt;

      theta_tree[i].push_back(theta);
    }
  }

  return theta_tree;
}

Option::Asian::Asian(std::string u, std::string c, float sp, float sk,
                     float exp, Side s, PayoffType pt) {
  underlying = u;
  currency = c;
  spot = sp;
  strike = sk;
  expiration = exp;
  type = Type::Asian;
  side = s;
  payoff_type = pt;
}

float Option::Asian::payout(std::vector<float> intervals) {
  float mean = 0;
  for (float i : intervals) {
    mean += i;
  }
  mean /= intervals.size();

  if (payoff_type == PayoffType::Fixed) {
    if (side == Side::Call) {
      return std::max(mean - strike, 0.f);
    } else /* side == Put */ {
      return std::max(strike - mean, 0.f);
    }
  } else /* Floating */ {
    float m = intervals.back(); // price at maturity (assumes last element of
                                // intervals is at time of maturity)

    if (side == Side::Call) {
      return std::max(m - mean, 0.f);
    } else /* side == Put */ {
      return std::max(mean - m, 0.f);
    }
  }
}

float Option::Asian::price() {
  // this implementation accounts for differences in how asian option payouts
  // are calculated
  std::vector<std::vector<float>> stock_tree{
      {spot}}; // holds possible values for the underlying asset at each time
               // step
  std::vector<float> tmp;
  for (int i = 0; i < model.steps; i++) {
    tmp = {};
    for (int ii = 0; ii < model.branches[i].size(); ii++) {
      tmp.push_back(stock_tree.back()[ii] * model.branches[i][ii].uFac);

      tmp.push_back(stock_tree.back()[ii] * model.branches[i][ii].dFac);
    }
    stock_tree.push_back(tmp);
  }

  std::vector<std::vector<float>> payout_tree =
      stock_tree; // holds the possible payout values of the option at each time
                  // step

  // initialize the search stack with the root node
  std::vector<std::vector<float>> outcome_paths = {{payout_tree[0][0]}};
  std::vector<int> steps = {0};
  std::vector<int> nodes = {0};

  while (!outcome_paths.empty()) {
    // pop from the stack
    std::vector<float> path = outcome_paths.back();
    outcome_paths.pop_back();
    int i = steps.back();
    steps.pop_back();
    int ii = nodes.back();
    nodes.pop_back();

    payout_tree[i][ii] = payout(path);

    // if there is a next step, push children
    if (i + 1 < payout_tree.size()) {
      int next_step = i + 1;
      int upward = ii * 2;
      int downward = ii * 2 + 1;

      // push downward node first
      std::vector<float> newPath = path;
      newPath.push_back(payout_tree[next_step][downward]);
      outcome_paths.push_back(newPath);
      steps.push_back(next_step);
      nodes.push_back(downward);

      // now add upward node, so it will be processed first
      newPath = path;
      newPath.push_back(payout_tree[next_step][upward]);
      outcome_paths.push_back(newPath);
      steps.push_back(next_step);
      nodes.push_back(upward);
    }
  }

  std::vector<std::vector<float>> option_tree =
      payout_tree; // holds the value of the option at each time step
  for (int i = option_tree.size() - 2; i >= 0; i--) {
    for (int ii = 0; ii < option_tree[i].size(); ii++) {
      option_tree[i][ii] =
          std::pow(std::numbers::e, -1 * model.rates[i] * model.dt) *
          ((model.branches[i][ii].uProb * option_tree[i + 1][(2 * ii)]) +
           (model.branches[i][ii].dProb * option_tree[i + 1][(2 * ii) + 1]));
    }
  }

  return option_tree[0][0];
}

std::vector<std::vector<float>> Option::Asian::delta() {
  std::vector<std::vector<float>> stock_tree{
      {spot}}; // holds possible values for the underlying asset at each time
               // step
  std::vector<float> tmp;
  for (int i = 0; i < model.steps; i++) {
    tmp = {};
    for (int ii = 0; ii < model.branches[i].size(); ii++) {
      tmp.push_back(stock_tree.back()[ii] * model.branches[i][ii].uFac);

      tmp.push_back(stock_tree.back()[ii] * model.branches[i][ii].dFac);
    }
    stock_tree.push_back(tmp);
  }

  std::vector<std::vector<float>> payout_tree =
      stock_tree; // holds the possible payout values of the option at each time
                  // step

  // initialize the search stack with the root node
  std::vector<std::vector<float>> outcome_paths = {{payout_tree[0][0]}};
  std::vector<int> steps = {0};
  std::vector<int> nodes = {0};

  while (!outcome_paths.empty()) {
    // pop from the stack
    std::vector<float> path = outcome_paths.back();
    outcome_paths.pop_back();
    int i = steps.back();
    steps.pop_back();
    int ii = nodes.back();
    nodes.pop_back();

    payout_tree[i][ii] = payout(path);

    // if there is a next step, push children
    if (i + 1 < payout_tree.size()) {
      int next_step = i + 1;
      int upward = ii * 2;
      int downward = ii * 2 + 1;

      // push downward node first
      std::vector<float> newPath = path;
      newPath.push_back(payout_tree[next_step][downward]);
      outcome_paths.push_back(newPath);
      steps.push_back(next_step);
      nodes.push_back(downward);

      // now add upward node, so it will be processed first
      newPath = path;
      newPath.push_back(payout_tree[next_step][upward]);
      outcome_paths.push_back(newPath);
      steps.push_back(next_step);
      nodes.push_back(upward);
    }
  }

  std::vector<std::vector<float>> option_tree =
      payout_tree; // holds the value of the option at each time step
  for (int i = option_tree.size() - 2; i >= 0; i--) {
    for (int ii = 0; ii < option_tree[i].size(); ii++) {
      option_tree[i][ii] =
          std::pow(std::numbers::e, -1 * model.rates[i] * model.dt) *
          ((model.branches[i][ii].uProb * option_tree[i + 1][(2 * ii)]) +
           (model.branches[i][ii].dProb * option_tree[i + 1][(2 * ii) + 1]));
    }
  }

  std::vector<std::vector<float>> delta_tree(
      model.steps, std::vector<float>{}); // holds the possible range of delta
                                          // values for each time step
  float delta, Vu, Vd, Su, Sd;
  for (int i = 1; i < stock_tree.size(); i++) {
    for (int ii = 0; ii < stock_tree[i].size(); ii += 2) {
      Su = stock_tree[i][ii];
      Sd = stock_tree[i][ii + 1];

      Vu = option_tree[i][ii];
      Vd = option_tree[i][ii + 1];

      delta = (Vu - Vd) / (Su - Sd);

      delta_tree[i - 1].push_back(delta);
    }
  }

  return delta_tree;
}

std::vector<std::vector<float>> Option::Asian::theta() {
  std::vector<std::vector<float>> stock_tree{
      {spot}}; // holds possible values for the underlying asset at each time
               // step
  std::vector<float> tmp;
  for (int i = 0; i < model.steps; i++) {
    tmp = {};
    for (int ii = 0; ii < model.branches[i].size(); ii++) {
      tmp.push_back(stock_tree.back()[ii] * model.branches[i][ii].uFac);

      tmp.push_back(stock_tree.back()[ii] * model.branches[i][ii].dFac);
    }
    stock_tree.push_back(tmp);
  }

  std::vector<std::vector<float>> payout_tree =
      stock_tree; // holds the possible payout values of the option at each time
                  // step

  // initialize the search stack with the root node
  std::vector<std::vector<float>> outcome_paths = {{payout_tree[0][0]}};
  std::vector<int> steps = {0};
  std::vector<int> nodes = {0};

  while (!outcome_paths.empty()) {
    // pop from the stack
    std::vector<float> path = outcome_paths.back();
    outcome_paths.pop_back();
    int i = steps.back();
    steps.pop_back();
    int ii = nodes.back();
    nodes.pop_back();

    payout_tree[i][ii] = payout(path);

    // if there is a next step, push children
    if (i + 1 < payout_tree.size()) {
      int next_step = i + 1;
      int upward = ii * 2;
      int downward = ii * 2 + 1;

      // push downward node first
      std::vector<float> newPath = path;
      newPath.push_back(payout_tree[next_step][downward]);
      outcome_paths.push_back(newPath);
      steps.push_back(next_step);
      nodes.push_back(downward);

      // now add upward node, so it will be processed first
      newPath = path;
      newPath.push_back(payout_tree[next_step][upward]);
      outcome_paths.push_back(newPath);
      steps.push_back(next_step);
      nodes.push_back(upward);
    }
  }

  std::vector<std::vector<float>> option_tree =
      payout_tree; // holds the value of the option at each time step
  for (int i = option_tree.size() - 2; i >= 0; i--) {
    for (int ii = 0; ii < option_tree[i].size(); ii++) {
      option_tree[i][ii] =
          std::pow(std::numbers::e, -1 * model.rates[i] * model.dt) *
          ((model.branches[i][ii].uProb * option_tree[i + 1][(2 * ii)]) +
           (model.branches[i][ii].dProb * option_tree[i + 1][(2 * ii) + 1]));
    }
  }

  std::vector<std::vector<float>> theta_tree(
      model.steps, std::vector<float>{}); // holds the possible range of theta
                                          // values for each time step
  for (int i = 0; i < option_tree.size() - 1; i++) {
    float theta, Vfuture, Vnow;
    for (int ii = 0; ii < option_tree[i].size(); ii++) {
      Vnow = option_tree[i][ii];
      Vfuture =
          std::pow(std::numbers::e, -1 * model.rates[i] * model.dt) *
          ((model.branches[i][ii].uProb * option_tree[i + 1][(2 * ii)]) +
           (model.branches[i][ii].dProb * option_tree[i + 1][(2 * ii) + 1]));

      theta = (Vfuture - Vnow) / model.dt;

      theta_tree[i].push_back(theta);
    }
  }

  return theta_tree;
}
