/// @file Sink.h
/// @author Braxton Salyer <braxtonsalyer@gmail.com>
/// @brief Provides basic logging sink implementations
/// @version 0.1
/// @date 2021-10-29
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

#include <Hyperion/FmtIO.h>
#include <Hyperion/Format.h>
#include <Hyperion/Ignore.h>
#include <Hyperion/Result.h>
#include <Hyperion/filesystem/File.h>
#include <Hyperion/logging/SinkType.h>
#include <Hyperion/mpl/List.h>
#include <cstddef>
#include <filesystem>
#include <type_traits>
#include <vector>

namespace hyperion {
	using namespace std::literals::string_literals;

	IGNORE_PADDING_START
	/// @brief Basic logging sink that writes to a specified file
	/// @ingroup logging
	/// @headerfile "Hyperion/logging/Sink.h"
	class FileSink final {
	  public:
		static constexpr auto DEFAULT_FILE_NAME = "Hyperion";
		static constexpr auto DEFAULT_FILE_SUBDIRECTORY = "Hyperion";

		/// @brief Delete default constructor
		/// @ingroup logging
		/// @headerfile "Hyperion/logging/Sink.h"
		FileSink() noexcept = delete;
		/// @brief Constructs a `FileSink` that will write to the given file
		///
		/// Constructs a `FileSink` with the default `LogLevel` (`LogLevel::MESSAGE`) that will sink
		/// logging entries at or above that level to the given `fs::File`
		///
		/// @param file - The file to write entries to
		/// @ingroup logging
		/// @headerfile "Hyperion/logging/Sink.h"
		constexpr explicit FileSink(fs::File&& file) noexcept : m_file(std::move(file)) {
		}
		/// @brief Constructs a `FileSink` that will write to the given file
		///
		/// Constructs a `FileSink` that will sink logging entries at or above the given `LogLevel`
		/// to the given `fs::File`
		///
		/// @param file - The file to write entries to
		/// @param level - The to configure this sink for
		/// @ingroup logging
		/// @headerfile "Hyperion/logging/Sink.h"
		constexpr FileSink(fs::File&& file, LogLevel level) noexcept
			: m_file(std::move(file)), m_log_level(level) {
		}
		/// @brief Deleted copy constructor
		/// @ingroup logging
		/// @headerfile "Hyperion/logging/Sink.h"
		FileSink(const FileSink& sink) = delete;
		/// @brief Move constructor
		/// @ingroup logging
		/// @headerfile "Hyperion/logging/Sink.h"
		constexpr FileSink(FileSink&& sink) noexcept = default;
		/// @brief Destructor
		/// @ingroup logging
		/// @headerfile "Hyperion/logging/Sink.h"
		constexpr ~FileSink() noexcept = default;

		/// @brief Sinks the given entry, writing it to the file associated with this
		///
		/// Sinks the given entry if it is at or above the configured `LogLevel`, writing it to the
		/// file. Otherwise, does nothing
		///
		/// @param entry - The entry to sink
		/// @ingroup logging
		/// @headerfile "Hyperion/logging/Sink.h"
		inline auto sink_entry(const Entry& entry) noexcept -> void {
			if(entry.level() >= m_log_level) {
				auto res = m_file.println("{}", entry.entry());
				ignore(res.is_ok());
			}
		}

		/// @brief Sinks the given entry, writing it to the file associated with this
		///
		/// Sinks the given entry if it is at or above the configured `LogLevel`, writing it to the
		/// file. Otherwise, does nothing
		///
		/// @param entry - The entry to sink
		/// @ingroup logging
		/// @headerfile "Hyperion/logging/Sink.h"
		inline auto sink_entry(Entry&& entry) noexcept -> void {
			if(entry.level() >= m_log_level) {
				auto res = m_file.println("{}", entry.entry());
				ignore(res.is_ok());
			}
		}

		/// @brief Returns the currently configured `LogLevel` for this sink
		/// @ingroup logging
		/// @headerfile "Hyperion/logging/Sink.h"
		[[nodiscard]] inline auto get_log_level() const noexcept -> LogLevel {
			return m_log_level;
		}

		/// @brief Sets the configured `LogLevel` for this sink to the given one
		///
		/// @param level - The `LogLevel` to configure this sink to
		/// @ingroup logging
		/// @headerfile "Hyperion/logging/Sink.h"
		inline auto set_log_level(LogLevel level) noexcept -> void {
			m_log_level = level;
		}

