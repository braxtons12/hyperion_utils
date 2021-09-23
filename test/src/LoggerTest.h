#pragma once

#include <HyperionUtils/Logger.h>

#include "microTest.h"

namespace hyperion::test {
	using hyperion::LoggerLevel;
	using hyperion::LoggerPolicy;

	const suite LoggerTests = [] { // NOLINT
		"loggingCase1"_test = [] {
			expect(aborts([] {
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
				   })
				   == FALSE);
		};
	};
} // namespace hyperion::test
