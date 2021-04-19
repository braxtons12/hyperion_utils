/// @brief Hyperion logging facilities.
///
/// Hyperion's logging facilities are robust and composable.
/// Behavioral (Policy) configuration is configurable at compile time with via template parameters,
/// and output configuration is configurable by supplying the desired `Sink`s on creation
#pragma once

#include <atomic>
#include <chrono>
#include <filesystem>
#include <gsl/gsl>
#include <iostream>
#include <memory>
#include <system_error>
#include <thread>

#include "BasicTypes.h"
#include "LockFreeQueue.h"
#include "Logger.h"
#include "logging/Config.h"
#include "logging/Entry.h"
#include "logging/Sink.h"
#include "logging/fmtIncludes.h"

namespace hyperion {

	/// @brief Possible Error categories that can occur when using the logger
	enum class LogErrorType : u8
	{
		/// @brief No Error occurred
		Success = 0,
		/// @brief failed to queue the entry for logging
		QueueingError = 1,
		/// @brief the requested log level for the entry is
		/// lower than the minium level for the logger
		LogLevelError = 2,
	};

	/// @brief Alias for the Error type we might recieve from the internal queue
	using QueueError = LockFreeQueueError;

	IGNORE_PADDING_START
	IGNORE_WEAK_VTABLES_START
	/// @brief `std::error_category` type for `LogErrorType`s. Provides error catgory functionality
	/// for `LogErrorType`s
	struct LoggerErrorCategory final : public std::error_category {
	  public:
		/// @brief Default Constructor
		LoggerErrorCategory() noexcept = default;
		/// @brief Copy Constructor. Deleted. Error Category types are singletons
		LoggerErrorCategory(const LoggerErrorCategory&) = delete;
		/// @brief Move Constructor. Deleted. Error Category types are singletons
		LoggerErrorCategory(LoggerErrorCategory&&) = delete;
		/// @brief Destructor
		~LoggerErrorCategory() noexcept final = default;

		/// @brief Returns the name of this error category
		///
		/// @return const char * - The name
		[[nodiscard]] inline constexpr auto name() const noexcept -> const char* final {
			return "Logger Error";
		}

		/// @brief Returns the message associated with the given error condition
		///
		/// @param condition - The condition to get the message for
		///
		/// @return The associated message
		[[nodiscard]] inline HYPERION_CONSTEXPR_STRINGS auto
		message(int condition) const noexcept -> std::string final {
			const auto category = static_cast<LogErrorType>(condition);
			if(category == LogErrorType::Success) {
				return "Success"s;
			}
			else if(category == LogErrorType::QueueingError) {
				return "Queueing entry failed"s;
			}
			else if(category == LogErrorType::QueueingError) {
				return "Configured logging level is higher than the given entry"s;
			}
			else {
				return "Unknown Error"s;
			}
		}

		/// @brief Copy assignment operator. Deleted. Error Category types are singletons
		auto operator=(const LoggerErrorCategory&) -> LoggerErrorCategory& = delete;
		/// @brief Move assignment operator. Deleted. Error Category types are singletons
		auto operator=(LoggerErrorCategory&&) -> LoggerErrorCategory& = delete;
	};
	IGNORE_WEAK_VTABLES_STOP

	static inline auto logger_category() noexcept -> const LoggerErrorCategory& {
		HYPERION_NO_DESTROY static const LoggerErrorCategory category{};
		return category;
	}

} // namespace hyperion

namespace std {
	template<>
	struct is_error_code_enum<hyperion::LogErrorType> : std::true_type { };

} // namespace std

inline auto make_error_code(hyperion::LogErrorType code) noexcept -> std::error_code {
	return {static_cast<int>(code), hyperion::logger_category()};
}

namespace hyperion {

	IGNORE_WEAK_VTABLES_START

