#include "interfaces.hpp"
#include <vector>

std::string div_line(int len) {
  // creates a dividing line
  std::string str;
  for (int i = 0; i < len; ++i)
    str += "─";
  return str;
}

Info::Info(std::string t, std::string c) {
  cons = Console(false, false, false, true);

  title = t;
  content = c;
}

void Info::show() {
  // configure terminal
  cons.bt.off();
  cons.et.off();
  cons.ct.off();
  cons.at.enable();

  line_cursor = 0;

  do {
    display();
  } while (await_input());

  // return terminal to defaults
  cons.bt.on();
  cons.et.on();
  cons.ct.on();
  cons.at.disable();
}

void Info::display() {
  update_size();

  std::vector<std::string> formatted = {};
  std::string copy = content;

  while (copy.length() > 0) {
    int next = 0;
    if (copy.find('\n') == std::string::npos) {
      next = std::min({(int)copy.size(), text_width});
    } else {
      next = std::min({(int)copy.size(), text_width, (int)copy.find('\n')});
    }

    formatted.push_back(std::string(copy.begin(), copy.begin() + next));

    if (copy.begin() + next + 1 < copy.end()) {
      if (copy[next] == '\n') {
        copy = std::string(copy.begin() + next + 1, copy.end());
      } else {
        copy = std::string(copy.begin() + next, copy.end());
      }
    } else {
      copy = "";
    }
  }

  content_lines = formatted.size();

  cons.clear();

  cons.print_ln();
  cons.print_ln(" " + bold_text(title));
  cons.print_ln();
  cons.print_ln(" Press ←  to return, press ↑ /↓ to scroll");
  cons.print_ln(div_line(cons.width));

  for (int i = line_cursor;
       i < std::min(line_cursor + visible_lines, (int)formatted.size()); i++) {
    cons.print_ln("  " + formatted[i]);
  }
}

bool Info::await_input() {
  char c = std::getchar();

  if (c == 27) /* ESC key */ {
    if (std::getchar() == '[') {
      switch (c = std::getchar()) {
      case 'A':
        // up arrow moves cursor up
        // decrement but dont let (cursor < 0)
        line_cursor -= (line_cursor > 0) ? 1 : 0;
        return true;

      case 'B':
        // down arrow moves cursor down
        // increment but dont let all text disappear
        line_cursor += (line_cursor < content_lines - 1) ? 1 : 0;
        return true;

      case 'D':
        // left arrow closes info page
        return false;
      }
    }
  }
  return true;
}

void Info::update_size() {
  cons.update_size();

  text_width = cons.width - 4; // 2 aside spacing
  visible_lines =
      cons.height - 6; // take six lines off for header (5) and footer (1) space
}

Input::Input(std::string t, std::vector<std::string> f) {
  cons = Console(false, false, false, true);

  title = t;
  fields = f;
  options = std::vector<std::vector<std::string>>(f.size(),
                                                  std::vector<std::string>());
  responses.resize(fields.size(), "");
}

Input::Input(std::string t, std::vector<std::string> f,
             std::vector<std::vector<std::string>> o) {
  cons = Console(false, false, false, true);

  title = t;
  fields = f;
  options = o;
  responses.resize(fields.size(), "");
}

std::vector<std::string> Input::show() {
  // configure terminal
  cons.bt.off();
  cons.et.off();
  cons.ct.off();
  cons.at.enable();

  cursor = 0; // reset so cursor is at the top
  selected = false;

  do {
    if (selected && options[cursor].size() > 0) {

      cons.bt.on();
      cons.et.on();
      cons.ct.on();
      cons.at.disable();

      Menu m("Sub-menu", options[cursor]);
      int choice = m.show();

      responses[cursor] = options[cursor][choice];

      cons.bt.off();
      cons.et.off();
      cons.ct.off();
      cons.at.enable();

      selected = false; // avoid recurrent looping
      display();        // reprint the input interface in current state

      continue; // don't need to await input so skip immediately

    } else {
      display();
    }

  } while (await_input());

  // return terminal to defaults
  cons.bt.on();
  cons.et.on();
  cons.ct.on();
  cons.at.disable();

  return responses;
}

