#include <gtest/gtest.h>

#include <sstream>
#include <string>
#include <vector>

#include "io/detail/ParseUtils.hpp"

namespace d = gk::io::detail;

TEST(ParseUtils, TrimSplitAndCase) {
    EXPECT_EQ(d::trim("  \t abc \r\n"), "abc");
    EXPECT_EQ(d::trim("   "), "");
    EXPECT_EQ(d::toLower("SoLiD"), "solid");
    EXPECT_TRUE(d::iequals("EndSolid", "endsolid"));
    EXPECT_FALSE(d::iequals("end", "endsolid"));

    std::vector<std::string_view> parts;
    d::splitWhitespace("  f 1/2/3\t4//5  6 ", parts);
    EXPECT_EQ(parts, (std::vector<std::string_view>{"f", "1/2/3", "4//5", "6"}));
}

TEST(ParseUtils, ParseDoubleIsStrictAndLocaleFree) {
    double v = 0.0;
    EXPECT_TRUE(d::parseDouble("1.5", v));
    EXPECT_DOUBLE_EQ(v, 1.5);
    EXPECT_TRUE(d::parseDouble("-2.5e-3", v));
    EXPECT_DOUBLE_EQ(v, -2.5e-3);
    EXPECT_TRUE(d::parseDouble("+4", v));
    EXPECT_DOUBLE_EQ(v, 4.0);
    EXPECT_FALSE(d::parseDouble("1,5", v)) << "decimal comma must be rejected";
    EXPECT_FALSE(d::parseDouble("1.5abc", v)) << "trailing garbage must be rejected";
    EXPECT_FALSE(d::parseDouble("", v));
}

TEST(ParseUtils, ParseInt64) {
    long long v = 0;
    EXPECT_TRUE(d::parseInt64("-42", v));
    EXPECT_EQ(v, -42);
    EXPECT_FALSE(d::parseInt64("4.2", v));
    EXPECT_FALSE(d::parseInt64("", v));
    EXPECT_FALSE(d::parseInt64("99999999999999999999999", v)) << "overflow";
}

TEST(ParseUtils, LittleEndianDecoding) {
    const unsigned char bytes[] = {0x78, 0x56, 0x34, 0x12};
    EXPECT_EQ(d::readU32LE(bytes), 0x12345678U);
    const unsigned char one[] = {0x00, 0x00, 0x80, 0x3F};  // 1.0f
    EXPECT_FLOAT_EQ(d::readF32LE(one), 1.0F);
}

TEST(ParseUtils, TokenStreamTracksLinesAndRestOfLine) {
    std::istringstream in("solid my part\n  facet normal 0 0 1\n\nendsolid\n");
    d::TokenStream ts(in);
    std::string_view tok;
    ASSERT_TRUE(ts.next(tok));
    EXPECT_EQ(tok, "solid");
    EXPECT_EQ(ts.restOfLine(), "my part");
    ASSERT_TRUE(ts.next(tok));
    EXPECT_EQ(tok, "facet");
    EXPECT_EQ(ts.line(), 2U);
    for (int i = 0; i < 4; ++i) {
        ASSERT_TRUE(ts.next(tok));
    }
    ASSERT_TRUE(ts.next(tok));
    EXPECT_EQ(tok, "endsolid");
    EXPECT_EQ(ts.line(), 4U) << "blank line 3 must still be counted";
    EXPECT_FALSE(ts.next(tok));
}

TEST(ParseUtils, RemainingBytesRestoresPosition) {
    std::istringstream in("0123456789");
    in.seekg(3);
    const auto n = d::remainingBytes(in);
    ASSERT_TRUE(n.has_value());
    EXPECT_EQ(*n, 7U);
    EXPECT_EQ(in.get(), '3');
}
