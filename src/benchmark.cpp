/// @file benchmark.cpp
/// @author Braxton Salyer <braxtonsalyer@gmail.com>
/// @brief Simple benchmark of Hyperion's logging facilities
/// @version 0.1
/// @date 2021-11-14
///
/// MIT License
/// @copyright Copyright (c) 2021 Braxton Salyer <braxtonsalyer@gmail.com>
///
/// Permission is hereby granted, free of charge, to any person obtaining a copy
/// of this software and associated documentation files (the "Software"), to deal
/// in the Software without restriction, including without limitation the rights
/// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
/// copies of the Software, and to permit persons to whom the Software is
/// furnished to do so, subject to the following conditions
///
/// The above copyright notice and this permission notice shall be included in all
/// copies or substantial portions of the Software.
///
/// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
/// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
/// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
/// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
/// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
/// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
/// SOFTWARE.
#include <Hyperion/Utils.h>
#include <chrono>

using namespace hyperion; // NOLINT

auto main([[maybe_unused]] i32 argc, [[maybe_unused]] char** argv) -> i32 { // NOLINT
	using Parameters = LoggerParameters<
		LoggerPolicy<LogThreadingPolicy::SingleThreadedAsync, LogAsyncPolicy::DropWhenFull>,
		LoggerLevel<LogLevel::MESSAGE>,
		128_usize>; // NOLINT

	if constexpr(HYPERION_PLATFORM_PROFILING_ENABLED) {
		while(!static_cast<bool>(std::cin.get())) {
		}
	}

	auto file = FileSink::create_file();
	if(!file) {
		eprintln("{}", file.unwrap_err().message());
	}
	auto file_sink = make_sink<FileSink>(file.expect("Failed to create test file!"));
	// auto stdout_sink = make_sink<StdoutSink<>>();
	// auto stderr_sink = make_sink<StderrSink<>>();
	auto sinks = Sinks({std::move(file_sink)});
	auto logger = Logger<Parameters>(std::move(sinks));

	constexpr auto num_entries = 1'000'000_usize;

	const auto now = std::chrono::high_resolution_clock::now();

	if constexpr(Parameters::threading_policy == LogThreadingPolicy::SingleThreaded
				 || Parameters::threading_policy == LogThreadingPolicy::SingleThreadedAsync)
	{
		for(auto i = 0U; i < num_entries; ++i) {
			ignore(logger.error("Hello logger: msg number {}", i));
		}
	}
	else {
		auto thread1 = std::thread([&]() {
			for(auto i = 0U; i < num_entries; ++i) {
				ignore(logger.error("Hello logger: msg number {}", i));
			}
		});

		auto thread2 = std::thread([&]() {
			for(auto i = 0U; i < num_entries; ++i) {
				ignore(logger.warn("Hello logger: msg number {}", i));
			}
		});

		thread1.join();
		thread2.join();
	}

	const auto done = std::chrono::high_resolution_clock::now();
	auto elapsed = done - now;

	println("Elapsed time: {}",
			std::chrono::duration_cast<std::chrono::duration<double, std::chrono::seconds::period>>(
				elapsed));
	std::fflush(stdout);

	return 0;
}
