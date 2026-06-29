#include "i3space/app.hpp"
#include "i3space/config.hpp"
#include "i3space/first_run.hpp"
#include "i3space/locale.hpp"
#include "i3space/version.hpp"

#include <cstring>
#include <iostream>
#include <string>

namespace i3space {

void print_version() {
  std::cout << "i3space " << I3SPACE_VERSION << "\n";
}

void print_help(const char* argv0) {
  const Locale loc = detect_locale();
  std::cout << msg(loc,
                   "Usage: ",
                   "Uso: ")
            << argv0 << " [--version] [--help] [--print-active] [--output NAME] [--toggle]\n\n"
            << msg(loc,
                   "  --print-active   List active workspaces per output (JSON)\n"
                   "  --output NAME    Filter --print-active to one output\n"
                   "  --toggle         Open workspace overview (i3 must be running)\n"
                   "  --version        Show version\n"
                   "  --help           Show this help\n\n"
                   "Config: ~/.config/i3space/i3space\n"
                   "i3 bind example:\n"
                   "  bindsym --release $mod+Shift+Tab exec --no-startup-id i3space --toggle\n",
                   "  --print-active   Lista workspaces activos por output (JSON)\n"
                   "  --output NAME    Filtra --print-active a un output\n"
                   "  --toggle         Abre el overview (i3 en ejecución)\n"
                   "  --version        Muestra versión\n"
                   "  --help           Muestra esta ayuda\n\n"
                   "Config: ~/.config/i3space/i3space\n"
                   "Ejemplo bind i3:\n"
                   "  bindsym --release $mod+Shift+Tab exec --no-startup-id i3space --toggle\n");
}

}  // namespace i3space

int main(int argc, char** argv) {
  using namespace i3space;

  bool do_print = false;
  bool do_toggle = false;
  bool do_version = false;
  bool do_help = false;
  std::string filter_output;

  for (int i = 1; i < argc; ++i) {
    if (std::strcmp(argv[i], "--version") == 0) {
      do_version = true;
    } else if (std::strcmp(argv[i], "--help") == 0 || std::strcmp(argv[i], "-h") == 0) {
      do_help = true;
    } else if (std::strcmp(argv[i], "--print-active") == 0) {
      do_print = true;
    } else if (std::strcmp(argv[i], "--toggle") == 0) {
      do_toggle = true;
    } else if (std::strcmp(argv[i], "--output") == 0 && i + 1 < argc) {
      filter_output = argv[++i];
    } else {
      std::cerr << "Unknown option: " << argv[i] << "\n";
      return 1;
    }
  }

  if (do_version) {
    print_version();
    return 0;
  }
  if (do_help || argc == 1) {
    print_help(argv[0]);
    return 0;
  }

  if (!ensure_first_run()) {
    return 1;
  }

  if (do_print) {
    std::cout << '{' << std::flush;
    const int rc = print_active_main(filter_output);
    std::cout << '}' << '\n';
    return rc;
  }
  if (do_toggle) {
    return toggle_main();
  }

  print_help(argv[0]);
  return 0;
}
