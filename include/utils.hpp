#ifndef UTILS_HPP
#define UTILS_HPP

#include <vector>
#include <format>
#include <filesystem>

std::string numeric_filter(
    std::string s); // removes every non numeric character (0-9, '.', ',')

std::string fvec_to_str(std::vector<float> f); // formats vector for printing

std::string fvecvec_to_str(
    std::vector<std::vector<float>> f); // formats 2D vector for printing

// returns all files in a dir
std::vector<std::string> list_dir(std::string dir);

// print binary trees - see 'pricing report' for usage
std::string print_tree(std::vector<std::vector<float>> f);

// accounts for indentation in block of text - see 'pricing report' for usage
std::string indent_linebreaks(std::string text, int indent);

// convert a string entirely to upper case
std::string str_toupper(std::string s);

#endif