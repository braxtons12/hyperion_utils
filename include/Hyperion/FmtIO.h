/// @file FmtIO.h
/// @author Braxton Salyer <braxtonsalyer@gmail.com>
/// @brief Formatted output via fmtlib (and in the future input?)
/// @version 0.1
/// @date 2021-10-15
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

/// @ingroup utils
/// @{
///	@defgroup io I/O
/// Formatted output via fmtlib
/// @}
#pragma once

#include <Hyperion/Fmt.h>

namespace hyperion {

	/// @brief Prints to `stdout`, formatting the given arguments into the specified format string
	///
	/// @tparam Args - The types of the arguments to format into the format string
	///
	/// @param format_string - The C++20/fmtlib compatible format string
	/// @param args - The arguments to format into the given format string
	/// @ingroup io
	template<typename... Args>
	inline constexpr auto
	print(fmt::format_string<Args...>&& format_string, Args&&... args) noexcept -> void {
		fmt::print(std::move(format_string), std::forward<Args>(args)...);
	}

	/// @brief Prints to `stdout`, formatting the given arguments into the specified format string
	///
	/// @tparam Args - The types of the arguments to format into the format string
	///
	/// @param style - The text style to pretty-print the formatted text with
	/// @param format_string - The C++20/fmtlib compatible format string
	/// @param args - The arguments to format into the given format string
	/// @ingroup io
	template<typename... Args>
	inline constexpr auto print(fmt::text_style style,
								fmt::format_string<Args...>&& format_string,
								Args&&... args) noexcept -> void {
		fmt::print(style, std::move(format_string), std::forward<Args>(args)...);
	}

	/// @brief Prints to `stdout`, formatting the given arguments into the specified format string,
	/// followed by a newline
	///
	/// @tparam Args - The types of the arguments to format into the format string
	///
	/// @param format_string - The C++20/fmtlib compatible format string
	/// @param args - The arguments to format into the given format string
	/// @ingroup io
	template<typename... Args>
	inline constexpr auto
	println(fmt::format_string<Args...>&& format_string, Args&&... args) noexcept -> void {
		fmt::print(stdout,
				   "{}\n",
				   fmt::format(std::move(format_string), std::forward<Args>(args)...));
	}

	/// @brief Prints to `stdout`, formatting the given arguments into the specified format string,
	/// followed by a newline
	///
	/// @tparam Args - The types of the arguments to format into the format string
	///
	/// @param style - The text style to pretty-print the formatted text with
	/// @param format_string - The C++20/fmtlib compatible format string
	/// @param args - The arguments to format into the given format string
	/// @ingroup io
	template<typename... Args>
	inline constexpr auto println(fmt::text_style style,
								  fmt::format_string<Args...>&& format_string,
								  Args&&... args) noexcept -> void {
		fmt::print(stdout,
				   style,
				   "{}\n",
				   fmt::format(std::move(format_string), std::forward<Args>(args)...));
	}

	/// @brief Prints to `stderr`, formatting the given arguments into the specified format string
	///
	/// @tparam Args - The types of the arguments to format into the format string
	///
	/// @param format_string - The C++20/fmtlib compatible format string
	/// @param args - The arguments to format into the given format string
	/// @ingroup io
	template<typename... Args>
	inline constexpr auto
	eprint(fmt::format_string<Args...>&& format_string, Args&&... args) noexcept -> void {
		fmt::print(stderr, std::move(format_string), std::forward<Args>(args)...);
	}

	/// @brief Prints to `stderr`, formatting the given arguments into the specified format string
	///
	/// @tparam Args - The types of the arguments to format into the format string
	///
	/// @param style - The text style to pretty-print the formatted text with
	/// @param format_string - The C++20/fmtlib compatible format string
	/// @param args - The arguments to format into the given format string
	/// @ingroup io
	template<typename... Args>
	inline constexpr auto eprint(fmt::text_style style,
								 fmt::format_string<Args...>&& format_string,
								 Args&&... args) noexcept -> void {
		fmt::print(stderr, style, std::move(format_string), std::forward<Args>(args)...);
	}

