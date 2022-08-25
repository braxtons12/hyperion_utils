/// @file Logger.h
/// @author Braxton Salyer <braxtonsalyer@gmail.com>
/// @brief Hyperion logging facilities.
/// Hyperion's logging facilities are robust and composable.
/// Behavioral (Policy) configuration is configurable at compile time via template parameters,
/// and output configuration is configurable by supplying the desired `hyperion::logging::Sink`s
/// at construction time.
/// @version 0.1
/// @date 2022-07-22
///
/// MIT License
/// @copyright Copyright (c) 2022 Braxton Salyer <braxtonsalyer@gmail.com>
///
/// Permission is hereby granted, free of charge, to any person obtaining a copy
/// of this software and associated documentation files (the "Software"), to deal
/// in the Software without restriction, including without limitation the rights
/// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
/// copies of the Software, and to permit persons to whom the Software is
/// furnished to do so, subject to the following conditions:
///
/// The above copyright notice and this permission notice shall be included in all
/// copies or substantial portions of the Software.
///
/// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
/// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
/// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
/// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
/// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
/// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
/// SOFTWARE.
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
#include <semaphore>
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
		LevelError = 2,
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
		else if(code == value_type::LevelError) {
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

		return make_status_code(error::Errno::Unknown);
	});

namespace hyperion {
	template<>
	struct error::status_code_enum_info<LoggerErrorCategory> {
		using domain_type [[maybe_unused]] = LoggerErrorDomain;
		static constexpr bool value = true;
	};

	using QueueError = logging::QueueError;

	IGNORE_PADDING_START

	namespace detail {
#if HYPERION_HAS_JTHREAD
		using thread = std::jthread;
#else
		using thread = std::thread;
#endif

		class ILogger {
		  public:
			ILogger() noexcept = default;
			ILogger(const ILogger&) noexcept = default;
			ILogger(ILogger&&) noexcept = default;
			virtual ~ILogger() noexcept = default;

			template<logging::Level Level, typename... Args>
			inline auto log(Option<usize> thread_id,
							fmt::format_string<Args...>&& format_string,
							Args&&... args) noexcept -> Result<None, LoggerError> {
				HYPERION_PROFILE_FUNCTION();
				return log(format_entry<Level>(std::move(thread_id),
											   std::move(format_string),
											   std::forward<Args>(args)...));
			}

			template<logging::Level, typename... Args>
			inline auto log(fmt::format_string<Args...>&& format_string, Args&&... args) noexcept
				-> Result<None, LoggerError> {
				HYPERION_PROFILE_FUNCTION();
				return log(None(), std::move(format_string), std::forward<Args>(args)...);
			}

			auto operator=(const ILogger&) noexcept -> ILogger& = default;
			auto operator=(ILogger&&) noexcept -> ILogger& = default;

		  protected:
			virtual auto log(const logging::Entry& entry) noexcept -> Result<None, LoggerError> = 0;
			virtual auto log(logging::Entry&& entry) noexcept -> Result<None, LoggerError> = 0;

			[[nodiscard]] static inline auto
			create_default_sinks() noexcept -> logging::Sinks { // NOLINT(bugprone-exception-escape)
				HYPERION_PROFILE_FUNCTION();
				auto file = logging::FileSink::create_file();
				auto file_sink = logging::make_sink<logging::FileSink>(
					file.expect("Failed to create default log file"));
				auto stderr_sink = logging::make_sink<logging::StderrSink<>>();
				return logging::Sinks({std::move(file_sink), std::move(stderr_sink)});
			}

		  private:
			template<logging::Level Level, typename... Args>
			static inline auto
			format_entry(Option<usize> thread_id, // NOLINT(bugprone-exception-escape)
						 fmt::format_string<Args...>&& format_string,
						 Args&&... args) noexcept -> logging::Entry {
				using namespace std::string_literals;

				HYPERION_PROFILE_FUNCTION();

				const auto timestamp = create_time_stamp();
				const auto entry
					= fmt::format(std::move(format_string), std::forward<Args>(args)...);
				const auto tid = thread_id.is_some() ?
									 thread_id.unwrap() :
									 std::hash<std::thread::id>()(std::this_thread::get_id());
				// ignore(thread_id);
				// ignore(timestamp);

				std::string log_type;
				if constexpr(Level == logging::Level::MESSAGE) {
					log_type = "MESSAGE"s;
				}
				else if constexpr(Level == logging::Level::TRACE) {
					log_type = "TRACE"s;
				}
				else if constexpr(Level == logging::Level::INFO) {
					log_type = "INFO"s;
				}
				else if constexpr(Level == logging::Level::WARN) {
					log_type = "WARN"s;
				}
				else if constexpr(Level == logging::Level::ERROR) {
					log_type = "ERROR"s;
				}

				return logging::make_entry<logging::entry_level_t<Level>>(
					FMT_COMPILE("{0} [Thread ID: {1}] [{2}]: {3}"),
					timestamp,
					tid,
					log_type,
					entry);
				//	return make_entry<entry_level_t<Level>>(FMT_COMPILE("[{0}]: {1}"),
				//	                                        log_type,
				//	                                        entry);
			}

