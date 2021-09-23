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
#include "FmtIO.h"
#include "LockFreeQueue.h"
#include "Logger.h"
#include "Option.h"
#include "logging/Config.h"
#include "logging/Entry.h"
#include "logging/Sink.h"

namespace hyperion {

	/// @brief Possible Error categories that can occur when using the logger
	enum class LoggerErrorCategory : i8
	{
		/// @brief No Error occurred
		Success = 0,
		/// @brief failed to queue the entry for logging
		QueueingError = 1,
		/// @brief the requested log level for the entry is
		/// lower than the minium level for the logger
		LogLevelError = 2,
		Unknown = -1
	};

	/// @brief Alias for the Error type we might recieve from the internal queue
	using QueueError = LockFreeQueueError;

	class LoggerErrorDomain {
	  public:
		using value_type = LoggerErrorCategory;
		using LoggerStatusCode = error::StatusCode<LoggerErrorDomain>;
		using LoggerErrorCode = error::ErrorCode<LoggerErrorDomain>;

		static constexpr const char (&UUID)[error::num_chars_in_uuid] // NOLINT
			= "045dd371-9552-4ce1-bd4d-8e95b654fbe0";

		static constexpr u64 ID = error::parse_uuid_from_string(UUID);

		constexpr LoggerErrorDomain() noexcept = default;
		explicit constexpr LoggerErrorDomain(u64 uuid) noexcept : m_uuid(uuid) {
		}
		explicit constexpr LoggerErrorDomain(const error::UUIDString auto& uuid) noexcept
			: m_uuid(error::parse_uuid_from_string(uuid)) {
		}
		constexpr LoggerErrorDomain(const LoggerErrorDomain&) noexcept = default;
		constexpr LoggerErrorDomain(LoggerErrorDomain&&) noexcept = default;
		constexpr ~LoggerErrorDomain() noexcept = default;

		[[nodiscard]] constexpr inline auto id() const noexcept -> u64 {
			return m_uuid;
		}

		[[nodiscard]] constexpr inline auto name() const noexcept -> std::string_view { // NOLINT
			return "LoggerErrorDomain";
		}

		[[nodiscard]] constexpr inline auto message(value_type code) // NOLINT
			const noexcept -> std::string_view {
			if(code == value_type::Success) {
				return "Success";
			}
			else if(code == value_type::QueueingError) {
				return "Logger failed to queue log entry.";
			}
			else if(code == value_type::LogLevelError) {
				return "Requested log level for entry is lower than minimum level configured for "
					   "logger.";
			}
			else {
				return "Unknown Logger error.";
			}
		}

		[[nodiscard]] constexpr inline auto message(const LoggerStatusCode& code) // NOLINT
			const noexcept -> std::string_view {
			return message(code.code());
		}

		[[nodiscard]] constexpr inline auto is_error(const LoggerStatusCode& code) // NOLINT
			const noexcept -> bool {
			return code.code() != value_type::Success;
		}

		[[nodiscard]] constexpr inline auto is_success(const LoggerStatusCode& code) // NOLINT
			const noexcept -> bool {
			return code.code() == value_type::Success;
		}

		template<typename Domain2>
		[[nodiscard]] constexpr inline auto
		are_equivalent(const LoggerStatusCode& lhs,
					   const error::StatusCode<Domain2>& rhs) const noexcept -> bool {
			if constexpr(concepts::Same<LoggerStatusCode, error::StatusCode<Domain2>>) {
				return lhs.code() == rhs.code();
			}
			else {
				return false;
			}
		}

		[[nodiscard]] constexpr inline auto as_generic_code(const LoggerStatusCode& code) // NOLINT
			const noexcept -> error::GenericStatusCode {
			if(code.code() == value_type::Success || code.code() == value_type::Unknown) {
				return make_status_code(static_cast<error::Errno>(code.code()));
			}
			else {
				return make_status_code(error::Errno::Unknown);
			}
		}

		[[nodiscard]] constexpr inline auto success_value() const noexcept -> value_type { // NOLINT
			return value_type::Success;
		}

		template<typename Domain>
		friend constexpr inline auto
		operator==(const LoggerErrorDomain& lhs, const Domain& rhs) noexcept -> bool {
			return rhs.id() == lhs.id();
		}

		template<typename Domain>
		friend constexpr inline auto
		operator!=(const LoggerErrorDomain& lhs, const Domain& rhs) noexcept -> bool {
			return rhs.id() != lhs.id();
		}

		constexpr auto operator=(const LoggerErrorDomain&) noexcept -> LoggerErrorDomain& = default;
		constexpr auto operator=(LoggerErrorDomain&&) noexcept -> LoggerErrorDomain& = default;

	  private:
		u64 m_uuid = ID;
	};

	using LoggerStatusCode = LoggerErrorDomain::LoggerStatusCode;
	using LoggerErrorCode = LoggerErrorDomain::LoggerErrorCode;
	using LoggerError = error::Error<LoggerErrorDomain>;

} // namespace hyperion

template<>
constexpr inline auto
make_status_code_domain<hyperion::LoggerErrorDomain>() noexcept -> hyperion::LoggerErrorDomain {
	return {};
}

namespace hyperion {
	static_assert(error::StatusCodeDomain<LoggerErrorDomain>);

