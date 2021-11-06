/// @file File.h
/// @author Braxton Salyer <braxtonsalyer@gmail.com>
/// @brief Basic file I/O support
/// @version 0.1
/// @date 2021-11-05
///
/// MIT License
/// @copyright Copyright (c) 2021 Braxton Salyer <braxtonsalyer@gmail.com>
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

#include <Hyperion/FmtIO.h>
#include <Hyperion/HyperionDef.h>
#include <Hyperion/Memory.h>
#include <Hyperion/Option.h>
#include <Hyperion/Result.h>
#include <cstdio>
#include <filesystem>

#ifdef HYPERION_PLATFORM_WINDOWS
	#ifndef ERROR_INVALID_PARAMETER
	// Define the Windows error code `ERROR_INVALID_PARAMETER` if it isn't already
		#define ERROR_INVALID_PARAMETER 87 // NOLINT(cppcoreguidelines-macro-usage)
	#endif								   // ERROR_INVALID_PARAMETER
	#ifndef ERROR_INVALID_FUNCTION
	// Define the Windows error code `ERROR_INVALID_FUNCTION` if it isn't already
		#define ERROR_INVALID_FUNCTION 0x00000001 // NOLINT(cppcoreguidelines-macro-usage)
	#endif										  // ERROR_INVALID_FUNCTION
#endif

namespace hyperion::fs {

	/// @brief Custom Deleter type for smart pointers to provide scoped closing of owned `std::FILE`
	/// handles
	/// @ingroup filesystem
	/// @headerfile "Hyperion/filesystem/File.h"
	struct FileDeleter {
		constexpr FileDeleter() noexcept = default;

		constexpr FileDeleter(const FileDeleter&) noexcept = default;

		constexpr FileDeleter(FileDeleter&&) noexcept = default;

		constexpr ~FileDeleter() noexcept = default;

		inline auto operator()(gsl::owner<std::FILE*> ptr) const noexcept {

			if(ptr != stdout && ptr != stderr && ptr != nullptr) {
				ignore(std::fclose(ptr));
			}
		}

		constexpr auto operator=(const FileDeleter&) noexcept -> FileDeleter& = default;

		constexpr auto operator=(FileDeleter&&) noexcept -> FileDeleter& = default;
	};

	/// @brief Basic scoped file type providing formatted output and byte and character input
	///
	/// Example:
	/// @code {.cpp}
	/// auto file = File::open(std::filesystem::path("/home/username/example.txt"),
	/// 					   {.type = File::AccessType::Write,
	/// 						.modifier = File::AccessModifier::Truncate})
	/// 			.expect("Failed to open example file!");
	/// file.println("Hello world!");
	/// @endcode
	/// @ingroup filesystem
	/// @headerfile "Hyperion/filesystem/File.h"
	class File {
	  public:
		/// @brief The default size for a `File`'s associated user-space buffer
		/// @ingroup filesystem
		static constexpr usize DEFAULT_FILE_BUFFER_SIZE
			// try to squeeze a little more perf out on Windows with a large cache
			= HYPERION_PLATFORM_WINDOWS ? 4_usize * BUFSIZ : BUFSIZ;

		using element_type = char;
		using buffer_type = element_type[]; // NOLINT (c-arrays)

		/// @brief The type of access permission to open a file with
		/// @ingroup filesystem
		enum class AccessType : u32 {
			Read = 0_u32,
			Write,
			ReadWrite
		};

		/// @brief The access permission modifiers to open a file with.
		/// @ingroup filesystem
		enum AccessModifier : u32 {
			None = 0_u32,
			Append = 1_u32,
			Truncate = 2_u32,
			Binary = 4_u32,
			FailIfExists = 8_u32
		};

		/// @brief The access permission options to open a file with
		///
		/// The type can be any of `File::AccessType`.
		/// The modifier can be any bitwise combination of `File::AccessModifier`.
		///
		/// Only combinations equivalent with valid access mode strings in the C standard are valid.
		/// For example, a `.modifier` of `AccessModifier::Append | AccessModifier::FailIfExists` is
		/// invalid and a combination of `.type == AccessType::Read`
		/// and `.modifier == AccessModifier::Truncate` is invalid
		/// @ingroup filesystem
		struct OpenOptions {
			AccessType type = AccessType::ReadWrite;
			u32 modifier = AccessModifier::Truncate;
		};

		/// @brief Validates the given `OpenOptions` and returns the corresponding C standard
		/// access mode string if they are valid
		///
		/// @param options - The `OpenOptions` to validate
		///
		/// @return The C standard access mode string if the `options` are valid, otherwise an error
		/// @ingroup filesystem
		static auto validate_open_options(OpenOptions options) noexcept -> Result<const char*>;