			[[nodiscard]] static inline auto
				create_time_stamp() noexcept -> std::string { // NOLINT(bugprone-exception-escape)
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
		};

		template<logging::Level MinimumLevel = logging::DefaultParameters::minimum_level,
				 logging::ThreadingPolicy ThreadingPolicy
				 = logging::DefaultParameters::threading_policy,
				 logging::AsyncPolicy AsyncPolicy = logging::DefaultParameters::async_policy,
				 usize QueueSize = logging::DefaultParameters::queue_size>
		class LogBase;
	} // namespace detail

	/// @brief Hyperion logging type for formatted logging.
	/// Uses fmtlib/fmt for entry formatting and stylizing
	///
	/// @tparam LogParameters - The parameters for how this logger should operate
	template<logging::ParametersType LogParameters = logging::DefaultParameters>
	class Logger final : public detail::LogBase<LogParameters::minimum_level,
												LogParameters::threading_policy,
												LogParameters::async_policy,
												LogParameters::queue_size> {
	  public:
		[[maybe_unused]] static constexpr logging::ThreadingPolicy THREADING_POLICY
			= LogParameters::threading_policy;
		[[maybe_unused]] static constexpr logging::AsyncPolicy ASYNC_POLICY
			= LogParameters::async_policy;
		[[maybe_unused]] static constexpr logging::Level MINIMUM_LEVEL
			= LogParameters::minimum_level;
		[[maybe_unused]] static constexpr usize QUEUE_SIZE = LogParameters::queue_size;
		using LogBase = detail::LogBase<LogParameters::minimum_level,
										LogParameters::threading_policy,
										LogParameters::async_policy,
										LogParameters::queue_size>;

		Logger() = default;
		explicit Logger(logging::Sinks&& sinks) noexcept : LogBase(std::move(sinks)) {
		}
		Logger(const Logger& logger) noexcept = delete;
		Logger(Logger&& logger) noexcept = default;

		~Logger() noexcept final = default;

		template<typename... Args>
		inline auto message(const Option<usize>& thread_id,
							fmt::format_string<Args...>&& format_string,
							Args&&... args) noexcept -> void {
			ignore(this->template log<logging::Level::MESSAGE>(thread_id,
															   std::move(format_string),
															   std::forward<Args>(args)...));
		}

		template<typename... Args>
		inline auto
		message(fmt::format_string<Args...>&& format_string, Args&&... args) noexcept -> void {
			ignore(message(None(), std::move(format_string), std::forward<Args>(args)...));
		}

		template<typename... Args>
		inline auto trace(const Option<usize>& thread_id,
						  fmt::format_string<Args...>&& format_string,
						  Args&&... args) noexcept -> void {
			ignore(this->template log<logging::Level::TRACE>(thread_id,
															 std::move(format_string),
															 std::forward<Args>(args)...));
		}

		template<typename... Args>
		inline auto
		trace(fmt::format_string<Args...>&& format_string, Args&&... args) noexcept -> void {
			ignore(trace(None(), std::move(format_string), std::forward<Args>(args)...));
		}

		template<typename... Args>
		inline auto info(const Option<usize>& thread_id,
						 fmt::format_string<Args...>&& format_string,
						 Args&&... args) noexcept -> void {
			ignore(this->template log<logging::Level::INFO>(thread_id,
															std::move(format_string),
															std::forward<Args>(args)...));
		}

		template<typename... Args>
		inline auto
		info(fmt::format_string<Args...>&& format_string, Args&&... args) noexcept -> void {
			ignore(info(None(), std::move(format_string), std::forward<Args>(args)...));
		}

		template<typename... Args>
		inline auto warn(const Option<usize>& thread_id,
						 fmt::format_string<Args...>&& format_string,
						 Args&&... args) noexcept -> void {
			ignore(this->template log<logging::Level::WARN>(thread_id,
															std::move(format_string),
															std::forward<Args>(args)...));
		}

		template<typename... Args>
		inline auto
		warn(fmt::format_string<Args...>&& format_string, Args&&... args) noexcept -> void {
			ignore(warn(None(), std::move(format_string), std::forward<Args>(args)...));
		}

		template<typename... Args>
		inline auto error(const Option<usize>& thread_id,
						  fmt::format_string<Args...>&& format_string,
						  Args&&... args) noexcept -> void {
			ignore(this->template log<logging::Level::ERROR>(thread_id,
															 std::move(format_string),
															 std::forward<Args>(args)...));
		}

		template<typename... Args>
		inline auto
		error(fmt::format_string<Args...>&& format_string, Args&&... args) noexcept -> void {
			ignore(error(None(), std::move(format_string), std::forward<Args>(args)...));
		}

		template<typename... Args>
		inline auto message_checked(const Option<usize>& thread_id,
									fmt::format_string<Args...>&& format_string,
									Args&&... args) noexcept {
			return this->template log<logging::Level::MESSAGE>(thread_id,
															   std::move(format_string),
															   std::forward<Args>(args)...);
		}

		template<typename... Args>
		inline auto
		message_checked(fmt::format_string<Args...>&& format_string, Args&&... args) noexcept {
			return message(None(), std::move(format_string), std::forward<Args>(args)...);
		}

		template<typename... Args>
		inline auto trace_checked(const Option<usize>& thread_id,
								  fmt::format_string<Args...>&& format_string,
								  Args&&... args) noexcept {
			return this->template log<logging::Level::TRACE>(thread_id,
															 std::move(format_string),
															 std::forward<Args>(args)...);
		}

		template<typename... Args>
		inline auto
		trace_checked(fmt::format_string<Args...>&& format_string, Args&&... args) noexcept {
			return trace(None(), std::move(format_string), std::forward<Args>(args)...);
		}

		template<typename... Args>
		inline auto info_checked(const Option<usize>& thread_id,
								 fmt::format_string<Args...>&& format_string,
								 Args&&... args) noexcept {
			return this->template log<logging::Level::INFO>(thread_id,
															std::move(format_string),
															std::forward<Args>(args)...);
		}

		template<typename... Args>
		inline auto
		info_checked(fmt::format_string<Args...>&& format_string, Args&&... args) noexcept {
			return info(None(), std::move(format_string), std::forward<Args>(args)...);
		}

		template<typename... Args>
		inline auto warn_checked(const Option<usize>& thread_id,
								 fmt::format_string<Args...>&& format_string,
								 Args&&... args) noexcept {
			return this->template log<logging::Level::WARN>(thread_id,
															std::move(format_string),
															std::forward<Args>(args)...);
		}

		template<typename... Args>
		inline auto
		warn_checked(fmt::format_string<Args...>&& format_string, Args&&... args) noexcept {
			return warn(None(), std::move(format_string), std::forward<Args>(args)...);
		}

		template<typename... Args>
		inline auto error_checked(const Option<usize>& thread_id,
								  fmt::format_string<Args...>&& format_string,
								  Args&&... args) noexcept {
			return this->template log<logging::Level::ERROR>(thread_id,
															 std::move(format_string),
															 std::forward<Args>(args)...);
		}

		template<typename... Args>
		inline auto
		error_checked(fmt::format_string<Args...>&& format_string, Args&&... args) noexcept {
			return error(None(), std::move(format_string), std::forward<Args>(args)...);
		}

		auto operator=(const Logger& logger) noexcept -> Logger& = delete;
		auto operator=(Logger&& logger) noexcept -> Logger& = default;
	};
	IGNORE_PADDING_STOP