	/// @brief Error type for communicating logger errors
	/// Errors can occur when an entry fails to be queued
	/// or an entry is passed in at an invalid logging level
	class LoggerError final : public Error {
	  public:
		/// @brief Default constructs a `LoggerError`
		LoggerError() noexcept {
			Error::m_message = "Error writing to logging queue"s;
		}
		/// @brief Constructs a `LoggerError` as a `std::error_code` from the given `LogErrorType`
		///
		/// @param type - The error type
		LoggerError(LogErrorType type) noexcept : Error(make_error_code(type)) { // NOLINT
			if(type == LogErrorType::QueueingError) {
				Error::m_message = "Error writing to logging queue"s;
			}
			else {
				Error::m_message = "Logging Level of this Logger is higher than the given entry"s;
			}
		}
		/// @brief Converts the given `QueueError` into a `LoggerError`
		///
		/// @param error - The error to convert
		LoggerError(const QueueError& error) noexcept // NOLINT
			: Error("Error writing to logging queue", error) {
		}
		/// @brief Converts the given `QueueError` into a `LoggerError`
		///
		/// @param error - The error to convert
		LoggerError(QueueError&& error) noexcept // NOLINT
			: Error("Error writing to logging queue", error) {
		}
		/// @brief Copy constructor
		LoggerError(const LoggerError& error) noexcept = default;
		/// @brief Move constructor
		LoggerError(LoggerError&& error) noexcept = default;
		/// @brief Destructor
		~LoggerError() noexcept final = default;

		/// @brief Copy assignment operator
		auto operator=(const LoggerError& error) noexcept -> LoggerError& = default;
		/// @brief Move assignment operator
		auto operator=(LoggerError&& error) noexcept -> LoggerError& = default;
	};
	IGNORE_WEAK_VTABLES_STOP

	/// @brief Hyperion logging type for formatted logging.
	/// Uses fmtlib/fmt for entry formatting and stylizing
	///
	/// @tparam LogParameters - The parameters for how this logger should operate
	template<LoggerParametersType LogParameters = DefaultLogParameters>
	class Logger {
	  public:
		static constexpr LogPolicy POLICY = LogParameters::policy;
		static constexpr LogLevel MINIMUM_LEVEL = LogParameters::minimum_level;

