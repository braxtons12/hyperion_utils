#pragma once

#include <atomic>
#include <chrono>
#include <filesystem>
#include <fmt/chrono.h>
#include <fmt/color.h>
#include <fmt/compile.h>
#include <fmt/format.h>
#include <fmt/os.h>
#include <gsl/gsl>
#include <iostream>
#include <memory>
#include <thread>

#include "LockFreeQueue.h"

namespace hyperion::utils {

	/// @brief Used to indicate the desired logging level of the logger.
	enum class LogLevel : size_t
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
	enum class LogPolicy : size_t
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

	/// @brief Possible Error categories that can occur when using the logger
	enum class LogErrorCategory : size_t
	{
		/// @brief failed to queue the entry for logging
		QueueingError = 0,
		/// @brief the requested log level for the entry is
		/// lower than the minium level for the logger
		LogLevelError = 1,
	};

	/// @brief Alias for the Error type we might recieve from the internal queue
	using QueueError = LockFreeQueueError<LockFreeQueueErrorCategory::QueueIsFull>;

	IGNORE_PADDING_START
	IGNORE_WEAK_VTABLES_START

	/// @brief Error type for communicating logger errors
	/// Errors can occur when an entry fails to be queued
	/// or an entry is passed in at an invalid logging level
	class LoggerError final : public Error {
	  public:
		LoggerError() noexcept {
			Error::m_message = "Error writing to logging queue"s;
		}
		LoggerError(LogErrorCategory type) noexcept {
			if(type == LogErrorCategory::QueueingError) {
				Error::m_message = "Error writing to logging queue"s;
			}
			else {
				Error::m_message = "Logging Level of this Logger is higher than the given entry"s;
			}
		}
		LoggerError(const QueueError& error) noexcept { // NOLINT
			Error::m_message = "Error writing to logging queue"s;
			Error::m_source = std::make_shared<Error>(error);
			Error::m_has_source = true;
		}
		LoggerError(QueueError&& error) noexcept { // NOLINT
			Error::m_message = "Error writing to logging queue"s;
			Error::m_source = std::make_shared<Error>(error);
			Error::m_has_source = true;
		}
		LoggerError(const LoggerError& error) noexcept = default;
		LoggerError(LoggerError& error) noexcept = default;
		~LoggerError() noexcept final = default;

		auto operator=(const LoggerError& error) noexcept -> LoggerError& = default;
		auto operator=(LoggerError&& error) noexcept -> LoggerError& = default;
	};
	IGNORE_WEAK_VTABLES_STOP

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

	/// @brief Hyperion logging type for formatted logging.
	/// Uses fmtlib/fmt for entry formatting and stylizing
	///
	/// @tparam LogParameters
	template<LoggerParametersType LogParameters = DefaultLogParameters>
	class Logger {
	  public:
		static constexpr LogPolicy POLICY = LogParameters::policy;
		static constexpr LogLevel MINIMUM_LEVEL = LogParameters::minimum_level;

