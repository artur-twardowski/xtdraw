#include <gtest/gtest.h>

#include "core/utils.h"
#include "gtest/gtest.h"

class UTF8EncodeDecode : public testing::TestWithParam<uint32_t> {};

TEST_P(UTF8EncodeDecode, Test) {
    uint32_t           input_char = GetParam();
    std::string        encoded    = xtdraw::EncodeUTF8(input_char);
    std::istringstream is(encoded);
    std::cout << "0x" << std::hex << std::setw(8) << std::setfill('0') << input_char
              << " ->";
    for (const uint8_t c : encoded) {
        std::cout << " " << std::setw(2) << (int)c;
    }

    char first_char;
    is.read(&first_char, 1);
    uint32_t decoded_char = xtdraw::ReadUTF8(is, first_char);

    std::cout << " -> " << std::setw(8) << decoded_char << std::endl;

    EXPECT_EQ(input_char, decoded_char);
}

INSTANTIATE_TEST_SUITE_P(, UTF8EncodeDecode,
                         testing::Values(0x20, 0x320, 0x2500, 0x16555));