		/// @brief Creates a file for logging in the system temporary files directory
		///
		/// Attempts to create an `fs::File` with the given root file name (it will be decorated
		/// with a timestamp and ".log" extension) in the given subdirectory of the the system
		/// temporary files directory. Attempts to create the subdirectory if it does not exist.
		///
		/// # Errors
		/// Returns an Error if:
		/// - accessing the system temporary files directory fails
		/// - creating or accessing the given subdirectory fails
		/// - creating a file with the given root file name fails
		///
		/// @param root_file_name - The root of the name for the file, with no timestamps or
		/// extension (i.e. "HyperionLog"). The used file name will have a timestamp prepended and
		/// the ".log" extension appended to this
		/// @param subdirectory_name - The relative subdirectory in the temporary files directory to
		/// create the file in (for "$temp_files_directory/subdirectory_name", just pass
		/// "subdirectory_name", e.g. for the "Hyperion" subdirectory, pass "Hyperion")
		///
		/// @return The `fs::File` on success, `error::SystemError` on error
		/// @ingroup logging
		/// @headerfile "Hyperion/logging/Sink.h"
		[[nodiscard]] inline static auto
		create_file(const std::string& root_file_name = DEFAULT_FILE_NAME, // NOLINT
					const std::string& subdirectory_name = DEFAULT_FILE_SUBDIRECTORY)
			-> Result<fs::File> {
			return get_temp_directory()
				.and_then(
					[&](std::filesystem::path temp_directory) -> Result<std::filesystem::path> {
						temp_directory.append(subdirectory_name);
						return create_directory(temp_directory);
					})
				.and_then([&](std::filesystem::path file_directory) -> Result<fs::File> {
					const auto time_string = create_time_stamp();
					file_directory.append(time_string + " "s + root_file_name);
					file_directory.replace_extension("log"s);
					return fs::File::open(file_directory);
				});
		}

		/// @brief Deleted copy-assignment operator
		/// @ingroup logging
		/// @headerfile "Hyperion/logging/Sink.h"
		auto operator=(const FileSink& sink) -> FileSink& = delete;
		/// @brief Move-assignment operator
		/// @ingroup logging
		/// @headerfile "Hyperion/logging/Sink.h"
		constexpr auto operator=(FileSink&& sink) noexcept -> FileSink& = default;

	  private:
		fs::File m_file;
		LogLevel m_log_level = LogLevel::MESSAGE;

		/// @brief Returns the system temporary files directory
		///
		/// # Errors
		/// Returns an error if accessing the temporary files directory fails
		///
		/// @return The `std::filesystem::path` to the temporary files directory on success,
		/// `FileCreationError` on error
		[[nodiscard]] inline static auto get_temp_directory() -> Result<std::filesystem::path> {
			std::error_code err_code;
			auto temp_dir = std::filesystem::temp_directory_path(err_code);

			if(err_code.value() != 0) {
				return Err(error::SystemError(
					static_cast<error::SystemError::value_type>(err_code.value())));
			}
			else {
				return Ok(temp_dir);
			}
		}

		/// @brief Creates the subdirectory for the given absolute subdirectory path
		///
		/// # Errors
		/// Returns an error if creating the subdirectory fails
		///
		/// @param subdirectory_path - The absolute path for the subdirectory
		///
		/// @return The given `std::filesystem::path` to the subdirectory on success,
		/// `FileCreationError` on error
		[[nodiscard]] inline static auto
		create_directory(const std::filesystem::path& subdirectory_path)
			-> Result<std::filesystem::path> {
			std::error_code err_code;
			std::filesystem::create_directory(subdirectory_path, err_code);

			if(err_code.value() != 0) {
				return Err(error::SystemError(
					static_cast<error::SystemError::value_type>(err_code.value())));
			}
			else {
				return Ok(subdirectory_path);
			}
		}

		/// @brief Creates a time stamp in the format
		/// [Year-Month-Day|Hour-Minute-Second] for the current local time
		///
		/// @return The time stamp
		[[nodiscard]] inline static auto create_time_stamp() -> std::string {
			return fmt::format("[{:%Y-%m-%d=%H-%M-%S}]", fmt::localtime(std::time(nullptr)));
		}
	};

	static_assert(SinkType<FileSink>, "FileSink failing SinkType requirements");

