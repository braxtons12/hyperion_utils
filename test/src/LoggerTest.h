#pragma once

#include <HyperionUtils/Logger.h>
#include <gtest/gtest.h>

namespace hyperion::utils::test {
	using hyperion::utils::LoggerLevel;
	using hyperion::utils::LoggerPolicy;

	TEST(LoggerTest, loggingCase1) {
		using Parameters = LoggerParameters<LoggerPolicy<LogPolicy::FlushWhenFull>,
											LoggerLevel<LogLevel::MESSAGE>>;
		ignore(initialize_global_logger<Parameters>());

		std::atomic_bool close = false;
		auto thread = std::thread([&]() {
			int i = 0;
			while(!close.load()) {
				MESSAGE<Parameters>("{0}{1}", "message"s, i);
			}
		});
	}
} // namespace hyperion::utils::test
