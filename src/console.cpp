#include "console.hpp"

// ANSI code for formatting console text
// treated like macros
std::string BOLD = "\033[1m";
std::string DEBOLD = "\033[0m";
std::string HIGHLIGHT =
    "\033[48;5;255m\033[38;5;16m"; // set background + set foreground;
std::string DEHIGHLIGHT = "\033[0m";
std::string RESET = "\033[0m";

std::string bold_text(std::string s) { return BOLD + s + RESET; }
std::string highlight_text(std::string s) { return HIGHLIGHT + s + RESET; }

void BufferModeToggle::off() {
  tcgetattr(STDIN_FILENO, &t);          // get the current terminal i/o flags
  t.c_lflag &= ~ICANON;                 // flip the bit related to buffering
  tcsetattr(STDIN_FILENO, TCSANOW, &t); // apply new settings
}

void BufferModeToggle::on() {
  // same as off() but inversed
  tcgetattr(STDIN_FILENO, &t);
  t.c_lflag |= ICANON;
  tcsetattr(STDIN_FILENO, TCSANOW, &t);
}

void EchoModeToggle::off() {
  tcgetattr(STDIN_FILENO, &t);
  t.c_lflag &= ~ECHO;
  tcsetattr(STDIN_FILENO, TCSANOW, &t);
}

void EchoModeToggle::on() {
  tcgetattr(STDIN_FILENO, &t);
  t.c_lflag |= ECHO;
  tcsetattr(STDIN_FILENO, TCSANOW, &t);
}

void CursorModeToggle::off() {
  std::cout << "\e[?25l"; // hides the cursor
}

void CursorModeToggle::on() {
  std::cout << "\e[?25h"; // enables the cursor
}

void AlternateTerminalToggle::enable() {
  std::cout << "\033[?1049h"; // switch to alternate terminal buffer
}

void AlternateTerminalToggle::disable() {
  std::cout << "\033[?1049l"; // switch back to the primary screen buffer
}

Console::Console() {
  struct winsize w;
  ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
  width = w.ws_col, height = w.ws_row;
}

Console::Console(bool buffered, bool echos, bool cursor, bool alt) {
  struct winsize w;
  ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
  width = w.ws_col, height = w.ws_row;

  // apply specified console settings
  (buffered == true) ? bt.on() : bt.off();
  (echos == true) ? et.on() : et.off();
  (cursor == true) ? ct.on() : ct.off();
  (alt == true) ? at.enable() : at.disable();
}

void Console::print(std::string p) { std::cout << p << " "; }

void Console::print_ln() { std::cout << std::endl; }

void Console::print_ln(std::string p) { std::cout << p << std::endl; }

void Console::clear() {
  std::cout << "\033[2J"; // clear screen
  std::cout << "\033[3J"; // clear the scrollback buffer
  std::cout << "\033[H";  // move the cursor to the top-left
}

void Console::clear_scrollback() { std::cout << "\033[3J"; }

void Console::update_size() {
  struct winsize w;
  ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
  width = w.ws_col, height = w.ws_row;
}
