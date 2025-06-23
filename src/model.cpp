#include "model.hpp"
#include <cstddef>
#include <iostream>

Model::Model() {}

Model::Model(int s, float e, float r, float v) {
  steps = s;

  // create arrays for rate and volatility values
  rates.resize(steps, r);
  vols.resize(steps, v);

  if (e == -1) {
    // if e == -1, that instance of model is a template not assigned to an
    // option
    dt = -1;
  } else {
    dt = e / steps;

    update_branches();
  }
}

Model::Model(int s, float e, std::vector<float> r, std::vector<float> v) {
  steps = s;

  // create arrays for rate and volatility values
  rates = r;
  vols = v;

  if (e == -1) {
    // if e == -1, that instance of model is a template not assigned to an
    // option
    dt = -1;
  } else {
    dt = e / steps;

    update_branches();
  }
}

Model::Model(int s, float e, std::vector<float> r, std::vector<float> v,
             std::vector<std::vector<Branch>> m) {
  steps = s;

  // create arrays for rate and volatility values
  rates = r;
  vols = v;

  branches = m;

  if (e == -1) {
    // if e == -1, that instance of model is a template not assigned to an
    // option
    dt = -1;
  } else {
    dt = e / steps;
  }
}

nlohmann::json Model::to_json() {
  nlohmann::json data;
  data["steps"] = steps;
  data["dt"] = dt;
  data["rates"] = rates;
  data["volatilities"] = vols;
  return data;
}

void Model::from_json(nlohmann::json data) {
  steps = data["steps"];
  dt = data["dt"];
  rates = data["rates"].get<std::vector<float>>();
  vols = data["volatilities"].get<std::vector<float>>();

  update_branches();
}

void Model::update_branches() {
  float u, d, p;

  branches.resize(steps);
  for (int i = 0; i < branches.size(); i++) {
    u = std::pow(std::numbers::e, vols[i] * std::sqrt(dt));
    d = 1 / u;

    p = (std::pow(std::numbers::e, rates[i] * dt) - d) / (u - d);

    branches[i].resize(std::pow(2, i), Branch(p, u, 1 - p, d));
  }
}
