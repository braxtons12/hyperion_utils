#pragma once

#include "Format.h"

namespace hyperion {

	template<typename... Args>
	constexpr inline auto
	print(fmt::format_string<Args...>&& string, Args... args) noexcept -> void {
		fmt::print("{}", std::move(string), std::forward<Args>(args)...);
	}

	template<typename... Args>
	constexpr inline auto
	fprint(std::FILE* file, fmt::format_string<Args...>&& string, Args... args) noexcept -> void {
		fmt::print(file, "{}", std::move(string), std::forward<Args>(args)...);
	}

	template<typename... Args>
	constexpr inline auto
	eprint(fmt::format_string<Args...>&& string, Args... args) noexcept -> void {
		fmt::print(stderr, "{}", std::move(string), std::forward<Args>(args)...);
	}

	template<typename... Args>
	constexpr inline auto
	println(fmt::format_string<Args...>&& string, Args... args) noexcept -> void {
		fmt::print("{}\n", std::move(string), std::forward<Args>(args)...);
	}

	template<typename... Args>
	constexpr inline auto
	eprintln(fmt::format_string<Args...>&& string, Args... args) noexcept -> void {
		eprint("{}\n", std::move(string), std::forward<Args>(args)...);
	}

	template<typename... Args>
	constexpr inline auto
	fprintln(std::FILE* file, fmt::format_string<Args...>&& string, Args... args) noexcept -> void {
		fmt::print(file, "{}\n", std::move(string), std::forward<Args>(args)...);
	}
} // namespace hyperion
