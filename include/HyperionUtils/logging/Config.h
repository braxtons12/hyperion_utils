/// @brief Logging configuration types
#pragma once

#include <cstddef>

#include "../Concepts.h"

namespace hyperion {

	/// @brief Used to indicate the desired logging level of the logger.
	enum class LogLevel : uint8_t
	{
		/// @brief General messages such as "log started" or "log closing"
		MESSAGE = 0,
		/// @brief Used for logging call traces
		TRACE = 1,
		/// @brief Used for general information reports
		INFO = 2,
		/// @brief Used for warnings and other more severe reports
		WARN = 3,
		/// @brief Used for system errors and other fatal or near-fatal reports
		ERROR = 4,
		/// @brief Disable logging.
		DISABLED = 5
	};

	/// @brief Used to indicate the behavior of the logger when
	/// queueing an entry for logging. This is the behavior the user will experience
	/// when calling `log` or one of the logging-level-specific methods.
	/// There are three policy types, each with differing levels of data preservation.
	///
	/// - `DropWhenFull`: The logger will return an error when the
	/// 				  logging queue is full and fail to log the given entry
	/// - `OverwriteWhenFull`: The logger's queue will act as a ring buffer,
	/// 					   and when full, entries not yet logged to disk will be
	/// 					   overwritten and discarded
	/// - `FlushWhenFull`: The logger will block when the logging queue is full
	/// 				   until the queue is empty again
	enum class LogPolicy : uint8_t
	{
		/// @brief The logger will return an error when the logging queue is full
		/// and fial to log the given entry
		DropWhenFull = 0,
		/// @brief The logger's queue will act as a ring buffer, and when full,
		/// entries not yet logged to disk will be overwritten and discarded
		OverwriteWhenFull = 1,
		/// @brief The logger will block when the loggign queue is full,
		/// until the queue is empty again
		FlushWhenFull = 2
	};

	/// @brief Wrapper type for `LogPolicy` for type-safe compile-time logger configuration
	///
	/// @tparam Policy - The `LogPolicy` to use for the logger
	template<LogPolicy Policy = LogPolicy::DropWhenFull>
	struct LoggerPolicy {
		static constexpr LogPolicy policy = Policy;
	};

	/// @brief Concept that requires `T` to be a `LoggerPolicy` type
	template<typename T>
	concept LoggerPolicyType = requires() {
		T::policy;
	};

	/// @brief Alias for the default logging policy
	using DefaultLogPolicy = LoggerPolicy<>;

	/// @brief Wrapper type for `LogLevel` for type-safe compile-time logger configuration
	///
	/// @tparam MinimumLevel - The `LogLevel` to use for the logger
	template<LogLevel MinimumLevel = LogLevel::INFO>
	struct LoggerLevel {
		static constexpr LogLevel minimum_level = MinimumLevel;
	};

	/// @brief Concept that requires `T` to be a `LoggerLevel` type
	template<typename T>
	concept LoggerLevelType = requires() {
		T::minimum_level;
	};

	/// @brief Alias for the default logging level
	using DefaultLogLevel = LoggerLevel<>;

	/// @brief Wrapper type for type-safe compile-time passing of logging configuration parameters
	///
	/// @tparam PolicyType - The policy to use for the logger
	/// @tparam MinimumLevelType - The minimum logging level for the logger
	template<LoggerPolicyType PolicyType = DefaultLogPolicy,
			 LoggerLevelType MinimumLevelType = DefaultLogLevel>
	struct LoggerParameters {
		static constexpr auto policy = PolicyType::policy;
		static constexpr auto minimum_level = MinimumLevelType::minimum_level;
	};

	/// @brief Concept that requires `T` to be a `LoggerParameters` type
	template<typename T>
	concept LoggerParametersType = requires() {
		T::policy;
		T::minimum_level;
	};

	/// @brief Alias for the default logging configuration parameters
	using DefaultLogParameters = LoggerParameters<>;

} // namespace hyperion