	/// @brief Basic logging sink that writes to `stdout`
	///
	/// @tparam Style - Whether the text should be styled
	/// @ingroup logging
	/// @headerfile "Hyperion/logging/Sink.h"
	template<SinkTextStyle Style = SinkTextStyle::Styled>
	class StdoutSink final {
	  public:
		/// @brief Constructs a `StdoutSink` that will log entries at or above the default
		/// `LogLevel` (`LogLevel::ERROR`)
		/// @ingroup logging
		/// @headerfile "Hyperion/logging/Sink.h"
		constexpr StdoutSink() noexcept = default;
		/// @brief Constructs a `StdoutSink` that will log entries at or above the given `LogLevel`
		///
		/// @param level - The `LogLevel` to configure this sink for
		/// @ingroup logging
		/// @headerfile "Hyperion/logging/Sink.h"
		explicit constexpr StdoutSink(LogLevel level) noexcept : m_log_level(level) {
		}
		/// @brief Copy constructor
		/// @ingroup logging
		/// @headerfile "Hyperion/logging/Sink.h"
		constexpr StdoutSink(const StdoutSink& sink) noexcept = default;
		/// @brief Move constructor
		/// @ingroup logging
		/// @headerfile "Hyperion/logging/Sink.h"
		constexpr StdoutSink(StdoutSink&& sink) noexcept = default;
		/// @brief Destructor
		/// @ingroup logging
		/// @headerfile "Hyperion/logging/Sink.h"
		constexpr ~StdoutSink() noexcept = default;

		/// @brief Sinks the given entry, writing it to `stdout`
		///
		/// Sinks the given entry if it is at or above the configured `LogLevel`, writing it to
		/// `stdout`. Otherwise, does nothing
		///
		/// @param entry - The entry to sink
		/// @ingroup logging
		/// @headerfile "Hyperion/logging/Sink.h"
		inline auto sink_entry(const Entry& entry) noexcept -> void {
			if(entry.level() >= m_log_level) {
				if constexpr(Style == SinkTextStyle::Styled) {
					println(entry.style(), "{}", entry.entry());
				}
				else {
					println("{}", entry.entry());
				}
			}
		}

		/// @brief Sinks the given entry, writing it to `stdout`
		///
		/// Sinks the given entry if it is at or above the configured `LogLevel`, writing it to
		/// `stdout`. Otherwise, does nothing
		///
		/// @param entry - The entry to sink
		/// @ingroup logging
		/// @headerfile "Hyperion/logging/Sink.h"
		inline auto sink_entry(Entry&& entry) noexcept -> void {
			if(entry.level() >= m_log_level) {
				if constexpr(Style == SinkTextStyle::Styled) {
					println(entry.style(), "{}", entry.entry());
				}
				else {
					println("{}", entry.entry());
				}
			}
		}

		/// @brief Returns the currently configured `LogLevel` for this sink
		/// @ingroup logging
		/// @headerfile "Hyperion/logging/Sink.h"
		[[nodiscard]] inline auto get_log_level() const noexcept -> LogLevel {
			return m_log_level;
		}

		/// @brief Sets the configured `LogLevel` for this sink to the given one
		///
		/// @param level - The `LogLevel` to configure this sink to
		/// @ingroup logging
		/// @headerfile "Hyperion/logging/Sink.h"
		inline auto set_log_level(LogLevel level) noexcept -> void {
			m_log_level = level;
		}

		/// @brief Copy-assignment operator
		/// @ingroup logging
		/// @headerfile "Hyperion/logging/Sink.h"
		constexpr auto operator=(const StdoutSink& sink) noexcept -> StdoutSink& = default;
		/// @brief Move-assignment operator
		/// @ingroup logging
		/// @headerfile "Hyperion/logging/Sink.h"
		constexpr auto operator=(StdoutSink&& sink) noexcept -> StdoutSink& = default;

	  private:
		LogLevel m_log_level = LogLevel::ERROR;
	};

	static_assert(SinkType<StdoutSink<>>, "StdoutSink failing SinkType requirements");

