#ifndef INTERFACES_HPP
#define INTERFACES_HPP

#include "console.hpp"
#include "nlohmann/json.hpp"

class Info {
public:
  Console cons;
  std::string title, content;

  Info(std::string t, std::string c);

  void show();

private:
  int line_cursor;   // current scroll value
  int text_width;    // width of text accounting for padding
  int content_lines; // total number of lines of content
  int visible_lines; // total number of lines in view

  void display();
  bool await_input();

  void update_size();
};

class Input {
public:
  Console cons;
  std::string title;
  std::vector<std::string> fields, responses;
  std::vector<std::vector<std::string>> options;

  Input(std::string t,
        std::vector<std::string> f); // default text only input interface
  Input(std::string t, std::vector<std::string> f,
        std::vector<std::vector<std::string>>
            o); // optionally can provide a set list of choices for some/all
                // input fields

  std::vector<std::string> show();

private:
  int cursor;        // current scroll value
  bool selected;     // element selection indicator
  int visible_lines; // total number of lines in view
  int start_line;    // index value of first line which shows in view

  void display();
  bool await_input();

  void update_size();
};

class Menu {
public:
  Console cons;
  std::string title;
  std::vector<std::string> options;

  Menu(std::string t, std::vector<std::string> o);

  int show(); // returns index of selected option

private:
  int cursor;        // current scroll value
  int visible_lines; // total number of lines in view
  int start_line;    // index value of first line which shows in view

  void display();
  bool await_input();

  void update_size();
};

class Table {
public:
  Console cons;
  std::string title;
  std::vector<std::string> columns;
  std::vector<nlohmann::json> data;

  Table(std::string t, std::vector<std::string> c,
        std::vector<nlohmann::json> d);

  void show();

private:
  int cursor;        // current scroll value
  int visible_lines; // number of table rows that will fit onto the terminal
  int start_line;    // index value of first row which shows in view
  int table_width;   // width of table in characters
  int column_width;  // width of each column in characters

  void display();
  int await_input();

  void update_size();
};

#endif