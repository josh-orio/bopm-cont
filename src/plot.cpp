#include "plot.hpp"
#include "rw.hpp"

// cannot call Py_Init and Py_Fin each time Plot::run() is used
bool python::instance_running = false;

void python::shutdown() {
  Py_Finalize();
  instance_running = false;
}

void python::start() {
  Py_Initialize();
  instance_running = true;
  std::atexit(
      shutdown); // should only shutdown the py instance when main() exits
}

Plot::Plot() {}

Plot::Plot(std::string shader_code, std::string json) {
  shader = shader_code;
  data = json;
}

void Plot::run() {
  // python instance must be started if not already running
  if (!python::instance_running) {
    python::start();
  }

  // copy data dict to python
  PyObject *main_module = PyImport_AddModule("__main__");
  PyObject *main_dict = PyModule_GetDict(main_module);
  PyObject *py_string =
      PyUnicode_Decode(data.c_str(), data.size(), nullptr, nullptr);
  PyDict_SetItemString(main_dict, "data_dict", py_string);

  // run some code to unpack the data_dict into variables
  // where variable name == key
  std::string loading_shader =
      "import json\n"
      "data_dict = json.loads(data_dict)\n"
      "for key in data_dict:\n"
      "\tif type(data_dict[key]) == str:\n"
      // if string, add extra quotes
      "\t\tdata_dict[key] = \"'\" + data_dict[key] + \"'\"\n"
      "\texec(key + \" = \" + str(data_dict[key]))\n";
  PyRun_SimpleString(loading_shader.c_str());

  // run the 'real' shader
  PyRun_SimpleString(shader.c_str());

  // must clear the (python) variable dictionary
  // otherwise variables will persist between instances of Plot class
  PyDict_Clear(main_dict);
}