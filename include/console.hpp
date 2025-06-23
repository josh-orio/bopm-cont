#ifndef CONSOLE_HPP
#define CONSOLE_HPP

#include <iostream>
#include <string>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

std::string bold_text(std::string s);
std::string highlight_text(std::string s);

class BufferModeToggle /* switches between terminal buffering modes */ {
private:
  struct termios t;

public:
  void off(); // disables buffered input
  void on();  // enables buffered input
};

class EchoModeToggle /* switches between input echo modes*/ {
private:
  struct termios t;

public:
  void off(); // disables input echoing
  void on();  // enables input echoing
};

class CursorModeToggle /* switches cursor on and off */ {
public:
  void off(); // hides the cursor
  void on();  // enables the cursor
};

class AlternateTerminalToggle /* switches between alt and main terminal buffer
                               */
{
public:
  void enable();  // switches to alternate terminal buffer
  void disable(); // switches back to primary terminal buffer
};

class Console {
public:
  int width, height;

  BufferModeToggle bt;
  EchoModeToggle et;
  CursorModeToggle ct;
  AlternateTerminalToggle at;

  Console();

  Console(bool buffered, bool echos, bool cursor, bool alt);

  void print(std::string p);
  void print_ln();
  void print_ln(std::string p);

  void clear();
  void clear_scrollback();

  void update_size();
};

#endif