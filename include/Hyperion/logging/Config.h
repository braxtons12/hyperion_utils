/// @file Config.h
/// @author Braxton Salyer <braxtonsalyer@gmail.com>
/// @brief Logging configuration types
/// @version 0.1
/// @date 2022-06-04
///
/// MIT License
/// @copyright Copyright (c) 2021 Braxton Salyer <braxtonsalyer@gmail.com>
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

namespace hyperion {

	/// @brief Used to configure the desired logging level of loggers and sinks.
	/// @ingroup logging
	/// @headerfile "Hyperion/logging/Config.h"
	enum class LogLevel : u8 {
		/// @brief General messages such as "log started" or "log closing"
		MESSAGE = 0,
		/// @brief Used for logging call traces
		TRACE,
		/// @brief Used for general information reports
		INFO,
		/// @brief Used for warnings and other more severe reports
		WARN,
		/// @brief Used for system errors and other fatal or near-fatal reports
		ERROR,
		/// @brief Disable logging.
		DISABLED
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
	/// `LogAsyncPolicy`
	/// @ingroup logging
	/// @headerfile "Hyperion/logging/Config.h"
	enum class LogThreadingPolicy : u8 {
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
	enum class LogAsyncPolicy : u8 {
		DropWhenFull = 0,
		OverwriteWhenFull,
		BlockWhenFull
	};

	/// @brief Configuration type for configuring a logger's threading and (potential) asynchronous
	/// policies
	///
	/// @tparam ThreadingPolicy - The `LogThreadingPolicy` to use for the logger
	/// @tparam AsyncPolicy - The `LogAsyncPolicy` to use for the logger
	/// @ingroup logging
	/// @headerfile "Hyperion/logging/Config.h"
	template<LogThreadingPolicy ThreadingPolicy = LogThreadingPolicy::SingleThreaded,
			 LogAsyncPolicy AsyncPolicy = LogAsyncPolicy::DropWhenFull>
	struct LoggerPolicy {
		static constexpr LogThreadingPolicy threading_policy = ThreadingPolicy;
		static constexpr LogAsyncPolicy async_policy = AsyncPolicy;
	};

	/// @brief Concept requiring `T` is a `LoggerPolicy` type, i.e., it provides a `static
	/// constexpr` member variable `threading_policy` of type `LogThreadingPolicy` and a `static
	/// constexpr` member variable `async_policy` of type `LogAsyncPolicy`
	/// @ingroup logging
	/// @headerfile "Hyperion/logging/Config.h"
	template<typename T>
	concept LoggerPolicyType = requires() {
		T::threading_policy;
		concepts::Same<std::remove_cvref_t<decltype(T::threading_policy)>, LogThreadingPolicy>;

		T::async_policy;
		concepts::Same<std::remove_cvref_t<decltype(T::async_policy)>, LogAsyncPolicy>;
	};

	/// @brief Alias for the default logging policy
	/// @ingroup logging
	/// @headerfile "Hyperion/logging/Config.h"
	using DefaultLogPolicy = LoggerPolicy<>;

	/// @brief Configuration type for configuring a logger's minimum `LogLevel`
	///
	/// @tparam MinimumLevel - The minimum `LogLevel` to use for the logger
	/// @ingroup logging
	/// @headerfile "Hyperion/logging/Config.h"
	template<LogLevel MinimumLevel = LogLevel::INFO>
	struct LoggerLevel {
		static constexpr LogLevel minimum_level = MinimumLevel;
	};

	/// @brief Concept requiring `T` is a `LoggerLevel` type, i.e., it provides a `static constexpr`
	/// member variable `minimum_level` of type `LogLevel`
	/// @ingroup logging
	/// @headerfile "Hyperion/logging/Config.h"
	template<typename T>
	concept LoggerLevelType = requires() {
		T::minimum_level;
		concepts::Same<std::remove_cvref_t<decltype(T::minimum_level)>, LogLevel>;
	};

	/// @brief Alias for the default logging level
	/// @ingroup logging
	/// @headerfile "Hyperion/logging/Config.h"
	using DefaultLogLevel = LoggerLevel<>;

	/// @brief Configuration type for configuring a logger's threading policy, potential async
	/// policy, minimum logging level, and potential async queue size.
	///
	/// @tparam PolicyType - The threading policy to use for the logger
	/// @tparam MinimumLevelType - The minimum logging level for the logger
	/// @tparam QueueSize - The size of the logging queue if the logger is asynchronous
	/// @ingroup logging
	/// @headerfile "Hyperion/logging/Config.h"
	template<LoggerPolicyType PolicyType = DefaultLogPolicy,
			 LoggerLevelType MinimumLevelType = DefaultLogLevel,
			 usize QueueSize = 1024> // NOLINT (magic numbers)
	struct LoggerParameters {
		static constexpr auto threading_policy = PolicyType::threading_policy;
		static constexpr auto async_policy = PolicyType::async_policy;
		static constexpr auto minimum_level = MinimumLevelType::minimum_level;
		static constexpr usize queue_size = QueueSize;
	};

	/// @brief Concept requiring `T` is a `LoggerParameters` type, i.e. it provides
	/// `static constexpr` members `threading_policy`, `async_policy`, `minimum_level`, and
	/// `queue_size` of types `LogThreadingPolicy`, `LogAsyncPolicy`, `LogLevel`, and `usize`,
	/// respectively.
	///
	/// @note `queue_size` may actually be any `concepts::Integral` type
	/// @ingroup logging
	/// @headerfile "Hyperion/logging/Config.h"
	template<typename T>
	concept LoggerParametersType = requires() {
		T::threading_policy;
		concepts::Same<std::remove_cvref_t<decltype(T::threading_policy)>, LogThreadingPolicy>;

		T::async_policy;
		concepts::Same<std::remove_cvref_t<decltype(T::async_policy)>, LogAsyncPolicy>;

		T::minimum_level;
		concepts::Same<std::remove_cvref_t<decltype(T::minimum_level)>, LogLevel>;

		T::queue_size;
		concepts::Integral<std::remove_cvref_t<decltype(T::queue_size)>>;
	};

	/// @brief Alias for the default logging configuration parameters
	/// @ingroup logging
	/// @headerfile "Hyperion/logging/Config.h"
	using DefaultLogParameters = LoggerParameters<>;

	static_assert(LoggerParametersType<DefaultLogParameters>,
				  "DefaultLogParameters failing LoggerParametersType requirements");
} // namespace hyperion
