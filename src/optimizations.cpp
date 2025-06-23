#include "optimizations.hpp"
#include <cmath>

float mean(std::vector<float> f) {
#ifdef USE_OPENBLAS
  std::vector<float> ones(f.size(), 1.0f);

  double sum = cblas_sdot(f.size(), f.data(), 1, ones.data(), 1);

  return sum / f.size();
#else
  float sum = std::accumulate(data.begin(), data.end(), 0.0f);

  return sum / f.size();
#endif
}

float std_dev(std::vector<float> f) {
  float m = mean(f);

#ifdef USE_OPENBLAS
  cblas_saxpy(f.size(), -1 * m, std::vector<float>(f.size(), 1.0f).data(), 0,
              f.data(), 1);

  float variance =
      cblas_sdot(f.size(), f.data(), 1, f.data(), 1) / (f.size() - 1);

  return std::sqrt(variance); // Sample variance
#else
  // calculate variance
  float variance = 0.0f;
  for (float x : f) {
    variance += std::pow(x - m, 2);
  }
  variance /= f.size() - 1; // use data.size() - 1 for sample std dev

  // standard deviation is sqrt of variance
  return std::sqrt(variance);
#endif
}