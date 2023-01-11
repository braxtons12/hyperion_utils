/// @file File.cpp
/// @author Braxton Salyer <braxtonsalyer@gmail.com>
/// @brief Basic file I/O support
/// @version 0.1
/// @date 2023-01-11
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
#include <Hyperion/filesystem/File.h>

namespace hyperion::fs {
	[[nodiscard]] auto
	// NOLINTNEXTLINE(readability-function-cognitive-complexity)
	File::validate_open_options(OpenOptions options) noexcept -> Result<const char*> {
		HYPERION_PROFILE_FUNCTION(); // NOLINT

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

			if(options.modifier == AccessModifier::Append) {
				return Ok("a+");
			}

			if((options.modifier & truncate_binary_mask) == 0) {
				return Ok("w+b");
			}

			if((options.modifier & truncate_fail_mask) == 0) {
				return Ok("w+x");
			}

			if((options.modifier & truncate_binary_fail_mask) == 0) {
				return Ok("w+xb");
			}

			if(options.modifier == AccessModifier::Truncate) {
				return Ok("w+");
			}

			if(options.modifier == AccessModifier::Binary) {
				return Ok("r+b");
			}

			if(options.modifier == AccessModifier::None) {
				return Ok("r+");
			}
		}
		else if(options.type == AccessType::Read) {
			if(options.modifier == AccessModifier::Binary) {
				return Ok("rb");
			}

			if(options.modifier == AccessModifier::None) {
				return Ok("r");
			}
		}
		else { // options.type == AccessType::Write
			if((options.modifier & append_binary_mask) == 0) {
				return Ok("ab");
			}

			if(options.modifier == AccessModifier::Append) {
				return Ok("a");
			}

			if((options.modifier & truncate_binary_mask) == 0) {
				return Ok("wb");
			}

			if((options.modifier & truncate_fail_mask) == 0) {
				return Ok("wx");
			}

			if((options.modifier & truncate_binary_fail_mask) == 0) {
				return Ok("wbx");
			}

			if(options.modifier == AccessModifier::Truncate) {
				return Ok("w");
			}
		}

#if HYPERION_PLATFORM_WINDOWS && !HYPERION_WINDOWS_USES_POSIX_CODES
		return Err(error::SystemError(ERROR_INVALID_PARAMETER));
#else
		return Err(error::SystemError(EINVAL));
#endif
	}

	[[nodiscard]] auto
	File::open(const std::filesystem::path& path, // NOLINT(bugprone-exception-escape)
			   OpenOptions open_options,
			   usize buffer_size) noexcept -> Result<File> {
		HYPERION_PROFILE_FUNCTION(); // NOLINT

		return validate_open_options(open_options)
			.and_then(
				[&path, &open_options, &buffer_size](const char* options) noexcept -> Result<File> {
#if HYPERION_PLATFORM_WINDOWS
					std::FILE* file = nullptr;
					ignore(fopen_s(&file, path.string().c_str(), options)); // NOLINT
#else
					auto* file = std::fopen(path.c_str(), options); // NOLINT
#endif // HYPERION_PLATFORM_WINDOWS
					if(file == nullptr) {
						return Err(error::SystemError(error::SystemDomain::get_last_error()));
					}

					auto buffer = make_unique<buffer_type>(buffer_size);

					if(std::setvbuf(file, buffer.get(), _IOFBF, buffer_size) != 0) {
						ignore(std::fclose(file)); // NOLINT
						return Err(error::SystemError(error::SystemDomain::get_last_error()));
					}

					return Ok(File(file, std::move(buffer), open_options));
				});
	}

	[[nodiscard]] auto
	File::open(const std::filesystem::path& path, // NOLINT(bugprone-exception-escape)
			   OpenOptions open_options) noexcept -> Result<File> {
		HYPERION_PROFILE_FUNCTION(); // NOLINT

		return open(path, open_options, DEFAULT_FILE_BUFFER_SIZE);
	}

	[[nodiscard]] auto
	File::open(const std::filesystem::path& path) // NOLINT(bugprone-exception-escape)
		noexcept -> Result<File> {
		HYPERION_PROFILE_FUNCTION(); // NOLINT

		return open(path, {}, DEFAULT_FILE_BUFFER_SIZE);
	}

	// NOLINTNEXTLINE(bugprone-exception-escape)
	[[nodiscard]] auto File::read(usize num_chars) noexcept -> Result<std::string> {
		HYPERION_PROFILE_FUNCTION(); // NOLINT

		if(m_options.type == AccessType::Write) {
#if HYPERION_PLATFORM_WINDOWS && !HYPERION_WINDOWS_USES_POSIX_CODES
			return Err(error::SystemError(ERROR_INVALID_FUNCTION));
#else
			return Err(error::SystemError(EPERM));
#endif
		}

		auto str = std::string(num_chars, 0);
		if(std::fread(str.data(), sizeof(element_type), num_chars, m_file.get()) < num_chars
		   && std::ferror(m_file.get()) != 0)
		{
			return Err(error::SystemError(error::SystemDomain::get_last_error()));
		}

		return Ok(std::move(str));
	}

	[[nodiscard]] auto
	File::write_bytes(usize num_bytes, const byte* bytes) noexcept -> Result<i32> { // NOLINT
		if(m_options.type == AccessType::Read) {
#if HYPERION_PLATFORM_WINDOWS && !HYPERION_WINDOWS_USES_POSIX_CODES
			return Err(error::SystemError(ERROR_INVALID_FUNCTION));
#else
			return Err(error::SystemError(EPERM));
#endif
		}

		auto res = fwrite(bytes, sizeof(byte), num_bytes, m_file.get());
		if(res < num_bytes && errno != 0) {
			return Err(error::SystemError(error::SystemDomain::get_last_error()));
		}

		return Ok{static_cast<i32>(res)};
	}

	[[nodiscard]] auto
	File::read_bytes(usize num_bytes) noexcept -> Result<UniquePtr<u8[]>> { // NOLINT
		HYPERION_PROFILE_FUNCTION();										// NOLINT

		if(m_options.type == AccessType::Write) {
#if HYPERION_PLATFORM_WINDOWS && !HYPERION_WINDOWS_USES_POSIX_CODES
			return Err(error::SystemError(ERROR_INVALID_FUNCTION));
#else
			return Err(error::SystemError(EPERM));
#endif
		}

		auto arr = make_unique<u8[]>(num_bytes); // NOLINT
		if(std::fread(arr.get(), sizeof(u8), num_bytes, m_file.get()) < num_bytes
		   && std::ferror(m_file.get()) != 0)
		{
			return Err(error::SystemError(error::SystemDomain::get_last_error()));
		}

		return Ok(std::move(arr));
	}

	// NOLINTNEXTLINE(bugprone-exception-escape)
	[[nodiscard]] auto File::read_line() noexcept -> Result<std::string> {
		HYPERION_PROFILE_FUNCTION(); // NOLINT

		if(m_options.type == AccessType::Write || m_options.modifier == AccessModifier::Binary) {
#if HYPERION_PLATFORM_WINDOWS && !HYPERION_WINDOWS_USES_POSIX_CODES
			return Err(error::SystemError(ERROR_INVALID_FUNCTION));
#else
			return Err(error::SystemError(EPERM));
#endif
		}

		auto str = std::string();

		auto c = fgetc(m_file.get()); // NOLINT(readability-identifier-length)
		while(c != '\n') {
			if(c == EOF) {
				if(std::ferror(m_file.get()) != 0) {
					return Err(error::SystemError(error::SystemDomain::get_last_error()));
				}

				return Ok(std::move(str));
			}
		}

		return Ok(std::move(str));
	}

	[[nodiscard]] auto File::flush() noexcept -> Result<> {
		HYPERION_PROFILE_FUNCTION(); // NOLINT

		if(fflush(m_file.get()) != 0) {
			return Err(error::SystemError(error::SystemDomain::get_last_error()));
		}

		return Ok();
	}

	[[nodiscard]] auto File::seek(i64 offset, SeekOrigin origin) noexcept -> Result<> {
		// NOLINTNEXTLINE(google-runtime-int)
		if(std::fseek(m_file.get(), gsl::narrow_cast<long>(offset), static_cast<i32>(origin)) != 0)
		{
			return Err(error::SystemError(error::SystemDomain::get_last_error()));
		}

		return Ok();
	}
	[[nodiscard]] auto File::position() noexcept -> Result<usize> {
		const auto res = std::ftell(m_file.get());
		if(res == -1) {
			return Err(error::SystemError(error::SystemDomain::get_last_error()));
		}

		return Ok(gsl::narrow_cast<usize>(res));
	}
	[[nodiscard]] auto File::size() noexcept -> Result<usize> {
		return position().and_then([this](auto original_pos) noexcept {
			auto ret = seek(0, SeekOrigin::End).and_then([this](auto&&) -> Result<usize> {
				const auto size = std::ftell(m_file.get());
				if(size == -1) {
					return Err(error::SystemError(error::SystemDomain::get_last_error()));
				}
				return Ok(gsl::narrow_cast<usize>(size));
			});
			ignore(seek(gsl::narrow_cast<i64>(original_pos), SeekOrigin::Begin));
			return ret;
		});
	}

} // namespace hyperion::fs