		Logger() noexcept
			: m_log_file(std::make_unique<fmt::ostream>(fmt::output_file(m_log_file_path))),
			  m_message_thread([&]() {
				  while(!m_exit_signal->load()) {
					  if(auto res = m_messages.read()) {
						  auto message = res.unwrap();
						  m_log_file->print("{}", message);
					  }
				  }
			  }) {
		}
		explicit Logger(const std::string& root_name) noexcept // NOLINT
			: m_root_name(root_name), m_log_file_path(create_log_file_path()),
			  m_log_file(std::make_unique<fmt::ostream>(fmt::output_file(m_log_file_path))),
			  m_message_thread([&]() {
				  while(!m_exit_signal->load()) {
					  if(auto res = m_messages.read()) {
						  auto message = res.unwrap();
						  m_log_file->print("{}", message);
					  }
				  }
			  }) {
		}
		explicit Logger(std::string&& root_name) noexcept
			: m_root_name(root_name), m_log_file_path(create_log_file_path()),
			  m_log_file(std::make_unique<fmt::ostream>(fmt::output_file(m_log_file_path))),
			  m_message_thread([&]() {
				  while(!m_exit_signal->load()) {
					  if(auto res = m_messages.read()) {
						  auto message = res.unwrap();
						  m_log_file->print("{}", message);
					  }
				  }
			  }) {
		}
		Logger(const std::string& root_name, const std::string& directory_name) noexcept // NOLINT
			: m_root_name(root_name), m_directory_name(directory_name),
			  m_log_file_path(create_log_file_path()),
			  m_log_file(std::make_unique<fmt::ostream>(fmt::output_file(m_log_file_path))),
			  m_message_thread([&]() {
				  while(!m_exit_signal->load()) {
					  if(auto res = m_messages.read()) {
						  auto message = res.unwrap();
						  m_log_file->print("{}", message);
					  }
				  }
			  }) {
		}
		Logger(const std::string& root_name, std::string&& directory_name) noexcept // NOLINT
			: m_root_name(root_name), m_directory_name(directory_name),
			  m_log_file_path(create_log_file_path()),
			  m_log_file(std::make_unique<fmt::ostream>(fmt::output_file(m_log_file_path))),
			  m_message_thread([&]() {
				  while(!m_exit_signal->load()) {
					  if(auto res = m_messages.read()) {
						  auto message = res.unwrap();
						  m_log_file->print("{}", message);
					  }
				  }
			  }) {
		}
		Logger(std::string&& root_name, const std::string& directory_name) noexcept // NOLINT
			: m_root_name(root_name), m_directory_name(directory_name),
			  m_log_file_path(create_log_file_path()),
			  m_log_file(std::make_unique<fmt::ostream>(fmt::output_file(m_log_file_path))),
			  m_message_thread([&]() {
				  while(!m_exit_signal->load()) {
					  if(auto res = m_messages.read()) {
						  auto message = res.unwrap();
						  m_log_file->print("{}", message);
					  }
				  }
			  }) {
		}
		Logger(std::string&& root_name, std::string&& directory_name) noexcept
			: m_root_name(root_name), m_directory_name(directory_name),
			  m_log_file_path(create_log_file_path()),
			  m_log_file(std::make_unique<fmt::ostream>(fmt::output_file(m_log_file_path))),
			  m_message_thread([&]() {
				  while(!m_exit_signal->load()) {
					  if(auto res = m_messages.read()) {
						  auto message = res.unwrap();
						  m_log_file->print("{}", message);
					  }
				  }
			  }) {
		}
		~Logger() noexcept {
			m_exit_signal->store(true);
			if(m_message_thread.joinable()) {
				m_message_thread.join();
			}
			m_log_file->close();
		}

		template<LogLevel Level, typename S, typename... Args, typename Char = fmt::char_t<S>>
		inline auto
		log(const S& format_string, Args&&... args) noexcept -> Result<bool, LoggerError>
		requires(POLICY == LogPolicy::DropWhenFull || POLICY == LogPolicy::FlushWhenFull) {
			if constexpr(Level >= MINIMUM_LEVEL && MINIMUM_LEVEL != LogLevel::DISABLED) {
				if constexpr(POLICY == LogPolicy::DropWhenFull) {
					return log_dropping<Level>(format_string, args...);
				}
				else {
					return log_flushing<Level>(format_string, args...);
				}
			}
			else {
				ignore(format_string, args...);
				return Err(LoggerError(LogErrorCategory::LogLevelError));
			}
		}

		template<LogLevel Level, typename S, typename... Args, typename Char = fmt::char_t<S>>
		inline constexpr auto log(const S& format_string, Args&&... args) noexcept
			-> void requires(POLICY == LogPolicy::OverwriteWhenFull) {
			if constexpr(Level >= MINIMUM_LEVEL && MINIMUM_LEVEL != LogLevel::DISABLED) {
				log_overwriting<Level>(format_string, args...);
			}
			else {
				ignore(format_string, args...);
			}
		}

		template<typename S, typename... Args, typename Char = fmt::char_t<S>>
		inline auto message(const S& format_string, Args&&... args) noexcept {
			return log<LogLevel::MESSAGE>(format_string, args...);
		}

		template<typename S, typename... Args, typename Char = fmt::char_t<S>>
		inline auto trace(const S& format_string, Args&&... args) noexcept {
			return log<LogLevel::TRACE>(format_string, args...);
		}

		template<typename S, typename... Args, typename Char = fmt::char_t<S>>
		inline auto info(const S& format_string, Args&&... args) noexcept {
			return log<LogLevel::INFO>(format_string, args...);
		}