	/// @brief Basic logging sink that writes to `stderr`
	///
	/// @tparam Style - Whether the text should be styled
	/// @ingroup logging
	/// @headerfile "Hyperion/logging/Sink.h"
	template<SinkTextStyle Style = SinkTextStyle::Styled>
	class StderrSink final {
	  public:
		/// @brief Constructs a `StderrSink` that will log entries at or above the default
		/// `LogLevel` (`LogLevel::ERROR`)
		/// @ingroup logging
		/// @headerfile "Hyperion/logging/Sink.h"
		constexpr StderrSink() noexcept = default;
		/// @brief Constructs a `StderrSink` that will log entries at or above the given `LogLevel`
		///
		/// @param level - The `LogLevel` to configure this sink for
		/// @ingroup logging
		/// @headerfile "Hyperion/logging/Sink.h"
		explicit constexpr StderrSink(LogLevel level) noexcept : m_log_level(level) {
		}
		/// @brief Copy constructor
		/// @ingroup logging
		/// @headerfile "Hyperion/logging/Sink.h"
		constexpr StderrSink(const StderrSink& sink) noexcept = default;
		/// @brief Move constructor
		/// @ingroup logging
		/// @headerfile "Hyperion/logging/Sink.h"
		constexpr StderrSink(StderrSink&& sink) noexcept = default;
		/// @brief Destructor
		/// @ingroup logging
		/// @headerfile "Hyperion/logging/Sink.h"
		constexpr ~StderrSink() noexcept = default;

		/// @brief Sinks the given entry, writing it to `stderr`
		///
		/// Sinks the given entry if it is at or above the configured `LogLevel`, writing it to
		/// `stderr`. Otherwise, does nothing
		///
		/// @param entry - The entry to sink
		/// @ingroup logging
		/// @headerfile "Hyperion/logging/Sink.h"
		inline auto sink_entry(const Entry& entry) noexcept -> void {
			if(entry.level() >= m_log_level) {
				if constexpr(Style == SinkTextStyle::Styled) {
					eprintln(entry.style(), "{}", entry.entry());
				}
				else {
					eprintln("{}", entry.entry());
				}
			}
		}

		/// @brief Sinks the given entry, writing it to `stderr`
		///
		/// Sinks the given entry if it is at or above the configured `LogLevel`, writing it to
		/// `stderr`. Otherwise, does nothing
		///
		/// @param entry - The entry to sink
		/// @ingroup logging
		/// @headerfile "Hyperion/logging/Sink.h"
		inline auto sink_entry(Entry&& entry) noexcept -> void {
			if(entry.level() >= m_log_level) {
				if constexpr(Style == SinkTextStyle::Styled) {
					eprintln(entry.style(), "{}", entry.entry());
				}
				else {
					eprintln("{}", entry.entry());
				}
			}
		}

		/// @brief Returns the currently configured `LogLevel` for this sink
		/// @ingroup logging
		/// @headerfile "Hyperion/logging/Sink.h"
		[[nodiscard]] inline auto get_log_level() const noexcept -> LogLevel {
			return m_log_level;
		}

		/// @brief Sets the configured `LogLevel` for this sink to the given one
		///
		/// @param level - The `LogLevel` to configure this sink to
		/// @ingroup logging
		/// @headerfile "Hyperion/logging/Sink.h"
		inline auto set_log_level(LogLevel level) noexcept -> void {
			m_log_level = level;
		}

		/// @brief Copy-assignment operator
		/// @ingroup logging
		/// @headerfile "Hyperion/logging/Sink.h"
		constexpr auto operator=(const StderrSink& sink) noexcept -> StderrSink& = default;
		/// @brief Move-assignment operator
		/// @ingroup logging
		/// @headerfile "Hyperion/logging/Sink.h"
		constexpr auto operator=(StderrSink&& sink) noexcept -> StderrSink& = default;

	  private:
		LogLevel m_log_level = LogLevel::INFO;
	};
	IGNORE_PADDING_STOP

	static_assert(SinkType<StderrSink<>>, "StderrSink failing SinkType requirements");

	/// @def HYPERION_LOGGING_USER_SINK_TYPES
	/// @brief List of `SinkType`s provided by the user to add to the possible `SinkType` variants
	/// a `Sink` can take.
	///
	/// The user can define this prior to including "Hyperion/logging/Sink.h" and
	/// "Hyperion/logging/Logger.h" in order to use their own custom `SinkType`s in a Hyperion
	/// `Logger`. In this case, `HYPERION_LOGGING_USER_SINK_TYPES` should be defined to a comma
	/// separated list of `SinkType`s
	/// @ingroup logging
	/// @headerfile "Hyperion/logging/Sink.h"
#ifndef HYPERION_LOGGING_USER_SINK_TYPES
	/// List of Sink types to sink logging entries to
	#define HYPERION_LOGGING_USER_SINK_TYPES
#endif

	IGNORE_RESERVED_IDENTIFIERS_START
// clang-format off
// NOLINTNEXTLINE(cert-dcl37-c, cert-dcl51-cpp, bugprone-reserved-identifier, cppcoreguidelines-macro-usage)
#define __HYPERION_LOGGING_SINKS(...) FileSink, StdoutSink<>, StderrSink<> __VA_OPT__(, ) __VA_ARGS__