	struct GlobalLog {
		static UniquePtr<detail::ILogger> GLOBAL_LOGGER; // NOLINT

		[[nodiscard]] static inline auto
		get_global_logger() noexcept -> Result<detail::ILogger*, LoggerError> {
			if(GLOBAL_LOGGER == nullptr) {
				return Err(LoggerError(LoggerErrorCategory::LoggerNotInitialized));
			}
			return Ok(GLOBAL_LOGGER.get());
		}

		template<logging::ParametersType Parameters>
		[[maybe_unused]] static inline auto
		set_global_logger(hyperion::UniquePtr<Logger<Parameters>>&& logger) noexcept -> void {
			GLOBAL_LOGGER = std::move(logger);
		}

		template<typename... Args>
		static inline auto MESSAGE(const Option<usize>& thread_id,
								   fmt::format_string<Args...>&& format_string,
								   Args&&... args) noexcept -> Result<None, LoggerError> {
			return get_global_logger().and_then([&thread_id,
												 &format_string,
												 ... _args = std::forward<Args>(args)](
													auto* logger) mutable noexcept {
				return logger->template log<logging::Level::MESSAGE>(thread_id,
																	 std::move(format_string),
																	 std::forward<Args>(_args)...);
			});
		}

		template<typename... Args>
		static inline auto
		MESSAGE(fmt::format_string<Args...>&& format_string, Args&&... args) noexcept {
			return MESSAGE(None(), std::move(format_string), std::forward<Args>(args)...);
		}

