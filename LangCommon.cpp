#include <sstream>

#include "LangCommon.h"

namespace lang {

std::string SourceLocation::toString() const {
  std::stringstream str;
  str << "<lineno=" << lineno << " colno=" << colno << ">";
  return str.str();
}

bool SourceLocation::operator==(const SourceLocation &Other) const {
  return lineno == Other.lineno && colno == Other.colno;
}

bool SourceLocation::operator!=(const SourceLocation &Other) const {
  return !(*this == Other);
}

}  // namespace lang
