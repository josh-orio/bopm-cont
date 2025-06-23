#ifndef RW_HPP
#define RW_HPP

#include "nlohmann/json.hpp"
#include <fstream>

std::string read_file(std::string fn);
void write_file(std::string fn, std::string str);

#endif