		/// @brief a `File` can't be default-constructed
		/// @ingroup filesystem
		File() = delete;

		/// @brief Constructs a `File` from the given `std::FILE*` handle with the given
		/// `OpenOptions`
		///
		/// @param file - The handle to the open file
		/// @param options - The `OpenOptions` the file was opened with
		/// @ingroup filesystem
		constexpr File(std::FILE* file, // NOLINT
					   OpenOptions options = {.type = AccessType::ReadWrite,
											  .modifier = AccessModifier::Truncate}) noexcept
			: m_file(file), m_options(options) {
		}

		/// @brief Constructs a `File` from the given `std::FILE*` handle with the given
		/// `OpenOptions`
		///
		/// @param file - The handle to the open file
		/// @param buffer - The userspace buffer associated with the opened file for buffering
		/// @param options - The `OpenOptions` the file was opened with
		/// @ingroup filesystem
		constexpr File(std::FILE* file,
					   UniquePtr<buffer_type>&& buffer,
					   OpenOptions options = {.type = AccessType::ReadWrite,
											  .modifier = AccessModifier::Truncate}) noexcept
			: m_file(file), m_buffer(std::move(buffer)), m_options(options) { // NOLINT
		}

		/// @brief a `File` can't be copy-constructed
		/// @ingroup filesystem
		File(const File&) = delete;

		/// @brief Move-constructs a `File` from the given one
		/// @ingroup filesystem
		constexpr File(File&& file) noexcept
			: m_file(std::move(file.m_file)),
			  m_buffer(std::move(file.m_buffer)),
			  m_options(file.m_options) {
		}

		constexpr ~File() noexcept = default;

		/// @brief Attempts to open the file located at the given `path`
		///
		/// Attempts to open the file located at `path` with the given `open_options` and associated
		/// `buffer_size`. If opening the file and associating a buffer with it is successful,
		/// returns the resulting `Ok(File)`. Otherwise, returns an `Err(error::SystemError)`.
		///
		/// @param path - The path to the file to open
		/// @param open_options - The file access options to open the file with
		/// @param buffer_size - The size of the userspace buffer to associate with the file
		///
		/// @return `Ok(File)` on success, `Err(error::SystemError)` on failure
		/// @ingroup filesystem
		[[nodiscard]] static auto
		open(const std::filesystem::path& path, // NOLINT(bugprone-exception-escape)
			 OpenOptions open_options,
			 usize buffer_size) noexcept -> Result<File>;

		/// @brief Attempts to open the file located at the given `path`
		///
		/// Attempts to open the file located at `path` with the given `open_options` and default
		/// buffer size. If opening the file and associating a buffer with it is successful,
		/// returns the resulting `Ok(File)`. Otherwise, returns an `Err(error::SystemError)`.
		///
		/// @param path - The path to the file to open
		/// @param open_options - The file access options to open the file with
		///
		/// @return `Ok(File)` on success, `Err(error::SystemError)` on failure
		/// @ingroup filesystem
		[[nodiscard]] static auto
		open(const std::filesystem::path& path, // NOLINT(bugprone-exception-escape)
			 OpenOptions open_options) noexcept -> Result<File>;

		/// @brief Attempts to open the file located at the given `path`
		///
		/// Attempts to open the file located at `path` with the default access options and default
		/// buffer size. If opening the file and associating a buffer with it is successful,
		/// returns the resulting `Ok(File)`. Otherwise, returns an `Err(error::SystemError)`.
		///
		/// @param path - The path to the file to open
		///
		/// @return `Ok(File)` on success, `Err(error::SystemError)` on failure
		/// @ingroup filesystem
		[[nodiscard]] static auto
		open(const std::filesystem::path& path) // NOLINT(bugprone-exception-escape)
			noexcept -> Result<File>;

		/// @brief Prints the formatted string to the file
		///
		/// Formats the given arguments into the given format string, and prints the result to the
		/// file. If printing is successful, returns an implementation defined `i32` (corresponds
		/// exactly with the system's `std::fputs` implementation). If printing is unsuccessful OR
		/// the file wasn't opened with write access, returns an error.
		///
		/// @tparam Args - The types of the arguments to format into the format string
		///
		/// @param format_string - The format string to print to the file
		/// @param args - The arguments to format into the format string
		///
		/// @return `Ok(i32)` on success, `Err(error::SystemError)` on failure
		/// @ingroup filesystem
		template<typename... Args>
		[[nodiscard]] inline auto
		print(fmt::format_string<Args...>&& format_string, Args&&... args) noexcept -> Result<i32> {

			if(m_options.type == AccessType::Read) {
#if HYPERION_PLATFORM_WINDOWS && !HYPERION_WINDOWS_USES_POSIX_CODES
				return Err(error::SystemError(ERROR_INVALID_FUNCTION));
#else
				return Err(error::SystemError(EPERM));
#endif
			}

			auto str = fmt::format(std::move(format_string), std::forward<Args>(args)...);
			auto res = fputs(str.c_str(), m_file.get());
			if(res == EOF) {
				return Err(error::SystemError(error::SystemDomain::get_last_error()));
			}
			else {
				return Ok(res);
			}
		}

