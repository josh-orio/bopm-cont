#ifndef PLOT_HPP
#define PLOT_HPP

#include <Python.h>
#include <string>

namespace python {
extern bool instance_running;

void shutdown();
void start();
} // namespace python

class Plot {
public:
  std::string shader;
  std::string data;

  Plot();
  Plot(std::string shader_code, std::string json);

  void run();
};

#endif