		template<typename S, typename... Args, typename Char = fmt::char_t<S>>
		inline auto warn(const S& format_string, Args&&... args) noexcept {
			return log<LogLevel::WARN>(format_string, args...);
		}

		template<typename S, typename... Args, typename Char = fmt::char_t<S>>
		inline auto error(const S& format_string, Args&&... args) noexcept {
			return log<LogLevel::ERROR>(format_string, args...);
		}

		auto operator=(const Logger& logger) noexcept -> Logger& = delete;
		auto operator=(Logger&& logger) noexcept -> Logger& = default;

	  private:
		[[nodiscard]] inline static constexpr auto get_queue_policy() noexcept -> QueuePolicy {
			if constexpr(POLICY == LogPolicy::DropWhenFull || POLICY == LogPolicy::FlushWhenFull) {
				return QueuePolicy::ErrWhenFull;
			}
			else {
				return QueuePolicy::OverwriteWhenFull;
			}
		}

		LockFreeQueue<std::string, get_queue_policy()> m_messages
			= LockFreeQueue<std::string, get_queue_policy()>();
		std::string m_root_name = "HyperionLog"s;
		std::string m_directory_name = "Hyperion"s;
		std::string m_log_file_path = create_log_file_path();
		std::unique_ptr<fmt::ostream> m_log_file = nullptr;
		std::thread m_message_thread;
		std::unique_ptr<std::atomic_bool> m_exit_signal = std::make_unique<std::atomic_bool>(false);

		static constexpr fmt::text_style MESSAGE_STYLE = fmt::fg(fmt::color::white);
		static constexpr fmt::text_style TRACE_STYLE = fmt::fg(fmt::color::steel_blue);
		static constexpr fmt::text_style INFO_STYLE
			= fmt::fg(fmt::color::light_green) | fmt::emphasis::italic;
		static constexpr fmt::text_style WARN_STYLE
			= fmt::fg(fmt::color::orange) | fmt::emphasis::bold;
		static constexpr fmt::text_style ERROR_STYLE
			= fmt::fg(fmt::color::red) | fmt::emphasis::bold;

		[[nodiscard]] inline static auto create_time_stamp() noexcept -> std::string {
			return fmt::format(FMT_COMPILE("[{:%Y-%m-%d::%H-%M-%S}] "), std::time(nullptr));
		}

		[[nodiscard]] inline auto create_log_file_path() const noexcept -> std::string {
			auto temp_dir = std::filesystem::temp_directory_path();
			temp_dir.append(m_directory_name);
			auto time_string = create_time_stamp();
			temp_dir.append(time_string);
			temp_dir.append(m_root_name);
			temp_dir.replace_extension("log");
			return temp_dir;
		}

		template<LogLevel Level, typename S, typename... Args, typename Char = fmt::char_t<S>>
		inline auto
		log_dropping(const S& format_string, Args&&... args) noexcept -> Result<bool, LoggerError>
		requires(POLICY == LogPolicy::DropWhenFull) {
			const auto timestamp = create_time_stamp();
			const auto entry = fmt::format(format_string, args...);
			std::string log_type;
			if constexpr(Level == LogLevel::MESSAGE) {
				log_type = "MESSAGE"s;
			}
			else if constexpr(Level == LogLevel::TRACE) {
				log_type = "TRACE"s;
			}
			else if constexpr(Level == LogLevel::INFO) {
				log_type = "INFO"s;
			}
			else if constexpr(Level == LogLevel::WARN) {
				log_type = "WARN"s;
			}
			else if constexpr(Level == LogLevel::ERROR) {
				log_type = "ERROR"s;
			}
			const auto logline = fmt::format(MESSAGE_STYLE,
											 FMT_COMPILE("{0} {1}: {2}\n"),
											 timestamp,
											 log_type,
											 entry);
			return m_messages.push(logline).template map_err<LoggerError>(
				[](const QueueError& error) { return LoggerError(error); });
		}

