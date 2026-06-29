#include "i3space/locale.hpp"

#include <cstdlib>
#include <cstring>
#include <iostream>

namespace i3space {

Locale detect_locale() {
  const char* vars[] = {"LC_MESSAGES", "LC_ALL", "LANG", nullptr};
  for (int i = 0; vars[i]; ++i) {
    if (const char* v = std::getenv(vars[i])) {
      if (std::strncmp(v, "es", 2) == 0) {
        return Locale::Es;
      }
    }
  }
  return Locale::En;
}

const char* msg(Locale loc, const char* en, const char* es) {
  return loc == Locale::Es ? es : en;
}

}  // namespace i3space
