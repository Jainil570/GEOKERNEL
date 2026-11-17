#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>

#include "geokernel/core/Log.hpp"

using gk::log::Level;

namespace {
/// Restores the global log level after each test.
class LogTest : public ::testing::Test {
protected:
    void SetUp() override { saved_ = gk::log::level(); }
    void TearDown() override { gk::log::setLevel(saved_); }

private:
    Level saved_{Level::Warn};
};
}  // namespace

TEST_F(LogTest, ParseLevelIsCaseInsensitiveWithAliases) {
    EXPECT_EQ(gk::log::parseLevel("TRACE"), Level::Trace);
    EXPECT_EQ(gk::log::parseLevel("Debug"), Level::Debug);
    EXPECT_EQ(gk::log::parseLevel("warning"), Level::Warn);
    EXPECT_EQ(gk::log::parseLevel("none"), Level::Off);
    EXPECT_FALSE(gk::log::parseLevel("verbose").has_value());
    EXPECT_FALSE(gk::log::parseLevel("").has_value());
}

TEST_F(LogTest, LevelRoundTripsThroughToString) {
    for (const auto lvl :
         {Level::Trace, Level::Debug, Level::Info, Level::Warn, Level::Error, Level::Critical, Level::Off}) {
        EXPECT_EQ(gk::log::parseLevel(gk::log::toString(lvl)), lvl);
    }
}

TEST_F(LogTest, SetLevelIsObservable) {
    gk::log::setLevel(Level::Debug);
    EXPECT_EQ(gk::log::level(), Level::Debug);
    gk::log::setLevel(Level::Off);
    EXPECT_EQ(gk::log::level(), Level::Off);
}

TEST_F(LogTest, FileSinkReceivesRecords) {
    const auto path = std::filesystem::temp_directory_path() / "geokernel_log_sink_test.log";
    std::filesystem::remove(path);

    ASSERT_TRUE(gk::log::addFileSink(path.string()));
    gk::log::setLevel(Level::Info);
    gk::log::write(Level::Info, "file-sink-marker-12345");
    gk::log::flush();

    std::ifstream in(path);
    std::stringstream content;
    content << in.rdbuf();
    EXPECT_NE(content.str().find("file-sink-marker-12345"), std::string::npos);
}