void Input::display() {
  update_size();
  cons.clear();

  cons.print_ln();
  cons.print_ln(" " + bold_text(title));
  cons.print_ln();
  cons.print_ln(" Press ↵ to select/deselect a field, press →  to finalize");
  cons.print_ln(div_line(cons.width));

  for (int i = start_line;
       i < std::min((int)fields.size(), start_line + visible_lines); i++) {
    if (i == cursor) {
      if (selected) {
        cons.print_ln(" " + bold_text("> " + fields[i]) + ": " +
                      highlight_text(responses[i] + " "));
      } else {
        cons.print_ln(" " + bold_text("> " + fields[i]) + ": " + responses[i]);
      }
    } else {
      cons.print_ln("   " + fields[i] + ": " + responses[i]);
    }
    cons.print_ln(); // for spacing
  }
}

bool Input::await_input() {
  // this function should take the user input and process changes such as text
  // input or element selection

  // returning false signals closing the input window, returning true signals
  // continuing to display and take more user input
  char c = std::getchar();

  if (c == 10) /* enter key */ {
    // signals (de)selection of current field, just requires a bool flip
    selected = !selected;
  }
  // processes regular character inputs
  else if (selected) {
    // accept characters in range 32 - 126
    // and delete (127)
    // input sanitization is done in classes, not here

    if (32 <= c && c <= 126) {
      responses[cursor] += c; // add input character to field string
    } else if (c == 127) {    /* backspace/delete */
      if (responses[cursor].size() > 0) {
        responses[cursor] =
            std::string(responses[cursor].begin(), responses[cursor].end() - 1);
      }
    } else if (c == '\e') {
      std::getchar();
      std::getchar();
      // ignore escape key inputs and clear next input chars
    }
  }
  // processing arrow commands
  else if (c == 27) /* ESC key */ {
    if (std::getchar() == '[') {
      switch (c = std::getchar()) {
      case 'A':
        // up arrow moves cursor up
        // decrement but dont let (cursor < 0)
        cursor -= (cursor > 0) ? 1 : 0;
        start_line -= (cursor < start_line) ? 1 : 0;
        return true;

      case 'B':
        // down arrow moves cursor down
        // increment but dont let (cursor > fields.size)
        cursor += (cursor < fields.size() - 1) ? 1 : 0;
        start_line += (cursor >= start_line + visible_lines) ? 1 : 0;
        return true;

      case 'C':
        // right arrow finalizes the responses
        // won't return if a field is selected (guaranteed)
        return false;
      }
    }
  }

  return true;
}

void Input::update_size() {
  cons.update_size();

  visible_lines = (cons.height - 6) /
                  2; // 6 for header and footer spacing, div 2 for field spacing
}

Menu::Menu(std::string t, std::vector<std::string> o) {
  cons = Console(false, false, false, true);

  title = t;
  options = o;
}

int Menu::show() {
  // configure terminal
  cons.bt.off();
  cons.et.off();
  cons.ct.off();
  cons.at.enable();

  cursor = 0;
  start_line = 0;

  do {
    display();
  } while (await_input());

  // return terminal to defaults
  cons.bt.on();
  cons.et.on();
  cons.ct.on();
  cons.at.disable();

  return cursor; // returns selected option
}

void Menu::display() {
  update_size();
  cons.clear();

  cons.print_ln();
  cons.print_ln(" " + bold_text(title));
  cons.print_ln();
  cons.print_ln(" Press ↵ to select, press ↑ /↓ to scroll");
  cons.print_ln(div_line(cons.width));

  for (int i = start_line;
       i < std::min((int)options.size(), start_line + visible_lines); i++) {
    if (i == cursor) {
      cons.print_ln(" > " + bold_text(options[i]));
    } else {
      cons.print_ln("   " + options[i]);
    }
    cons.print_ln();
  }
}

