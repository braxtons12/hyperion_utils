/// @file Config.h
/// @author Braxton Salyer <braxtonsalyer@gmail.com>
/// @brief Logging configuration types
/// @version 0.1
/// @date 2022-06-15
///
/// MIT License
/// @copyright Copyright (c) 2022 Braxton Salyer <braxtonsalyer@gmail.com>
///
/// Permission is hereby granted, free of charge, to any person obtaining a copy
/// of this software and associated documentation files (the "Software"), to
/// deal in the Software without restriction, including without limitation the
/// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
/// sell copies of the Software, and to permit persons to whom the Software is
/// furnished to do so, subject to the following conditions:
///
/// The above copyright notice and this permission notice shall be included in
/// all copies or substantial portions of the Software.
///
/// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
/// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
/// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
/// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
/// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
/// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
/// IN THE SOFTWARE.
#pragma once

#include <Hyperion/Concepts.h>
#include <cstddef>

#if HYPERION_PLATFORM_WINDOWS
	#ifdef ERROR
		#undef ERROR
	#endif
#endif

namespace hyperion::logging {

	/// @brief Used to configure the desired logging level of loggers and sinks.
	/// @ingroup logging
	/// @headerfile "Hyperion/logging/Config.h"
	enum class Level : u8 {
		/// @brief Disable logging.
		DISABLED = 0,
		/// @brief General messages such as "log started" or "log closing"
		MESSAGE,
		/// @brief Used for logging call traces
		TRACE,
		/// @brief Used for general information reports
		INFO,
		/// @brief Used for warnings and other more severe reports
		WARN,
		/// @brief Used for system errors and other fatal or near-fatal reports
		ERROR,
	};

	/// @brief Used to configure the threading policy of loggers
	///
	/// - `SingleThreaded`: Used to configure a logger to be used on a single thread only. Using a
	/// single thread logger across multiple threads is unsafe.
	/// - `MultiThreaded`: Used to configure a logger to be used on multiple threads. A
	/// Multithreaded logger is safe to use across multiple threads, but is not non-blocking.
	/// - `MultiThreadedAsync`: Used to configure a logger to be used on multiple threads and log
	/// asynchronously. An asynchronous logger will push logging entries into a message queue and
	/// pop them to the sinks on a separate thread. Asynchronous behavior can be customized with
	/// `AsyncPolicy`
	/// @ingroup logging
	/// @headerfile "Hyperion/logging/Config.h"
	enum class ThreadingPolicy : u8 {
		SingleThreaded = 0,
		SingleThreadedAsync,
		MultiThreaded,
		MultiThreadedAsync
	};

	/// @brief Used to configure the data-retention strategy of an asynchronous logger.
	///
	/// Asynchronous loggers have multiple possible data-retention policies, each with different
	/// tradeoffs and performance characteristics:
	///
	/// - `DropWhenFull`: When a logging function is called the logger will return an error if the
	/// logging queue is full and fail to log the given entry. Useful when you need a custom
	/// strategy for when logging fails and/or when data-preservation is not necessary and
	/// throughput and latency are more important.
	/// - `OverwriteWhenFull`: The logger's queue will act as a ring buffer. When a logging function
	/// is called, if the queue is full entries not yet logged to disk will be overwritten by new
	/// entries. Useful when data-preservation is not necessary and throughput and latency are more
	/// important.
	/// - `BlockWhenFull`: The logger will block the calling thread when the logging queue is full
	/// until the queue is empty again. Useful when data-preservation is essential and throughput
	/// and (particularly) latency are unimportant
	/// @ingroup logging
	/// @headerfile "Hyperion/logging/Config.h"
	enum class AsyncPolicy : u8 {
		DropWhenFull = 0,
		OverwriteWhenFull,
		BlockWhenFull
	};

	/// @brief Configuration type for configuring a logger's threading and (potential) asynchronous
	/// policies
	///
	/// @tparam ThreadingPolicy - The `ThreadingPolicy` to use for the logger
	/// @tparam AsyncPolicy - The `AsyncPolicy` to use for the logger
	/// @ingroup logging
	/// @headerfile "Hyperion/logging/Config.h"
	template<ThreadingPolicy ThreadingPolicy = ThreadingPolicy::SingleThreaded,
			 AsyncPolicy AsyncPolicy = AsyncPolicy::DropWhenFull>
	struct Policy {
		static_assert(!(ThreadingPolicy == ThreadingPolicy::MultiThreadedAsync
						&& AsyncPolicy == AsyncPolicy::OverwriteWhenFull),
					  "AsyncPolicy::OverwriteWhenFull is not currently supported when using "
					  "ThreadingPolicy::MultiThreadedAsync (OverwriteWhenFull is not currently "
					  "supported with multi-threaded asynchronous loggers)");
		static constexpr enum ThreadingPolicy threading_policy = ThreadingPolicy;
		static constexpr enum AsyncPolicy async_policy = AsyncPolicy;
	};

