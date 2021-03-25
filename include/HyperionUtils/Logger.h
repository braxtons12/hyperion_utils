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
#include "synchronization/ReadWriteLock.h"

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
		ERROR = 4
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

	using QueueError = LockFreeQueueError<LockFreeQueueErrorTypes::QueueIsFull>;

	IGNORE_PADDING_START
	IGNORE_WEAK_VTABLES_START
	class LoggerError final : public Error {
	  public:
		LoggerError() noexcept {
			Error::m_message = "Error writing to logging queue"s;
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

		static inline auto
		from_LockFreeQueueError(const QueueError& error) noexcept -> LoggerError {
			return {error};
		}
		static inline auto from_LockFreeQueueError(QueueError&& error) noexcept -> LoggerError {
			return {std::move(error)};
		}

		auto operator=(const LoggerError& error) noexcept -> LoggerError& = default;
		auto operator=(LoggerError&& error) noexcept -> LoggerError& = default;
	};
	IGNORE_WEAK_VTABLES_STOP

	template<LogPolicy Policy = LogPolicy::DropWhenFull>
	class Logger {
	  public:
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

		template<typename S,
				 typename... Args,
				 typename Char = fmt::char_t<S>,
				 LogLevel Level = LogLevel::INFO>
		inline auto
		log(const S& format_string, Args&&... args) noexcept -> Result<bool, LoggerError>
		requires(Policy == LogPolicy::DropWhenFull || Policy == LogPolicy::FlushWhenFull) {
			if constexpr(Policy == LogPolicy::DropWhenFull) {
				return log_dropping(format_string, args...);
			}
			else {
				return log_flushing(format_string, args...);
			}
		}

		template<typename S,
				 typename... Args,
				 typename Char = fmt::char_t<S>,
				 LogLevel Level = LogLevel::INFO>
		inline constexpr auto log(const S& format_string, Args&&... args) noexcept
			-> void requires(Policy == LogPolicy::OverwriteWhenFull) {
			log_overwriting(format_string, args...);
		}

		template<typename S, typename... Args, typename Char = fmt::char_t<S>>
		inline auto message(const S& format_string, Args&&... args) noexcept {
			return log<S, Args..., Char, LogLevel::MESSAGE>(format_string, args...);
		}

		template<typename S, typename... Args, typename Char = fmt::char_t<S>>
		inline auto trace(const S& format_string, Args&&... args) noexcept {
			return log<S, Args..., Char, LogLevel::TRACE>(format_string, args...);
		}

		template<typename S, typename... Args, typename Char = fmt::char_t<S>>
		inline auto info(const S& format_string, Args&&... args) noexcept {
			return log<S, Args..., Char, LogLevel::INFO>(format_string, args...);
		}

		template<typename S, typename... Args, typename Char = fmt::char_t<S>>
		inline auto warn(const S& format_string, Args&&... args) noexcept {
			return log<S, Args..., Char, LogLevel::WARN>(format_string, args...);
		}

		template<typename S, typename... Args, typename Char = fmt::char_t<S>>
		inline auto error(const S& format_string, Args&&... args) noexcept {
			return log<S, Args..., Char, LogLevel::ERROR>(format_string, args...);
		}

		auto operator=(const Logger& logger) noexcept -> Logger& = delete;
		auto operator=(Logger&& logger) noexcept -> Logger& = default;

	  private:
		[[nodiscard]] inline static constexpr auto get_queue_policy() noexcept -> QueuePolicy {
			if constexpr(Policy == LogPolicy::DropWhenFull || Policy == LogPolicy::FlushWhenFull) {
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

		template<typename S,
				 typename... Args,
				 typename Char = fmt::char_t<S>,
				 LogLevel Level = LogLevel::INFO>
		inline auto
		log_dropping(const S& format_string, Args&&... args) noexcept -> Result<bool, LoggerError>
		requires(Policy == LogPolicy::DropWhenFull) {
			if constexpr(Level == LogLevel::MESSAGE) {
				return m_messages.push(fmt::format(MESSAGE_STYLE, format_string, args...))
					.template map_err<LoggerError>(
						[](const QueueError& error) { return LoggerError(error); });
			}
			else if constexpr(Level == LogLevel::TRACE) {
				return m_messages.push(fmt::format(TRACE_STYLE, format_string, args...))
					.template map_err<LoggerError>(
						[](const QueueError& error) { return LoggerError(error); });
			}
			else if constexpr(Level == LogLevel::INFO) {
				return m_messages.push(fmt::format(INFO_STYLE, format_string, args...))
					.template map_err<LoggerError>(
						[](const QueueError& error) { return LoggerError(error); });
			}
			else if constexpr(Level == LogLevel::WARN) {
				return m_messages.push(fmt::format(WARN_STYLE, format_string, args...))
					.template map_err<LoggerError>(
						[](const QueueError& error) { return LoggerError(error); });
			}
			else if constexpr(Level == LogLevel::ERROR) {
				return m_messages.push(fmt::format(ERROR_STYLE, format_string, args...))
					.template map_err<LoggerError>(
						[](const QueueError& error) { return LoggerError(error); });
			}
		}

		template<typename S,
				 typename... Args,
				 typename Char = fmt::char_t<S>,
				 LogLevel Level = LogLevel::INFO>
		inline constexpr auto log_overwriting(const S& format_string, Args&&... args) noexcept
			-> void requires(Policy == LogPolicy::OverwriteWhenFull) {
			if constexpr(Level == LogLevel::MESSAGE) {
				m_messages.push(fmt::format(MESSAGE_STYLE, format_string, args...));
			}
			else if constexpr(Level == LogLevel::TRACE) {
				m_messages.push(fmt::format(TRACE_STYLE, format_string, args...));
			}
			else if constexpr(Level == LogLevel::INFO) {
				m_messages.push(fmt::format(INFO_STYLE, format_string, args...));
			}
			else if constexpr(Level == LogLevel::WARN) {
				m_messages.push(fmt::format(WARN_STYLE, format_string, args...));
			}
			else if constexpr(Level == LogLevel::ERROR) {
				m_messages.push(fmt::format(ERROR_STYLE, format_string, args...));
			}
		}

		template<typename S,
				 typename... Args,
				 typename Char = fmt::char_t<S>,
				 LogLevel Level = LogLevel::INFO>
		inline auto
		log_flushing(const S& format_string, Args&&... args) noexcept -> Result<bool, LoggerError>
		requires(Policy == LogPolicy::FlushWhenFull) {
			if(m_messages.full()) {
				while(!m_messages.empty()) {
				}
			}
			if constexpr(Level == LogLevel::MESSAGE) {
				return m_messages.push(fmt::format(MESSAGE_STYLE, format_string, args...))
					.template map_err<LoggerError>(
						[](const QueueError& error) { return LoggerError(error); });
			}
			else if constexpr(Level == LogLevel::TRACE) {
				return m_messages.push(fmt::format(TRACE_STYLE, format_string, args...))
					.template map_err<LoggerError>(
						[](const QueueError& error) { return LoggerError(error); });
			}
			else if constexpr(Level == LogLevel::INFO) {
				return m_messages.push(fmt::format(INFO_STYLE, format_string, args...))
					.template map_err<LoggerError>(
						[](const QueueError& error) { return LoggerError(error); });
			}
			else if constexpr(Level == LogLevel::WARN) {
				return m_messages.push(fmt::format(WARN_STYLE, format_string, args...))
					.template map_err<LoggerError>(
						[](const QueueError& error) { return LoggerError(error); });
			}
			else if constexpr(Level == LogLevel::ERROR) {
				return m_messages.push(fmt::format(ERROR_STYLE, format_string, args...))
					.template map_err<LoggerError>(
						[](const QueueError& error) { return LoggerError(error); });
			}
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

	template<LogPolicy Policy = LogPolicy::DropWhenFull>
	static std::atomic_bool GLOBAL_LOGGER_INITIALIZED;

	template<LogPolicy Policy = LogPolicy::DropWhenFull>
	static Logger<Policy> GLOBAL_LOGGER;

	template<LogPolicy Policy = LogPolicy::DropWhenFull>
	inline static auto initialize_global_logger() noexcept -> Result<bool, LoggerInitError> {
		auto initialized = false;
		if(GLOBAL_LOGGER_INITIALIZED<Policy>.compare_and_exchange_strong(initialized,
																		 true,
																		 std::memory_order_seq_cst))
		{
			GLOBAL_LOGGER<Policy> = std::move(Logger<Policy>());
			return Ok(true);
		}
		else {
			return Err(LoggerInitError());
		}
	}

	template<typename S,
			 typename... Args,
			 LogPolicy Policy = LogPolicy::DropWhenFull,
			 typename Char = fmt::char_t<S>>
	inline auto MESSAGE(const S& format_string, Args&&... args) noexcept {
		return GLOBAL_LOGGER<Policy>.template log<S, Args..., Char, LogLevel::MESSAGE>(
			format_string,
			args...);
	}

	template<typename S,
			 typename... Args,
			 LogPolicy Policy = LogPolicy::DropWhenFull,
			 typename Char = fmt::char_t<S>>
	inline auto TRACE(const S& format_string, Args&&... args) noexcept {
		return GLOBAL_LOGGER<Policy>.template log<S, Args..., Char, LogLevel::TRACE>(format_string,
																					 args...);
	}

	template<typename S,
			 typename... Args,
			 LogPolicy Policy = LogPolicy::DropWhenFull,
			 typename Char = fmt::char_t<S>>
	inline auto INFO(const S& format_string, Args&&... args) noexcept {
		return GLOBAL_LOGGER<Policy>.template log<S, Args..., Char, LogLevel::INFO>(format_string,
																					args...);
	}

	template<typename S,
			 typename... Args,
			 LogPolicy Policy = LogPolicy::DropWhenFull,
			 typename Char = fmt::char_t<S>>
	inline auto WARN(const S& format_string, Args&&... args) noexcept {
		return GLOBAL_LOGGER<Policy>.template log<S, Args..., Char, LogLevel::WARN>(format_string,
																					args...);
	}

	template<typename S,
			 typename... Args,
			 LogPolicy Policy = LogPolicy::DropWhenFull,
			 typename Char = fmt::char_t<S>>
	inline auto ERROR(const S& format_string, Args&&... args) noexcept {
		return GLOBAL_LOGGER<Policy>.template log<S, Args..., Char, LogLevel::ERROR>(format_string,
																					 args...);
	}
} // namespace hyperion::utils