		/// @brief Default Constructor
		Logger()
			// clang-format off
			: m_message_thread([messages = m_messages, log_file_path = m_log_file_path]
					(const std::stop_token& stop)
				{
					auto log_file = fmt::output_file(log_file_path);
					while(!stop.stop_requested()) {
						if(auto res = messages->read()) {
							auto message = res.unwrap();
								log_file.print(message.style(), "{}", message.entry());
								//fmt::print(message.style(), "{}", message.entry());
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
							log_file.print(message.style(), "{}", message.entry());
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
							log_file.print(message.style(), "{}", message.entry());
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
							log_file.print(message.style(), "{}", message.entry());
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
							log_file.print(message.style(), "{}", message.entry());
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
							log_file.print(message.style(), "{}", message.entry());
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
							log_file.print(message.style(), "{}", message.entry());
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
		inline auto log(Option<usize> thread_id, const S& format_string, Args&&... args) noexcept
			-> Result<bool, LoggerError> {
			if constexpr(Level >= MINIMUM_LEVEL && MINIMUM_LEVEL != LogLevel::DISABLED) {
				if constexpr(POLICY == LogPolicy::DropWhenFull) {
					return log_dropping<Level>(thread_id, format_string, args...);
				}
				else if constexpr(POLICY == LogPolicy::FlushWhenFull) {
					log_flushing<Level>(thread_id, format_string, args...);
					return Ok(true);
				}
				else {
					log_overwriting<Level>(thread_id, format_string, args...);
					return Ok(true);
				}
			}
			else {
				ignore(format_string, args...);
				return Err(LoggerError(LogErrorType::LogLevelError));
			}
		}

		template<typename S, typename... Args, typename Char = fmt::char_t<S>>
		inline auto
		message(Option<usize> thread_id, const S& format_string, Args&&... args) noexcept {
			return log<LogLevel::MESSAGE>(thread_id, format_string, args...);
		}

		template<typename S, typename... Args, typename Char = fmt::char_t<S>>
		inline auto
		trace(Option<usize> thread_id, const S& format_string, Args&&... args) noexcept {
			return log<LogLevel::TRACE>(thread_id, format_string, args...);
		}

		template<typename S, typename... Args, typename Char = fmt::char_t<S>>
		inline auto
		info(Option<usize> thread_id, const S& format_string, Args&&... args) noexcept {
			return log<LogLevel::INFO>(thread_id, format_string, args...);
		}

		template<typename S, typename... Args, typename Char = fmt::char_t<S>>
		inline auto
		warn(Option<usize> thread_id, const S& format_string, Args&&... args) noexcept {
			return log<LogLevel::WARN>(thread_id, format_string, args...);
		}

		template<typename S, typename... Args, typename Char = fmt::char_t<S>>
		inline auto
		error(Option<usize> thread_id, const S& format_string, Args&&... args) noexcept {
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

		std::shared_ptr<LockFreeQueue<Entry, get_queue_policy()>> m_messages
			= std::make_shared<LockFreeQueue<Entry, get_queue_policy()>>();
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
		log_dropping(Option<usize> thread_id, const S& format_string, Args&&... args) noexcept
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

			return m_messages->push(make_entry<entry_level_t<Level>>(
									"{0}  [Thread ID: {1}] [{2}]: {3}\n",
									 timestamp,
									 id,
									 log_type,
									 entry))
						.template map_err<LoggerError>(
								[](const QueueError& error) { return LoggerError(error); });
		}

		template<LogLevel Level, typename S, typename... Args, typename Char = fmt::char_t<S>>
		inline auto
		log_overwriting(Option<usize> thread_id, const S& format_string, Args&&... args) noexcept
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

			m_messages->push(make_entry<entry_level_t<Level>>(
									"{0}  [Thread ID: {1}] [{2}]: {3}\n",
									 timestamp,
									 id,
									 log_type,
									 entry));
		}

		template<LogLevel Level, typename S, typename... Args, typename Char = fmt::char_t<S>>
		inline auto
		log_flushing(Option<usize> thread_id, const S& format_string, Args&&... args) noexcept
			-> void
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

			while(!m_messages->push(make_entry<entry_level_t<Level>>(
									"{0}  [Thread ID: {1}] [{2}]: {3}\n",
									 timestamp,
									 id,
									 log_type,
									 entry))) {}
		}
	};

	IGNORE_WEAK_VTABLES_START
	class LoggerInitError final : public Error {
	  public:
		LoggerInitError() noexcept {
			Error::m_message = "Global logger already initialized"s;
		}
		LoggerInitError(const LoggerInitError& error) noexcept = default;
		LoggerInitError(LoggerInitError&& error) noexcept = default;
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
	inline auto MESSAGE(Option<usize> thread_id, const S& format_string, Args&&... args) noexcept {
		return get_global_logger<LogParameters>().message(thread_id, format_string, args...);
	}

	template<LoggerParametersType LogParameters,
			 typename S,
			 typename... Args,
			 typename Char = fmt::char_t<S>>
	inline auto TRACE(Option<usize> thread_id, const S& format_string, Args&&... args) noexcept {
		return get_global_logger<LogParameters>().trace(thread_id, format_string, args...);
	}

	template<LoggerParametersType LogParameters,
			 typename S,
			 typename... Args,
			 typename Char = fmt::char_t<S>>
	inline auto INFO(Option<usize> thread_id, const S& format_string, Args&&... args) noexcept {
		return get_global_logger<LogParameters>().info(thread_id, format_string, args...);
	}

	template<LoggerParametersType LogParameters,
			 typename S,
			 typename... Args,
			 typename Char = fmt::char_t<S>>
	inline auto WARN(Option<usize> thread_id, const S& format_string, Args&&... args) noexcept {
		return get_global_logger<LogParameters>().warn(thread_id, format_string, args...);
	}

	template<LoggerParametersType LogParameters,
			 typename S,
			 typename... Args,
			 typename Char = fmt::char_t<S>>
	inline auto ERROR(Option<usize> thread_id, const S& format_string, Args&&... args) noexcept {
		return get_global_logger<LogParameters>().error(thread_id, format_string, args...);
	}

	IGNORE_UNUSED_TEMPLATES_STOP

} // namespace hyperion
