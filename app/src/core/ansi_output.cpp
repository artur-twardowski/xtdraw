#include "core/ansi_output.h"
#include "core/utils.h"

namespace xtdraw {
static constexpr const char ESC = '\033';

bool operator==(const AbstractANSIOutput::Color &c1,
                const AbstractANSIOutput::Color &c2) {
    using RGB = AbstractANSIOutput::RGB;
    if (std::holds_alternative<uint8_t>(c1) && std::holds_alternative<uint8_t>(c2)) {
        return std::get<uint8_t>(c1) == std::get<uint8_t>(c2);
    } else if (std::holds_alternative<RGB>(c1) && std::holds_alternative<RGB>(c2)) {
        const RGB &rgb1 = std::get<RGB>(c1);
        const RGB &rgb2 = std::get<RGB>(c2);
        return rgb1.r == rgb2.r && rgb1.g == rgb2.g && rgb1.b == rgb2.b;
    } else {
        return false;
    }
}

bool operator!=(const AbstractANSIOutput::Color &c1,
                const AbstractANSIOutput::Color &c2) {
    return !(c1 == c2);
}

static void ColorToANSICode(std::ostream &os, const ANSIOutput::Color &color) {
    if (std::holds_alternative<ANSIOutput::RGB>(color)) {
        const auto &rgb = std::get<ANSIOutput::RGB>(color);
        os << "2;" << static_cast<int>(rgb.r) << ";" << static_cast<int>(rgb.b) << ";"
           << static_cast<int>(rgb.b);
    } else {
        os << "5;" << static_cast<int>(std::get<uint8_t>(color));
    }
}

void ANSIOutput::SetColor(std::optional<Color> bg, std::optional<Color> fg) {
    bool put_bg = (bg.has_value() && last_bg_color != *bg);
    bool put_fg = (fg.has_value() && last_fg_color != *fg);
    if (!put_bg && !put_fg) {
        return;
    }
    out << ESC << "[";
    if (put_bg) {
        out << "48;";
        ColorToANSICode(out, *bg);
        last_bg_color = *bg;
    }
    if (put_fg) {
        if (put_bg) {
            out << ";";
        }
        out << "38;";
        ColorToANSICode(out, *fg);
        last_fg_color = *fg;
    }
    out << "m";
}
void ANSIOutput::ResetFormatting() {
    last_bg_color = uint8_t{0};
    last_fg_color = uint8_t{255};
    out << ESC << "[0m";
}

void ANSIOutput::Write(const std::string &data) { out << data; }
void ANSIOutput::Write(uint32_t character) { out << EncodeUTF8(character); }

}  // namespace xtdraw