	// NOLINTNEXTLINE(cert-dcl37-c, cert-dcl51-cpp, bugprone-reserved-identifier, cppcoreguidelines-macro-usage)
#define __HYPERION_SINKS __HYPERION_LOGGING_SINKS(HYPERION_LOGGING_USER_SINK_TYPES)
	// clang-format on
	IGNORE_RESERVED_IDENTIFIERS_STOP

	IGNORE_PADDING_START
	/// @brief Universal Hyperion logging Sink type.
	/// This class is compile-time polymorphic and can take the form of any sink meeting
	/// the requirements of `SinkType` and either already provided by Hyperion (e.g. `FileSink`) or
	/// listed by the `HYPERION_LOGGING_USER_SINK_TYPES` macro.
	///
	/// The user may write their own custom `SinkType`s to use in addition to or, in combination
	/// with, those already provided by Hyperion. In this case, they should define
	/// `HYPERION_LOGGING_USER_SINK_TYPES` to be the comma separated list of their desired possible
	/// `SinkType`s BEFORE including this header
	/// (`Hyperion/logging/Sinks.h`) and/or the Hyperion logging header
	/// (`Hyperion/Logger.h`) and/or the global Hyperion Utils header
	/// (`Hyperion/Utils.h`), and/or any Hyperion headers provided by Hyperion libraries other than
	/// Hyperion Utils.
	/// @ingroup logging
	/// @headerfile "Hyperion/logging/Sink.h"
	class Sink : private std::variant<__HYPERION_SINKS> {
	  public:
		using rep = std::variant<__HYPERION_SINKS>;
		using types_list = mpl::list<__HYPERION_SINKS>;

		/// @brief Constructs a `Sink` as the given `SinkType`
		///
		/// @param sink - The sink for this `Sink` to be constructed as
		///
		/// # Requirements
		/// - `mpl::contains_v<std::remove_cvref_t<decltype(sink), types_list>`: The type of `sink`
		/// must be contained in the list of possible `SinkType`s a `Sink` can be. I.E. it must be
		/// either a `SinkType` already provided by Hyperion, or one specified in
		/// `HYPERION_LOGGING_USER_SINK_TYPES`
		///
		/// @ingroup logging
		/// @headerfile "Hyperion/logging/Sink.h"
		explicit Sink(SinkType auto&& sink) noexcept // NOLINT
													 // (bugprone-forwarding-reference-overload)
													 // the forwarding reference here is fine
													 // because it's constrained by the concept
			requires mpl::contains_v<std::remove_cvref_t<decltype(sink)>, types_list>
			: rep(std::forward<decltype(sink)>(sink)) {
		}

		/// @brief Constructs a `Sink` as a `SinkType` constructed from the arguments `args`
		///
		/// @tparam T - The `SinkType` to construct this as
		/// @tparam Args - The types of the arguments to construct the `SinkType` from
		/// @param tag - Tag type for template type deduction
		/// @param args - The arguments to pass to `T`'s constructor
		///
		/// # Requirements
		/// - `mpl::contains_v<T, types_list>`: The `SinkType`, `T`, must be contained in the list
		/// of possible `SinkType`s a `Sink` can be. I.E. it must be either a `SinkType` already
		/// provided by Hyperion, or one specified in `HYPERION_LOGGING_USER_SINK_TYPES`
		///
		/// @ingroup logging
		/// @headerfile "Hyperion/logging/Sink.h"
		template<SinkType T, typename... Args>
		requires mpl::contains_v<T, types_list>
		explicit Sink(std::in_place_type_t<T> tag, Args&&... args) noexcept
			: rep(tag, std::forward<Args>(args)...) {
		}
		/// @brief Deleted Copy constructor
		/// @ingroup logging
		/// @headerfile "Hyperion/logging/Sink.h"
		Sink(const Sink& sink) noexcept = delete;
		/// @brief Move constructor
		/// @ingroup logging
		/// @headerfile "Hyperion/logging/Sink.h"
		Sink(Sink&& sink) noexcept = default;
		/// @brief Destructor
		/// @ingroup logging
		/// @headerfile "Hyperion/logging/Sink.h"
		~Sink() noexcept = default;

