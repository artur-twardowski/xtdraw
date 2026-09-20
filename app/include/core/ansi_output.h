#ifndef XTDRAW_ANSI_H
#define XTDRAW_ANSI_H
#include <stdint.h>

#include <iostream>
#include <optional>
#include <variant>

namespace xtdraw {
class AbstractANSIOutput {
   public:
    struct RGB {
        uint8_t r, g, b;
        bool    operator==(const RGB &other) const {
            return r == other.r && g == other.g && b == other.b;
        }
    };
    enum struct ColorMode { COLOR_8, COLOR_256, COLOR_RGB };
    typedef std::variant<uint8_t, RGB> Color;

    virtual void SetColor(std::optional<Color> bg, std::optional<Color> fg) = 0;
    virtual void ResetFormatting()                                          = 0;
    virtual void Write(const std::string &data)                             = 0;
    virtual void Write(uint32_t character)                                  = 0;
};

class ANSIOutput : public AbstractANSIOutput {
   public:
    explicit ANSIOutput(std::ostream &out) : out(out) {}
    virtual ~ANSIOutput() = default;
    void SetColor(std::optional<Color> bg, std::optional<Color> fg) override;
    void ResetFormatting() override;
    void Write(const std::string &data) override;
    void Write(uint32_t character) override;

   private:
    std::ostream &out;
    Color         last_bg_color, last_fg_color;
    ColorMode     color_mode;
};

}  // namespace xtdraw

#endif
