#include "rw.hpp"

std::string read_file(std::string fn) {
  // if file doesn't exists return empty string
  if (!std::filesystem::exists(fn)) {
    return "";
  }

  std::vector<char> bin((std::filesystem::file_size(fn)));

  std::ifstream i(fn, std::ifstream::binary);
  i.read(bin.data(), bin.size());
  i.close();

  return std::string(bin.begin(), bin.end());
}

void write_file(std::string fn, std::string str) {
  std::vector<char> bin(str.begin(), str.end());

  std::ofstream o(fn, std::ofstream::binary | std::ofstream::out);
  o.write((char *)bin.data(), bin.size() * sizeof(char));
  o.close();
}