		/// @brief Prints the formatted string to the file, followed by a newline
		///
		/// Formats the given arguments into the given format string, and prints the result,
		/// followed by a newline, to the file. If printing is successful, returns an implementation
		/// defined `i32` (corresponds exactly with the system's `std::fputs` implementation).
		/// If printing is unsuccessful OR the file wasn't opened with write access, returns an
		/// error.
		///
		/// @tparam Args - The types of the arguments to format into the format string
		///
		/// @param format_string - The format string to print to the file
		/// @param args - The arguments to format into the format string
		///
		/// @return `Ok(i32)` on success, `Err(error::SystemError)` on failure
		/// @ingroup filesystem
		template<typename... Args>
		[[nodiscard]] inline auto
		println(fmt::format_string<Args...>&& format_string, Args&&... args) noexcept
			-> Result<i32> {

			return print("{}\n",
						 fmt::format(std::move(format_string), std::forward<Args>(args)...));
		}

		/// @brief Reads up to `num_chars` characters from the file
		///
		/// Reads up to `num_chars` characters from the file and returns the characters as a
		/// `std::string`. May read less than `num_chars` if `EOF` is encountered before `num_chars`
		/// have been read. If the file wasn't opened with read access OR an error occurs while
		/// reading the characters, an error is returned.
		///
		/// @param num_chars - The number of characters to read
		///
		/// @return `Ok(std::string)` if successful, `Err(error::SystemError)` on failure
		/// @ingroup filesystem
		[[nodiscard]] inline auto read(usize num_chars) noexcept -> Result<std::string>;

		/// @brief Reads up to `num_bytes` bytes from the file
		///
		/// Reads up to `num_bytes` bytes from the file and returns the bytes as a
		/// `UniquePtr<byte[]>`. May read less than `num_bytes` if `EOF` is encountered before
		/// `num_bytes` have been read. If the file wasn't opened with read access OR an error
		/// occurs while reading the characters, an error is returned.
		///
		/// @param num_bytes - The number of bytes to read
		///
		/// @return `Ok(std::string)` if successful, `Err(error::SystemError)` on failure
		/// @ingroup filesystem
		[[nodiscard]] inline auto
		read_bytes(usize num_bytes) noexcept -> Result<UniquePtr<byte[]>>; // NOLINT
		/// @brief Reads up to one text line from the file
		///
		/// Reads up to one text line from the file and returns the line as a
		/// `std::string`. May read less than a line if `EOF` is encountered before an end line
		/// character has been read. If the file wasn't opened with read access OR an error occurs
		/// while reading the characters, an error is returned.
		///
		/// @return `Ok(std::string)` if successful, `Err(error::SystemError)` on failure
		/// @ingroup filesystem
		[[nodiscard]] inline auto read_line() noexcept -> Result<std::string>;

		/// @brief Flushes the file's buffer, forcing buffered output to be written
		///
		/// Flushes the file's buffer, forcing buffered output to be written to the file.
		/// Returns an `Err(error::SystemError)` if an error occurs when flushing OR if the file
		/// wasn't opened with write access (Hyperion does not support using implementation-defined
		/// flush behavior for input streams). On success, returns an empty `Result<>`
		///
		/// @return `Ok()` on success, `Err(error::SystemError)` on failure
		/// @ingroup filesystem
		[[nodiscard]] inline auto flush() noexcept -> Result<>;

		/// @brief `File`s can't be copy-assigned
		/// @ingroup filesystem
		auto operator=(const File&) -> File& = delete;

		/// @brief Move-assigns this file from the given one
		/// @ingroup filesystem
		constexpr auto operator=(File&& file) noexcept -> File& {

			if(this == &file) {
				return *this;
			}
			m_file = std::move(file.m_file);
			m_buffer = std::move(file.m_buffer);
			m_options = file.m_options;

			return *this;
		}

	  private:
		UniquePtr<std::FILE, FileDeleter> m_file = nullptr;

