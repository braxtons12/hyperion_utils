/// @brief `None` represents a null, missing, or invalid value
#pragma once

namespace hyperion::option {

	/// @brief Represents a null, missing, invalid, etc value
	struct None {
		/// @brief Default Constructor
		constexpr None() noexcept = default;
		/// @brief Copy Constructor
		constexpr None(const None& none) noexcept = default;
		/// @brief Move Constructor
		constexpr None(None&& none) noexcept = default;
		/// @brief Destructor
		constexpr ~None() noexcept = default;

		/// @brief Copy Assignment Operator
		constexpr auto operator=(const None& none) noexcept -> None& = default;
		/// @brief Move Assignment Operator
		constexpr auto operator=(None&& none) noexcept -> None& = default;
	};

	/// @brief Static `None` to use to represent any null value
	static constexpr None none;
} // namespace hyperion::option
