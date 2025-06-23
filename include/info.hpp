#ifndef INFO_HPP
#define INFO_HPP

#include <cpr/cpr.h>
#include <string>

float current_spot(std::string a);
float current_vol(std::string a);
float current_rfr();
float exchange_rate(std::string cur1, std::string cur2);

#endif