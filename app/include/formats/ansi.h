#ifndef FORMATS_ANSI_H
#define FORMATS_ANSI_H
#include <iostream>
#include <string>

namespace xtdraw {
class Board;
}

namespace xtdraw::format {
class ANSI {
   public:
    explicit ANSI(Board &board) : board(board) {}
    void Read(std::istream &file);
    void Write(std::ostream &file);

   private:
    Board &board;
};
}  // namespace xtdraw::format

#endif
