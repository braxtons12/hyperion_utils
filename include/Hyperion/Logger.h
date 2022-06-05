/// @brief Hyperion logging facilities.
///
/// Hyperion's logging facilities are robust and composable.
/// Behavioral (Policy) configuration is configurable at compile time with via template parameters,
/// and output configuration is configurable by supplying the desired `Sink`s on creation
#pragma once

#include <Hyperion/BasicTypes.h>
#include <Hyperion/FmtIO.h>
#include <Hyperion/Option.h>
#include <Hyperion/logging/Config.h>
#include <Hyperion/logging/Entry.h>
#include <Hyperion/logging/Queue.h>
#include <Hyperion/logging/Sink.h>
#include <Hyperion/synchronization/ReadWriteLock.h>
#include <algorithm>
#include <atomic>
#include <chrono>
#include <filesystem>
#include <gsl/gsl>
#include <iostream>
#include <memory>
#include <mutex>
#include <system_error>
#include <thread>

namespace hyperion {

	/// @brief Possible Error categories that can occur when using the logger
	enum class LoggerErrorCategory : i8 {
		/// @brief No Error occurred
		Success = 0,
		/// @brief failed to queue the entry for logging
		QueueingError = 1,
		/// @brief the requested log level for the entry is
		/// lower than the minimum level for the logger
		LogLevelError = 2,
		LoggerNotInitialized = 3,
		Unknown = -1
	};
} // namespace hyperion

STATUS_CODE_DOMAIN(
	LoggerErrorDomain /**NOLINT**/,
	Logger,
	hyperion,
	hyperion::LoggerErrorCategory,
	true,
	"045dd371-9552-4ce1-bd4d-8e95b654fbe0",
	"LoggerErrorDomain",
	hyperion::LoggerErrorCategory::Success,
	hyperion::LoggerErrorCategory::Unknown,
	[](hyperion::LoggerErrorCategory code) noexcept -> std::string_view {
		if(code == value_type::Success) {
			return "Success";
		}
		// NOLINTNEXTLINE(readability-else-after-return)
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
	},
	[](const auto& code) noexcept -> hyperion::error::GenericStatusCode {
		if(code == value_type::Success || code == value_type::Unknown) {
			return make_status_code(static_cast<error::Errno>(code));
		}
		else {
			return make_status_code(error::Errno::Unknown);
		}
	});

namespace hyperion {
	template<>
	struct error::status_code_enum_info<LoggerErrorCategory> {
		using domain_type [[maybe_unused]] = LoggerErrorDomain;
		static constexpr bool value = true;
	};

	using QueueError = LoggingQueueError;

	IGNORE_PADDING_START

	namespace detail {
#if HYPERION_HAS_JTHREAD
		using thread = std::jthread;
#else
		using thread = std::thread;
#endif

		IGNORE_UNNEEDED_INTERNAL_DECL_START
		[[nodiscard]] static inline auto create_time_stamp() noexcept -> std::string {
			HYPERION_PROFILE_FUNCTION();
			//	return fmt::format(FMT_COMPILE("[{:%Y-%m-%d|%H:%M:%S}]"),
			//	                   fmt::gmtime(std::time(nullptr)));
			const auto now = fmt::gmtime(
				std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()));
			const auto years = 1900 + now.tm_year;
			const auto months = 1 + now.tm_mon;
			return fmt::format(FMT_COMPILE("[{:#04}-{:#02}-{:#02}|{:#02}:{:#02}:{:#02}]"),
							   years,
							   months,
							   now.tm_mday,
							   now.tm_hour,
							   now.tm_min,
							   now.tm_sec);
		}

		static inline auto
		create_default_sinks() noexcept -> Sinks { // NOLINT(bugprone-exception-escape)
			HYPERION_PROFILE_FUNCTION();
			auto file = FileSink::create_file();
			auto file_sink = make_sink<FileSink>(file.expect("Failed to create default log file"));
			auto stdout_sink = make_sink<StdoutSink<>>();
			auto stderr_sink = make_sink<StderrSink<>>();
			return Sinks({std::move(file_sink), std::move(stdout_sink), std::move(stderr_sink)});
		}
		IGNORE_UNNEEDED_INTERNAL_DECL_STOP

