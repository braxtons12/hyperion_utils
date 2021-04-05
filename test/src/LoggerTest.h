#pragma once

#include <HyperionUtils/Logger.h>
#include <gtest/gtest.h>

namespace hyperion::utils::test {
	using hyperion::utils::LoggerLevel;
	using hyperion::utils::LoggerPolicy;

	TEST(LoggerTest, loggingCase1) {
		using Parameters = LoggerParameters<LoggerPolicy<LogPolicy::FlushWhenFull>,
											LoggerLevel<LogLevel::MESSAGE>>;

		auto thread = std::jthread([&](const std::stop_token& stop) {
			int i = 0;
			while(!stop.stop_requested()) {
				auto result = INFO<Parameters>(None(), "{0}{1}", "info"s, i);
				ignore(result.is_ok());
				i++;
			}
		});

		for(int i = 0; i < 512; ++i) {
			auto result = MESSAGE<Parameters>(None(), "{0}{1}", "message"s, i);
			ignore(result.is_ok());
		}
		thread.request_stop();

		ASSERT_TRUE(true);
	}
} // namespace hyperion::utils::test