		UniquePtr<buffer_type> m_buffer = nullptr;

		OpenOptions m_options = OpenOptions{};
	};

	inline constexpr auto
	operator|(const File::AccessModifier& lhs, const File::AccessModifier& rhs) noexcept -> u32 {

		return static_cast<const u32>(lhs) | static_cast<const u32>(rhs);
	}

	inline constexpr auto operator|(const File::AccessModifier& lhs, u32 rhs) noexcept -> u32 {

		return static_cast<const u32>(lhs) | rhs;
	}

	inline constexpr auto operator|(u32 lhs, const File::AccessModifier& rhs) noexcept -> u32 {

		return lhs | static_cast<const u32>(rhs);
	}

	inline constexpr auto
	operator&(const File::AccessModifier& lhs, const File::AccessModifier& rhs) noexcept -> u32 {

		return static_cast<const u32>(lhs) & static_cast<const u32>(rhs);
	}

	inline constexpr auto operator&(const File::AccessModifier& lhs, u32 rhs) noexcept -> u32 {

		return static_cast<const u32>(lhs) & rhs;
	}

	inline constexpr auto operator&(u32 lhs, const File::AccessModifier& rhs) noexcept -> u32 {

		return lhs & static_cast<const u32>(rhs);
	}

	inline constexpr auto operator~(const File::AccessModifier& mod) noexcept -> u32 {

		return ~static_cast<const u32>(mod);
	}

	inline constexpr auto operator==(const File::AccessModifier& lhs, u32 rhs) noexcept -> bool {

		return static_cast<const u32>(lhs) == rhs;
	}

	inline constexpr auto operator==(u32 lhs, const File::AccessModifier& rhs) noexcept -> bool {

		return lhs == static_cast<const u32>(rhs);
	}

	// NOLINTNEXTLINE(readability-function-cognitive-complexity)
	inline auto File::validate_open_options(OpenOptions options) noexcept -> Result<const char*> {

		static constexpr u32 append_binary_mask
			= ~(AccessModifier::Append | AccessModifier::Binary);
		static constexpr u32 truncate_binary_mask
			= ~(AccessModifier::Truncate | AccessModifier::Binary);
		static constexpr u32 truncate_fail_mask
			= ~(AccessModifier::Truncate | AccessModifier::FailIfExists);
		static constexpr u32 truncate_binary_fail_mask
			= ~(AccessModifier::Truncate | AccessModifier::Binary | AccessModifier::FailIfExists);

		if(options.type == AccessType::ReadWrite) {
			if((options.modifier & append_binary_mask) == 0) {
				return Ok("a+b");
			}
			else {
				if(options.modifier == AccessModifier::Append) {
					return Ok("a+");
				}
				else {
					if((options.modifier & truncate_binary_mask) == 0) {
						return Ok("w+b");
					}
					else {
						if((options.modifier & truncate_fail_mask) == 0) {
							return Ok("w+x");
						}
						else {
							if((options.modifier & truncate_binary_fail_mask) == 0) {
								return Ok("w+xb");
							}
							else {
								if(options.modifier == AccessModifier::Truncate) {
									return Ok("w+");
								}
								else {
									if(options.modifier == AccessModifier::Binary) {
										return Ok("r+b");
									}
									else {
										if(options.modifier == AccessModifier::None) {
											return Ok("r+");
										}
									}
								}
							}
						}
					}
				}
			}
		}
		else {
			if(options.type == AccessType::Read) {
				if(options.modifier == AccessModifier::Binary) {
					return Ok("rb");
				}
				else {
					if(options.modifier == AccessModifier::None) {
						return Ok("r");
					}
				}
			}
			else {
				if(options.type == AccessType::Write) {
					if((options.modifier & append_binary_mask) == 0) {
						return Ok("ab");
					}
					else {
						if(options.modifier == AccessModifier::Append) {
							return Ok("a");
						}
						else {
							if((options.modifier & truncate_binary_mask) == 0) {
								return Ok("wb");
							}
							else {
								if((options.modifier & truncate_fail_mask) == 0) {
									return Ok("wx");
								}
								else {
									if((options.modifier & truncate_binary_fail_mask) == 0) {
										return Ok("wbx");
									}
									else {
										if(options.modifier == AccessModifier::Truncate) {
											return Ok("w");
										}
									}
								}
							}
						}
					}
				}
			}
		}

#if HYPERION_PLATFORM_WINDOWS && !HYPERION_WINDOWS_USES_POSIX_CODES
		return Err(error::SystemError(ERROR_INVALID_PARAMETER));
#else
		return Err(error::SystemError(EINVAL));
#endif
	}

