#ifndef OPTIMIZATIONS_HPP
#define OPTIMIZATIONS_HPP

#include <vector>

#if defined(USE_CUBLAS)
#include <cublas_v2.h>
#include <cuda_runtime.h>
#elif defined(USE_OPENBLAS)
#include <cblas.h>
#endif

float mean(std::vector<float> f);
float std_dev(std::vector<float> f);

#endif