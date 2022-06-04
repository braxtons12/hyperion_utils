/// @file benchmark.cpp
/// @author Braxton Salyer <braxtonsalyer@gmail.com>
/// @brief Simple benchmark of Hyperion's logging facilities
/// @version 0.1
/// @date 2022-06-04
///
/// MIT License
/// @copyright Copyright (c) 2022 Braxton Salyer <braxtonsalyer@gmail.com>
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
		LoggerPolicy<LogThreadingPolicy::MultiThreadedAsync, LogAsyncPolicy::BlockWhenFull>,
		LoggerLevel<LogLevel::MESSAGE>,
		1024_usize>; // NOLINT

	if constexpr(HYPERION_PLATFORM_PROFILING_ENABLED) {
		while(!static_cast<bool>(std::cin.get())) {
			// wait for input to start profiling
		}
	}

	auto file = FileSink::create_file();
	if(!file) {
		eprintln("{}", file.unwrap_err().message());
	}
	auto file_sink = make_sink<FileSink>(file.expect("Failed to create test file!"));
	// auto stdout_sink = make_sink<StdoutSink<>>();
	// auto stderr_sink = make_sink<StderrSink<>>();
	auto sinks = Sinks();
	sinks.push_back(std::move(file_sink));
	auto logger = hyperion::make_unique<Logger<Parameters>>(std::move(sinks));

	constexpr auto num_entries = 1'000'000_usize;
	constexpr auto num_threads = []() noexcept -> usize {
		if constexpr(Parameters::threading_policy == LogThreadingPolicy::SingleThreaded
					 || Parameters::threading_policy == LogThreadingPolicy::SingleThreadedAsync)
		{
			return 1_usize;
		}
		else {
			return 10_usize;
		}
	}();

	const auto now = std::chrono::high_resolution_clock::now();

	auto longest = ReadWriteLock<decltype(now - now)>(); // NOLINT
	if constexpr(Parameters::threading_policy == LogThreadingPolicy::SingleThreaded
				 || Parameters::threading_policy == LogThreadingPolicy::SingleThreadedAsync)
	{
		auto single_threaded_longest = decltype(now - now)(); // NOLINT
		for(auto i = 0U; i < num_entries; ++i) {
			const auto curr = std::chrono::high_resolution_clock::now();
			logger->error("Hello logger: msg number {}", i);
			//ignore(logger->error("Hello logger: msg number {}", i));
			const auto fin = std::chrono::high_resolution_clock::now();
			const auto diff = fin - curr;
			if(diff > single_threaded_longest) {
				single_threaded_longest = diff;
			}
		}
		if(single_threaded_longest > longest.read().read()) {
			longest.write() = single_threaded_longest;
		}
	}
	else {
		auto threads = std::vector<std::thread>();
		threads.reserve(num_threads);

		for(auto index = 0_usize; index < num_threads; ++index) {
			threads.emplace_back([&logger, &longest]() {
				auto threaded_longest = decltype(now - now)(); // NOLINT
				for(auto i = 0U; i < num_entries; ++i) {
					const auto curr = std::chrono::high_resolution_clock::now();
					logger->error("Hello logger: msg number {}", i);
					//ignore(logger->error("Hello logger: msg number {}", i));
					const auto fin = std::chrono::high_resolution_clock::now();
					const auto diff = fin - curr;
					if(diff > threaded_longest) {
						threaded_longest = diff;
					}
				}
				if(threaded_longest > longest.read().read()) {
					longest.write() = threaded_longest;
				}
			});
		}

		for(auto& thread : threads) {
			thread.join();
		}
	}

	const auto done = std::chrono::high_resolution_clock::now();
	auto elapsed = done - now;

	const auto seconds = gsl::narrow_cast<f32>(elapsed.count())
						 / gsl::narrow_cast<f32>(decltype(elapsed)::period::den);
	println("Num Threads: {}", num_threads);
	println("Elapsed time: {}",
			std::chrono::duration_cast<std::chrono::duration<double, std::chrono::seconds::period>>(
				elapsed));
	println("Entries / second : {}", gsl::narrow_cast<float>(num_entries * num_threads) / seconds);

	const auto latency_as_nanoseconds
		= std::chrono::duration_cast<std::chrono::nanoseconds>(longest.read().read());
	println("Longest latency (nanoseconds) {}", latency_as_nanoseconds);
	ignore(std::fflush(stdout));

	while(!static_cast<bool>(std::cin.get())) {
		// wait for input to stop profiling
	}
	return 0;
}