	[[nodiscard]] inline auto
	File::open(const std::filesystem::path& path, // NOLINT(bugprone-exception-escape)
			   OpenOptions open_options,
			   usize buffer_size) noexcept -> Result<File> {

		return validate_open_options(open_options)
			.and_then([&](const char* options) noexcept -> Result<File> {
#if HYPERION_PLATFORM_WINDOWS
				std::FILE* file = nullptr;
				ignore(fopen_s(&file, path.string().c_str(), options)); // NOLINT
#else
				auto* file = std::fopen(path.c_str(), options); // NOLINT
#endif // HYPERION_PLATFORM_WINDOWS
				if(file != nullptr) {
					auto buffer = make_unique<buffer_type>(buffer_size);
					if(std::setvbuf(file, buffer.get(), _IOFBF, buffer_size) != 0) {
						return Err(error::SystemError(error::SystemDomain::get_last_error()));
					}
					return Ok(File(file, std::move(buffer), open_options));
				}
				else {
					return Err(error::SystemError(error::SystemDomain::get_last_error()));
				}
			});
	}

	[[nodiscard]] inline auto
	File::open(const std::filesystem::path& path, // NOLINT(bugprone-exception-escape)
			   OpenOptions open_options) noexcept -> Result<File> {

		return open(path, open_options, DEFAULT_FILE_BUFFER_SIZE);
	}

	[[nodiscard]] inline auto
	File::open(const std::filesystem::path& path) // NOLINT(bugprone-exception-escape)
		noexcept -> Result<File> {

		return open(path, {}, DEFAULT_FILE_BUFFER_SIZE);
	}

	[[nodiscard]] inline auto File::read(usize num_chars) noexcept -> Result<std::string> {

		if(m_options.type == AccessType::Write) {
#if HYPERION_PLATFORM_WINDOWS && !HYPERION_WINDOWS_USES_POSIX_CODES
			return Err(error::SystemError(ERROR_INVALID_FUNCTION));
#else
			return Err(error::SystemError(EPERM));
#endif
		}

		auto str = std::string(num_chars, 0);
		auto res = std::fread(str.data(), sizeof(element_type), num_chars, m_file.get());
		if(res < num_chars && std::ferror(m_file.get()) != 0) {
			return Err(error::SystemError(error::SystemDomain::get_last_error()));
		}

		return Ok(std::move(str));
	}

	[[nodiscard]] inline auto
	File::read_bytes(usize num_bytes) noexcept -> Result<UniquePtr<u8[]>> { // NOLINT
		if(m_options.type == AccessType::Write) {
#if HYPERION_PLATFORM_WINDOWS && !HYPERION_WINDOWS_USES_POSIX_CODES
			return Err(error::SystemError(ERROR_INVALID_FUNCTION));
#else
			return Err(error::SystemError(EPERM));
#endif
		}

		auto arr = make_unique<u8[]>(num_bytes); // NOLINT
		auto res = std::fread(arr.get(), sizeof(u8), num_bytes, m_file.get());
		if(res < num_bytes && std::ferror(m_file.get()) != 0) {
			return Err(error::SystemError(error::SystemDomain::get_last_error()));
		}

		return Ok(std::move(arr));
	}

	[[nodiscard]] inline auto File::read_line() noexcept -> Result<std::string> {

		if(m_options.type == AccessType::Write || m_options.modifier == AccessModifier::Binary) {
#if HYPERION_PLATFORM_WINDOWS && !HYPERION_WINDOWS_USES_POSIX_CODES
			return Err(error::SystemError(ERROR_INVALID_FUNCTION));
#else
			return Err(error::SystemError(EPERM));
#endif
		}

		auto str = std::string();

		auto c = fgetc(m_file.get());
		while(c != '\n') {
			if(c == EOF) {
				if(std::ferror(m_file.get()) != 0) {
					return Err(error::SystemError(error::SystemDomain::get_last_error()));
				}
				else {
					return Ok(std::move(str));
				}
			}
		}
		return Ok(std::move(str));
	}

	[[nodiscard]] inline auto File::flush() noexcept -> Result<> {

		if(m_options.type == AccessType::Read) {
#if HYPERION_PLATFORM_WINDOWS && !HYPERION_WINDOWS_USES_POSIX_CODES
			return Err(error::SystemError(ERROR_INVALID_FUNCTION));
#else
			return Err(error::SystemError(EPERM));
#endif
		}

		if(fflush(m_file.get()) != 0) {
			return Err(error::SystemError(error::SystemDomain::get_last_error()));
		}

		return Ok();
	}
} // namespace hyperion::fs
