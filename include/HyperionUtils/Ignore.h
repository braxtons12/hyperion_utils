/// @brief Ignore is used to ignore an arbitrary number of unused parameters
#pragma once

#include <tuple>

namespace hyperion {

	/// @brief Utility function to ignore the given parameters
	///
	/// @tparam Args - The Types of the things to ignore
	/// @param args - The things to ignore
	template<typename... Args>
	inline constexpr auto ignore(Args&&... args) noexcept -> void {
		std::ignore = std::make_tuple<Args...>(std::forward<Args>(args)...);
	}
} // namespace hyperion
