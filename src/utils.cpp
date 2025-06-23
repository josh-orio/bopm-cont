#include "utils.hpp"

std::string numeric_filter(std::string str) {
  // removes every non numeric character (0-9, '.', ',')
  str.erase(std::remove_if(str.begin(), str.end(),
                           [](char c) {
                             return !std::isdigit(c) && c != '.' && c != ',';
                           }),
            str.end());

  return str;
}

std::string fvec_to_str(std::vector<float> f) {
  // formats vector for printing
  std::string str;
  for (auto i : f) {
    str += std::format("{:.3f}", i) + ", ";
  }
  str = std::string(str.begin(), str.end() - 2); // remove last comma
  return "[" + str + "]";
}

std::string fvecvec_to_str(std::vector<std::vector<float>> f) {
  // formats 2D vector for printing
  std::string str;
  for (auto i : f) {
    str += "[";
    for (auto ii : i) {
      str += std::format("{:.3f}", ii) + ", ";
    }
    str = std::string(str.begin(), str.end() - 2); // remove last comma

    str += "]";
  }
  str = std::string(str.begin(), str.end() - 2); // remove last comma
  return "[" + str + "]";
}

std::vector<std::string> list_dir(std::string dir) {
  // returns all files in a dir
  std::vector<std::string> fs;
  for (const auto &entry : std::filesystem::directory_iterator(dir))
    fs.push_back(entry.path());

  return fs;
}

std::string print_tree(std::vector<std::vector<float>> f) {
  // print binary trees - see 'pricing report' for usage
  std::vector<std::vector<std::string>> transposed(
      f.back().size(), std::vector<std::string>(f.size(), ""));

  // transpose so the tree is read left to right
  for (int i = 0; i < f.size(); i++) {
    for (int ii = 0; ii < f[i].size(); ii++) {
      transposed[ii * (f.back().size() / f[i].size())][i] =
          std::format("{:.3f}", f[i][ii]);
    }
  }

  std::string output;
  for (int i = 0; i < transposed.size(); i++) {
    for (int ii = 0; ii < transposed[i].size(); ii++) {
      if (transposed[i][ii] == "") {
        output += std::string(9, ' ');
      } else {
        output += transposed[i][ii] + " -> ";
      }
    }
    output = std::string(output.begin(), output.end() - 4);
    output += '\n';
  }

  return output;
}

std::string indent_linebreaks(std::string text, int indent) {
  // accounts for indentation in block of text
  // see the representation of greek trees in pricing report
  std::string orig = "\n";
  std::string replacement = '\n' + std::string(indent, ' ');

  size_t pos = 0;
  while ((pos = text.find(orig, pos)) != std::string::npos) {
    text.replace(pos, orig.length(), replacement);
    pos += replacement.length(); // move past the replacement
  }

  return text;
}

std::string str_toupper(std::string s) {
  // convert a string entirely to upper case
  std::transform(s.begin(), s.end(), s.begin(),
                 [](unsigned char c) { return std::toupper(c); });
  return s;
}