		IGNORE_UNUSED_TEMPLATES_START
		template<LogLevel Level, typename... Args>
		static inline auto
		format_entry(Option<usize> thread_id, // NOLINT(bugprone-exception-escape)
					 fmt::format_string<Args...>&& format_string,
					 Args&&... args) noexcept -> Entry {
			using namespace std::string_literals;

			HYPERION_PROFILE_FUNCTION();

			const auto timestamp = create_time_stamp();
			const auto entry = fmt::format(std::move(format_string), std::forward<Args>(args)...);
			const auto tid = thread_id.is_some() ?
								 thread_id.unwrap() :
								 std::hash<std::thread::id>()(std::this_thread::get_id());
			// ignore(thread_id);
			// ignore(timestamp);

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

			return make_entry<entry_level_t<Level>>(FMT_COMPILE("{0} [Thread ID: {1}] [{2}]: {3}"),
													timestamp,
													tid,
													log_type,
													entry);
			//	return make_entry<entry_level_t<Level>>(FMT_COMPILE("[{0}]: {1}"),
			//	                                        log_type,
			//	                                        entry);
		}
		IGNORE_UNUSED_TEMPLATES_STOP

		template<LogLevel MinimumLevel = DefaultLogParameters::minimum_level,
				 LogThreadingPolicy ThreadingPolicy = DefaultLogParameters::threading_policy,
				 LogAsyncPolicy AsyncPolicy = DefaultLogParameters::async_policy,
				 usize QueueSize = DefaultLogParameters::queue_size>
		class LogBase;

		template<LogLevel MinimumLevel, LogAsyncPolicy AsyncPolicy, usize QueueSize>
		class LogBase<MinimumLevel, LogThreadingPolicy::SingleThreaded, AsyncPolicy, QueueSize> {
		  public:
			[[maybe_unused]] static constexpr auto THREADING_POLICY
				= LogThreadingPolicy::SingleThreaded;
			static constexpr auto ASYNC_POLICY = AsyncPolicy;
			static constexpr auto MINIMUM_LEVEL = MinimumLevel;

			LogBase() : LogBase(create_default_sinks()) {
			}
			explicit LogBase(Sinks&& sinks) noexcept : m_sinks(std::move(sinks)) {
			}
			LogBase(const LogBase&) = delete;
			LogBase(LogBase&&) noexcept = default;
			~LogBase() noexcept = default;

			template<LogLevel Level, typename... Args>
			inline auto log(Option<usize> thread_id,
							fmt::format_string<Args...>&& format_string,
							Args&&... args) noexcept -> void {
				HYPERION_PROFILE_FUNCTION();
				const auto message = format_entry<Level>(std::move(thread_id),
														 std::move(format_string),
														 std::forward<Args>(args)...);

				std::ranges::for_each(m_sinks, [&message](const auto& sink) noexcept -> void {
					sink->sink(message);
				});
			}

			template<LogLevel, typename... Args>
			inline auto
			log(fmt::format_string<Args...>&& format_string, Args&&... args) noexcept -> void {
				HYPERION_PROFILE_FUNCTION();
				return log(None(), std::move(format_string), std::forward<Args>(args)...);
			}

			[[maybe_unused]] inline auto flush() const noexcept -> void {
				// intentionally does nothing
			}

			auto operator=(const LogBase&) -> LogBase& = delete;
			auto operator=(LogBase&&) noexcept -> LogBase& = default;

		  private:
			Sinks m_sinks;
		};