		/// @brief Sinks the given entry, writing it to the output location
		/// corresponding with the `SinkType` this has been constructed as
		///
		/// @param entry - The entry to sink
		/// @ingroup logging
		/// @headerfile "Hyperion/logging/Sink.h"
		inline constexpr auto sink(const Entry& entry) noexcept -> void {
			std::visit([&](auto& sink) { sink.sink_entry(entry); }, *static_cast<rep*>(this));
		}

		/// @brief Sinks the given entry, writing it to the output location
		/// corresponding with the `SinkType` this has been constructed as
		///
		/// @param entry - The entry to sink
		/// @ingroup logging
		/// @headerfile "Hyperion/logging/Sink.h"
		inline constexpr auto sink(Entry&& entry) noexcept -> void {
			std::visit([&](auto& sink) { sink.sink_entry(std::move(entry)); },
					   *static_cast<rep*>(this));
		}

		/// @brief Deleted copy-assignment operator
		/// @ingroup logging
		/// @headerfile "Hyperion/logging/Sink.h"
		auto operator=(const Sink& sink) noexcept -> Sink& = delete;
		/// @brief Move-assignment operator
		/// @ingroup logging
		/// @headerfile "Hyperion/logging/Sink.h"
		auto operator=(Sink&& sink) noexcept -> Sink& = default;
	};
	IGNORE_PADDING_STOP

	/// @brief Creates a `Sink` of the given `SinkType` from the given arguments
	/// Constructs the underlying `SinkType` in place in the `Sink`
	///
	/// @tparam T - The `SinkType` to create the `Sink` as
	/// @tparam Args - The types of the arguments for constructing the `SinkType`
	/// @param args - The arguments to pass to the `SinkType`'s constructor
	///
	/// @return A `Sink`
	/// @ingroup logging
	/// @headerfile "Hyperion/logging/Sink.h"
	template<SinkType T, typename... Args>
	requires concepts::ConstructibleFrom<T, Args...> && mpl::contains_v<T, Sink::types_list>
	inline auto make_sink(Args&&... args) noexcept -> Sink {
		return Sink(std::in_place_type_t<T>(), std::forward<Args>(args)...);
	}

	/// @brief Basic dynamically-sized contiguous container to store `Sink`s in
	/// @ingroup logging
	/// @headerfile "Hyperion/logging/Sink.h"
	class Sinks {
	  public:
		using size_type = std::vector<Sink>::size_type;
		using iterator = std::vector<Sink>::iterator;
		using const_iterator = std::vector<Sink>::const_iterator;
		using reverse_iterator = std::vector<Sink>::reverse_iterator;
		using const_reverse_iterator = std::vector<Sink>::const_reverse_iterator;

		/// @brief Constructs a default (empty) `Sinks`
		/// @ingroup logging
		/// @headerfile "Hyperion/logging/Sink.h"
		constexpr Sinks() noexcept = default;
		/// @brief Constructs a `Sinks` from an array of rvalue `Sink`s.
		/// This allows for braced-initialization of a `Sinks` even though `Sink`s
		/// are not copyable.
		///
		/// @tparam N - The size of the array
		/// @param sinks - The array of sinks to initialize from
		/// @ingroup logging
		/// @headerfile "Hyperion/logging/Sink.h"
		template<size_t N>
		explicit Sinks(Sink(&&sinks)[N]) noexcept { // NOLINT
			m_sinks.reserve(N);
			for(auto&& sink : sinks) {
				m_sinks.push_back(std::move(sink));
			}
		}
		/// @brief Deleted copy-constructor
		/// @ingroup logging
		/// @headerfile "Hyperion/logging/Sink.h"
		Sinks(const Sinks& sinks) noexcept = delete;
		/// @brief Move-constructor
		/// @ingroup logging
		/// @headerfile "Hyperion/logging/Sink.h"
		Sinks(Sinks&& sinks) noexcept = default;
		/// @brief Destructor
		/// @ingroup logging
		/// @headerfile "Hyperion/logging/Sink.h"
		~Sinks() noexcept = default;

		/// @brief Adds a `Sink` to the end of the container
		///
		/// @param sink - The `Sink` to add
		/// @ingroup logging
		/// @headerfile "Hyperion/logging/Sink.h"
		inline auto push_back(Sink&& sink) noexcept -> void {
			m_sinks.push_back(std::forward<Sink>(sink));
		}

		/// @brief Constructs a `Sink` in place at the end of the container
		///
		/// @tparam Args - The types of the arguments to pass to `Sink`'s constructor
		/// @param args - The arguments to pass to `Sink`'s constructor
		///
		/// @return A reference to the constructed `Sink`
		/// @ingroup logging
		/// @headerfile "Hyperion/logging/Sink.h"
		template<typename... Args>
		requires concepts::ConstructibleFrom<Sink, Args...>
		inline auto emplace_back(Args&&... args) noexcept -> Sink& {
			return m_sinks.emplace_back(std::forward<Args>(args)...);
		}