		template<LogLevel Level, typename S, typename... Args, typename Char = fmt::char_t<S>>
		inline auto log_overwriting(const S& format_string, Args&&... args) noexcept
			-> void requires(POLICY == LogPolicy::OverwriteWhenFull) {
			const auto timestamp = create_time_stamp();
			const auto entry = fmt::format(format_string, args...);
			std::string log_type;
			if constexpr(Level == LogLevel::MESSAGE) {
				log_type = "MESSAGE"s;
			}
			else if constexpr(Level == LogLevel::TRACE) {
				log_type = "TRACE"s;
			}
			else if constexpr(Level == LogLevel::INFO) {
				log_type = "INFO"s;
			}
			else if constexpr(Level == LogLevel::WARN) {
				log_type = "WARN"s;
			}
			else if constexpr(Level == LogLevel::ERROR) {
				log_type = "ERROR"s;
			}
			const auto logline = fmt::format(MESSAGE_STYLE,
											 FMT_COMPILE("{0} {1}: {2}\n"),
											 timestamp,
											 log_type,
											 entry);

			m_messages.push(logline);
		}

		template<LogLevel Level, typename S, typename... Args, typename Char = fmt::char_t<S>>
		inline auto
		log_flushing(const S& format_string, Args&&... args) noexcept -> Result<bool, LoggerError>
		requires(POLICY == LogPolicy::FlushWhenFull) {
			if(m_messages.full()) {
				while(!m_messages.empty()) {
				}
			}
			return log_dropping<Level>(format_string, args...);
		}
	};

	IGNORE_WEAK_VTABLES_START
	class LoggerInitError final : public Error {
	  public:
		LoggerInitError() noexcept {
			Error::m_message = "Global logger already initialized"s;
		}
		LoggerInitError(const LoggerInitError& error) noexcept = default;
		LoggerInitError(LoggerInitError& error) noexcept = default;
		~LoggerInitError() noexcept final = default;

		auto operator=(const LoggerInitError& error) noexcept -> LoggerInitError& = default;
		auto operator=(LoggerInitError&& error) noexcept -> LoggerInitError& = default;
	};
	IGNORE_WEAK_VTABLES_STOP
	IGNORE_PADDING_STOP

	template<LoggerParametersType LogParameters = DefaultLogParameters>
	static std::atomic_bool GLOBAL_LOGGER_INITIALIZED;

	template<LoggerParametersType LogParameters = DefaultLogParameters>
	static Logger<LogParameters> GLOBAL_LOGGER;

	template<LoggerParametersType LogParameters = DefaultLogParameters>
	[[nodiscard]] inline static auto
	initialize_global_logger() noexcept -> Result<bool, LoggerInitError> {
		auto initialized = false;
		if(GLOBAL_LOGGER_INITIALIZED<LogParameters>.compare_and_exchange_strong(
			   initialized,
			   true,
			   std::memory_order_seq_cst))
		{
			GLOBAL_LOGGER<LogParameters> = std::move(Logger<LogParameters>());
			return Ok(true);
		}
		else {
			return Err(LoggerInitError());
		}
	}

	template<LoggerParametersType LogParameters = DefaultLogParameters>
	[[nodiscard]] inline static auto initialize_global_logger(const std::string& root_name) noexcept
		-> Result<bool, LoggerInitError> {
		auto initialized = false;
		if(GLOBAL_LOGGER_INITIALIZED<LogParameters>.compare_and_exchange_strong(
			   initialized,
			   true,
			   std::memory_order_seq_cst))
		{
			GLOBAL_LOGGER<LogParameters> = std::move(Logger<LogParameters>(root_name));
			return Ok(true);
		}
		else {
			return Err(LoggerInitError());
		}
	}

	template<LoggerParametersType LogParameters = DefaultLogParameters>
	[[nodiscard]] inline static auto
	initialize_global_logger(std::string&& root_name) noexcept -> Result<bool, LoggerInitError> {
		auto initialized = false;
		if(GLOBAL_LOGGER_INITIALIZED<LogParameters>.compare_and_exchange_strong(
			   initialized,
			   true,
			   std::memory_order_seq_cst))
		{
			GLOBAL_LOGGER<LogParameters> = std::move(Logger<LogParameters>(root_name));
			return Ok(true);
		}
		else {
			return Err(LoggerInitError());
		}
	}