		template<typename... Args>
		static inline auto TRACE(const Option<usize>& thread_id,
								 fmt::format_string<Args...>&& format_string,
								 Args&&... args) noexcept -> Result<None, LoggerError> {
			return get_global_logger().and_then([&thread_id,
												 &format_string,
												 ... _args = std::forward<Args>(args)](
													auto* logger) mutable noexcept {
				return logger->template log<logging::Level::TRACE>(thread_id,
																   std::move(format_string),
																   std::forward<Args>(_args)...);
			});
		}

		template<typename... Args>
		static inline auto
		TRACE(fmt::format_string<Args...>&& format_string, Args&&... args) noexcept {
			return TRACE(None(), std::move(format_string), std::forward<Args>(args)...);
		}

		template<typename... Args>
		static inline auto INFO(const Option<usize>& thread_id,
								fmt::format_string<Args...>&& format_string,
								Args&&... args) noexcept -> Result<None, LoggerError> {
			return get_global_logger().and_then(
				[&thread_id, &format_string, ... _args = std::forward<Args>(args)](
					auto* logger) mutable noexcept {
					return logger->template log<logging::Level::INFO>(thread_id,
																	  std::move(format_string),
																	  std::forward<Args>(_args)...);
				});
		}

		template<typename... Args>
		static inline auto
		INFO(fmt::format_string<Args...>&& format_string, Args&&... args) noexcept {
			return INFO(None(), std::move(format_string), std::forward<Args>(args)...);
		}

		template<typename... Args>
		static inline auto WARN(const Option<usize>& thread_id,
								fmt::format_string<Args...>&& format_string,
								Args&&... args) noexcept -> Result<None, LoggerError> {
			return get_global_logger().and_then(
				[&thread_id, &format_string, ... _args = std::forward<Args>(args)](
					auto* logger) mutable noexcept {
					return logger->template log<logging::Level::WARN>(thread_id,
																	  std::move(format_string),
																	  std::forward<Args>(_args)...);
				});
		}

		template<typename... Args>
		static inline auto
		WARN(fmt::format_string<Args...>&& format_string, Args&&... args) noexcept {
			return WARN(None(), std::move(format_string), std::forward<Args>(args)...);
		}

		template<typename... Args>
		static inline auto ERROR(const Option<usize>& thread_id,
								 fmt::format_string<Args...>&& format_string,
								 Args&&... args) noexcept -> Result<None, LoggerError> {
			return get_global_logger().and_then([&thread_id,
												 &format_string,
												 ... _args = std::forward<Args>(args)](
													auto* logger) mutable noexcept {
				return logger->template log<logging::Level::ERROR>(thread_id,
																   std::move(format_string),
																   std::forward<Args>(_args)...);
			});
		}

		template<typename... Args>
		static inline auto
		ERROR(fmt::format_string<Args...>&& format_string, Args&&... args) noexcept {
			return ERROR(None(), std::move(format_string), std::forward<Args>(args)...);
		}
	};

	IGNORE_UNUSED_TEMPLATES_START

	template<typename... Args>
	[[maybe_unused]] static inline auto MESSAGE(const Option<usize>& thread_id,
												fmt::format_string<Args...>&& format_string,
												Args&&... args) noexcept -> void {
		ignore(
			GlobalLog::MESSAGE(thread_id, std::move(format_string), std::forward<Args>(args)...));
	}

	template<typename... Args>
	[[maybe_unused]] static inline auto
	MESSAGE(fmt::format_string<Args...>&& format_string, Args&&... args) noexcept -> void {
		ignore(GlobalLog::MESSAGE(None(), std::move(format_string), std::forward<Args>(args)...));
	}

	template<typename... Args>
	[[maybe_unused]] static inline auto TRACE(const Option<usize>& thread_id,
											  fmt::format_string<Args...>&& format_string,
											  Args&&... args) noexcept -> void {
		ignore(GlobalLog::TRACE(thread_id, std::move(format_string), std::forward<Args>(args)...));
	}

