#ifndef XTDRAW_ANSI_SEQUENCES_H
#define XTDRAW_ANSI_SEQUENCES_H

#include "ansi_output.h"

namespace xtdraw::ansi {
enum struct SequenceType { kUnknown = 0, kInvalid = 1, kSGR };

struct SGRSequence {
    std::optional<ANSIOutput::Color> background_color{};
    std::optional<ANSIOutput::Color> foreground_color{};
    std::optional<bool>              reset{};
    std::optional<bool>              bold{};
};

typedef std::pair<uint16_t, uint16_t> CursorPosition;

typedef std::variant<SGRSequence, size_t, CursorPosition, std::string, std::monostate>
    SequenceParams;

struct Sequence {
    SequenceType   type;
    SequenceParams params;
};

std::string EncodeANSISequence(const Sequence &seq);
Sequence    DecodeANSISequence(const std::string &seq);
}  // namespace xtdraw::ansi

#endif
