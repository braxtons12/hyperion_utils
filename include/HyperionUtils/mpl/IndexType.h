/// @brief Index type to use in meta-programming functions
#pragma once

#include "../BasicTypes.h"

namespace hyperion::mpl {

	/// @brief Basic meta-programming index wrapper type
	///
	/// @tparam N - The index
	template<usize N>
	struct index_type : std::integral_constant<usize, N> { };

	/// @brief Value of `index_type`
	///
	/// @tparam N - The Index
	template<usize N>
	inline static constexpr usize index_t_v = index_type<N>::value;
} // namespace hyperion::mpl
