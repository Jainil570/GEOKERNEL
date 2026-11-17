#include <gtest/gtest.h>

#include <memory>
#include <string>

#include "geokernel/core/Result.hpp"

using gk::core::BadResultAccess;
using gk::core::Error;
using gk::core::ErrorCode;
using gk::core::makeError;
using gk::core::Result;
using gk::core::Status;

namespace {
Result<int> parsePositive(int v) {
    if (v <= 0) {
        return makeError(ErrorCode::InvalidArgument, "must be positive");
    }
    return v;
}

Result<std::unique_ptr<int>> makeBox(int v) {
    auto p = std::make_unique<int>(v);
    return p;  // implicit move of a move-only type into Result
}
}  // namespace

TEST(Result, HoldsValueOnSuccess) {
    const auto r = parsePositive(7);
    ASSERT_TRUE(r.ok());
    EXPECT_TRUE(static_cast<bool>(r));
    EXPECT_EQ(r.value(), 7);
    EXPECT_EQ(*r, 7);
    EXPECT_EQ(r.valueOr(0), 7);
}

TEST(Result, HoldsErrorOnFailure) {
    const auto r = parsePositive(-1);
    ASSERT_FALSE(r.ok());
    EXPECT_EQ(r.error().code, ErrorCode::InvalidArgument);
    EXPECT_EQ(r.error().toString(), "InvalidArgument: must be positive");
    EXPECT_EQ(r.valueOr(42), 42);
}

TEST(Result, AccessingWrongAlternativeThrows) {
    EXPECT_THROW(static_cast<void>(parsePositive(-1).value()), BadResultAccess);
    EXPECT_THROW(static_cast<void>(parsePositive(1).error()), std::logic_error);
}

TEST(Result, SupportsMoveOnlyTypes) {
    auto r = makeBox(5);
    ASSERT_TRUE(r.ok());
    std::unique_ptr<int> owned = std::move(r).value();
    ASSERT_NE(owned, nullptr);
    EXPECT_EQ(*owned, 5);
}

TEST(Status, DefaultIsSuccessAndCarriesErrors) {
    EXPECT_TRUE(Status{}.ok());
    EXPECT_TRUE(Status::success().ok());

    const Status failed = makeError(ErrorCode::IoError, "disk");
    EXPECT_FALSE(failed.ok());
    EXPECT_EQ(failed.error().code, ErrorCode::IoError);
    EXPECT_THROW(static_cast<void>(Status::success().error()), std::logic_error);
}

TEST(ErrorCode, EveryCodeHasAName) {
    for (const auto code : {ErrorCode::InvalidArgument, ErrorCode::NotFound, ErrorCode::AlreadyExists,
                            ErrorCode::IoError, ErrorCode::ParseError, ErrorCode::UnsupportedFormat,
                            ErrorCode::LimitExceeded, ErrorCode::Internal}) {
        EXPECT_NE(gk::core::toString(code), "Unknown");
    }
}