	template<typename... Args>
	[[maybe_unused]] static inline auto
	TRACE(fmt::format_string<Args...>&& format_string, Args&&... args) noexcept -> void {
		ignore(GlobalLog::TRACE(None(), std::move(format_string), std::forward<Args>(args)...));
	}

	template<typename... Args>
	[[maybe_unused]] static inline auto INFO(const Option<usize>& thread_id,
											 fmt::format_string<Args...>&& format_string,
											 Args&&... args) noexcept -> void {
		ignore(GlobalLog::INFO(thread_id, std::move(format_string), std::forward<Args>(args)...));
	}

	template<typename... Args>
	[[maybe_unused]] static inline auto
	INFO(fmt::format_string<Args...>&& format_string, Args&&... args) noexcept -> void {
		ignore(GlobalLog::INFO(None(), std::move(format_string), std::forward<Args>(args)...));
	}

	template<typename... Args>
	[[maybe_unused]] static inline auto WARN(const Option<usize>& thread_id,
											 fmt::format_string<Args...>&& format_string,
											 Args&&... args) noexcept -> void {
		ignore(GlobalLog::WARN(thread_id, std::move(format_string), std::forward<Args>(args)...));
	}

	template<typename... Args>
	[[maybe_unused]] static inline auto
	WARN(fmt::format_string<Args...>&& format_string, Args&&... args) noexcept -> void {
		ignore(GlobalLog::WARN(None(), std::move(format_string), std::forward<Args>(args)...));
	}

	template<typename... Args>
	[[maybe_unused]] static inline auto ERROR(const Option<usize>& thread_id,
											  fmt::format_string<Args...>&& format_string,
											  Args&&... args) noexcept -> void {
		ignore(GlobalLog::ERROR(thread_id, std::move(format_string), std::forward<Args>(args)...));
	}

	template<typename... Args>
	[[maybe_unused]] static inline auto
	ERROR(fmt::format_string<Args...>&& format_string, Args&&... args) noexcept -> void {
		ignore(GlobalLog::ERROR(None(), std::move(format_string), std::forward<Args>(args)...));
	}

	template<typename... Args>
	[[maybe_unused]] static inline auto
	MESSAGE_CHECKED(const Option<usize>& thread_id,
					fmt::format_string<Args...>&& format_string,
					Args&&... args) noexcept -> Result<None, LoggerError> {
		return GlobalLog::MESSAGE(thread_id, std::move(format_string), std::forward<Args>(args)...);
	}

	template<typename... Args>
	[[maybe_unused]] static inline auto
	MESSAGE_CHECKED(fmt::format_string<Args...>&& format_string, Args&&... args) noexcept
		-> Result<None, LoggerError> {
		return GlobalLog::MESSAGE(None(), std::move(format_string), std::forward<Args>(args)...);
	}

	template<typename... Args>
	[[maybe_unused]] static inline auto
	TRACE_CHECKED(const Option<usize>& thread_id,
				  fmt::format_string<Args...>&& format_string,
				  Args&&... args) noexcept -> Result<None, LoggerError> {
		return GlobalLog::TRACE(thread_id, std::move(format_string), std::forward<Args>(args)...);
	}

	template<typename... Args>
	[[maybe_unused]] static inline auto
	TRACE_CHECKED(fmt::format_string<Args...>&& format_string, Args&&... args) noexcept
		-> Result<None, LoggerError> {
		return GlobalLog::TRACE(None(), std::move(format_string), std::forward<Args>(args)...);
	}

	template<typename... Args>
	[[maybe_unused]] static inline auto
	INFO_CHECKED(const Option<usize>& thread_id,
				 fmt::format_string<Args...>&& format_string,
				 Args&&... args) noexcept -> Result<None, LoggerError> {
		return GlobalLog::INFO(thread_id, std::move(format_string), std::forward<Args>(args)...);
	}

	template<typename... Args>
	[[maybe_unused]] static inline auto
	INFO_CHECKED(fmt::format_string<Args...>&& format_string, Args&&... args) noexcept
		-> Result<None, LoggerError> {
		return GlobalLog::INFO(None(), std::move(format_string), std::forward<Args>(args)...);
	}

	template<typename... Args>
	[[maybe_unused]] static inline auto
	WARN_CHECKED(const Option<usize>& thread_id,
				 fmt::format_string<Args...>&& format_string,
				 Args&&... args) noexcept -> Result<None, LoggerError> {
		return GlobalLog::WARN(thread_id, std::move(format_string), std::forward<Args>(args)...);
	}

