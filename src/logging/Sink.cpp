/// @file Sink.cpp
/// @author Braxton Salyer <braxtonsalyer@gmail.com>
/// @brief Provides basic logging sink implementations
/// @version 0.1
/// @date 2022-07-09
///
/// MIT License
/// @copyright Copyright (c) 2022 Braxton Salyer <braxtonsalyer@gmail.com>
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

#include <Hyperion/logging/Sink.h>

namespace hyperion::logging {
	/// @brief Returns the system temporary files directory
	///
	/// # Errors
	/// Returns an error if accessing the temporary files directory fails
	///
	/// @return The `std::filesystem::path` to the temporary files directory on success,
	/// `FileCreationError` on error
	[[nodiscard]] static inline auto get_temp_directory() -> Result<std::filesystem::path> {
		std::error_code err_code;
		auto temp_dir = std::filesystem::temp_directory_path(err_code);

		if(err_code.value() != 0) {
			return Err(
				error::SystemError(static_cast<error::SystemError::value_type>(err_code.value())));
		}

		return Ok(temp_dir);
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
	[[nodiscard]] static inline auto
	create_directory(const std::filesystem::path& subdirectory_path)
		-> Result<std::filesystem::path> {
		std::error_code err_code;
		std::filesystem::create_directory(subdirectory_path, err_code);

		if(err_code.value() != 0) {
			return Err(
				error::SystemError(static_cast<error::SystemError::value_type>(err_code.value())));
		}

		return Ok(subdirectory_path);
	}

	/// @brief Creates a time stamp in the format
	/// [Year-Month-Day|Hour-Minute-Second] for the current local time
	///
	/// @return The time stamp
	[[nodiscard]] static inline auto create_time_stamp() -> std::string {
		HYPERION_PROFILE_FUNCTION();
		const auto now
			= fmt::gmtime(std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()));
		const auto years = 1900 + now.tm_year;
		const auto months = 1 + now.tm_mon;
		return fmt::format(FMT_COMPILE("[{:#04}-{:#02}-{:#02}={:#02}-{:#02}-{:#02}]"),
						   years,
						   months,
						   now.tm_mday,
						   now.tm_hour,
						   now.tm_min,
						   now.tm_sec);
	}

	// NOLINTNEXTLINE(bugprone-exception-escape)
	auto FileSink::sink(const Entry& entry) noexcept -> void {
		if(entry.level() >= m_log_level) {
			auto res = m_file.println("{}", entry.entry());
			ignore(res.is_ok());
		}
	}

	// NOLINTNEXTLINE(bugprone-exception-escape)
	auto FileSink::sink(Entry&& entry) noexcept -> void {
		if(entry.level() >= m_log_level) {
			auto res = m_file.println("{}", entry.entry());
			ignore(res.is_ok());
		}
	}

	[[nodiscard]] auto
	// NOLINTNEXTLINE(bugprone-easily-swappable-parameters, bugprone-exception-escape)
	FileSink::create_file(const std::string& root_file_name,
						  const std::string& subdirectory_name) noexcept -> Result<fs::File> {
		return get_temp_directory()
			.and_then([&subdirectory_name](
						  std::filesystem::path temp_directory) -> Result<std::filesystem::path> {
				temp_directory.append(subdirectory_name);
				return hyperion::logging::create_directory(temp_directory);
			})
			.and_then([&root_file_name](std::filesystem::path file_directory) -> Result<fs::File> {
				using namespace std::literals::string_literals;

				const auto time_string = create_time_stamp();
				file_directory.append(time_string + " "s + root_file_name);
				file_directory.replace_extension("log"s);
				return fs::File::open(file_directory);
			});
	}

	template<>
	// NOLINTNEXTLINE(bugprone-exception-escape)
	auto StdoutSink<SinkTextStyle::Styled>::sink(const Entry& entry) noexcept -> void {
		if(entry.level() >= m_log_level) {
			println(entry.style(), "{}", entry.entry());
		}
	}

	template<>
	// NOLINTNEXTLINE(bugprone-exception-escape)
	auto StdoutSink<SinkTextStyle::NotStyled>::sink(const Entry& entry) noexcept -> void {
		if(entry.level() >= m_log_level) {
			println("{}", entry.entry());
		}
	}

	template<>
	// NOLINTNEXTLINE(bugprone-exception-escape)
	auto StdoutSink<SinkTextStyle::Styled>::sink(Entry&& entry) noexcept -> void {
		if(entry.level() >= m_log_level) {
			println(entry.style(), "{}", entry.entry());
		}
	}

	template<>
	// NOLINTNEXTLINE(bugprone-exception-escape)
	auto StdoutSink<SinkTextStyle::NotStyled>::sink(Entry&& entry) noexcept -> void {
		if(entry.level() >= m_log_level) {
			println("{}", entry.entry());
		}
	}

} // namespace hyperion::logging