	/// @brief Concept requiring `T` is a `Policy` type, i.e., it provides a `static
	/// constexpr` member variable `threading_policy` of type `ThreadingPolicy` and a `static
	/// constexpr` member variable `async_policy` of type `AsyncPolicy`
	/// @ingroup logging
	/// @headerfile "Hyperion/logging/Config.h"
	template<typename T>
	concept PolicyType = requires() {
		T::threading_policy;
		requires concepts::Same<std::remove_cvref_t<decltype(T::threading_policy)>,
								ThreadingPolicy>;

		T::async_policy;
		requires concepts::Same<std::remove_cvref_t<decltype(T::async_policy)>, AsyncPolicy>;

		requires !(T::threading_policy == ThreadingPolicy::MultiThreadedAsync
				   && T::async_policy == AsyncPolicy::OverwriteWhenFull);
	};

	/// @brief Alias for the default logging policy
	/// @ingroup logging
	/// @headerfile "Hyperion/logging/Config.h"
	using DefaultPolicy = Policy<>;

	/// @brief Configuration type for configuring a logger's minimum `Level`
	///
	/// @tparam MinimumLevel - The minimum `Level` to use for the logger
	/// @ingroup logging
	/// @headerfile "Hyperion/logging/Config.h"
	template<Level MinimumLevel = Level::INFO>
	struct LoggingLevel {
		static constexpr Level minimum_level = MinimumLevel;
	};

	/// @brief Concept requiring `T` is a `Level` type, i.e., it provides a `static constexpr`
	/// member variable `minimum_level` of type `Level`
	/// @ingroup logging
	/// @headerfile "Hyperion/logging/Config.h"
	template<typename T>
	concept LoggingLevelType = requires() {
		T::minimum_level;
		requires concepts::Same<std::remove_cvref_t<decltype(T::minimum_level)>, Level>;
	};

	/// @brief Alias for the default logging level
	/// @ingroup logging
	/// @headerfile "Hyperion/logging/Config.h"
	using DefaultLevel = LoggingLevel<>;

	/// @brief Configuration type for configuring a logger's threading policy, potential async
	/// policy, minimum logging level, and potential async queue size.
	///
	/// @tparam PolicyType - The threading policy to use for the logger
	/// @tparam MinimumLevelType - The minimum logging level for the logger
	/// @tparam QueueSize - The size of the logging queue if the logger is asynchronous
	/// @ingroup logging
	/// @headerfile "Hyperion/logging/Config.h"
	template<PolicyType PolicyType = DefaultPolicy,
			 LoggingLevelType MinimumLevelType = DefaultLevel,
			 usize QueueSize = 1024> // NOLINT (magic numbers)
	struct Parameters {
		static constexpr auto threading_policy = PolicyType::threading_policy;
		static constexpr auto async_policy = PolicyType::async_policy;
		static constexpr auto minimum_level = MinimumLevelType::minimum_level;
		static constexpr usize queue_size = QueueSize;
	};

	/// @brief Concept requiring `T` is a `Parameters` type, i.e. it provides
	/// `static constexpr` members `threading_policy`, `async_policy`, `minimum_level`, and
	/// `queue_size` of types `ThreadingPolicy`, `AsyncPolicy`, `Level`, and `usize`,
	/// respectively.
	///
	/// @note `queue_size` may actually be any `concepts::Integral` type
	/// @ingroup logging
	/// @headerfile "Hyperion/logging/Config.h"
	template<typename T>
	concept ParametersType = requires() {
		T::threading_policy;
		requires concepts::Same<std::remove_cvref_t<decltype(T::threading_policy)>,
								ThreadingPolicy>;

		T::async_policy;
		requires concepts::Same<std::remove_cvref_t<decltype(T::async_policy)>, AsyncPolicy>;

		T::minimum_level;
		requires concepts::Same<std::remove_cvref_t<decltype(T::minimum_level)>, Level>;

		T::queue_size;
		requires concepts::Integral<std::remove_cvref_t<decltype(T::queue_size)>>;
	};

	/// @brief Alias for the default logging configuration parameters
	/// @ingroup logging
	/// @headerfile "Hyperion/logging/Config.h"
	using DefaultParameters = Parameters<>;

	static_assert(ParametersType<DefaultParameters>,
				  "DefaultParameters failing ParametersType requirements");
} // namespace hyperion::logging