		template<LogLevel MinimumLevel, LogAsyncPolicy AsyncPolicy, usize QueueSize>
		class LogBase<MinimumLevel,
					  LogThreadingPolicy::SingleThreadedAsync,
					  AsyncPolicy,
					  QueueSize> {
		  public:
			[[maybe_unused]] static constexpr auto THREADING_POLICY
				= LogThreadingPolicy::SingleThreadedAsync;
			static constexpr auto ASYNC_POLICY = AsyncPolicy;
			static constexpr auto MINIMUM_LEVEL = MinimumLevel;
			static constexpr usize QUEUE_SIZE = QueueSize;

			LogBase() : LogBase(create_default_sinks()) {
			}
			explicit LogBase(Sinks&& sinks) noexcept : m_sinks(std::move(sinks)), m_queue() {
#if HYPERION_HAS_JTHREAD
				m_logging_thread = detail::thread(
					[this](const std::stop_token& token) { message_thread_function(token); });
#else
				m_logging_thread = detail::thread([this]() { message_thread_function(); });
#endif
			}
			LogBase(const LogBase&) = delete;
			LogBase(LogBase&&) = delete;
#if HYPERION_HAS_JTHREAD
			~LogBase() noexcept = default;
#else
			~LogBase() noexcept {
				request_thread_stop();
				m_logging_thread.join();
			}
#endif

			template<LogLevel Level, typename... Args>
			[[maybe_unused]] inline auto log(Option<usize> thread_id,
											 fmt::format_string<Args...>&& format_string,
											 Args&&... args) noexcept {
				HYPERION_PROFILE_FUNCTION();
				if constexpr(Level >= MINIMUM_LEVEL && MINIMUM_LEVEL != LogLevel::DISABLED) {
					auto message = format_entry<Level>(std::move(thread_id),
													   std::move(format_string),
													   std::forward<Args>(args)...);
					if constexpr(ASYNC_POLICY == LogAsyncPolicy::DropWhenFull) {
						return m_queue.push(std::move(message))
							.map_err([]([[maybe_unused]] const QueueError& error) {
								return LoggerError(LoggerErrorCategory::QueueingError);
							});
					}
					else {
						m_queue.push(std::move(message));
					}
				}
				else {
					ignore(format_string, std::forward<Args>(args)...);
					return Err(LoggerError(make_error_code(LoggerErrorCategory::LogLevelError)));
				}
			}

			template<LogLevel, typename... Args>
			inline auto log(fmt::format_string<Args...>&& format_string, Args&&... args) noexcept {
				HYPERION_PROFILE_FUNCTION();
				return log(None(), std::move(format_string), std::forward<Args>(args)...);
			}

			auto operator=(const LogBase&) -> LogBase& = delete;
			auto operator=(LogBase&&) -> LogBase& = delete;

		  private:
			[[nodiscard]] static inline consteval auto get_queue_policy() noexcept -> QueuePolicy {
				if constexpr(ASYNC_POLICY == LogAsyncPolicy::DropWhenFull) {
					return QueuePolicy::ErrWhenFull;
				}
				else if constexpr(ASYNC_POLICY == LogAsyncPolicy::OverwriteWhenFull) {
					return QueuePolicy::OverwriteWhenFull;
				}
				else {
					return QueuePolicy::BlockWhenFull;
				}
			}

			using Queue = LoggingQueue<Entry, get_queue_policy(), QUEUE_SIZE>;

			Sinks m_sinks;
			Queue m_queue;

#if !HYPERION_HAS_JTHREAD
			std::atomic_bool m_exit_flag = false;
#endif
			detail::thread m_logging_thread;

#if !HYPERION_HAS_JTHREAD
			inline auto request_thread_stop() noexcept -> void {
				m_exit_flag.store(true);
			}
#endif

			inline auto try_read() noexcept -> Result<Entry, QueueError> {
				HYPERION_PROFILE_FUNCTION();
				return m_queue.read();
			}

#if HYPERION_HAS_JTHREAD
			// NOLINTNEXTLINE(readability-function-cognitive-complexity)
			inline auto message_thread_function(const std::stop_token& token) noexcept -> void {
				while(!token.stop_requested()) {
#else
			// NOLINTNEXTLINE(readability-function-cognitive-complexity)
			inline auto message_thread_function() noexcept -> void {
				while(!m_exit_flag.load()) {
#endif

					ignore(try_read().and_then([this](const auto& message) noexcept -> None {
						std::ranges::for_each(
							m_sinks,
							[&message](const auto& sink) noexcept -> void { sink->sink(message); });
						return {};
					}));
				}
				while(try_read().and_then([this](const auto& message) noexcept -> None {
					std::ranges::for_each(m_sinks, [&message](const auto& sink) noexcept -> void {
						sink->sink(message);
					});
					return {};
				}))
				{
					// loop until we flush the queue
				}
			}
		};

		template<LogLevel MinimumLevel, LogAsyncPolicy AsyncPolicy, usize QueueSize>
		class LogBase<MinimumLevel, LogThreadingPolicy::MultiThreaded, AsyncPolicy, QueueSize> {
		  public:
			[[maybe_unused]] static constexpr auto THREADING_POLICY
				= LogThreadingPolicy::MultiThreaded;
			static constexpr auto ASYNC_POLICY = AsyncPolicy;
			static constexpr auto MINIMUM_LEVEL = MinimumLevel;

			LogBase() : LogBase(create_default_sinks()) {
			}
			explicit LogBase(Sinks&& sinks) noexcept : m_sinks(std::move(sinks)) {
			}
			LogBase(const LogBase&) = delete;
			LogBase(LogBase&&) noexcept = default;
			~LogBase() noexcept = default;

			template<LogLevel Level, typename... Args>
			inline auto log(Option<usize> thread_id,
							fmt::format_string<Args...>&& format_string,
							Args&&... args) noexcept -> void {
				HYPERION_PROFILE_FUNCTION();
				const auto message = format_entry<Level>(std::move(thread_id),
														 std::move(format_string),
														 std::forward<Args>(args)...);

				{
					auto sinks_guard = m_sinks.write();
					std::ranges::for_each(
						*sinks_guard,
						[&message](const auto& sink) noexcept -> void { sink->sink(message); });
				}
			}

			template<LogLevel, typename... Args>
			inline auto
			log(fmt::format_string<Args...>&& format_string, Args&&... args) noexcept -> void {
				HYPERION_PROFILE_FUNCTION();
				return log(None(), std::move(format_string), std::forward<Args>(args)...);
			}

			[[maybe_unused]] [[maybe_unused]] [[maybe_unused]] inline auto
			flush() const noexcept -> void {
				// intentionally does nothing
			}

			auto operator=(const LogBase&) -> LogBase& = delete;
			auto operator=(LogBase&&) noexcept -> LogBase& = default;

		  private:
			ReadWriteLock<Sinks> m_sinks;
		};

		template<LogLevel MinimumLevel, LogAsyncPolicy AsyncPolicy, usize QueueSize>
		class LogBase<MinimumLevel,
					  LogThreadingPolicy::MultiThreadedAsync,
					  AsyncPolicy,
					  QueueSize> {
		  public:
			[[maybe_unused]] static constexpr auto THREADING_POLICY
				= LogThreadingPolicy::MultiThreadedAsync;
			static constexpr auto ASYNC_POLICY = AsyncPolicy;
			static constexpr auto MINIMUM_LEVEL = MinimumLevel;
			static constexpr usize QUEUE_SIZE = QueueSize;

			LogBase() : LogBase(create_default_sinks()) {
			}
			explicit LogBase(Sinks&& sinks) noexcept : m_sinks(std::move(sinks)), m_queue() {
#if HYPERION_HAS_JTHREAD
				m_logging_thread = detail::thread(
					[this](const std::stop_token& token) { message_thread_function(token); });
#else
				m_logging_thread = detail::thread([this]() { message_thread_function(); });
#endif
			}
			LogBase(const LogBase&) = delete;
			LogBase(LogBase&&) = delete;
#if HYPERION_HAS_JTHREAD
			~LogBase() noexcept = default;
#else
			~LogBase() noexcept {
				request_thread_stop();
				m_logging_thread.join();
			}
#endif

			template<LogLevel Level, typename... Args>
			[[maybe_unused]] inline auto log(Option<usize> thread_id,
											 fmt::format_string<Args...>&& format_string,
											 Args&&... args) noexcept {
				HYPERION_PROFILE_FUNCTION();
				if constexpr(Level >= MINIMUM_LEVEL && MINIMUM_LEVEL != LogLevel::DISABLED) {
					auto message = format_entry<Level>(std::move(thread_id),
													   std::move(format_string),
													   std::forward<Args>(args)...);
					if constexpr(ASYNC_POLICY == LogAsyncPolicy::DropWhenFull) {
						return m_queue.push(std::move(message))
							.map_err([]([[maybe_unused]] const QueueError& error) {
								return LoggerError(LoggerErrorCategory::QueueingError);
							});
					}
					else {
						m_queue.push(std::move(message));
					}
				}
				else {
					ignore(format_string, std::forward<Args>(args)...);
					return Err(LoggerError(make_error_code(LoggerErrorCategory::LogLevelError)));
				}
			}

			template<LogLevel, typename... Args>
			inline auto log(fmt::format_string<Args...>&& format_string, Args&&... args) noexcept {
				HYPERION_PROFILE_FUNCTION();
				return log(None(), std::move(format_string), std::forward<Args>(args)...);
			}

			auto operator=(const LogBase&) -> LogBase& = delete;
			auto operator=(LogBase&& logger) -> LogBase& = delete;

		  private:
			[[nodiscard]] static inline consteval auto get_queue_policy() noexcept -> QueuePolicy {
				if constexpr(ASYNC_POLICY == LogAsyncPolicy::DropWhenFull) {
					return QueuePolicy::ErrWhenFull;
				}
				else if constexpr(ASYNC_POLICY == LogAsyncPolicy::OverwriteWhenFull) {
					return QueuePolicy::OverwriteWhenFull;
				}
				else {
					return QueuePolicy::BlockWhenFull;
				}
			}

			using Queue = LoggingQueue<Entry, get_queue_policy(), QUEUE_SIZE>;

			Sinks m_sinks;
			Queue m_queue;

#if !HYPERION_HAS_JTHREAD
			std::atomic_bool m_exit_flag = false;
#endif
			detail::thread m_logging_thread;
#if !HYPERION_HAS_JTHREAD
			inline auto request_thread_stop() noexcept -> void {
				m_exit_flag.store(true);
			}
#endif

			inline auto try_read() noexcept -> Result<Entry, QueueError> {
				HYPERION_PROFILE_FUNCTION();
				auto res = m_queue.read();
				return res;
			}

#if HYPERION_HAS_JTHREAD
			// NOLINTNEXTLINE(readability-function-cognitive-complexity)
			inline auto message_thread_function(const std::stop_token& token) noexcept -> void {
				while(!token.stop_requested()) {
#else
			// NOLINTNEXTLINE(readability-function-cognitive-complexity)
			inline auto message_thread_function() noexcept -> void {
				while(!m_exit_flag.load()) {
#endif

					ignore(try_read().and_then([this](const auto& message) noexcept -> None {
						std::ranges::for_each(
							m_sinks,
							[&message](const auto& sink) noexcept -> void { sink->sink(message); });
						return {};
					}));
				}
				while(try_read().and_then([this](const auto& message) noexcept -> None {
					std::ranges::for_each(m_sinks, [&message](const auto& sink) noexcept -> void {
						sink->sink(message);
					});
					return {};
				}))
				{
					// loop until we flush the queue
				}
			}
		};
	} // namespace detail

	/// @brief Hyperion logging type for formatted logging.
	/// Uses fmtlib/fmt for entry formatting and stylizing
	///
	/// @tparam LogParameters - The parameters for how this logger should operate
	template<LoggerParametersType LogParameters = DefaultLogParameters>
	class Logger : public detail::LogBase<LogParameters::minimum_level,
										  LogParameters::threading_policy,
										  LogParameters::async_policy,
										  LogParameters::queue_size> {
	  public:
		[[maybe_unused]] static constexpr LogThreadingPolicy THREADING_POLICY
			= LogParameters::threading_policy;
		[[maybe_unused]] static constexpr LogAsyncPolicy ASYNC_POLICY = LogParameters::async_policy;
		[[maybe_unused]] static constexpr LogLevel MINIMUM_LEVEL = LogParameters::minimum_level;
		[[maybe_unused]] static constexpr usize QUEUE_SIZE = LogParameters::queue_size;
		using LogBase = detail::LogBase<LogParameters::minimum_level,
										LogParameters::threading_policy,
										LogParameters::async_policy,
										LogParameters::queue_size>;

		Logger() = default;
		explicit Logger(Sinks&& sinks) noexcept : LogBase(std::move(sinks)) {
		}
		Logger(const Logger& logger) noexcept = delete;
		Logger(Logger&& logger) noexcept = default;

		~Logger() noexcept = default;

		template<typename... Args>
		inline auto message(const Option<usize>& thread_id,
							fmt::format_string<Args...>&& format_string,
							Args&&... args) noexcept {
			return this->template log<LogLevel::MESSAGE>(thread_id,
														 std::move(format_string),
														 std::forward<Args>(args)...);
		}

		template<typename... Args>
		inline auto message(fmt::format_string<Args...>&& format_string, Args&&... args) noexcept {
			return message(None(), std::move(format_string), std::forward<Args>(args)...);
		}

		template<typename... Args>
		inline auto trace(const Option<usize>& thread_id,
						  fmt::format_string<Args...>&& format_string,
						  Args&&... args) noexcept {
			return this->template log<LogLevel::TRACE>(thread_id,
													   std::move(format_string),
													   std::forward<Args>(args)...);
		}

		template<typename... Args>
		inline auto trace(fmt::format_string<Args...>&& format_string, Args&&... args) noexcept {
			return trace(None(), std::move(format_string), std::forward<Args>(args)...);
		}

		template<typename... Args>
		inline auto info(const Option<usize>& thread_id,
						 fmt::format_string<Args...>&& format_string,
						 Args&&... args) noexcept {
			return this->template log<LogLevel::INFO>(thread_id,
													  std::move(format_string),
													  std::forward<Args>(args)...);
		}

		template<typename... Args>
		inline auto info(fmt::format_string<Args...>&& format_string, Args&&... args) noexcept {
			return info(None(), std::move(format_string), std::forward<Args>(args)...);
		}

		template<typename... Args>
		inline auto warn(const Option<usize>& thread_id,
						 fmt::format_string<Args...>&& format_string,
						 Args&&... args) noexcept {
			return this->template log<LogLevel::WARN>(thread_id,
													  std::move(format_string),
													  std::forward<Args>(args)...);
		}

		template<typename... Args>
		inline auto warn(fmt::format_string<Args...>&& format_string, Args&&... args) noexcept {
			return warn(None(), std::move(format_string), std::forward<Args>(args)...);
		}

		template<typename... Args>
		inline auto error(const Option<usize>& thread_id,
						  fmt::format_string<Args...>&& format_string,
						  Args&&... args) noexcept {
			return this->template log<LogLevel::ERROR>(thread_id,
													   std::move(format_string),
													   std::forward<Args>(args)...);
		}

		template<typename... Args>
		inline auto error(fmt::format_string<Args...>&& format_string, Args&&... args) noexcept {
			return error(None(), std::move(format_string), std::forward<Args>(args)...);
		}

		auto operator=(const Logger& logger) noexcept -> Logger& = delete;
		auto operator=(Logger&& logger) noexcept -> Logger& = default;
	};
	IGNORE_PADDING_STOP

	IGNORE_UNUSED_TEMPLATES_START

#ifndef HYPERION_LOG_GLOBAL_LOGGER_PARAMETERS
	// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
	#define HYPERION_LOG_GLOBAL_LOGGER_PARAMETERS DefaultLogParameters
#endif

	struct GlobalLog {
		using Parameters = HYPERION_LOG_GLOBAL_LOGGER_PARAMETERS;
		static UniquePtr<Logger<Parameters>> GLOBAL_LOGGER; // NOLINT

		[[nodiscard]] static inline auto
		get_global_logger() noexcept -> Result<Logger<Parameters>*, LoggerError> {
			if(GLOBAL_LOGGER == nullptr) {
				return Err(LoggerError(LoggerErrorCategory::LoggerNotInitialized));
			}
			return Ok(GLOBAL_LOGGER.get());
		}

		[[maybe_unused]] static inline auto
		set_global_logger(hyperion::UniquePtr<Logger<Parameters>>&& logger) noexcept -> void {
			GLOBAL_LOGGER = std::move(logger);
		}

		template<typename... Args>
		static inline auto MESSAGE(const Option<usize>& thread_id,
								   fmt::format_string<Args...>&& format_string,
								   Args&&... args) noexcept {
			return get_global_logger()
				.expect("Global Logger not initialized!")
				->message(thread_id, std::move(format_string), std::forward<Args>(args)...);
		}

		template<typename... Args>
		static inline auto
		MESSAGE(fmt::format_string<Args...>&& format_string, Args&&... args) noexcept {
			return MESSAGE(None(), std::move(format_string), std::forward<Args>(args)...);
		}

		template<typename... Args>
		static inline auto TRACE(const Option<usize>& thread_id,
								 fmt::format_string<Args...>&& format_string,
								 Args&&... args) noexcept {
			return get_global_logger()
				.expect("Global Logger not initialized!")
				->trace(thread_id, std::move(format_string), std::forward<Args>(args)...);
		}

		template<typename... Args>
		static inline auto
		TRACE(fmt::format_string<Args...>&& format_string, Args&&... args) noexcept {
			return TRACE(None(), std::move(format_string), std::forward<Args>(args)...);
		}

		template<typename... Args>
		static inline auto INFO(const Option<usize>& thread_id,
								fmt::format_string<Args...>&& format_string,
								Args&&... args) noexcept {
			return get_global_logger()
				.expect("Global Logger not initialized!")
				->info(thread_id, std::move(format_string), std::forward<Args>(args)...);
		}

		template<typename... Args>
		static inline auto
		INFO(fmt::format_string<Args...>&& format_string, Args&&... args) noexcept {
			return INFO(None(), std::move(format_string), std::forward<Args>(args)...);
		}

		template<typename... Args>
		static inline auto WARN(const Option<usize>& thread_id,
								fmt::format_string<Args...>&& format_string,
								Args&&... args) noexcept {
			return get_global_logger()
				.expect("Global Logger not initialized!")
				->warn(thread_id, std::move(format_string), std::forward<Args>(args)...);
		}

		template<typename... Args>
		static inline auto
		WARN(fmt::format_string<Args...>&& format_string, Args&&... args) noexcept {
			return WARN(None(), std::move(format_string), std::forward<Args>(args)...);
		}

		template<typename... Args>
		static inline auto ERROR(const Option<usize>& thread_id,
								 fmt::format_string<Args...>&& format_string,
								 Args&&... args) noexcept {
			return get_global_logger()
				.expect("Global Logger not initialized!")
				->error(thread_id, std::move(format_string), std::forward<Args>(args)...);
		}

		template<typename... Args>
		static inline auto
		ERROR(fmt::format_string<Args...>&& format_string, Args&&... args) noexcept {
			return ERROR(None(), std::move(format_string), std::forward<Args>(args)...);
		}
	};

	UniquePtr<Logger<GlobalLog::Parameters>> GlobalLog::GLOBAL_LOGGER = nullptr; // NOLINT

	template<typename... Args>
	[[maybe_unused]] static inline auto MESSAGE(const Option<usize>& thread_id,
												fmt::format_string<Args...>&& format_string,
												Args&&... args) noexcept {
		return GlobalLog::MESSAGE(thread_id, std::move(format_string), std::forward<Args>(args)...);
	}

	template<typename... Args>
	[[maybe_unused]] static inline auto
	MESSAGE(fmt::format_string<Args...>&& format_string, Args&&... args) noexcept {
		return GlobalLog::MESSAGE(None(), std::move(format_string), std::forward<Args>(args)...);
	}

	template<typename... Args>
	[[maybe_unused]] static inline auto TRACE(const Option<usize>& thread_id,
											  fmt::format_string<Args...>&& format_string,
											  Args&&... args) noexcept {
		return GlobalLog::TRACE(thread_id, std::move(format_string), std::forward<Args>(args)...);
	}

	template<typename... Args>
	[[maybe_unused]] static inline auto
	TRACE(fmt::format_string<Args...>&& format_string, Args&&... args) noexcept {
		return GlobalLog::TRACE(None(), std::move(format_string), std::forward<Args>(args)...);
	}

	template<typename... Args>
	[[maybe_unused]] static inline auto INFO(const Option<usize>& thread_id,
											 fmt::format_string<Args...>&& format_string,
											 Args&&... args) noexcept {
		return GlobalLog::INFO(thread_id, std::move(format_string), std::forward<Args>(args)...);
	}

	template<typename... Args>
	[[maybe_unused]] static inline auto
	INFO(fmt::format_string<Args...>&& format_string, Args&&... args) noexcept {
		return GlobalLog::INFO(None(), std::move(format_string), std::forward<Args>(args)...);
	}

	template<typename... Args>
	[[maybe_unused]] static inline auto WARN(const Option<usize>& thread_id,
											 fmt::format_string<Args...>&& format_string,
											 Args&&... args) noexcept {
		return GlobalLog::WARN(thread_id, std::move(format_string), std::forward<Args>(args)...);
	}

	template<typename... Args>
	[[maybe_unused]] static inline auto
	WARN(fmt::format_string<Args...>&& format_string, Args&&... args) noexcept {
		return GlobalLog::WARN(None(), std::move(format_string), std::forward<Args>(args)...);
	}

	template<typename... Args>
	[[maybe_unused]] static inline auto ERROR(const Option<usize>& thread_id,
											  fmt::format_string<Args...>&& format_string,
											  Args&&... args) noexcept {
		return GlobalLog::ERROR(thread_id, std::move(format_string), std::forward<Args>(args)...);
	}

	template<typename... Args>
	[[maybe_unused]] static inline auto
	ERROR(fmt::format_string<Args...>&& format_string, Args&&... args) noexcept {
		return GlobalLog::ERROR(None(), std::move(format_string), std::forward<Args>(args)...);
	}

	IGNORE_UNUSED_TEMPLATES_STOP
} // namespace hyperion
