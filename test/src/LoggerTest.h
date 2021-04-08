#pragma once

#include <HyperionUtils/Logger.h>
#include <gtest/gtest.h>

namespace hyperion::utils::test {
	using hyperion::utils::LoggerLevel;
	using hyperion::utils::LoggerPolicy;

	TEST(LoggerTest, loggingCase1) {
		using Parameters = LoggerParameters<LoggerPolicy<LogPolicy::FlushWhenFull>,
											LoggerLevel<LogLevel::MESSAGE>>;

		constexpr auto num_entries = 512;
		auto thread = std::jthread([&]() {
			for(int i = 0; i < num_entries; ++i) {
				auto result = INFO<Parameters>(None(), "{0}{1}", "info"s, i);
				ignore(result.is_ok());
			}
		});

		for(int i = 0; i < num_entries; ++i) {
			auto result = MESSAGE<Parameters>(None(), "{0}{1}", "message"s, i);
			ignore(result.is_ok());
		}
		thread.join();

		ASSERT_TRUE(true);
	}
} // namespace hyperion::utils::test
