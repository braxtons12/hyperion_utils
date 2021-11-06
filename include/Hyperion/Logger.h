/// @brief Hyperion logging facilities.
///
/// Hyperion's logging facilities are robust and composable.
/// Behavioral (Policy) configuration is configurable at compile time with via template parameters,
/// and output configuration is configurable by supplying the desired `Sink`s on creation
#pragma once

#include <Hyperion/BasicTypes.h>
#include <Hyperion/FmtIO.h>
#include <Hyperion/Logger.h>
#include <Hyperion/Option.h>
#include <Hyperion/logging/Config.h>
#include <Hyperion/logging/Entry.h>
#include <Hyperion/logging/Queue.h>
#include <Hyperion/logging/Sink.h>
#include <Hyperion/synchronization/ReadWriteLock.h>
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
		QueueingError,
		/// @brief the requested log level for the entry is
		/// lower than the minimum level for the logger
		LogLevelError,
		LoggerNotInitialized,
		Unknown = -1
	};

	/// @brief Alias for the Error type we might receive from the internal queue
	using QueueError = LoggingQueueError;

	class LoggerErrorDomain {
	  public:
		using value_type = LoggerErrorCategory;
		using LoggerStatusCode = error::StatusCode<LoggerErrorDomain>;
		using LoggerErrorCode = error::ErrorCode<LoggerErrorDomain>;

		static const constexpr char (&UUID)[error::num_chars_in_uuid] // NOLINT
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

		[[nodiscard]] inline constexpr auto id() const noexcept -> u64 {
			return m_uuid;
		}

		[[nodiscard]] inline constexpr auto name() const noexcept -> std::string_view { // NOLINT
			return "LoggerErrorDomain";
		}

		[[nodiscard]] inline constexpr auto message(value_type code) // NOLINT
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

		[[nodiscard]] inline constexpr auto message(const LoggerStatusCode& code) // NOLINT
			const noexcept -> std::string_view {
			return message(code.code());
		}

		[[nodiscard]] inline constexpr auto is_error(const LoggerStatusCode& code) // NOLINT
			const noexcept -> bool {
			return code.code() != value_type::Success;
		}

		[[nodiscard]] inline constexpr auto is_success(const LoggerStatusCode& code) // NOLINT
			const noexcept -> bool {
			return code.code() == value_type::Success;
		}

		template<typename Domain2>
		[[nodiscard]] inline constexpr auto
		are_equivalent(const LoggerStatusCode& lhs,
					   const error::StatusCode<Domain2>& rhs) const noexcept -> bool {
			if constexpr(concepts::Same<LoggerStatusCode, error::StatusCode<Domain2>>) {
				return lhs.code() == rhs.code();
			}
			else {
				return false;
			}
		}

		[[nodiscard]] inline constexpr auto as_generic_code(const LoggerStatusCode& code) // NOLINT
			const noexcept -> error::GenericStatusCode {
			if(code.code() == value_type::Success || code.code() == value_type::Unknown) {
				return make_status_code(static_cast<error::Errno>(code.code()));
			}
			else {
				return make_status_code(error::Errno::Unknown);
			}
		}

		[[nodiscard]] inline static constexpr auto success_value() noexcept -> value_type {
			return value_type::Success;
		}

		template<typename Domain>
		friend inline constexpr auto
		operator==(const LoggerErrorDomain& lhs, const Domain& rhs) noexcept -> bool {
			return rhs.id() == lhs.id();
		}

		template<typename Domain>
		friend inline constexpr auto
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
inline constexpr auto
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

	IGNORE_PADDING_START

	namespace detail {
#if HYPERION_HAS_JTHREAD
		using thread = std::jthread;
#else
		using thread = std::thread;
#endif
		static constexpr fmt::text_style MESSAGE_STYLE = fmt::fg(fmt::color::white);
		static constexpr fmt::text_style TRACE_STYLE = fmt::fg(fmt::color::steel_blue);
		static constexpr fmt::text_style INFO_STYLE
			= fmt::fg(fmt::color::light_green) | fmt::emphasis::italic;
		static constexpr fmt::text_style WARN_STYLE
			= fmt::fg(fmt::color::orange) | fmt::emphasis::bold;
		static constexpr fmt::text_style ERROR_STYLE
			= fmt::fg(fmt::color::red) | fmt::emphasis::bold;

		IGNORE_UNNEEDED_INTERNAL_DECL_START
		[[nodiscard]] inline static auto create_time_stamp() noexcept -> std::string {
			return fmt::format("[{:%Y-%m-%d|%H-%M-%S}]", fmt::localtime(std::time(nullptr)));
		}

		inline static auto
		create_default_sinks() noexcept -> Sinks { // NOLINT(bugprone-exception-escape)
			auto file = FileSink::create_file();
			auto file_sink = make_sink<FileSink>(file.expect("Failed to create default log file"));
			auto stdout_sink = make_sink<StdoutSink<>>();
			auto stderr_sink = make_sink<StderrSink<>>();
			return Sinks({std::move(file_sink), std::move(stdout_sink), std::move(stderr_sink)});
		}
		IGNORE_UNNEEDED_INTERNAL_DECL_STOP

		IGNORE_UNUSED_TEMPLATES_START
		template<LogLevel Level, typename... Args>
		inline static auto
		format_entry(Option<usize> thread_id, // NOLINT(bugprone-exception-escape)
					 fmt::format_string<Args...>&& format_string,
					 Args&&... args) noexcept -> Entry {
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
			return make_entry<entry_level_t<Level>>("{0}  [Thread ID: {1}] [{2}]: {3}",
													timestamp,
													id,
													log_type,
													entry);
		}
		IGNORE_UNUSED_TEMPLATES_STOP

		template<LogLevel MinimumLevel = DefaultLogParameters::minimum_level,
				 LogThreadingPolicy ThreadingPolicy = DefaultLogParameters::threading_policy,
				 LogAsyncPolicy AsyncPolicy = DefaultLogParameters::async_policy,
				 usize QueueSize = DefaultLogParameters::queue_size>
		class LogBase;

		template<LogLevel MinimumLevel, LogAsyncPolicy AsyncPolicy>
		class LogBase<MinimumLevel, LogThreadingPolicy::SingleThreaded, AsyncPolicy> {
		  public:
			static constexpr auto THREADING_POLICY = LogThreadingPolicy::SingleThreaded;
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
				const auto message = format_entry<Level>(std::move(thread_id),
														 std::move(format_string),
														 std::forward<Args>(args)...);

				std::for_each(m_sinks.begin(), m_sinks.end(), [&](Sink& sink) noexcept -> void {
					sink.sink(message);
				});
			}

			template<LogLevel, typename... Args>
			inline auto
			log(fmt::format_string<Args...>&& format_string, Args&&... args) noexcept -> void {
				return log(None(), std::move(format_string), std::forward<Args>(args)...);
			}

			inline auto flush() const noexcept -> void {
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
			static constexpr auto THREADING_POLICY = LogThreadingPolicy::SingleThreadedAsync;
			static constexpr auto ASYNC_POLICY = AsyncPolicy;
			static constexpr auto MINIMUM_LEVEL = MinimumLevel;
			static constexpr usize QUEUE_SIZE = QueueSize;

			LogBase() : LogBase(create_default_sinks()) {
			}
			explicit LogBase(Sinks&& sinks) noexcept : m_sinks(std::move(sinks)), m_queue() {
#if HYPERION_HAS_JTHREAD
				m_logging_thread = detail::thread(
					[&](const std::stop_token& token) { message_thread_function(token); });
#else
				m_logging_thread = detail::thread([&]() { message_thread_function(); });
#endif
			}
			LogBase(const LogBase&) = delete;
			LogBase(LogBase&& logger) noexcept {
				std::atomic_thread_fence(std::memory_order_acquire);
				logger.request_thread_stop();
				logger.m_logging_thread.join();
				// NOLINTNEXTLINE(cppcoreguidelines-prefer-member-initializer)
				m_sinks = std::move(logger.m_sinks);
				// NOLINTNEXTLINE(cppcoreguidelines-prefer-member-initializer)
				m_queue = std::move(logger.m_queue);
#if HYPERION_HAS_JTHREAD
				m_logging_thread = detail::thread(
					[&](const std::stop_token& token) { message_thread_function(token); });
#else
				m_logging_thread = detail::thread([&]() { message_thread_function(); });
#endif
				std::atomic_thread_fence(std::memory_order_release);
			}
			~LogBase() noexcept {
				request_thread_stop();
				m_logging_thread.join();
			}

			template<LogLevel Level, typename... Args>
			inline auto log(Option<usize> thread_id,
							fmt::format_string<Args...>&& format_string,
							Args&&... args) noexcept {
				if constexpr(Level >= MINIMUM_LEVEL && MINIMUM_LEVEL != LogLevel::DISABLED) {
					auto message = format_entry<Level>(std::move(thread_id),
													   std::move(format_string),
													   std::forward<Args>(args)...);
					if constexpr(ASYNC_POLICY == LogAsyncPolicy::DropWhenFull) {
						return log_dropping(std::move(message));
					}
					else if constexpr(ASYNC_POLICY == LogAsyncPolicy::FlushWhenFull) {
						log_flushing(std::move(message));
					}
					else {
						log_overwriting(std::move(message));
					}
				}
				else {
					ignore(format_string, std::forward<Args>(args)...);
					return Err(LoggerError(make_error_code(LoggerErrorCategory::LogLevelError)));
				}
			}

			template<LogLevel, typename... Args>
			inline auto log(fmt::format_string<Args...>&& format_string, Args&&... args) noexcept {
				return log(None(), std::move(format_string), std::forward<Args>(args)...);
			}

			inline auto flush() noexcept -> void {
				m_flush.store(true);
			}

			auto operator=(const LogBase&) -> LogBase& = delete;
			auto operator=(LogBase&& logger) noexcept -> LogBase& {
				std::atomic_thread_fence(std::memory_order_acquire);
				if(this == &logger) {
					return *this;
				}

				logger.request_thread_stop();
				logger.m_logging_thread.join();
				m_sinks = std::move(logger.m_sinks);
				m_queue = std::move(logger.m_queue);

				std::atomic_thread_fence(std::memory_order_release);
				return *this;
			}

		  private:
			[[nodiscard]] inline static consteval auto get_queue_policy() noexcept -> QueuePolicy {
				if constexpr(ASYNC_POLICY == LogAsyncPolicy::DropWhenFull
							 || ASYNC_POLICY == LogAsyncPolicy::FlushWhenFull)
				{
					return QueuePolicy::ErrWhenFull;
				}
				else {
					return QueuePolicy::OverwriteWhenFull;
				}
			}
			using Queue = LoggingQueue<Entry, get_queue_policy(), QUEUE_SIZE>;

			Sinks m_sinks;
			Queue m_queue;
			std::atomic_bool m_flush = false;
#if !HYPERION_HAS_JTHREAD
			std::atomic_bool m_exit_flag = false;
#endif
			detail::thread m_logging_thread;

			inline auto request_thread_stop() noexcept -> void {
#if HYPERION_HAS_JTHREAD
				ignore(m_logging_thread.request_stop());
#else
				m_exit_flag.store(true);
#endif
			}

			inline auto log_dropping(Entry&& message) noexcept -> Result<bool, LoggerError>
			requires(ASYNC_POLICY == LogAsyncPolicy::DropWhenFull) {
				return m_queue.push(std::move(message))
					.map_err([]([[maybe_unused]] const QueueError& error) {
						return LoggerError(LoggerErrorCategory::QueueingError);
					});
			}

			inline auto log_overwriting(Entry&& message) noexcept
				-> void requires(ASYNC_POLICY == LogAsyncPolicy::OverwriteWhenFull) {
				m_queue.push(std::move(message));
			}

			inline auto log_flushing(Entry&& message) noexcept
				-> void requires(ASYNC_POLICY == LogAsyncPolicy::FlushWhenFull) {
				if(m_queue.full()) {
					m_flush.store(true);
				}

				const auto& mess = message;
				while(!m_queue.push(mess)) {
				}
			}

			inline auto try_read() noexcept -> Result<Entry, QueueError> {
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

					auto res = try_read();
					if(res) {
						const auto message = res.unwrap();
						std::atomic_thread_fence(std::memory_order_acquire);
						std::for_each(m_sinks.begin(),
									  m_sinks.end(),
									  [&](Sink& sink) noexcept -> void { sink.sink(message); });
						std::atomic_thread_fence(std::memory_order_release);
					}

					if(m_flush.load()) {
						m_flush.store(false);
						do {
							auto res2 = m_queue.read();
							if(res2) {
								const auto message = res2.unwrap();
								std::atomic_thread_fence(std::memory_order_acquire);
								std::for_each(
									m_sinks.begin(),
									m_sinks.end(),
									[&](Sink& sink) noexcept -> void { sink.sink(message); });
								std::atomic_thread_fence(std::memory_order_release);
							}
							else {
								break;
							}
						} while(true);
					}
				}
				do {
					auto res2 = m_queue.read();
					if(res2) {
						const auto message = res2.unwrap();
						std::atomic_thread_fence(std::memory_order_acquire);
						std::for_each(m_sinks.begin(),
									  m_sinks.end(),
									  [&](Sink& sink) noexcept -> void { sink.sink(message); });
						std::atomic_thread_fence(std::memory_order_release);
					}
					else {
						break;
					}
				} while(true);
			}
		};

		template<LogLevel MinimumLevel, LogAsyncPolicy AsyncPolicy>
		class LogBase<MinimumLevel, LogThreadingPolicy::MultiThreaded, AsyncPolicy> {
		  public:
			static constexpr auto THREADING_POLICY = LogThreadingPolicy::MultiThreaded;
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
				const auto message = format_entry<Level>(std::move(thread_id),
														 std::move(format_string),
														 std::forward<Args>(args)...);

				{
					auto sinks_guard = m_sinks.write();
					std::for_each(sinks_guard->begin(),
								  sinks_guard->end(),
								  [&](Sink& sink) noexcept -> void { sink.sink(message); });
				}
			}

			template<LogLevel, typename... Args>
			inline auto
			log(fmt::format_string<Args...>&& format_string, Args&&... args) noexcept -> void {
				return log(None(), std::move(format_string), std::forward<Args>(args)...);
			}

			inline auto flush() const noexcept -> void {
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
			static constexpr auto THREADING_POLICY = LogThreadingPolicy::MultiThreadedAsync;
			static constexpr auto ASYNC_POLICY = AsyncPolicy;
			static constexpr auto MINIMUM_LEVEL = MinimumLevel;
			static constexpr usize QUEUE_SIZE = QueueSize;

			LogBase() : LogBase(create_default_sinks()) {
			}
			explicit LogBase(Sinks&& sinks) noexcept : m_sinks(std::move(sinks)), m_queue() {
#if HYPERION_HAS_JTHREAD
				m_logging_thread = detail::thread(
					[&](const std::stop_token& token) { message_thread_function(token); });
#else
				m_logging_thread = detail::thread([&]() { message_thread_function(); });
#endif
			}
			LogBase(const LogBase&) = delete;
			LogBase(LogBase&& logger) noexcept {
				std::atomic_thread_fence(std::memory_order_seq_cst);
				logger.request_thread_stop();
				logger.m_logging_thread.join();
				// NOLINTNEXTLINE(cppcoreguidelines-prefer-member-initializer)
				m_sinks = std::move(logger.m_sinks);
				// NOLINTNEXTLINE(cppcoreguidelines-prefer-member-initializer)
				m_queue = std::move(logger.m_queue);
#if HYPERION_HAS_JTHREAD
				m_logging_thread = detail::thread(
					[&](const std::stop_token& token) { message_thread_function(token); });
#else
				m_logging_thread = detail::thread([&]() { message_thread_function(); });
#endif
				std::atomic_thread_fence(std::memory_order_seq_cst);
			}
			~LogBase() noexcept {
				request_thread_stop();
				m_logging_thread.join();
			}

			template<LogLevel Level, typename... Args>
			inline auto log(Option<usize> thread_id,
							fmt::format_string<Args...>&& format_string,
							Args&&... args) noexcept {
				if constexpr(Level >= MINIMUM_LEVEL && MINIMUM_LEVEL != LogLevel::DISABLED) {
					auto message = format_entry<Level>(std::move(thread_id),
													   std::move(format_string),
													   std::forward<Args>(args)...);
					if constexpr(ASYNC_POLICY == LogAsyncPolicy::DropWhenFull) {
						return log_dropping(std::move(message));
					}
					else if constexpr(ASYNC_POLICY == LogAsyncPolicy::FlushWhenFull) {
						log_flushing(std::move(message));
					}
					else {
						log_overwriting(std::move(message));
					}
				}
				else {
					ignore(format_string, std::forward<Args>(args)...);
					return Err(LoggerError(make_error_code(LoggerErrorCategory::LogLevelError)));
				}
			}

			template<LogLevel, typename... Args>
			inline auto log(fmt::format_string<Args...>&& format_string, Args&&... args) noexcept {
				return log(None(), std::move(format_string), std::forward<Args>(args)...);
			}

			inline auto flush() noexcept -> void {
				m_flush.store(true);
			}

			auto operator=(const LogBase&) -> LogBase& = delete;
			auto operator=(LogBase&& logger) noexcept -> LogBase& {
				std::atomic_thread_fence(std::memory_order_seq_cst);
				if(this == &logger) {
					return *this;
				}

				logger.request_thread_stop();
				logger.m_logging_thread.join();
				m_sinks = std::move(logger.m_sinks);
				m_queue = std::move(logger.m_queue);

				std::atomic_thread_fence(std::memory_order_seq_cst);
				return *this;
			}

		  private:
			[[nodiscard]] inline static consteval auto get_queue_policy() noexcept -> QueuePolicy {
				if constexpr(ASYNC_POLICY == LogAsyncPolicy::DropWhenFull
							 || ASYNC_POLICY == LogAsyncPolicy::FlushWhenFull)
				{
					return QueuePolicy::ErrWhenFull;
				}
				else {
					return QueuePolicy::OverwriteWhenFull;
				}
			}
			using Queue = LoggingQueue<Entry, get_queue_policy(), QUEUE_SIZE>;

			Sinks m_sinks;
			Queue m_queue;
			std::atomic_bool m_flush = false;
#if !HYPERION_HAS_JTHREAD
			std::atomic_bool m_exit_flag = false;
#endif
			detail::thread m_logging_thread;

			inline auto request_thread_stop() noexcept -> void {
				std::atomic_thread_fence(std::memory_order_seq_cst);
#if HYPERION_HAS_JTHREAD
				ignore(m_logging_thread.request_stop());
#else
				m_exit_flag.store(true);
#endif
				std::atomic_thread_fence(std::memory_order_seq_cst);
			}

			inline auto log_dropping(Entry&& message) noexcept -> Result<bool, LoggerError>
			requires(ASYNC_POLICY == LogAsyncPolicy::DropWhenFull) {
				std::atomic_thread_fence(std::memory_order_seq_cst);
				auto res = m_queue.push(std::move(message))
							   .map_err([]([[maybe_unused]] const QueueError& error) {
								   return LoggerError(LoggerErrorCategory::QueueingError);
							   });
				std::atomic_thread_fence(std::memory_order_seq_cst);
				return res;
			}

			inline auto log_overwriting(Entry&& message) noexcept
				-> void requires(ASYNC_POLICY == LogAsyncPolicy::OverwriteWhenFull) {
				std::atomic_thread_fence(std::memory_order_seq_cst);
				m_queue.push(std::move(message));
				std::atomic_thread_fence(std::memory_order_seq_cst);
			}

			inline auto log_flushing(Entry&& message) noexcept
				-> void requires(ASYNC_POLICY == LogAsyncPolicy::FlushWhenFull) {
				std::atomic_thread_fence(std::memory_order_seq_cst);
				if(m_queue.full()) {
					m_flush.store(true);
				}

				const auto& mess = message;
				while(!m_queue.push(mess)) {
				}
				std::atomic_thread_fence(std::memory_order_seq_cst);
			}

			inline auto try_read() noexcept -> Result<Entry, QueueError> {
				std::atomic_thread_fence(std::memory_order_seq_cst);
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

					auto res = try_read();
					if(res) {
						std::atomic_thread_fence(std::memory_order_seq_cst);
						const auto message = res.unwrap();
						std::for_each(m_sinks.begin(),
									  m_sinks.end(),
									  [&](Sink& sink) noexcept -> void { sink.sink(message); });
						std::atomic_thread_fence(std::memory_order_seq_cst);
					}

					if(m_flush.load()) {
						m_flush.store(false);
						do {
							std::atomic_thread_fence(std::memory_order_seq_cst);
							auto res2 = m_queue.read();
							if(res2) {
								const auto message = res2.unwrap();
								std::for_each(
									m_sinks.begin(),
									m_sinks.end(),
									[&](Sink& sink) noexcept -> void { sink.sink(message); });
								std::atomic_thread_fence(std::memory_order_seq_cst);
							}
							else {
								std::atomic_thread_fence(std::memory_order_seq_cst);
								break;
							}
						} while(true);
					}
				}
				do {
					std::atomic_thread_fence(std::memory_order_seq_cst);
					auto res2 = m_queue.read();
					if(res2) {
						const auto message = res2.unwrap();
						std::for_each(m_sinks.begin(),
									  m_sinks.end(),
									  [&](Sink& sink) noexcept -> void { sink.sink(message); });
						std::atomic_thread_fence(std::memory_order_seq_cst);
					}
					else {
						std::atomic_thread_fence(std::memory_order_seq_cst);
						break;
					}
				} while(true);
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
		static constexpr LogThreadingPolicy THREADING_POLICY = LogParameters::threading_policy;
		static constexpr LogAsyncPolicy ASYNC_POLICY = LogParameters::async_policy;
		static constexpr LogLevel MINIMUM_LEVEL = LogParameters::minimum_level;
		static constexpr usize QUEUE_SIZE = LogParameters::queue_size;
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

		[[nodiscard]] inline static auto
		get_global_logger() noexcept -> Result<Logger<Parameters>*, LoggerError> {
			if(GLOBAL_LOGGER == nullptr) {
				return Err(LoggerError(LoggerErrorCategory::LoggerNotInitialized));
			}
			return Ok(GLOBAL_LOGGER.get());
		}

		inline static auto set_global_logger(Logger<Parameters>&& logger) noexcept -> void {
			GLOBAL_LOGGER = make_unique<Logger<Parameters>>(std::move(logger));
		}

		template<typename... Args>
		inline static auto MESSAGE(const Option<usize>& thread_id,
								   fmt::format_string<Args...>&& format_string,
								   Args&&... args) noexcept {
			return get_global_logger()
				.expect("Global Logger not initialized!")
				->message(thread_id, std::move(format_string), std::forward<Args>(args)...);
		}

		template<typename... Args>
		inline static auto
		MESSAGE(fmt::format_string<Args...>&& format_string, Args&&... args) noexcept {
			return MESSAGE(None(), std::move(format_string), std::forward<Args>(args)...);
		}

		template<typename... Args>
		inline static auto TRACE(const Option<usize>& thread_id,
								 fmt::format_string<Args...>&& format_string,
								 Args&&... args) noexcept {
			return get_global_logger()
				.expect("Global Logger not initialized!")
				->trace(thread_id, std::move(format_string), std::forward<Args>(args)...);
		}

		template<typename... Args>
		inline static auto
		TRACE(fmt::format_string<Args...>&& format_string, Args&&... args) noexcept {
			return TRACE(None(), std::move(format_string), std::forward<Args>(args)...);
		}

		template<typename... Args>
		inline static auto INFO(const Option<usize>& thread_id,
								fmt::format_string<Args...>&& format_string,
								Args&&... args) noexcept {
			return get_global_logger()
				.expect("Global Logger not initialized!")
				->info(thread_id, std::move(format_string), std::forward<Args>(args)...);
		}

		template<typename... Args>
		inline static auto
		INFO(fmt::format_string<Args...>&& format_string, Args&&... args) noexcept {
			return INFO(None(), std::move(format_string), std::forward<Args>(args)...);
		}

		template<typename... Args>
		inline static auto WARN(const Option<usize>& thread_id,
								fmt::format_string<Args...>&& format_string,
								Args&&... args) noexcept {
			return get_global_logger()
				.expect("Global Logger not initialized!")
				->warn(thread_id, std::move(format_string), std::forward<Args>(args)...);
		}

		template<typename... Args>
		inline static auto
		WARN(fmt::format_string<Args...>&& format_string, Args&&... args) noexcept {
			return WARN(None(), std::move(format_string), std::forward<Args>(args)...);
		}

		template<typename... Args>
		inline static auto ERROR(const Option<usize>& thread_id,
								 fmt::format_string<Args...>&& format_string,
								 Args&&... args) noexcept {
			return get_global_logger()
				.expect("Global Logger not initialized!")
				->error(thread_id, std::move(format_string), std::forward<Args>(args)...);
		}

		template<typename... Args>
		inline static auto
		ERROR(fmt::format_string<Args...>&& format_string, Args&&... args) noexcept {
			return ERROR(None(), std::move(format_string), std::forward<Args>(args)...);
		}
	};

	UniquePtr<Logger<GlobalLog::Parameters>> GlobalLog::GLOBAL_LOGGER = nullptr; // NOLINT

	template<typename... Args>
	inline auto MESSAGE(const Option<usize>& thread_id,
						fmt::format_string<Args...>&& format_string,
						Args&&... args) noexcept {
		return GlobalLog::MESSAGE(thread_id, std::move(format_string), std::forward<Args>(args)...);
	}

	template<typename... Args>
	inline static auto
	MESSAGE(fmt::format_string<Args...>&& format_string, Args&&... args) noexcept {
		return GlobalLog::MESSAGE(None(), std::move(format_string), std::forward<Args>(args)...);
	}

	template<typename... Args>
	inline static auto TRACE(const Option<usize>& thread_id,
							 fmt::format_string<Args...>&& format_string,
							 Args&&... args) noexcept {
		return GlobalLog::TRACE(thread_id, std::move(format_string), std::forward<Args>(args)...);
	}

	template<typename... Args>
	inline static auto TRACE(fmt::format_string<Args...>&& format_string, Args&&... args) noexcept {
		return GlobalLog::TRACE(None(), std::move(format_string), std::forward<Args>(args)...);
	}

	template<typename... Args>
	inline auto INFO(const Option<usize>& thread_id,
					 fmt::format_string<Args...>&& format_string,
					 Args&&... args) noexcept {
		return GlobalLog::INFO(thread_id, std::move(format_string), std::forward<Args>(args)...);
	}

	template<typename... Args>
	inline auto INFO(fmt::format_string<Args...>&& format_string, Args&&... args) noexcept {
		return GlobalLog::INFO(None(), std::move(format_string), std::forward<Args>(args)...);
	}

	template<typename... Args>
	inline auto WARN(const Option<usize>& thread_id,
					 fmt::format_string<Args...>&& format_string,
					 Args&&... args) noexcept {
		return GlobalLog::WARN(thread_id, std::move(format_string), std::forward<Args>(args)...);
	}

	template<typename... Args>
	inline auto WARN(fmt::format_string<Args...>&& format_string, Args&&... args) noexcept {
		return GlobalLog::WARN(None(), std::move(format_string), std::forward<Args>(args)...);
	}

	template<typename... Args>
	inline auto ERROR(const Option<usize>& thread_id,
					  fmt::format_string<Args...>&& format_string,
					  Args&&... args) noexcept {
		return GlobalLog::ERROR(thread_id, std::move(format_string), std::forward<Args>(args)...);
	}

	template<typename... Args>
	inline auto ERROR(fmt::format_string<Args...>&& format_string, Args&&... args) noexcept {
		return GlobalLog::ERROR(None(), std::move(format_string), std::forward<Args>(args)...);
	}

	IGNORE_UNUSED_TEMPLATES_STOP
} // namespace hyperion