	template<typename... Args>
	[[maybe_unused]] static inline auto
	WARN_CHECKED(fmt::format_string<Args...>&& format_string, Args&&... args) noexcept
		-> Result<None, LoggerError> {
		return GlobalLog::WARN(None(), std::move(format_string), std::forward<Args>(args)...);
	}

	template<typename... Args>
	[[maybe_unused]] static inline auto
	ERROR_CHECKED(const Option<usize>& thread_id,
				  fmt::format_string<Args...>&& format_string,
				  Args&&... args) noexcept -> Result<None, LoggerError> {
		return GlobalLog::ERROR(thread_id, std::move(format_string), std::forward<Args>(args)...);
	}

	template<typename... Args>
	[[maybe_unused]] static inline auto
	ERROR_CHECKED(fmt::format_string<Args...>&& format_string, Args&&... args) noexcept
		-> Result<None, LoggerError> {
		return GlobalLog::ERROR(None(), std::move(format_string), std::forward<Args>(args)...);
	}
	IGNORE_UNUSED_TEMPLATES_STOP

	namespace detail {
		template<logging::Level MinimumLevel, logging::AsyncPolicy AsyncPolicy, usize QueueSize>
		class LogBase<MinimumLevel,
					  logging::ThreadingPolicy::SingleThreaded,
					  AsyncPolicy,
					  QueueSize> : public detail::ILogger {
		  public:
			[[maybe_unused]] static constexpr auto THREADING_POLICY
				= logging::ThreadingPolicy::SingleThreaded;
			static constexpr auto ASYNC_POLICY = AsyncPolicy;
			static constexpr auto MINIMUM_LEVEL = MinimumLevel;

			LogBase() : LogBase(detail::ILogger::create_default_sinks()) {
			}
			explicit LogBase(logging::Sinks&& sinks) noexcept : m_sinks(std::move(sinks)) {
			}
			LogBase(const LogBase&) = delete;
			LogBase(LogBase&&) noexcept = default;
			~LogBase() noexcept override = default;

			auto operator=(const LogBase&) -> LogBase& = delete;
			auto operator=(LogBase&&) noexcept -> LogBase& = default;

		  protected:
			inline auto log(const logging::Entry& entry) // NOLINT(bugprone-exception-escape)
				noexcept -> Result<None, LoggerError> final {

				HYPERION_PROFILE_FUNCTION();
				if(entry.level() > MINIMUM_LEVEL) {
					std::ranges::for_each(m_sinks, [&entry](const auto& sink) noexcept -> void {
						sink->sink(entry);
					});
				}

				return Ok();
			}

			inline auto log(logging::Entry&& entry) // NOLINT(bugprone-exception-escape)
				noexcept -> Result<None, LoggerError> final {

				HYPERION_PROFILE_FUNCTION();
				if(entry.level() > MINIMUM_LEVEL) {
					std::ranges::for_each(
						m_sinks,
						[_entry = std::move(entry)](const auto& sink) noexcept -> void {
							sink->sink(_entry);
						});
				}

				return Ok();
			}

		  private:
			logging::Sinks m_sinks;
		};

