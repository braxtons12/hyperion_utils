#pragma once

#include <atomic>
#include <chrono>
#include <filesystem>
#include <gsl/gsl>
#include <iostream>
#include <memory>
#include <thread>

#include "LockFreeQueue.h"
#include "fmtIncludes.h"

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
		LoggerError(LogErrorCategory type) noexcept { // NOLINT
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
	/// @tparam LogParameters - The parameters for how this logger should operate
	template<LoggerParametersType LogParameters = DefaultLogParameters>
	class Logger {
	  public:
		static constexpr LogPolicy POLICY = LogParameters::policy;
		static constexpr LogLevel MINIMUM_LEVEL = LogParameters::minimum_level;

		Logger()
			// clang-format off
			: m_message_thread([messages = m_messages, log_file_path = m_log_file_path]
					(const std::stop_token& stop)
				{
					auto log_file = fmt::output_file(log_file_path);
					while(!stop.stop_requested()) {
						if(auto res = messages->read()) {
							auto message = res.unwrap();
							log_file.print("{}", message);
							//fmt::print("{}", message);
						}
					}
					log_file.close();
				})
		{
				//clang-format on
		}
		explicit Logger(const std::string& root_name) // NOLINT
			: m_root_name(root_name), m_log_file_path(create_log_file_path()),
			  // clang-format off
			  m_message_thread([messages = m_messages, log_file_path = m_log_file_path]
					(const std::stop_token& stop)
				{
					auto log_file = fmt::output_file(log_file_path);
					while(!stop.stop_requested()) {
						if(auto res = messages->read()) {
							auto message = res.unwrap();
							log_file.print("{}", message);
							//fmt::print("{}", message);
						}
					}
					log_file.close();
				})
		{
				//clang-format on
		}
		explicit Logger(std::string&& root_name)
			: m_root_name(root_name), m_log_file_path(create_log_file_path()),
			  // clang-format off
			  m_message_thread([messages = m_messages, log_file_path = m_log_file_path]
					(const std::stop_token& stop)
				{
					auto log_file = fmt::output_file(log_file_path);
					while(!stop.stop_requested()) {
						if(auto res = messages->read()) {
							auto message = res.unwrap();
							log_file.print("{}", message);
							//fmt::print("{}", message);
						}
					}
					log_file.close();
				})
		{
				//clang-format on
		}
		Logger(const std::string& root_name, const std::string& directory_name) // NOLINT
			: m_root_name(root_name), m_directory_name(directory_name),
			  m_log_file_path(create_log_file_path()),
			  // clang-format off
			  m_message_thread([messages = m_messages, log_file_path = m_log_file_path]
					(const std::stop_token& stop)
				{
					auto log_file = fmt::output_file(log_file_path);
					while(!stop.stop_requested()) {
						if(auto res = messages->read()) {
							auto message = res.unwrap();
							log_file.print("{}", message);
							//fmt::print("{}", message);
						}
					}
					log_file.close();
				})
		{
				//clang-format on
		}
		Logger(const std::string& root_name, std::string&& directory_name) // NOLINT
			: m_root_name(root_name), m_directory_name(directory_name),
			  m_log_file_path(create_log_file_path()),
			  // clang-format off
			  m_message_thread([messages = m_messages, log_file_path = m_log_file_path]
					(const std::stop_token& stop)
				{
					auto log_file = fmt::output_file(log_file_path);
					while(!stop.stop_requested()) {
						if(auto res = messages->read()) {
							auto message = res.unwrap();
							log_file.print("{}", message);
							//fmt::print("{}", message);
						}
					}
					log_file.close();
				})
		{
				//clang-format on
		}
		Logger(std::string&& root_name, const std::string& directory_name) // NOLINT
			: m_root_name(root_name), m_directory_name(directory_name),
			  m_log_file_path(create_log_file_path()),
			  // clang-format off
			  m_message_thread([messages = m_messages, log_file_path = m_log_file_path]
			  		(const std::stop_token& stop)
				{
					auto log_file = fmt::output_file(log_file_path);
					while(!stop.stop_requested()) {
						if(auto res = messages->read()) {
							auto message = res.unwrap();
							log_file.print("{}", message);
							//fmt::print("{}", message);
						}
					}
					log_file.close();
				})
		{
				//clang-format on
		}
		Logger(std::string&& root_name, std::string&& directory_name)
			: m_root_name(root_name), m_directory_name(directory_name),
			  m_log_file_path(create_log_file_path()),
			  // clang-format off
			  m_message_thread([messages = m_messages, log_file_path = m_log_file_path]
			  		(const std::stop_token& stop)
				{
					auto log_file = fmt::output_file(log_file_path);
					while(!stop.stop_requested()) {
						if(auto res = messages->read()) {
							auto message = res.unwrap();
							log_file.print("{}", message);
							//fmt::print("{}", message);
						}
					}
					log_file.close();
				})
		{
				//clang-format on
		}
		Logger(const Logger& logger) noexcept = delete;
		Logger(Logger&& logger) noexcept = default;

		~Logger() noexcept = default;

		template<LogLevel Level, typename S, typename... Args, typename Char = fmt::char_t<S>>
		inline auto log(Option<size_t> thread_id, const S& format_string, Args&&... args) noexcept
			-> Result<bool, LoggerError> {
			if constexpr(Level >= MINIMUM_LEVEL && MINIMUM_LEVEL != LogLevel::DISABLED) {
				if constexpr(POLICY == LogPolicy::DropWhenFull) {
					return log_dropping<Level>(thread_id, format_string, args...);
				}
				else if constexpr(POLICY == LogPolicy::FlushWhenFull) {
					return log_flushing<Level>(thread_id, format_string, args...);
				}
				else {
					log_overwriting<Level>(thread_id, format_string, args...);
					return Ok(true);
				}
			}
			else {
				ignore(format_string, args...);
				return Err(LoggerError(LogErrorCategory::LogLevelError));
			}
		}

		template<typename S, typename... Args, typename Char = fmt::char_t<S>>
		inline auto
		message(Option<size_t> thread_id, const S& format_string, Args&&... args) noexcept {
			return log<LogLevel::MESSAGE>(thread_id, format_string, args...);
		}

		template<typename S, typename... Args, typename Char = fmt::char_t<S>>
		inline auto
		trace(Option<size_t> thread_id, const S& format_string, Args&&... args) noexcept {
			return log<LogLevel::TRACE>(thread_id, format_string, args...);
		}

		template<typename S, typename... Args, typename Char = fmt::char_t<S>>
		inline auto
		info(Option<size_t> thread_id, const S& format_string, Args&&... args) noexcept {
			return log<LogLevel::INFO>(thread_id, format_string, args...);
		}

		template<typename S, typename... Args, typename Char = fmt::char_t<S>>
		inline auto
		warn(Option<size_t> thread_id, const S& format_string, Args&&... args) noexcept {
			return log<LogLevel::WARN>(thread_id, format_string, args...);
		}

		template<typename S, typename... Args, typename Char = fmt::char_t<S>>
		inline auto
		error(Option<size_t> thread_id, const S& format_string, Args&&... args) noexcept {
			return log<LogLevel::ERROR>(thread_id, format_string, args...);
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

		std::shared_ptr<LockFreeQueue<std::string, get_queue_policy()>> m_messages
			= std::make_shared<LockFreeQueue<std::string, get_queue_policy()>>();
		std::string m_root_name = "HyperionLog"s;
		std::string m_directory_name = "Hyperion"s;
		std::string m_log_file_path = create_log_file_path();
		std::jthread m_message_thread;

		static constexpr fmt::text_style MESSAGE_STYLE = fmt::fg(fmt::color::white);
		static constexpr fmt::text_style TRACE_STYLE = fmt::fg(fmt::color::steel_blue);
		static constexpr fmt::text_style INFO_STYLE
			= fmt::fg(fmt::color::light_green) | fmt::emphasis::italic;
		static constexpr fmt::text_style WARN_STYLE
			= fmt::fg(fmt::color::orange) | fmt::emphasis::bold;
		static constexpr fmt::text_style ERROR_STYLE
			= fmt::fg(fmt::color::red) | fmt::emphasis::bold;

		[[nodiscard]] inline static auto create_time_stamp() noexcept -> std::string {
			return fmt::format("[{:%Y-%m-%d|%H-%M-%S}]", fmt::localtime(std::time(nullptr)));
		}

		[[nodiscard]] inline auto create_log_file_path() const -> std::string {
			auto temp_dir = std::filesystem::temp_directory_path();
			temp_dir.append(m_directory_name);
			std::filesystem::create_directory(temp_dir);
			auto time_string = create_time_stamp();
			temp_dir.append(time_string + " "s + m_root_name);
			temp_dir.replace_extension("log");
			return temp_dir;
		}

		template<LogLevel Level, typename S, typename... Args, typename Char = fmt::char_t<S>>
		inline auto
		log_dropping(Option<size_t> thread_id, const S& format_string, Args&&... args) noexcept
			-> Result<bool, LoggerError>
		requires(POLICY == LogPolicy::DropWhenFull) {
			const auto timestamp = create_time_stamp();
			const auto entry = fmt::format(format_string, args...);
			const auto id = thread_id.is_some() ?
								  thread_id.unwrap() :
								  std::hash<std::thread::id>()(std::this_thread::get_id());

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
											 "{0}  [Thread ID: {1}] [{2}]: {3}\n",
											 timestamp,
											 id,
											 log_type,
											 entry);

			return m_messages->push(logline).template map_err<LoggerError>(
				[](const QueueError& error) { return LoggerError(error); });
		}

		template<LogLevel Level, typename S, typename... Args, typename Char = fmt::char_t<S>>
		inline auto
		log_overwriting(Option<size_t> thread_id, const S& format_string, Args&&... args) noexcept
			-> void requires(POLICY == LogPolicy::OverwriteWhenFull) {
			const auto timestamp = create_time_stamp();
			const auto entry = fmt::format(format_string, args...);
			const auto id = thread_id.is_some() ?
								  thread_id.unwrap() :
								  std::hash<std::thread::id>()(std::this_thread::get_id());

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
											 "{0}  [Thread ID: {1}] [{2}]: {3}\n",
											 timestamp,
											 id,
											 log_type,
											 entry);

			m_messages->push(logline);
		}

		template<LogLevel Level, typename S, typename... Args, typename Char = fmt::char_t<S>>
		inline auto
		log_flushing(Option<size_t> thread_id, const S& format_string, Args&&... args) noexcept
			-> Result<bool, LoggerError>
		requires(POLICY == LogPolicy::FlushWhenFull) {
			const auto timestamp = create_time_stamp();
			const auto entry = fmt::format(format_string, args...);
			const auto id = thread_id.is_some() ?
								  thread_id.unwrap() :
								  std::hash<std::thread::id>()(std::this_thread::get_id());

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
											 "{0}  [Thread ID: {1}] [{2}]: {3}\n",
											 timestamp,
											 id,
											 log_type,
											 entry);

			if(m_messages->full()) {
				while(!m_messages->empty()) {
				}
			}

			return m_messages->push(logline).template map_err<LoggerError>(
				[](const QueueError& error) { return LoggerError(error); });
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

	IGNORE_UNUSED_TEMPLATES_START

	template<LoggerParametersType LogParameters = DefaultLogParameters>
	[[nodiscard]] static auto get_global_logger() -> Logger<LogParameters>& {
		HYPERION_NO_DESTROY static Logger<LogParameters> GLOBAL_LOGGER{};
		return GLOBAL_LOGGER;
	}

	template<LoggerParametersType LogParameters = DefaultLogParameters>
	inline static auto initialize_global_logger(const std::string& root_name) -> void {
		get_global_logger<LogParameters>() = Logger<LogParameters>(root_name);
	}

	template<LoggerParametersType LogParameters = DefaultLogParameters>
	inline static auto initialize_global_logger(std::string&& root_name) -> void {
		get_global_logger<LogParameters>() = Logger<LogParameters>(root_name);
	}

	template<LoggerParametersType LogParameters = DefaultLogParameters>
	inline static auto
	initialize_global_logger(const std::string& root_name, const std::string& directory_name)
		-> void {
		get_global_logger<LogParameters>() = Logger<LogParameters>(root_name, directory_name);
	}

	template<LoggerParametersType LogParameters = DefaultLogParameters>
	inline static auto
	initialize_global_logger(std::string&& root_name, const std::string& directory_name) -> void {
		get_global_logger<LogParameters>() = Logger<LogParameters>(root_name, directory_name);
	}

	template<LoggerParametersType LogParameters = DefaultLogParameters>
	inline static auto
	initialize_global_logger(const std::string& root_name, std::string&& directory_name) -> void {
		get_global_logger<LogParameters>() = Logger<LogParameters>(root_name, directory_name);
	}

	template<LoggerParametersType LogParameters = DefaultLogParameters>
	inline static auto
	initialize_global_logger(std::string&& root_name, std::string&& directory_name) -> void {
		get_global_logger<LogParameters>() = Logger<LogParameters>(root_name, directory_name);
	}

	template<LoggerParametersType LogParameters,
			 typename S,
			 typename... Args,
			 typename Char = fmt::char_t<S>>
	inline auto MESSAGE(Option<size_t> thread_id, const S& format_string, Args&&... args) noexcept {
		return get_global_logger<LogParameters>().message(thread_id, format_string, args...);
	}

	template<LoggerParametersType LogParameters,
			 typename S,
			 typename... Args,
			 typename Char = fmt::char_t<S>>
	inline auto TRACE(Option<size_t> thread_id, const S& format_string, Args&&... args) noexcept {
		return get_global_logger<LogParameters>().trace(thread_id, format_string, args...);
	}

	template<LoggerParametersType LogParameters,
			 typename S,
			 typename... Args,
			 typename Char = fmt::char_t<S>>
	inline auto INFO(Option<size_t> thread_id, const S& format_string, Args&&... args) noexcept {
		return get_global_logger<LogParameters>().info(thread_id, format_string, args...);
	}

	template<LoggerParametersType LogParameters,
			 typename S,
			 typename... Args,
			 typename Char = fmt::char_t<S>>
	inline auto WARN(Option<size_t> thread_id, const S& format_string, Args&&... args) noexcept {
		return get_global_logger<LogParameters>().warn(thread_id, format_string, args...);
	}

	template<LoggerParametersType LogParameters,
			 typename S,
			 typename... Args,
			 typename Char = fmt::char_t<S>>
	inline auto ERROR(Option<size_t> thread_id, const S& format_string, Args&&... args) noexcept {
		return get_global_logger<LogParameters>().error(thread_id, format_string, args...);
	}

	IGNORE_UNUSED_TEMPLATES_STOP

} // namespace hyperion::utils