	/// @brief Prints to `stderr`, formatting the given arguments into the specified format string,
	/// followed by a newline
	///
	/// @tparam Args - The types of the arguments to format into the format string
	///
	/// @param format_string - The C++20/fmtlib compatible format string
	/// @param args - The arguments to format into the given format string
	/// @ingroup io
	template<typename... Args>
	inline constexpr auto
	eprintln(fmt::format_string<Args...>&& format_string, Args&&... args) noexcept -> void {
		fmt::print(stderr,
				   "{}\n",
				   fmt::format(std::move(format_string), std::forward<Args>(args)...));
	}

	/// @brief Prints to `stderr`, formatting the given arguments into the specified format string,
	/// followed by a newline
	///
	/// @tparam Args - The types of the arguments to format into the format string
	///
	/// @param style - The text style to pretty-print the formatted text with
	/// @param format_string - The C++20/fmtlib compatible format string
	/// @param args - The arguments to format into the given format string
	/// @ingroup io
	template<typename... Args>
	inline constexpr auto eprintln(fmt::text_style style,
								   fmt::format_string<Args...>&& format_string,
								   Args&&... args) noexcept -> void {
		fmt::print(stderr,
				   style,
				   "{}\n",
				   fmt::format(std::move(format_string), std::forward<Args>(args)...));
	}

	/// @brief Prints to the given FILE, formatting the given arguments into the specified format
	/// string
	///
	/// @tparam Args - The types of the arguments to format into the format string
	///
	/// @param file - The file to print formatted output to
	/// @param format_string - The C++20/fmtlib compatible format string
	/// @param args - The arguments to format into the given format string
	/// @ingroup io
	template<typename... Args>
	inline constexpr auto
	fprint(std::FILE* file, fmt::format_string<Args...>&& format_string, Args&&... args) noexcept
		-> void {
		fmt::print(file, std::move(format_string), std::forward<Args>(args)...);
	}

	/// @brief Prints to the given FILE, formatting the given arguments into the specified format
	/// string
	///
	/// @tparam Args - The types of the arguments to format into the format string
	///
	/// @param file - The file to print formatted output to
	/// @param style - The text style to pretty-print the formatted text with
	/// @param format_string - The C++20/fmtlib compatible format string
	/// @param args - The arguments to format into the given format string
	/// @ingroup io
	template<typename... Args>
	inline constexpr auto fprint(std::FILE* file,
								 fmt::text_style style,
								 fmt::format_string<Args...>&& format_string,
								 Args&&... args) noexcept -> void {
		fmt::print(file, style, std::move(format_string), std::forward<Args>(args)...);
	}

	/// @brief Prints to the given FILE, formatting the given arguments into the specified format
	/// string, followed by a newline
	///
	/// @tparam Args - The types of the arguments to format into the format string
	///
	/// @param file - The file to print formatted output to
	/// @param format_string - The C++20/fmtlib compatible format string
	/// @param args - The arguments to format into the given format string
	/// @ingroup io
	template<typename... Args>
	inline constexpr auto
	fprintln(std::FILE* file, fmt::format_string<Args...>&& format_string, Args&&... args) noexcept
		-> void {
		fmt::print(file,
				   "{}\n",
				   fmt::format(std::move(format_string), std::forward<Args>(args)...));
	}

	/// @brief Prints to the given FILE, formatting the given arguments into the specified format
	/// string, followed by a newline
	///
	/// @tparam Args - The types of the arguments to format into the format string
	///
	/// @param file - The file to print formatted output to
	/// @param style - The text style to pretty-print the formatted text with
	/// @param format_string - The C++20/fmtlib compatible format string
	/// @param args - The arguments to format into the given format string
	/// @ingroup io
	template<typename... Args>
	inline constexpr auto fprintln(std::FILE* file,
								   fmt::text_style style,
								   fmt::format_string<Args...>&& format_string,
								   Args&&... args) noexcept -> void {
		fmt::print(file,
				   style,
				   "{}\n",
				   fmt::format(std::move(format_string), std::forward<Args>(args)...));
	}
} // namespace hyperion
