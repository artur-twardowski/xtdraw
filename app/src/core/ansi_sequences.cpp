#include <exception>
#include <stdexcept>
#include <string>

#include "core/ansi_output.h"
#include "core/ansi_sequences.h"

namespace xtdraw::ansi {

static std::string FetchParam(const char *&p, const char *last_char) {
    std::string result;
    while (p < last_char && *p != ';') {
        result += *p;
        p++;
    }
    if (*p == ';') p++;
    return result;
}

std::string EncodeANSISequence(const Sequence &) { return ""; }

static Sequence DecodeSGRSequence(const char *ptr, const char *last_char) {
    Sequence result;
    result.type   = SequenceType::kSGR;
    result.params = SGRSequence();
    auto &params  = std::get<SGRSequence>(result.params);
    while (ptr < last_char) {
        std::string param = FetchParam(ptr, last_char);
        if (param == "0") {
            params.reset.emplace(true);
        } else if (param == "1") {
            params.bold.emplace(true);
        } else if (param[0] == '3' || param[0] == '4') {
            bool              for_background = (param[0] == '4');
            ANSIOutput::Color color_spec;

            if (param[1] >= '0' && param[1] <= '7') {
                // param 30..37, 40..47 encode standard 8-color background and foreground
                color_spec = static_cast<uint8_t>(param[1] - '0');

            } else if (param[1] == '8') {
                // param 38, 48 indicate that we use richer palette
                std::string format = FetchParam(ptr, last_char);
                if (format == "5") {
                    // [34]8;5;xxx indicates a 256-color format. The parameter must be an
                    // integer between 0 and 255
                    std::string color_index_s = FetchParam(ptr, last_char);
                    try {
                        int color_index = std::stoi(color_index_s);
                        if (color_index < 0 || color_index > 255) {
                            return {.type = SequenceType::kInvalid,
                                    .params =
                                        std::string{
                                            "Invalid color index for 256-color mode: "} +
                                        color_index_s};
                        }
                        color_spec = static_cast<uint8_t>(color_index);
                    } catch (...) {
                        return {
                            .type = SequenceType::kInvalid,
                            .params =
                                std::string{"Invalid color index for 256-color mode: "} +
                                color_index_s};
                    }
                } else if (format == "2") {
                    // [34]8;2;rrr;ggg;bbb indicates a 24-bit TrueColor format. Three
                    // parameters must be present, each one being an integer between 0 and
                    // 255
                    std::string red_s   = FetchParam(ptr, last_char);
                    std::string green_s = FetchParam(ptr, last_char);
                    std::string blue_s  = FetchParam(ptr, last_char);
                    try {
                        int red   = std::stoi(red_s);
                        int green = std::stoi(green_s);
                        int blue  = std::stoi(blue_s);

                        if (red < 0 || red > 255) {
                            return {
                                .type = SequenceType::kInvalid,
                                .params =
                                    std::string{
                                        "Invalid red component for 24-bit color mode: "} +
                                    red_s};
                        }
                        if (green < 0 || green > 255) {
                            return {.type   = SequenceType::kInvalid,
                                    .params = std::string{"Invalid green component for "
                                                          "24-bit color mode: "} +
                                              green_s};
                        }
                        if (blue < 0 || blue > 255) {
                            return {.type   = SequenceType::kInvalid,
                                    .params = std::string{"Invalid blue component for "
                                                          "24-bit color mode: "} +
                                              blue_s};
                        }
                        color_spec = ANSIOutput::RGB{static_cast<uint8_t>(red),
                                                     static_cast<uint8_t>(green),
                                                     static_cast<uint8_t>(blue)};
                    } catch (...) {
                        return {.type   = SequenceType::kInvalid,
                                .params = std::string{"Invalid color specification for "
                                                      "24-bit color mode: R="} +
                                          red_s + ", G=" + green_s + ", B=" + blue_s};
                    }
                }
            }

            if (for_background) {
                params.background_color.emplace(color_spec);
            } else {
                params.foreground_color.emplace(color_spec);
            }
        }
    }

    return result;
}

Sequence DecodeANSISequence(const std::string &seq) {
    const char *str       = seq.c_str();
    const char *last_char = str + seq.size() - 1;

    if (*str == '[') {
        str++;
        switch (*last_char) {
            case 'm':
                return DecodeSGRSequence(str, last_char);
        }
    } else {
    }
    return Sequence{.type = SequenceType::kUnknown, .params = std::monostate{}};
}

}  // namespace xtdraw::ansi
