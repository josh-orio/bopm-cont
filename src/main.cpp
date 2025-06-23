#include "info.hpp"
#include "model.hpp"
#include "nlohmann/json.hpp"
#include "options.hpp"
#include "plot.hpp"
#include "rw.hpp"
#include <format>
#include <sstream>
#include <string>
#include <termui/termui.hpp>
#include <vector>

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

int main() {
  Menu m1("Binomial Option Pricing - Joshua O'Riordan",
          {"Option Pricing", "About the Project", "User Manual", "Exit"});

  while (true) {
    int t1 /* task 1 */ = m1.show();
    if (t1 == 0) {

      std::unique_ptr<Option::Base> option;
      std::unique_ptr<Model> model;

      while (true) {
        // menu has to be declared in the loop to dynamically update
        std::vector<std::string> m2opts{"Define Option", "Define Model",
                                        "Run Pricing", "Return to Main Menu"};

        // if an option is declared, display its parameters on the 'Option Pricing' interface
        if (option != nullptr) {
          m2opts[0] = "Option (select to edit) - ";
          if (option->type == Type::European) {
            m2opts[0] +=
            std::format("({}) "
              "Type: European, "
              "Spot: {:.3f} {}, "
              "Strike: {:.3f} {}, "
              "Expiration: {:.2f}, "
              "Side: {}",
              option->underlying, option->spot, option->currency, option->strike, option->currency, option->expiration, side_str(option->side)
            );
          } else if (option->type == Type::American) {
            m2opts[0] +=
            std::format("({}) "
              "Type: American, "
              "Spot: {:.3f} {}, "
              "Strike: {:.3f} {}, "
              "Expiration: {:.2f}, "
              "Side: {}",
              option->underlying, option->spot, option->currency, option->strike, option->currency, option->expiration, side_str(option->side)
            );
          } else if (option->type == Type::Asian) {
            m2opts[0] +=
            std::format("({}) "
              "Type: European, "
              "Spot: {:.3f} {}, "
              "Strike: {:.3f} {}, "
              "Expiration: {:.2f}, "
              "Side: {}, "
              "Strike Type: {}",
              option->underlying, option->spot, option->currency, option->strike, option->currency, option->expiration, side_str(option->side), payoff_type_str(option->payoff_type)
            );
          }
        }

        // if a model is declared, display its parameters on the 'Option
        // Pricing' interface
        if (model) {
          m2opts[1] = "Model (select to edit) - " +
                      std::format("{} Steps: ", model->steps);

          // display RFR and Vol at each time step
          for (int i = 0; i < (*model).steps; i++) {
            m2opts[1] += std::format("[{:.3f}% {:.3f}σ] ", model->rates[i],
                                     model->vols[i]);
          }
        }

        Menu m2("Option Pricing", m2opts);
        int t2 = m2.show();
        if (t2 == 0) /* define option */ {
          Menu m3o("Option Pricing - Define Option",
                   {"European", "American", "Asian", "Load from File", "Back"});
          while (true) {
            int t3o = m3o.show();

            if (t3o == 0)
            /* european option */ {
              std::vector<std::string> opt_fields{"Equity (optional)",
                                                  "Currency (optional)",
                                                  "Spot",
                                                  "Strike",
                                                  "Time to Expiration (yrs)",
                                                  "Side (C/P)"};

              // put some of the hardcoded option params into a vector
              std::vector<std::vector<std::string>> opt_supp(
                  opt_fields.size(), std::vector<std::string>());
              opt_supp[5] = {"Call", "Put"};

              Input m3i("Define Option Parameters", opt_fields, opt_supp);

              // preload existing option parameters
              if (option && option->type == Type::European) {
                m3i.responses = {option->underlying,
                                 option->currency,
                                 std::format("{:.3f}", option->spot),
                                 std::format("{:.3f}", option->strike),
                                 std::format("{:.3f}", option->expiration),
                                 side_str(option->side)};
              }

              std::vector<std::string> opt_input = m3i.show();

               // clean all numeric inputs, only 0-9, '.' and ',' allowed
               for (int i = 2; i <= 4; i++) {
                opt_input[i] = numeric_filter(opt_input[i]);
              }

              // convert equity and currency to uppercase
              std::string eqt = str_toupper(opt_input[0]);
              std::string cur = str_toupper(opt_input[1]);

              float exch_rate = 1;
              if (cur != "" && cur != "USD") { // if currency is not default (USD)
                if (!option || cur != option->currency) { // if option is not defined, or stored currency does not match input
                  // fetch currency exchange ratio
                  exch_rate = exchange_rate("USD", cur);

                  Info("Conversion Rate",
                       std::format("Fetching conversion rate...\n\n"
                                   "1.000 USD = {:.3f} {}",
                                   exch_rate, cur))
                      .show();
                }
              }
              else {
                cur = "USD"; // fill in default
              }

              if (opt_input[2] == "") { // if spot is empty
                if (eqt == "") { // if equity is not defined, show error
                  Info("Input Error",
                       "'Equity' field is not defined, please fill this in to "
                       "use the automatic price fetching feature.")
                      .show();
                  continue;
                }

                else { // otherwise fetch the asset price, convert currency if required
                  float spot = current_spot(eqt) * exch_rate;
                  opt_input[2] = std::to_string(spot);

                    Info ("Latest Asset Price", std::format("Fetching latest asset price...\n\n"
                "{}: {:.3f} {}", eqt, spot, cur)).show();


                if (opt_input[3] == "") {
                  opt_input[3] = "0"; // user may not want to fill in a strike immediately when using price fetching
                }
                }
              }

                if (opt_input[3] == "") {
                Info("Input Error",
                  "'Strike' field is not defined, please fill this in to continue.\n\n"
                "You can enter an absolute price, or if using the automatic price fetching feature, specify strike as a ratio of spot.")
                 .show();
             continue;
              }

              if (opt_input[4] == "") {
                Info("Input Error",
                  "'Expiration' field is not defined, please fill this in to continue.")
                 .show();
             continue;
              }


              if (opt_input[5] == "") {
                Info("Input Error",
                  "'Side' field is not defined, please fill this in to continue.")
                 .show();
             continue;
              }

              float sp = std::stof(opt_input[2]);
              float st = std::stof(opt_input[3]);
              float exp = std::stof(opt_input[4]);
              Side s = opt_input[5] == "Call" ? Side::Call : Side::Put;

              option = std::make_unique<Option::European>(
                  Option::European(eqt, cur, sp, st, exp, s));
              break;

            } else if (t3o == 1) /* american option */ {
              std::vector<std::string> opt_fields{"Equity (optional)",
                                                  "Currency (optional)",
                                                  "Spot",
                                                  "Strike",
                                                  "Time to Expiration (yrs)",
                                                  "Side (C/P)"};

              // put some of the hardcoded option params into a vector
              std::vector<std::vector<std::string>> opt_supp(
                  opt_fields.size(), std::vector<std::string>());
              opt_supp[5] = {"Call", "Put"};

              Input m3i("Define Option Parameters", opt_fields, opt_supp);

              // preload existing option parameters
              if (option && option->type == Type::American) {
                m3i.responses = {option->underlying,
                                 option->currency,
                                 std::format("{:.3f}", option->spot),
                                 std::format("{:.3f}", option->strike),
                                 std::format("{:.3f}", option->expiration),
                                 side_str(option->side)};
              }

              std::vector<std::string> opt_input = m3i.show();

              // clean all numeric inputs, only 0-9, '.' and ',' allowed
              for (int i = 2; i <= 4; i++) {
                opt_input[i] = numeric_filter(opt_input[i]);
              }

              // convert equity and currency to uppercase
              std::string eqt = str_toupper(opt_input[0]);
              std::string cur = str_toupper(opt_input[1]);

              float exch_rate = 1;
              if (cur != "" && cur != "USD") { // if currency is not default (USD)
                if (!option || cur != option->currency) { // if option is not defined, or stored currency does not match input
                  // fetch currency exchange ratio
                  exch_rate = exchange_rate("USD", cur);

                  Info("Conversion Rate",
                       std::format("Fetching conversion rate...\n\n"
                                   "1.000 USD = {:.3f} {}",
                                   exch_rate, cur))
                      .show();
                }
              }
              else {
                cur = "USD"; // fill in default
              }

              if (opt_input[3] == "") {
                Info("Input Error",
                  "'Strike' field is not defined, please fill this in to continue.\n\n"
                "You can enter an absolute price, or if using the automatic price fetching feature, specify strike as a ratio of spot.")
                 .show();
             continue;
              }

              if (opt_input[2] == "") { // if spot is empty
                if (eqt == "") { // if equity is not defined, show error
                  Info("Input Error",
                       "'Equity' field is not defined, please fill this in to "
                       "use the automatic price fetching feature.")
                      .show();
                  continue;
                }

                else { // otherwise fetch the asset price, convert currency if required
                  float spot = current_spot(eqt) * exch_rate;
                  opt_input[2] = std::to_string(spot);

                    Info ("Latest Asset Price", std::format("Fetching latest asset price...\n\n"
                "{}: {:.3f} {}", eqt, spot, cur)).show();
                }
              }

              if (opt_input[4] == "") {
                Info("Input Error",
                  "'Expiration' field is not defined, please fill this in to continue.")
                 .show();
             continue;
              }


              if (opt_input[5] == "") {
                Info("Input Error",
                  "'Side' field is not defined, please fill this in to continue.")
                 .show();
             continue;
              }

              float sp = std::stof(opt_input[2]);
              float st = std::stof(opt_input[3]);
              float exp = std::stof(opt_input[4]);
              Side s = opt_input[5] == "Call" ? Side::Call : Side::Put;

              option = std::make_unique<Option::American>(
                  Option::American(eqt, cur, sp, st, exp, s));
              break;

            } else if (t3o == 2) /* asian option */ {
              std::vector<std::string> opt_fields{"Equity (optional)",
                                                  "Currency (optional)",
                                                  "Spot",
                                                  "Strike",
                                                  "Time to Expiration (yrs)",
                                                  "Side (C/P)",
                                                  "Payoff Type"};

              // put some of the hardcoded option params into a vector
              std::vector<std::vector<std::string>> opt_supp(
                  opt_fields.size(), std::vector<std::string>());
              opt_supp[5] = {"Call", "Put"};
              opt_supp[6] = {"Fixed", "Floating"};

              Input m3i("Define Option Parameters", opt_fields, opt_supp);

              // preload existing option parameters
              if (option && option->type == Type::Asian) {
                m3i.responses = {option->underlying,
                                 option->currency,
                                 std::format("{:.3f}", option->spot),
                                 std::format("{:.3f}", option->strike),
                                 std::format("{:.3f}", option->expiration),
                                 side_str(option->side),
                                 payoff_type_str(option->payoff_type)};
              }

              std::vector<std::string> opt_input = m3i.show();

              // clean all numeric inputs, only 0-9, '.' and ',' allowed
              for (int i = 2; i <= 4; i++) {
                opt_input[i] = numeric_filter(opt_input[i]);
              }

              // convert equity and currency to uppercase
              std::string eqt = str_toupper(opt_input[0]);
              std::string cur = str_toupper(opt_input[1]);

              float exch_rate = 1;
              if (cur != "" && cur != "USD") { // if currency is not default (USD)
                if (!option || cur != option->currency) { // if option is not defined, or stored currency does not match input
                  // fetch currency exchange ratio
                  exch_rate = exchange_rate("USD", cur);

                  Info("Conversion Rate",
                       std::format("Fetching conversion rate...\n\n"
                                   "1.000 USD = {:.3f} {}",
                                   exch_rate, cur))
                      .show();
                }
              }
              else {
                cur = "USD"; // fill in default
              }

              if (opt_input[3] == "") {
                Info("Input Error",
                  "'Strike' field is not defined, please fill this in to continue.\n\n"
                "You can enter an absolute price, or if using the automatic price fetching feature, specify strike as a ratio of spot.")
                 .show();
             continue;
              }

              if (opt_input[2] == "") { // if spot is empty
                if (eqt == "") { // if equity is not defined, show error
                  Info("Input Error",
                       "'Equity' field is not defined, please fill this in to "
                       "use the automatic price fetching feature.")
                      .show();
                  continue;
                }

                else { // otherwise fetch the asset price, convert currency if required
                  float spot = current_spot(eqt) * exch_rate;
                  opt_input[2] = std::to_string(spot);

                    Info ("Latest Asset Price", std::format("Fetching latest asset price...\n\n"
                "{}: {:.3f} {}", eqt, spot, cur)).show();
                }
              }

              if (opt_input[4] == "") {
                Info("Input Error",
                  "'Expiration' field is not defined, please fill this in to continue.")
                 .show();
             continue;
              }


              if (opt_input[5] == "") {
                Info("Input Error",
                  "'Side' field is not defined, please fill this in to continue.")
                 .show();
             continue;
              }

              if (opt_input[6] == "") {
                Info("Input Error",
                  "'Payoff Type' field is not defined, please fill this in to continue.")
                 .show();
             continue;
              }

              float sp = std::stof(opt_input[2]);
              float st = std::stof(opt_input[3]);
              float exp = std::stof(opt_input[4]);
              Side s = opt_input[5] == "Call" ? Side::Call : Side::Put;
              PayoffType pt = opt_input[4] == "Fixed" ? PayoffType::Fixed
                                                      : PayoffType::Floating;

              option = std::make_unique<Option::Asian>(
                  Option::Asian(eqt, cur, sp, st, exp, s, pt));
              break;

            } else if (t3o == 3) /* load from json file */ {
              std::vector<std::string> files = list_dir("./options");

              if (files.size() == 0) {
                Info("Error",
                     "No files to load, please return to Manual Entry.")
                    .show();
                continue;
              }

              int file_index = Menu("Select an Option File", files).show();

              try {
                nlohmann::json opt_json =
                    nlohmann::json::parse(read_file(files[file_index]));

                if (opt_json["type"] == "European") {
                  option =
                      std::make_unique<Option::European>(Option::European());
                  option->from_json(opt_json);

                } else if (opt_json["type"] == "American") {
                  option =
                      std::make_unique<Option::American>(Option::American());
                  option->from_json(opt_json);

                } else if (opt_json["type"] == "Asian") {
                  option = std::make_unique<Option::Asian>(Option::Asian());
                  option->from_json(opt_json);
                }

              } catch (...) {
                Info("Error",
                     "The program was unable to read in the selected file.")
                    .show();
              }

              break;

            } else if (t3o == 4) /* back */ {
              break;
            }
          }
        } else if (t2 == 1) /* define model */ {
          Menu m3m("Option Pricing - Define Model",
                   {"Manual Entry", "Load from File", "Back"});

          while (true) {
            int t3m = m3m.show();

            if (t3m == 0) /* manual model param entry */ {
              std::vector<std::string> mod_fields{
                  "Steps", "Risk Free Rate (float)", "Volatility (float)"};

              Input m3i("Define Model Parameters", mod_fields);

              // preload existing model parameters
              if (model) {
                m3i.responses = {std::to_string(model->steps),
                                 fvec_to_str(model->rates),
                                 fvec_to_str(model->vols)};
              }

              std::vector<std::string> mod_input = m3i.show();

              if (mod_input[0] == "") {
                Info("Input Error",
                     "'Steps' field is not defined, please fill this in.")
                    .show();
                continue;
              }

              if (mod_input[1] == "") {
                float rfr = current_rfr();

                Info("Risk Free Rate",
                     std::format("Fetching risk free rate...\n\n"
                                 "RFR: {:.3f}%\n\n"
                                 "Data sourced from FRED, 3-month US Treasury "
                                 "bill rate.",
                                 rfr * 100))
                    .show();

                mod_input[1] = std::to_string(rfr);
              }

              if (mod_input[2] == "") {
                if (option != nullptr) {
                  if (option->underlying != "") {
                    float vol = current_vol(option->underlying);

                    Info("Volatility",
                         std::format("Fetching volatility...\n\n"
                                     "{} σ: {:.3f}%",
                                     option->underlying, vol * 100))
                        .show();

                    mod_input[2] = std::to_string(vol);
                  }
                }

                // if vol field still == 0, then error has occurred.
                if (mod_input[2] == "") {
                  Info(
                      "Input Error",
                      "Option is not defined or is missing the 'Equity' field.")
                      .show();
                  continue;
                }
              }

              // clean all inputs, only 0-9, '.' and ',' allowed
              for (int i = 0; i < mod_input.size(); i++) {
                mod_input[i] = numeric_filter(mod_input[i]);
              }

              int s = std::stoi(mod_input[0]);
              std::vector<float> r = {}, v = {};

              // splits input if it is comma seperated, or just instantiates the same value for each time step 
              std::string r_in = mod_input[1], v_in = mod_input[2], tmp;
              if (r_in.find(',') != std::string::npos) {
                std::stringstream ss(r_in);
                while (std::getline(ss, tmp, ',')) {
                  r.push_back(stof(tmp));
                }
              } else {
                r.resize(s, std::stod(r_in));
              }
              if (v_in.find(',') != std::string::npos) {
                std::stringstream ss(v_in);
                while (std::getline(ss, tmp, ',')) {
                  v.push_back(stof(tmp));
                }
              } else {
                v.resize(s, std::stod(v_in));
              }

              if (s != r.size() || s != v.size()) {
                Info("Input Error",
                     "Inputs for rate and/or volatility should be a single "
                     "float value or n floats where n == steps.")
                    .show();
                continue;
              }

              model = std::make_unique<Model>(Model(s, -1, r, v));

              break;
            } else if (t3m == 1) /* load from json file */ {
              std::vector<std::string> files = list_dir("./models");

              if (files.size() == 0) {
                Info("Error",
                     "No files to load, please return to Manual Entry.")
                    .show();
                continue;
              }

              int file_index = Menu("Select an Model File", files).show();

              nlohmann::json model_json =
                  nlohmann::json::parse(read_file(files[file_index]));

              model = std::make_unique<Model>(Model());
              model->from_json(model_json);

              try {

              } catch (...) {
              }

              break;

            } else if (t3m == 2) /* back */ {
              break;
            }
          }
        } else if (t2 == 2) /* run pricing */ {
          // reinitialise model so branches are calculated properly
          // and attach to the option object
          option->model = Model(model->steps, option->expiration, model->rates,
                                model->vols);

          float price = option->price();

          Menu m3(
              std::format("Option Price: {:.3f} {}", price, option->currency),
              {"View Pricing Report", "Save Option to File",
               "Save Model to File", "Show Binomial Model", "Show Delta Plot", "Show Theta Plot",
               "Back to Option Pricing"});

          while (true) {
            int t3 = m3.show();

            if (t3 == 0) /* display full pricing report */ {
              /* report structure:
              calculated option price
              option parameters
              model parameters
              greek value trees
              */

              std::string pricing_report =
                  std::format("\033[1m"
                              "{:<26} : {:.3f}\n"
                              "\033[0m",
                              "Calculated Option Price", price);

              std::string option_report = std::format(
                  "Option Parameters\n"
                  "\t{:<20} : {}\n"
                  "\t{:<20} : {}\n"
                  "\t{:<20} : {:.3f}\n"
                  "\t{:<20} : {}\n"
                  "\t{:<20} : {}\n",
                  "Option Type", type_str(option->type), "Underlying Asset",
                  option->underlying, "Strike Price", option->strike,
                  "Currency", option->currency, "Side", side_str(option->side));
              if (option->type == Type::Asian) {
                option_report += std::format(
                    "\tPayoff Type: {}", payoff_type_str(option->payoff_type));
              }

              std::string model_report = std::format(
                  "Model Parameters\n"
                  "\t{:<20} : {}\n"
                  "\t{:<20} : {}\n"
                  "\t{:<20} : {}\n",
                  "Steps", model->steps, "Rates", fvec_to_str(model->rates),
                  "Volatilities", fvec_to_str(model->vols));

              std::string delta_report =
                  indent_linebreaks(print_tree(option->delta()), 29);
                  std::string theta_report =
                      indent_linebreaks(print_tree(option->theta()), 29);
                      // std::string vega_report =
                      // indent_linebreaks(print_tree(option->vega()), 29);

              std::string greeks_report =
                  std::format("Greeks\n"
                              "\t{:<20} : {}\n"
                              "\t{:<20} : {}\n"
                              "\t{:<20} : {}\n",
                              "Delta", delta_report, "Theta",
                              theta_report, "Vega", "[]");

              Info i3("Option Pricing Report",
                      pricing_report + "\n" + option_report + "\n" +
                          model_report + "\n" + greeks_report);
              i3.show();

            } else if (t3 == 1) /* save option to file */ {
              std::vector<std::string> fn =
                  Input("Enter File Name", {"File Name"}).show();
              if (fn[0] == "") {
                Info("Input Error", "You have not entered a file name, the "
                                    "file will not be saved.")
                    .show();
                continue;
              }

              write_file("./options/" + fn[0] + ".json",
                         option->to_json().dump(2));

            } else if (t3 == 2) /* save model to file */ {
              std::vector<std::string> fn =
                  Input("Enter File Name", {"File Name"}).show();

              if (fn[0] == "") {
                Info("Input Error", "You have not entered a file name, the "
                                    "file will not be saved.")
                    .show();
                continue;
              }

              write_file("./models/" + fn[0] + ".json",
                         model->to_json().dump(2));

            } else if (t3 == 3) /* show binomial model */ {
              std::vector<std::vector<float>> factor_tree;
              factor_tree.resize(option->model.steps);

              for (int i = 0; i < option->model.steps; i++) {
                for (int ii = 0; ii < option->model.branches[i].size(); ii++) {
                  factor_tree[i].push_back(option->model.branches[i][i].uFac);
                  factor_tree[i].push_back(option->model.branches[i][i].dFac);
                }
              }

              std::vector<std::vector<float>> value_tree = {{option->spot}};
              std::vector<float> tmp;

              for (int i = 0; i < option->model.steps; i++) {
                tmp = {};

                for (int ii = 0; ii < option->model.branches[i].size(); ii++) {
                  tmp.push_back(value_tree.back()[ii] *
                                option->model.branches[i][ii].uFac);
                  tmp.push_back(value_tree.back()[ii] *
                                option->model.branches[i][ii].dFac);
                }

                value_tree.push_back(tmp);
              }

              nlohmann::json data;
              data["strike"] = option->strike;
              data["v"] = value_tree;

              std::string shader =
                  "import matplotlib.pyplot as plt\n"

                  "fig, ax = plt.subplots()\n"
                  "for i in range(len(v)-1):\n"
                  "\ttmp=[x for x in v[i] for _ in (0, 1)]\n"
                  "\tfor ii in range(len(tmp)):\n"
                  "\t\tax.plot([i,i+1], [tmp[ii], v[i+1][ii]], color='black', label='Price Path')\n"
                  "\t\tax.axhline(v[i+1][ii], color='grey', linestyle='dotted', linewidth=0.5, label='Outcomes')\n"

                  "ax.axhline(strike, color='red', linestyle='dashed', linewidth=0.8, label='Strike')\n"
                  "ax.set_title('Binomial Model')\n"
                  "ax.set_xlabel('Steps')\n"
                  "ax.set_ylabel('Asset Price')\n"

                  "handles, labels = plt.gca().get_legend_handles_labels()\n"
                  "lgnd = dict(zip(labels, handles))\n"
                  "ax.legend(lgnd.values(), lgnd.keys())\n"
                  "plt.show()\n";

              Plot(shader, data.dump()).run();
            } else if (t3 == 4) /* show delta plot */ {
              std::vector<std::vector<float>> delta_tree = option->delta();

              std::vector<std::vector<float>> value_tree = {{option->spot}};
              std::vector<float> tmp;

              for (int i = 0; i < option->model.steps; i++) {
                tmp = {};

                for (int ii = 0; ii < option->model.branches[i].size(); ii++) {
                  tmp.push_back(value_tree.back()[ii] *
                                option->model.branches[i][ii].uFac);
                  tmp.push_back(value_tree.back()[ii] *
                                option->model.branches[i][ii].dFac);
                }

                value_tree.push_back(tmp);
              }

              // print_tree(delta_tree);

              nlohmann::json data;
              data["delta_tree"] = delta_tree;
              data["value_tree"] = value_tree;

              std::string shader =
                  "import matplotlib.pyplot as plt\n"

                  "fig, ax = plt.subplots()\n"

                  "x = []\n"
                  "y = []\n"
                  "labels = []\n"
                  "for i in range(len(delta_tree)):\n"
                  "\tfor ii in range(len(delta_tree[i])):\n"
                  "\t\tx.append(i+1)\n"
                  "\t\ty.append(value_tree[i][ii])\n"
                  "\t\tlabels.append(delta_tree[i][ii])\n"

                  "scatter = ax.scatter(x, y, c=labels, cmap='coolwarm', s=300, edgecolors='black')\n"
                  "plt.colorbar(scatter, label='Label Value')\n"

                  "for i in range(len(x)):\n"
                  "\tax.text(x[i], y[i], f'{labels[i]:.2f}', color='black', ha='center', va='center', fontsize=8)\n"

                  "ax.set_xlabel('Steps')\n"
                  "ax.set_ylabel('Asset Value')\n"
                  "ax.set_title('Binomial Model - Delta Plot')\n"
                  "plt.show()\n";

              Plot(shader, data.dump()).run();
            }else if (t3 == 5) /* show theta plot */ {
              std::vector<std::vector<float>> theta_tree = option->theta();

              std::vector<std::vector<float>> value_tree = {{option->spot}};
              std::vector<float> tmp;

              for (int i = 0; i < option->model.steps; i++) {
                tmp = {};

                for (int ii = 0; ii < option->model.branches[i].size(); ii++) {
                  tmp.push_back(value_tree.back()[ii] *
                                option->model.branches[i][ii].uFac);
                  tmp.push_back(value_tree.back()[ii] *
                                option->model.branches[i][ii].dFac);
                }

                value_tree.push_back(tmp);
              }

              nlohmann::json data;
              data["theta_tree"] = theta_tree;
              data["value_tree"] = value_tree;

              std::string shader =
                  "import matplotlib.pyplot as plt\n"

                  "fig, ax = plt.subplots()\n"

                  "x = []\n"
                  "y = []\n"
                  "labels = []\n"
                  "for i in range(len(theta_tree)):\n"
                  "\tfor ii in range(len(theta_tree[i])):\n"
                  "\t\tx.append(i+1)\n"
                  "\t\ty.append(value_tree[i][ii])\n"
                  "\t\tlabels.append(theta_tree[i][ii])\n"

                  "scatter = ax.scatter(x, y, c=labels, cmap='coolwarm', s=300, edgecolors='black')\n"
                  "plt.colorbar(scatter, label='Label Value')\n"

                  "for i in range(len(x)):\n"
                  "\tax.text(x[i], y[i], f'{labels[i]:.2f}', color='black', ha='center', va='center', fontsize=8)\n"

                  "ax.set_xlabel('Steps')\n"
                  "ax.set_ylabel('Asset Value')\n"
                  "ax.set_title('Binomial Model - Delta Plot')\n"
                  "plt.show()\n";

              Plot(shader, data.dump()).run();
            }else if (t3 == -1) /* show vega plot */ {
              continue;
              std::vector<std::vector<float>> vega_tree = option->vega();

              std::vector<std::vector<float>> value_tree = {{option->spot}};
              std::vector<float> tmp;

              for (int i = 0; i < option->model.steps; i++) {
                tmp = {};

                for (int ii = 0; ii < option->model.branches[i].size(); ii++) {
                  tmp.push_back(value_tree.back()[ii] *
                                option->model.branches[i][ii].uFac);
                  tmp.push_back(value_tree.back()[ii] *
                                option->model.branches[i][ii].dFac);
                }

                value_tree.push_back(tmp);
              }

              nlohmann::json data;
              data["vega_tree"] = vega_tree;
              data["value_tree"] = value_tree;

              std::string shader =
                  "import matplotlib.pyplot as plt\n"

                  "fig, ax = plt.subplots()\n"

                  "x = []\n"
                  "y = []\n"
                  "labels = []\n"
                  "for i in range(len(vega_tree)):\n"
                  "\tfor ii in range(len(vega_tree[i])):\n"
                  "\t\tx.append(i+1)\n"
                  "\t\ty.append(value_tree[i][ii])\n"
                  "\t\tlabels.append(vega_tree[i][ii])\n"

                  "scatter = ax.scatter(x, y, c=labels, cmap='coolwarm', s=300, edgecolors='black')\n"
                  "plt.colorbar(scatter, label='Label Value')\n"

                  "for i in range(len(x)):\n"
                  "\tax.text(x[i], y[i], f'{labels[i]:.2f}', color='black', ha='center', va='center', fontsize=8)\n"

                  "ax.set_xlabel('Steps')\n"
                  "ax.set_ylabel('Asset Value')\n"
                  "ax.set_title('Binomial Model - Delta Plot')\n"
                  "plt.show()\n";

              Plot(shader, data.dump()).run();
            } else if (t3 == 6) /* back to option pricing */ {
              break;
            }
          }

        } else if (t2 == 3) /* return to main menu */ {
          break;
        }
      }
    } else if (t1 == 1) /* display about page */ {
      Info about("About", read_file("resources/about.txt"));
      about.show();
    } else if (t1 == 2) /* display user manual */ {
      Info manual("User Manual", read_file("resources/manual.txt"));
      manual.show();
    } else if (t1 == 3) /* exit */ {
      std::exit(0);
    }
  }
  return 0;
}