		/// @brief Returns a reference to the `Sink` at the beginning of the container
		///
		/// @return A reference to the first `Sink`
		/// @ingroup logging
		/// @headerfile "Hyperion/logging/Sink.h"
		[[nodiscard]] inline auto front() noexcept -> Sink& {
			return m_sinks.front();
		}
		/// @brief Returns a reference to the `Sink` at the beginning of the container
		///
		/// @return A reference to the first `Sink`
		/// @ingroup logging
		/// @headerfile "Hyperion/logging/Sink.h"
		[[nodiscard]] inline auto front() const noexcept -> const Sink& {
			return m_sinks.front();
		}
		/// @brief Returns a reference to the `Sink` at the end of the container
		///
		/// @return A reference to the last `Sink`
		/// @ingroup logging
		/// @headerfile "Hyperion/logging/Sink.h"
		[[nodiscard]] inline auto back() noexcept -> Sink& {
			return m_sinks.back();
		}
		/// @brief Returns a reference to the `Sink` at the end of the container
		///
		/// @return A reference to the last `Sink`
		/// @ingroup logging
		/// @headerfile "Hyperion/logging/Sink.h"
		[[nodiscard]] inline auto back() const noexcept -> const Sink& {
			return m_sinks.back();
		}

		/// @brief Returns the number of `Sink`s in the container
		///
		/// @return The number of `Sink`s
		/// @ingroup logging
		/// @headerfile "Hyperion/logging/Sink.h"
		[[nodiscard]] inline auto size() const noexcept -> size_type {
			return m_sinks.size();
		}

		/// @brief Returns the maximum possible number of `Sink`s this can store
		///
		/// @return The maximum number of `Sink`s
		/// @ingroup logging
		/// @headerfile "Hyperion/logging/Sink.h"
		[[nodiscard]] inline auto max_size() const noexcept -> size_type {
			return m_sinks.max_size();
		}

		/// @brief Returns the current capacity of the container
		///
		/// @return The capacity of the container
		/// @ingroup logging
		/// @headerfile "Hyperion/logging/Sink.h"
		[[nodiscard]] inline auto capacity() const noexcept -> size_type {
			return m_sinks.capacity();
		}

		/// @brief Returns whether the container is currently empty
		///
		/// @return `true` if the container is empty, `false` otherwise
		/// @ingroup logging
		/// @headerfile "Hyperion/logging/Sink.h"
		[[nodiscard]] inline auto empty() const noexcept -> bool {
			return m_sinks.empty();
		}

		/// @brief Returns a reference to the `Sink` at the given `index`
		///
		/// @param index - The index of the desired `Sink`
		///
		/// @return A reference to the associated `Sink`
		/// @ingroup logging
		/// @headerfile "Hyperion/logging/Sink.h"
		inline auto at(concepts::UnsignedIntegral auto index) noexcept -> Sink& {
			return m_sinks.at(static_cast<size_type>(index));
		}
		/// @brief Returns a reference to the `Sink` at the given `index`
		///
		/// @param index - The index of the desired `Sink`
		///
		/// @return A reference to the associated `Sink`
		/// @ingroup logging
		/// @headerfile "Hyperion/logging/Sink.h"
		inline auto at(concepts::UnsignedIntegral auto index) const noexcept -> const Sink& {
			return m_sinks.at(static_cast<size_type>(index));
		}

		/// @brief Returns an iterator pointing to the beginning of the container
		///
		/// @return An iterator at the beginning of the container
		/// @ingroup logging
		/// @headerfile "Hyperion/logging/Sink.h"
		[[nodiscard]] inline auto begin() noexcept -> iterator {
			return m_sinks.begin();
		}
		/// @brief Returns an iterator pointing to the beginning of the container
		///
		/// @return An iterator at the beginning of the container
		/// @ingroup logging
		/// @headerfile "Hyperion/logging/Sink.h"
		[[nodiscard]] inline auto begin() const noexcept -> const_iterator {
			return m_sinks.begin();
		}
		/// @brief Returns an iterator pointing to the end of the container
		///
		/// @return An iterator at the end of the container
		/// @ingroup logging
		/// @headerfile "Hyperion/logging/Sink.h"
		[[nodiscard]] inline auto end() noexcept -> iterator {
			return m_sinks.end();
		}
		/// @brief Returns an iterator pointing to the end of the container
		///
		/// @return An iterator at the end of the container
		/// @ingroup logging
		/// @headerfile "Hyperion/logging/Sink.h"
		[[nodiscard]] inline auto end() const noexcept -> const_iterator {
			return m_sinks.end();
		}

