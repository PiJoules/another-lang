#ifndef LANG_COMMON_H
#define LANG_COMMON_H

#include <cassert>
#include <string>

#define LANG_UNREACHABLE(msg) assert(0 && msg)

namespace lang {

struct SourceLocation {
  // These index from zero.
  int lineno;
  int colno;

  SourceLocation() : lineno(0), colno(0) {}
  SourceLocation(int lineno, int colno) : lineno(lineno), colno(colno) {}

  std::string toString() const;

  bool operator==(const SourceLocation &Other) const;
  bool operator!=(const SourceLocation &Other) const;
};

}  // namespace lang

#endif