		template<logging::Level MinimumLevel, logging::AsyncPolicy AsyncPolicy, usize QueueSize>
		class LogBase<MinimumLevel,
					  logging::ThreadingPolicy::SingleThreadedAsync,
					  AsyncPolicy,
					  QueueSize> : public detail::ILogger {
		  public:
			[[maybe_unused]] static constexpr auto THREADING_POLICY
				= logging::ThreadingPolicy::SingleThreadedAsync;
			static constexpr auto ASYNC_POLICY = AsyncPolicy;
			static constexpr auto MINIMUM_LEVEL = MinimumLevel;
			static constexpr usize QUEUE_SIZE = QueueSize;

			LogBase() : LogBase(detail::ILogger::create_default_sinks()) {
			}
			explicit LogBase(logging::Sinks&& sinks) noexcept
				: m_sinks(std::move(sinks)), m_queue() {
#if HYPERION_HAS_JTHREAD
				m_logging_thread = detail::thread(
					[this](const std::stop_token& token) { message_thread_function(token); });
#else
				m_logging_thread = detail::thread([this]() { message_thread_function(); });
#endif
			}
			LogBase(const LogBase&) = delete;
			LogBase(LogBase&&) = delete;
			~LogBase() noexcept override {
				request_thread_stop();
				m_logging_thread.join();
			}

			auto operator=(const LogBase&) -> LogBase& = delete;
			auto operator=(LogBase&&) -> LogBase& = delete;

		  protected:
			inline auto log(const logging::Entry& entry) // NOLINT(bugprone-exception-escape)
				noexcept -> Result<None, LoggerError> final {

				HYPERION_PROFILE_FUNCTION();
				if(entry.level() > MINIMUM_LEVEL) {
					m_signal.release(1);
					if constexpr(ASYNC_POLICY == logging::AsyncPolicy::DropWhenFull) {
						return m_queue.push(entry).map_err(
							[]([[maybe_unused]] const QueueError& error) {
								return LoggerError(LoggerErrorCategory::QueueingError);
							});
					}
					else {
						m_queue.push(entry);
					}
				}

				return Ok();
			}

			inline auto log(logging::Entry&& entry) // NOLINT(bugprone-exception-escape)
				noexcept -> Result<None, LoggerError> final {

				HYPERION_PROFILE_FUNCTION();
				if(entry.level() > MINIMUM_LEVEL) {
					m_signal.release(1);
					if constexpr(ASYNC_POLICY == logging::AsyncPolicy::DropWhenFull) {
						return m_queue.push(std::move(entry))
							.map_err([]([[maybe_unused]] const QueueError& error) {
								return LoggerError(LoggerErrorCategory::QueueingError);
							});
					}
					else {
						m_queue.push(std::move(entry));
					}
				}

				return Ok();
			}

		  private:
			[[nodiscard]] static inline consteval auto
			get_queue_policy() noexcept -> logging::QueuePolicy {
				if constexpr(ASYNC_POLICY == logging::AsyncPolicy::DropWhenFull) {
					return logging::QueuePolicy::ErrWhenFull;
				}
				else if constexpr(ASYNC_POLICY == logging::AsyncPolicy::OverwriteWhenFull) {
					return logging::QueuePolicy::OverwriteWhenFull;
				}
				else {
					return logging::QueuePolicy::BlockWhenFull;
				}
			}

			using Queue = logging::Queue<logging::Entry, get_queue_policy(), QUEUE_SIZE>;

			logging::Sinks m_sinks;
			Queue m_queue;
			std::counting_semaphore<QUEUE_SIZE> m_signal = std::counting_semaphore<QUEUE_SIZE>(0);

#if !HYPERION_HAS_JTHREAD
			std::atomic_bool m_exit_flag = false;
#endif
			detail::thread m_logging_thread;

			inline auto request_thread_stop() noexcept -> void {
#if !HYPERION_HAS_JTHREAD
				m_exit_flag.store(true);
#else
				m_logging_thread.request_stop();
#endif
				m_signal.release(1);
			}

			inline auto try_read() noexcept -> Result<logging::Entry, QueueError> {
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
					// waiting for the semaphore __after__ we attempt the read can reduce latency
					// under extreme contention
					m_signal.acquire();
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

		template<logging::Level MinimumLevel, logging::AsyncPolicy AsyncPolicy, usize QueueSize>
		class LogBase<MinimumLevel, logging::ThreadingPolicy::MultiThreaded, AsyncPolicy, QueueSize>
			: public detail::ILogger {
		  public:
			[[maybe_unused]] static constexpr auto THREADING_POLICY
				= logging::ThreadingPolicy::MultiThreaded;
			static constexpr auto ASYNC_POLICY = AsyncPolicy;
			static constexpr auto MINIMUM_LEVEL = MinimumLevel;

			LogBase() : LogBase(detail::ILogger::create_default_sinks()) {
			}
			explicit LogBase(logging::Sinks&& sinks) noexcept : m_sinks(std::move(sinks)) {
			}
			LogBase(const LogBase&) = delete;
			LogBase(LogBase&&) noexcept = default;
			~LogBase() noexcept override = default;

			auto operator=(const LogBase&) -> LogBase& = delete;
			auto operator=(LogBase&&) noexcept -> LogBase& = default;

		  protected:
			inline auto log(const logging::Entry& entry) // NOLINT(bugprone-exception-escape)
				noexcept -> Result<None, LoggerError> final {

				HYPERION_PROFILE_FUNCTION();
				if(entry.level() > MINIMUM_LEVEL) {
					auto sinks_guard = m_sinks.write();
					std::ranges::for_each(
						*sinks_guard,
						[&entry](const auto& sink) noexcept -> void { sink->sink(entry); });
				}

				return Ok();
			}

			inline auto log(logging::Entry&& entry) // NOLINT(bugprone-exception-escape)
				noexcept -> Result<None, LoggerError> final {

				HYPERION_PROFILE_FUNCTION();
				if(entry.level() > MINIMUM_LEVEL) {
					auto sinks_guard = m_sinks.write();
					std::ranges::for_each(
						*sinks_guard,
						[_entry = std::move(entry)](const auto& sink) noexcept -> void {
							sink->sink(_entry);
						});
				}

				return Ok();
			}

		  private:
			ReadWriteLock<logging::Sinks> m_sinks;
		};

		template<logging::Level MinimumLevel, logging::AsyncPolicy AsyncPolicy, usize QueueSize>
		class LogBase<MinimumLevel,
					  logging::ThreadingPolicy::MultiThreadedAsync,
					  AsyncPolicy,
					  QueueSize> : public detail::ILogger {
		  public:
			[[maybe_unused]] static constexpr auto THREADING_POLICY
				= logging::ThreadingPolicy::MultiThreadedAsync;
			static constexpr auto ASYNC_POLICY = AsyncPolicy;
			static constexpr auto MINIMUM_LEVEL = MinimumLevel;
			static constexpr usize QUEUE_SIZE = QueueSize;

			LogBase() : LogBase(detail::ILogger::create_default_sinks()) {
			}
			explicit LogBase(logging::Sinks&& sinks) noexcept
				: m_sinks(std::move(sinks)), m_queue() {
#if HYPERION_HAS_JTHREAD
				m_logging_thread = detail::thread(
					[this](const std::stop_token& token) { message_thread_function(token); });
#else
				m_logging_thread = detail::thread([this]() { message_thread_function(); });
#endif
			}
			LogBase(const LogBase&) = delete;
			LogBase(LogBase&&) = delete;
			~LogBase() noexcept override {
				request_thread_stop();
				m_logging_thread.join();
			}

			auto operator=(const LogBase&) -> LogBase& = delete;
			auto operator=(LogBase&& logger) -> LogBase& = delete;

		  protected:
			inline auto log(const logging::Entry& entry) // NOLINT(bugprone-exception-escape)
				noexcept -> Result<None, LoggerError> final {

				HYPERION_PROFILE_FUNCTION();
				if(entry.level() > MINIMUM_LEVEL) {
					if constexpr(ASYNC_POLICY == logging::AsyncPolicy::DropWhenFull) {
						return m_queue.push(entry).map_err(
							[]([[maybe_unused]] const QueueError& error) {
								return LoggerError(LoggerErrorCategory::QueueingError);
							});
					}
					else {
						m_queue.push(entry);
					}
				}

				return Ok();
			}

			inline auto log(logging::Entry&& entry) // NOLINT(bugprone-exception-escape)
				noexcept -> Result<None, LoggerError> final {

				HYPERION_PROFILE_FUNCTION();
				if(entry.level() > MINIMUM_LEVEL) {
					if constexpr(ASYNC_POLICY == logging::AsyncPolicy::DropWhenFull) {
						return m_queue.push(std::move(entry))
							.map_err([]([[maybe_unused]] const QueueError& error) {
								return LoggerError(LoggerErrorCategory::QueueingError);
							});
					}
					else {
						m_queue.push(std::move(entry));
					}
				}

				return Ok();
			}

		  private:
			[[nodiscard]] static inline consteval auto
			get_queue_policy() noexcept -> logging::QueuePolicy {
				if constexpr(ASYNC_POLICY == logging::AsyncPolicy::DropWhenFull) {
					return logging::QueuePolicy::ErrWhenFull;
				}
				else if constexpr(ASYNC_POLICY == logging::AsyncPolicy::OverwriteWhenFull) {
					return logging::QueuePolicy::OverwriteWhenFull;
				}
				else {
					return logging::QueuePolicy::BlockWhenFull;
				}
			}

			using Queue = logging::Queue<logging::Entry, get_queue_policy(), QUEUE_SIZE>;

			logging::Sinks m_sinks;
			Queue m_queue;
			std::counting_semaphore<QUEUE_SIZE> m_signal = std::counting_semaphore<QUEUE_SIZE>(0);

#if !HYPERION_HAS_JTHREAD
			std::atomic_bool m_exit_flag = false;
#endif
			detail::thread m_logging_thread;

			inline auto request_thread_stop() noexcept -> void {
#if !HYPERION_HAS_JTHREAD
				m_exit_flag.store(true);
#else
				m_logging_thread.request_stop();
#endif
				m_signal.release(1);
			}

			inline auto try_read() noexcept -> Result<logging::Entry, QueueError> {
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
					// waiting for the semaphore __after__ we attempt the read can reduce latency
					// under extreme contention
					m_signal.acquire();
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
} // namespace hyperion