		/// @brief Returns an iterator pointing to the beginning of the container
		///
		/// @return An iterator at the beginning of the container
		/// @ingroup logging
		/// @headerfile "Hyperion/logging/Sink.h"
		[[nodiscard]] inline auto cbegin() const noexcept -> const_iterator {
			return m_sinks.cbegin();
		}
		/// @brief Returns an iterator pointing to the end of the container
		///
		/// @return An iterator at the end of the container
		/// @ingroup logging
		/// @headerfile "Hyperion/logging/Sink.h"
		[[nodiscard]] inline auto cend() const noexcept -> const_iterator {
			return m_sinks.cend();
		}

		/// @brief Returns a reverse iterator pointing to the beginning of the container
		///
		/// @return A reverse iterator at the beginning of the container
		/// @ingroup logging
		/// @headerfile "Hyperion/logging/Sink.h"
		[[nodiscard]] inline auto rbegin() noexcept -> reverse_iterator {
			return m_sinks.rbegin();
		}
		/// @brief Returns a reverse iterator pointing to the beginning of the container
		///
		/// @return A reverse iterator at the beginning of the container
		/// @ingroup logging
		/// @headerfile "Hyperion/logging/Sink.h"
		[[nodiscard]] inline auto rbegin() const noexcept -> const_reverse_iterator {
			return m_sinks.rbegin();
		}
		/// @brief Returns a reverse iterator pointing to the end of the container
		///
		/// @return A reverse iterator at the end of the container
		/// @ingroup logging
		/// @headerfile "Hyperion/logging/Sink.h"
		[[nodiscard]] inline auto rend() noexcept -> reverse_iterator {
			return m_sinks.rend();
		}
		/// @brief Returns a reverse iterator pointing to the end of the container
		///
		/// @return A reverse iterator at the end of the container
		/// @ingroup logging
		/// @headerfile "Hyperion/logging/Sink.h"
		[[nodiscard]] inline auto rend() const noexcept -> const_reverse_iterator {
			return m_sinks.rend();
		}

		/// @brief Returns a reverse iterator pointing to the beginning of the container
		///
		/// @return A reverse iterator at the beginning of the container
		/// @ingroup logging
		/// @headerfile "Hyperion/logging/Sink.h"
		[[nodiscard]] inline auto crbegin() const noexcept -> const_reverse_iterator {
			return m_sinks.crbegin();
		}
		/// @brief Returns a reverse iterator pointing to the end of the container
		///
		/// @return A reverse iterator at the end of the container
		/// @ingroup logging
		/// @headerfile "Hyperion/logging/Sink.h"
		[[nodiscard]] inline auto crend() const noexcept -> const_reverse_iterator {
			return m_sinks.crend();
		}

		/// @brief Returns a reference to the `Sink` at the given `index`
		///
		/// @param index - The index of the desired `Sink`
		///
		/// @return A reference to the associated `Sink`
		/// @ingroup logging
		/// @headerfile "Hyperion/logging/Sink.h"
		inline auto operator[](concepts::UnsignedIntegral auto index) noexcept -> Sink& {
			return m_sinks[static_cast<size_type>(index)];
		}
		/// @brief Returns a reference to the `Sink` at the given `index`
		///
		/// @param index - The index of the desired `Sink`
		///
		/// @return A reference to the associated `Sink`
		/// @ingroup logging
		/// @headerfile "Hyperion/logging/Sink.h"
		inline auto
		operator[](concepts::UnsignedIntegral auto index) const noexcept -> const Sink& {
			return m_sinks[static_cast<size_type>(index)];
		}
		/// @brief Deleted copy-assignment operator
		/// @ingroup logging
		/// @headerfile "Hyperion/logging/Sink.h"
		auto operator=(const Sinks& sinks) noexcept -> Sinks& = delete;
		/// @brief Move-assignment operator
		/// @ingroup logging
		/// @headerfile "Hyperion/logging/Sink.h"
		auto operator=(Sinks&& sinks) noexcept -> Sinks& = default;

	  private:
		std::vector<Sink> m_sinks = std::vector<Sink>();
	};
} // namespace hyperion
