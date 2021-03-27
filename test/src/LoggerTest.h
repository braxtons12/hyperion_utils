#pragma once

#include <HyperionUtils/Logger.h>
#include <gtest/gtest.h>

namespace hyperion::utils::test {
	using hyperion::utils::LoggerLevel;
	using hyperion::utils::LoggerPolicy;

	TEST(LoggerTest, loggingCase1) {
		using Parameters = LoggerParameters<LoggerPolicy<LogPolicy::FlushWhenFull>,
											LoggerLevel<LogLevel::MESSAGE>>;

		std::atomic_bool close = false;
		auto thread = std::thread([&]() {
			int i = 0;
			while(!close.load()) {
				auto result = MESSAGE<Parameters>("{0}{1}", "message"s, i);
				ignore(result.is_ok());
			}
		});

		for(int i = 0; i < 512; ++i) {
			auto result = MESSAGE<Parameters>("{0}{1}", "message"s, i);
			ignore(result.is_ok());
		}
		close.store(true);

		ASSERT_TRUE(true);
	}
} // namespace hyperion::utils::test
