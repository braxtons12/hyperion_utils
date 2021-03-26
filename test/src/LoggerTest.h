#pragma once

#include <HyperionUtils/Logger.h>
#include <gtest/gtest.h>

namespace hyperion::utils::test {
	using hyperion::utils::LoggerLevel;
	using hyperion::utils::LoggerPolicy;

	TEST(LoggerTest, loggingCase1) {
		ignore(initialize_global_logger<LoggerParameters<LoggerPolicy<LogPolicy::FlushWhenFull>,
														 LoggerLevel<LogLevel::MESSAGE>>>());
	}
} // namespace hyperion::utils::test
