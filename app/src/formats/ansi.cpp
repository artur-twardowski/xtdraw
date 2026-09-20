#include "formats/ansi.h"

#include "board.h"
#include "core/ansi_output.h"
#include "core/ansi_sequences.h"
#include "core/terminal_io.h"
#include "core/utils.h"

namespace xtdraw::format {
static std::string ReadANSISequence(std::istream &f) {
    std::string result;
    bool        read_first_char = true;
    while (!f.eof()) {
        uint8_t data = ReadByteFromFile(f);
        result += data;
        if (read_first_char) {
            read_first_char = false;
        } else if (data >= 0x40 && data <= 0x7e) {
            break;
        }
    }
    return result;
}

void ANSI::Read(std::istream &file) {
    BoardCell                           cell;
    std::vector<std::vector<BoardCell>> cells;
    cells.emplace_back();
    while (file) {
        uint8_t data = ReadByteFromFile(file);
        if (data == '\x1b') {
            std::string seq_s = ReadANSISequence(file);
            auto        seq   = ansi::DecodeANSISequence(seq_s);
            if (seq.type == ansi::SequenceType::kSGR) {
                auto sgr = std::get<ansi::SGRSequence>(seq.params);
                if (sgr.background_color.has_value()) {
                    cell.background_color = sgr.background_color.value();
                }
                if (sgr.foreground_color.has_value()) {
                    cell.foreground_color = sgr.foreground_color.value();
                }
            }
        } else if (data == '\n') {
            cells.emplace_back();
        } else {
            cell.character = ReadUTF8(file, data);
            cells.back().push_back(cell);
        }
    }
    size_t height = cells.size();
    size_t width  = cells[0].size();
    for (const auto &row : cells) {
        width = row.size() > width ? row.size() : width;
    }
    board.Reset(width, height);
    for (size_t y = 0; y < height; y++) {
        BoardCell cell;
        for (size_t x = 0; x < width; x++) {
            if (x < cells[y].size()) {
                cell = cells[y][x];
            }
            board.SetCell(x, y, cell);
        }
    }
}

void ANSI::Write(std::ostream &file) {
    ANSIOutput output(file);

    for (size_t y = 0; y < board.GetRows(); y++) {
        for (size_t x = 0; x < board.GetCols(); x++) {
            BoardCell cell = board.GetCell(y, x);

            output.SetColor(cell.background_color, cell.foreground_color);
            output.Write(cell.character);
        }
        output.ResetFormatting();
        output.Write('\n');
    }
}
}  // namespace xtdraw::format
