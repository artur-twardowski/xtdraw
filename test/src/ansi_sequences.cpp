#include <gtest/gtest.h>

#include <variant>

#include "core/ansi_output.h"
#include "core/ansi_sequences.h"

using xtdraw::ansi::Sequence;
using xtdraw::ansi::SequenceType;
using xtdraw::ansi::SGRSequence;
using RGB = xtdraw::ANSIOutput::RGB;

void ValidateSGRSequence(const Sequence &seq, const SGRSequence &expectation) {
    EXPECT_EQ(seq.type, SequenceType::kSGR);
    ASSERT_TRUE(std::holds_alternative<SGRSequence>(seq.params));

    const auto &params = std::get<SGRSequence>(seq.params);

    if (expectation.background_color.has_value()) {
        ASSERT_TRUE(params.background_color.has_value());
        EXPECT_EQ(params.background_color, expectation.background_color);
    } else {
        EXPECT_FALSE(params.background_color.has_value());
    }

    if (expectation.foreground_color.has_value()) {
        ASSERT_TRUE(params.foreground_color.has_value());
        EXPECT_EQ(params.foreground_color, expectation.foreground_color);
    } else {
        EXPECT_FALSE(params.foreground_color.has_value());
    }

    if (expectation.reset.has_value()) {
        ASSERT_TRUE(params.reset.has_value());
        EXPECT_EQ(*params.reset, *expectation.reset);
    } else {
        EXPECT_FALSE(params.reset.has_value());
    }

    if (expectation.bold.has_value()) {
        ASSERT_TRUE(params.bold.has_value());
        EXPECT_EQ(*params.bold, *expectation.bold);
    } else {
        EXPECT_FALSE(params.bold.has_value());
    }
}

void ValidateErrorSequence(const Sequence &seq) {
    EXPECT_EQ(seq.type, SequenceType::kInvalid);
    ASSERT_TRUE(std::holds_alternative<std::string>(seq.params));
    EXPECT_FALSE(std::get<std::string>(seq.params).empty());

    std::cout << "  # " << std::get<std::string>(seq.params) << std::endl;
}

TEST(ANSISequencesDecoding, BasicForegroundColor) {
    auto decoded = xtdraw::ansi::DecodeANSISequence("[34m");
    ValidateSGRSequence(decoded, SGRSequence{.foreground_color = uint8_t{4}});
}

TEST(ANSISequencesDecoding, Foreground8bitColor) {
    auto decoded = xtdraw::ansi::DecodeANSISequence("[38;5;67m");
    ValidateSGRSequence(decoded, SGRSequence{.foreground_color = uint8_t{67}});
}

TEST(ANSISequencesDecoding, Foreground24bitColor) {
    auto decoded = xtdraw::ansi::DecodeANSISequence("[38;2;255;24;0m");
    ValidateSGRSequence(decoded, SGRSequence{.foreground_color = RGB{255, 24, 0}});
}

TEST(ANSISequencesDecoding, BasicBackgroundColor) {
    auto decoded = xtdraw::ansi::DecodeANSISequence("[46m");
    ValidateSGRSequence(decoded, SGRSequence{.background_color = uint8_t{6}});
}
TEST(ANSISequencesDecoding, Background8bitColor) {
    auto decoded = xtdraw::ansi::DecodeANSISequence("[48;5;42m");
    ValidateSGRSequence(decoded, SGRSequence{.background_color = uint8_t{42}});
}
TEST(ANSISequencesDecoding, Background24bitColor) {
    auto decoded = xtdraw::ansi::DecodeANSISequence("[48;2;128;64;11m");
    ValidateSGRSequence(decoded, SGRSequence{.background_color = RGB{128, 64, 11}});
}

TEST(ANSISequencesDecoding, ColorsCombinationBasic) {
    auto decoded = xtdraw::ansi::DecodeANSISequence("[34;45m");
    ValidateSGRSequence(decoded, SGRSequence{
                                     .background_color = uint8_t{5},
                                     .foreground_color = uint8_t{4},
                                 });
}

TEST(ANSISequencesDecoding, ColorsCombination8bit) {
    auto decoded = xtdraw::ansi::DecodeANSISequence("[38;5;76;48;5;212m");
    ValidateSGRSequence(decoded, SGRSequence{
                                     .background_color = uint8_t{212},
                                     .foreground_color = uint8_t{76},
                                 });
}

TEST(ANSISequencesDecoding, ColorsCombination24bit) {
    auto decoded = xtdraw::ansi::DecodeANSISequence("[38;2;255;224;64;48;2;0;72;72m");
    ValidateSGRSequence(decoded, SGRSequence{.background_color = RGB{0, 72, 72},
                                             .foreground_color = RGB{255, 224, 64}});
}

TEST(ANSISequencesDecoding, Invalid8bitForeground) {
    auto decoded = xtdraw::ansi::DecodeANSISequence("[38;5;2137m");
    ValidateErrorSequence(decoded);
}


