/**
 * c++ -std=c++11 -Wall -ledit prompt.cpp
 */

#include <iostream>

#include "readinput.h"

int main(int argc, char** argv) {
  std::cout << "Lispy Version 0.0.0.0.1" << std::endl;
  std::cout << "Press Ctrl+c to Exit" << std::endl;

  while (true) {
    char *input_cstr = readline("lispy> ");
    std::string input(input_cstr);
    add_history(input_cstr);
    free(input_cstr);

    std::cout << "Input: " << input << std::endl;
  }

  return 0;
}
