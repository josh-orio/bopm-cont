#include "info.hpp"
#include "optimizations.hpp"
#include "rw.hpp"
#include <chrono>
#include <cmath>
#include <string>

// keys required for API authentication
std::string polygon_key = read_file("auth/polygon_key.txt");
std::string fred_key = read_file("auth/fred_key.txt");

long time_now() {
  auto now = std::chrono::system_clock::now();
  auto seconds = duration_cast<std::chrono::seconds>(now.time_since_epoch());

  return seconds.count();
}

float current_spot(std::string a) {
  std::string url = std::format(
      "https://api.polygon.io/v2/aggs/ticker/{}/prev?adjusted=true&apiKey={}",
      a, polygon_key);
  cpr::Response r = cpr::Get(cpr::Url{url});
  nlohmann::json data = nlohmann::json::parse(r.text);

  return data["results"][0]["c"];
}

float current_vol(std::string a) {
  long long t = time_now();
  // timestamps to fetch last 14 days of returns info
  std::string to = std::to_string(t * 1000),
              from = std::to_string((t - (14 * 86400)) * 1000);

  std::string url =
      std::format("https://api.polygon.io/v2/aggs/ticker/{}/range/1/day/{}/"
                  "{}?adjusted=true&sort=asc&limit=14&apiKey={}",
                  a, from, to, polygon_key);
  cpr::Response r = cpr::Get(cpr::Url{url});
  nlohmann::json data = nlohmann::json::parse(r.text);

  // use closing values to calculate a series of daily returns
  std::vector<float> closes = {}, returns = {};
  for (int i = 0; i < data["results"].size(); i++) {
    closes.push_back(data["results"][i]["c"]);

    if (i > 0) {
      float index = closes.size() - 1;
      float rt =
          (closes.at(index) - closes.at(index - 1)) / closes.at(index - 1);
      returns.push_back(rt);
    }
  }

  return std_dev(returns) * std::sqrt(252);
}

float current_rfr() {

  std::string url = "https://api.stlouisfed.org/fred/series/observations";
  cpr::Response r =
      cpr::Get(cpr::Url{url}, cpr::Parameters{{"series_id", "DGS3MO"},
                                              {"api_key", fred_key},
                                              {"file_type", "json"},
                                              {"sort_order", "desc"},
                                              {"limit", "1"}});
  nlohmann::json data = nlohmann::json::parse(r.text);

  std::string rfr_string = data["observations"][0]["value"];
  float rfr =
      std::stof(rfr_string) /
      100; // divide by 100 to get into decimal format (comes as % through api)

  return rfr;
}

float exchange_rate(std::string cur1, std::string cur2) {
  // 1 * cur1 == n * cur2
  // returns n

  long long t = time_now();
  std::string to = std::to_string(t * 1000),
              from = std::to_string((t - (2 * 86400)) * 1000);

  std::string url =
      std::format("https://api.polygon.io/v2/aggs/ticker/C:{}{}/range/1/day/{}/"
                  "{}?adjusted=true&sort=desc&limit=1&apiKey={}",
                  cur1, cur2, from, to, polygon_key);
  cpr::Response r = cpr::Get(cpr::Url{url});
  nlohmann::json data = nlohmann::json::parse(r.text);

  return data["results"][0]["c"];
}
