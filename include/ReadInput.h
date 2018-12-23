#ifndef READINPUT_H
#define READINPUT_H

// If we are compiling on Windows, compile these functions. */
#ifdef _WIN32
#include <cstring>
inline char *readline(char *prompt) {
  fputs(prompt, stdout);
  fgets(buffer, 2048, stdin);
  char *cpy = malloc(strlen(buffer) + 1);
  strcpy(cpy, buffer);
  cpy[strlen(cpy) - 1] = '\0';
  return cpy;
}

// Stubbed out add_history function.
inline void add_history(char *unused) {}
#else
#ifndef __APPLE__
// This is not defined in OSX.
#include <editline/history.h>
#endif
#include <editline/readline.h>
#endif

#endif