bool Menu::await_input() {
  char c = std::getchar();

  if (c == 10) /* enter key */ {
    return false;
  }
  // processing arrow commands
  else if (c == 27) /* ESC key */ {
    if (std::getchar() == '[') {
      switch (c = std::getchar()) {
      case 'A':
        cursor -= (cursor > 0) ? 1 : 0; // decrement but dont let (cursor <0)

        start_line -= (cursor < start_line) ? 1 : 0;
        return true;
      case 'B':
        cursor += (cursor < options.size() - 1)
                      ? 1
                      : 0; // increment but dont let (cursor > options.size)

        start_line += (cursor >= start_line + visible_lines) ? 1 : 0;
        return true;
      }
    }
  }

  return true;
}

void Menu::update_size() {
  cons.update_size();

  visible_lines = (cons.height - 6) /
                  2; // 6 for header and footer spacing, div 2 for field spacing
}

Table::Table(std::string t, std::vector<std::string> c,
             std::vector<nlohmann::json> d) {
  cons = Console(false, false, false, true);

  title = t;
  columns = c;
  data = d;
}

void Table::show() {
  // configure terminal
  cons.bt.off();
  cons.et.off();
  cons.ct.off();
  cons.at.enable();

  cursor = 0;
  start_line = 0;

  int cont = 0;
  do {
    display();
    cont = await_input();
  } while (cont >= 0);

  // return terminal to defaults
  cons.bt.on();
  cons.et.on();
  cons.ct.on();
  cons.at.disable();
}

void Table::display() {
  update_size();
  cons.clear();

  cons.print_ln();
  cons.print_ln(" " + bold_text(title));
  cons.print_ln();
  cons.print_ln(" Press ←  to exit, press ↑ /↓ to scroll");

  std::string header = "";
  header += " |";

  for (std::string c : columns) {
    if (c.size() > column_width - 2) {
      header +=
          " " + std::string(c.begin(), c.begin() + column_width - 5) + "... ";
    } else if (c.size() <= column_width - 2) {
      header += " " + c + std::string((column_width - 2) - c.size(), ' ') + " ";
    }
    header += "|";
  }

  cons.print_ln(div_line(cons.width));
  cons.print_ln(bold_text(header));
  cons.print_ln(div_line(cons.width));

  std::string line, cell;
  for (int i = start_line;
       i < std::min((int)data.size(), start_line + visible_lines); i++) {

    line = " |";

    for (std::string c : columns) {
      cell = data[i][c].get<std::string>();

      if (cell.size() > column_width - 2) {
        line += " " +
                std::string(cell.begin(), cell.begin() + column_width - 5) +
                "... ";
      } else if (cell.size() <= column_width - 2) {
        line += " " + cell +
                std::string((column_width - 2) - cell.size(), ' ') + " ";
      }
      line += "|";
    }

    if (i == cursor) {
      cons.print_ln(bold_text(line));
    } else {
      cons.print_ln(line);
    }

    cons.print_ln();
  }
}

int Table::await_input() {
  /*
  return -1 -> exit
  return 0 -> continue
  return 1 -> select row
  */
  char c = std::getchar();
  // processing arrow commands
  if (c == 10) /* enter key */ {
    return 1;
  } else if (c == 27) /* ESC key */ {
    if (std::getchar() == '[') {
      switch (c = std::getchar()) {
      case 'A':
        // decrement but dont let (cursor < 0)
        cursor -= (cursor > 0) ? 1 : 0;
        start_line -= (cursor < start_line) ? 1 : 0;
        return 0;

      case 'B':
        // increment but dont let (cursor > options.size)
        cursor += (cursor < data.size() - 1) ? 1 : 0;

        start_line += (cursor >= start_line + visible_lines) ? 1 : 0;
        return 0;

      case 'D':
        // left arrow closes info page
        return -1;
      }
    }
  }

  return 0;
}

void Table::update_size() {
  cons.update_size();

  visible_lines = (cons.height - 8) / 2; // take 8 for header, heading & footer
                                         // spacing, div 2 for element spacing
  table_width = cons.width - 2;          // take 2 for horizontal spacing
  column_width = (table_width - (columns.size() + 1)) / columns.size();
}
