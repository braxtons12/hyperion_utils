// NOLINTNEXTLINE
#include <Hyperion/Utils.h>
#include <chrono>

using namespace hyperion; // NOLINT

auto main([[maybe_unused]] i32 argc, [[maybe_unused]] char** argv) -> i32 { // NOLINT
	using Parameters = LoggerParameters<
		LoggerPolicy<LogThreadingPolicy::SingleThreadedAsync, LogAsyncPolicy::DropWhenFull>,
		LoggerLevel<LogLevel::MESSAGE>,
		128_usize>; // NOLINT

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
