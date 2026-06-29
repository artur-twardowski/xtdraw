#include "colors.h"

namespace xtdraw {
void SetColor(TerminalIO &terminal_io, const ColorPair &color_pair) {
    terminal_io.SetColor(color_pair.background, color_pair.foreground);
}
}  // namespace xtdraw