	template<LoggerParametersType LogParameters = DefaultLogParameters>
	[[nodiscard]] inline static auto
	initialize_global_logger(const std::string& root_name,
							 const std::string& directory_name) noexcept
		-> Result<bool, LoggerInitError> {
		auto initialized = false;
		if(GLOBAL_LOGGER_INITIALIZED<LogParameters>.compare_and_exchange_strong(
			   initialized,
			   true,
			   std::memory_order_seq_cst))
		{
			GLOBAL_LOGGER<LogParameters> = std::move(
				Logger<LogParameters>(root_name, directory_name));
			return Ok(true);
		}
		else {
			return Err(LoggerInitError());
		}
	}

	template<LoggerParametersType LogParameters = DefaultLogParameters>
	[[nodiscard]] inline static auto
	initialize_global_logger(std::string&& root_name, const std::string& directory_name) noexcept
		-> Result<bool, LoggerInitError> {
		auto initialized = false;
		if(GLOBAL_LOGGER_INITIALIZED<LogParameters>.compare_and_exchange_strong(
			   initialized,
			   true,
			   std::memory_order_seq_cst))
		{
			GLOBAL_LOGGER<LogParameters> = std::move(
				Logger<LogParameters>(root_name, directory_name));
			return Ok(true);
		}
		else {
			return Err(LoggerInitError());
		}
	}

	template<LoggerParametersType LogParameters = DefaultLogParameters>
	[[nodiscard]] inline static auto
	initialize_global_logger(const std::string& root_name, std::string&& directory_name) noexcept
		-> Result<bool, LoggerInitError> {
		auto initialized = false;
		if(GLOBAL_LOGGER_INITIALIZED<LogParameters>.compare_and_exchange_strong(
			   initialized,
			   true,
			   std::memory_order_seq_cst))
		{
			GLOBAL_LOGGER<LogParameters> = std::move(
				Logger<LogParameters>(root_name, directory_name));
			return Ok(true);
		}
		else {
			return Err(LoggerInitError());
		}
	}

	template<LoggerParametersType LogParameters = DefaultLogParameters>
	[[nodiscard]] inline static auto
	initialize_global_logger(std::string&& root_name, std::string&& directory_name) noexcept
		-> Result<bool, LoggerInitError> {
		auto initialized = false;
		if(GLOBAL_LOGGER_INITIALIZED<LogParameters>.compare_and_exchange_strong(
			   initialized,
			   true,
			   std::memory_order_seq_cst))
		{
			GLOBAL_LOGGER<LogParameters> = std::move(
				Logger<LogParameters>(root_name, directory_name));
			return Ok(true);
		}
		else {
			return Err(LoggerInitError());
		}
	}

	template<LoggerParametersType LogParameters,
			 typename S,
			 typename... Args,
			 typename Char = fmt::char_t<S>>
	inline auto MESSAGE(const S& format_string, Args&&... args) noexcept {
		return GLOBAL_LOGGER<LogParameters>.template message<S, Args..., Char>(format_string,
																			   args...);
	}

	template<LoggerParametersType LogParameters,
			 typename S,
			 typename... Args,
			 typename Char = fmt::char_t<S>>
	inline auto TRACE(const S& format_string, Args&&... args) noexcept {
		return GLOBAL_LOGGER<LogParameters>.template trace<S, Args..., Char>(format_string,
																			 args...);
	}

	template<LoggerParametersType LogParameters,
			 typename S,
			 typename... Args,
			 typename Char = fmt::char_t<S>>
	inline auto INFO(const S& format_string, Args&&... args) noexcept {
		return GLOBAL_LOGGER<LogParameters>.template info<S, Args..., Char>(format_string, args...);
	}

	template<LoggerParametersType LogParameters,
			 typename S,
			 typename... Args,
			 typename Char = fmt::char_t<S>>
	inline auto WARN(const S& format_string, Args&&... args) noexcept {
		return GLOBAL_LOGGER<LogParameters>.template warn<S, Args..., Char>(format_string, args...);
	}

	template<LoggerParametersType LogParameters,
			 typename S,
			 typename... Args,
			 typename Char = fmt::char_t<S>>
	inline auto ERROR(const S& format_string, Args&&... args) noexcept {
		return GLOBAL_LOGGER<LogParameters>.template error<S, Args..., Char>(format_string,
																			 args...);
	}
} // namespace hyperion::utils
