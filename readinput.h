#ifndef READINPUT_H_
#define READINPUT_H_

// If we are compiling on Windows, compile these functions. */
#ifdef _WIN32
#include <cstring>
inline char *readline(char *prompt) {
  fputs(prompt, stdout);
  fgets(buffer, 2048, stdin);
  char* cpy = malloc(strlen(buffer)+1);
  strcpy(cpy, buffer);
  cpy[strlen(cpy)-1] = '\0';
  return cpy;
}

// Stubbed out add_history function.
inline void add_history(char *unused){}
#else
#include <editline/readline.h>
#include <editline/history.h>
#endif

#endif