	template<>
	struct error::status_code_enum_info<LoggerErrorCategory> {
		using domain_type = LoggerErrorDomain;
		static constexpr bool value = true;
	};

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
								log_file.print("{}", message.entry());
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
							log_file.print("{}", message.entry());
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
							log_file.print("{}", message.entry());
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
							log_file.print("{}", message.entry());
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
							log_file.print("{}", message.entry());
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
							log_file.print("{}", message.entry());
							//fmt::print("{}", message);
						}
					}
					log_file.close();
				})
		{
				//clang-format on
		}
		Logger(std::string&& root_name, std::string&& directory_name) // NOLINT
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
							log_file.print("{}", message.entry());
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

		template<LogLevel Level, typename... Args>
		inline auto log(Option<usize> thread_id, fmt::format_string<Args...>&& format_string, Args&&... args) noexcept
			-> Result<bool, LoggerError> {
			if constexpr(Level >= MINIMUM_LEVEL && MINIMUM_LEVEL != LogLevel::DISABLED) {
				if constexpr(POLICY == LogPolicy::DropWhenFull) {
					return log_dropping<Level>(thread_id, std::move(format_string), std::forward<Args>(args)...);
				}
				else if constexpr(POLICY == LogPolicy::FlushWhenFull) {
					log_flushing<Level>(thread_id, std::move(format_string), std::forward<Args>(args)...);
					return Ok(true);
				}
				else {
					log_overwriting<Level>(thread_id, std::move(format_string), std::forward<Args>(args)...);
					return Ok(true);
				}
			}
			else {
				ignore(format_string, std::forward<Args>(args)...);
				return Err(LoggerError(make_error_code(LoggerErrorCategory::LogLevelError)));
			}
		}

		template<typename... Args>
		inline auto
		message(Option<usize> thread_id, fmt::format_string<Args...>&& format_string, Args&&... args) noexcept {
			return log<LogLevel::MESSAGE>(thread_id, std::move(format_string), std::forward<Args>(args)...);
		}

		template<typename... Args>
		inline auto
		trace(Option<usize> thread_id, fmt::format_string<Args...>&& format_string, Args&&... args) noexcept {
			return log<LogLevel::TRACE>(thread_id, std::move(format_string), std::forward<Args>(args)...);
		}

		template<typename... Args>
		inline auto
		info(Option<usize> thread_id, fmt::format_string<Args...>&& format_string, Args&&... args) noexcept {
			return log<LogLevel::INFO>(thread_id, std::move(format_string), std::forward<Args>(args)...);
		}

		template<typename... Args>
		inline auto
		warn(Option<usize> thread_id, fmt::format_string<Args...>&& format_string, Args&&... args) noexcept {
			return log<LogLevel::WARN>(thread_id, std::move(format_string), std::forward<Args>(args)...);
		}

		template<typename... Args>
		inline auto
		error(Option<usize> thread_id, fmt::format_string<Args...>&& format_string, Args&&... args) noexcept {
			return log<LogLevel::ERROR>(thread_id, std::move(format_string), std::forward<Args>(args)...);
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

		template<LogLevel Level, typename... Args>
		inline auto
		log_dropping(Option<usize> thread_id, fmt::format_string<Args...>&& format_string, Args&&... args) noexcept
			-> Result<bool, LoggerError>
		requires(POLICY == LogPolicy::DropWhenFull) {
			const auto timestamp = create_time_stamp();
			const auto entry = fmt::format(format_string, std::forward<Args>(args)...);
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
								[]([[maybe_unused]] const QueueError& error) { return LoggerError(make_error_code(LoggerErrorCategory::QueueingError)); });
		}

		template<LogLevel Level, typename... Args>
		inline auto
		log_overwriting(Option<usize> thread_id, fmt::format_string<Args...>&& format_string, Args&&... args) noexcept
			-> void requires(POLICY == LogPolicy::OverwriteWhenFull) {
			const auto timestamp = create_time_stamp();
			const auto entry = fmt::format(format_string, std::forward<Args>(args)...);
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

		template<LogLevel Level, typename... Args>
		inline auto
		log_flushing(Option<usize> thread_id, fmt::format_string<Args...>&& format_string, Args&&... args) noexcept
			-> void
		requires(POLICY == LogPolicy::FlushWhenFull) {
			const auto timestamp = create_time_stamp();
			const auto entry = fmt::format(format_string, std::forward<Args>(args)...);
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

		//const auto logline = fmt::format(MESSAGE_STYLE,
		//								 "{0}  [Thread ID: {1}] [{2}]: {3}\n",
		//								 timestamp,
		//								 id,
		//								 log_type,
		//								 entry);

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
			 typename... Args>
	inline auto MESSAGE(Option<usize> thread_id, fmt::format_string<Args...>&& format_string, Args&&... args) noexcept {
		return get_global_logger<LogParameters>().message(thread_id, std::move(format_string), std::forward<Args>(args)...);
	}

	template<LoggerParametersType LogParameters,
			 typename... Args>
	inline auto TRACE(Option<usize> thread_id, fmt::format_string<Args...>&& format_string, Args&&... args) noexcept {
		return get_global_logger<LogParameters>().trace(thread_id, std::move(format_string), std::forward<Args>(args)...);
	}

	template<LoggerParametersType LogParameters,
			 typename... Args>
	inline auto INFO(Option<usize> thread_id, fmt::format_string<Args...>&& format_string, Args&&... args) noexcept {
		return get_global_logger<LogParameters>().info(thread_id, std::move(format_string), std::forward<Args>(args)...);
	}

	template<LoggerParametersType LogParameters,
			 typename... Args>
	inline auto WARN(Option<usize> thread_id, fmt::format_string<Args...>&& format_string, Args&&... args) noexcept {
		return get_global_logger<LogParameters>().warn(thread_id, std::move(format_string), std::forward<Args>(args)...);
	}

	template<LoggerParametersType LogParameters,
			 typename... Args>
	inline auto ERROR(Option<usize> thread_id, fmt::format_string<Args...>&& format_string, Args&&... args) noexcept {
		return get_global_logger<LogParameters>().error(thread_id, std::move(format_string), std::forward<Args>(args)...);
	}

	IGNORE_UNUSED_TEMPLATES_STOP

} // namespace hyperion
