/// @file Span.h
/// @author Braxton Salyer <braxtonsalyer@gmail.com>
/// @brief A thin wrapper around `gsl::span`, providing `at` as a member function instead of a free
/// function
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
#pragma once

#include <Hyperion/BasicTypes.h>
#include <Hyperion/HyperionDef.h>
#include <array>
#include <gsl/gsl>
#include <gsl/span>

namespace hyperion {

	/// @brief Thin wrapper around `gsl::span`
	///
	/// @tparam T - The type contained in the `Span`
	/// @tparam Size - The number of elements for constexpr sizes, or gsl::dynamic_extent (default)
	/// for run-time sizes
	/// @ingroup utils
	/// @headerfile "Hyperion/Span.h"
	template<typename T, usize Size = gsl::dynamic_extent>
	class Span {
	  public:
		using Iterator = typename gsl::span<T, Size>::iterator;
		using ReverseIterator = typename gsl::span<T, Size>::reverse_iterator;
		template<usize Offset, usize Count>
		using SubSpan = typename gsl::details::calculate_subspan_type<T, Size, Offset, Count>::type;

		Span() noexcept = default;

		/// @brief Constructs a `Span` from a `gsl::span`
		///
		/// @param span - The `gsl::span` to wrap
		/// @ingroup utils
		/// @headerfile "Hyperion/Span.h"
		explicit constexpr Span(gsl::span<T, Size> span) noexcept : m_span_internal(span) {
		}

		/// @brief Copy constructs a `Span` from the given one
		///
		/// @param span - The `Span` to copy
		/// @ingroup utils
		/// @headerfile "Hyperion/Span.h"
		constexpr Span(const Span<T, Size>& span) noexcept = default;

		/// @brief Move constructs the given `Span`
		///
		/// @param span - The `Span` to move
		/// @ingroup utils
		/// @headerfile "Hyperion/Span.h"
		constexpr Span(Span<T, Size>&& span) noexcept = default;
		~Span() noexcept = default;

		/// @brief Returns the element at the given `index`
		///
		/// @param index - The index of the desired element
		/// @return - The element at `index`
		/// @ingroup utils
		/// @headerfile "Hyperion/Span.h"
		[[nodiscard]] inline constexpr auto at(usize index) noexcept -> T& {
			return gsl::at(m_span_internal, static_cast<gsl::index>(index));
		}

		/// @brief Returns the first `Count` elements in the `Span`
		///
		/// @tparam Count - The number of elements to get
		/// @tparam size - The size of the `Span`, this should always be left defaulted
		/// @return - The first `Count` elements in the `Span`, as a `Span`
		/// @ingroup utils
		/// @headerfile "Hyperion/Span.h"
		template<usize Count, usize size = Size>
		requires(size != gsl::dynamic_extent)
			[[nodiscard]] inline constexpr auto first() const noexcept -> Span<T, Count> {
			return Span(m_span_internal.template first<Count>());
		}

		/// @brief Returns the first `count` elements in the `Span`
		///
		/// @tparam size - The size of the `Span`, this should always be left defaulted
		/// @param count - The number of elements to get
		/// @return - The first `count` elements in the `Span`, as a `Span`
		/// @ingroup utils
		/// @headerfile "Hyperion/Span.h"
		template<usize size = Size>
		requires(size == gsl::dynamic_extent)
			[[nodiscard]] inline constexpr auto first(usize count) const noexcept -> Span<T, Size> {
			return Span(m_span_internal.first(count));
		}

		/// @brief Returns the last `Count` elements in the `Span`
		///
		/// @tparam Count - The number of elements to get
		/// @tparam size - The size of the `Span`, this should always be left defaulted
		/// @return - The last `Count` elements in the `Span`, as a `Span`
		/// @ingroup utils
		/// @headerfile "Hyperion/Span.h"
		template<usize Count, usize size = Size>
		requires(size != gsl::dynamic_extent)
			[[nodiscard]] inline constexpr auto last() const noexcept -> Span<T, Count> {
			return Span(m_span_internal.template last<Count>());
		}

		/// @brief Returns the last `count` elements in the `Span`
		///
		/// @tparam size - The size of the `Span`, this should always be left defaulted
		/// @param count - The number of elements to get
		/// @return- The last `count` elements in the `Span` as a `Span`
		/// @ingroup utils
		/// @headerfile "Hyperion/Span.h"
		template<usize size = Size>
		requires(size == gsl::dynamic_extent)
			[[nodiscard]] inline constexpr auto last(usize count) const noexcept -> Span<T, Size> {
			return Span(m_span_internal.last(count));
		}

		/// @brief Returns a subspan starting at the given `Offset` with `Count` elements
		///
		/// @tparam Offset - The offset to start the subspan at
		/// @tparam Count - The number of elements to get in the subspan
		/// @return - The subspan starting at `Offset` of size `Count`
		/// @ingroup utils
		/// @headerfile "Hyperion/Span.h"
		template<usize Offset, usize Count = gsl::dynamic_extent>
		[[nodiscard]] inline constexpr auto subspan() const noexcept -> SubSpan<Offset, Count> {
			return Span(m_span_internal.template subspan<Offset, Count>());
		}

		/// @brief Returns a subspan starting at the given `offset` with `count` elements
		///
		/// @param offset - The offset to start the subspan at
		/// @param count - The number of elements to get in the subspan
		/// @return - The subspan starting at `offset` of size `count`
		/// @ingroup utils
		/// @headerfile "Hyperion/Span.h"
		[[nodiscard]] inline constexpr auto
		subspan(usize offset, usize count = gsl::dynamic_extent) const noexcept
			-> Span<T, gsl::dynamic_extent> {
			return Span(m_span_internal.subspan(offset, count));
		}

		/// @brief Returns the number of elements in the `Span`
		///
		/// @return - The number of elements in the `Span`
		/// @ingroup utils
		/// @headerfile "Hyperion/Span.h"
		[[nodiscard]] inline constexpr auto size() const noexcept -> usize {
			return m_span_internal.size();
		}

		/// @brief Returns the size of the `Span` in bytes
		///
		/// @return - The size of the `Span` in bytes
		/// @ingroup utils
		/// @headerfile "Hyperion/Span.h"
		[[nodiscard]] inline constexpr auto size_bytes() const noexcept -> usize {
			return m_span_internal.size_bytes();
		}

		/// @brief Returns whether the `Span` is empty
		///
		/// @return - If the `Span` is empty
		/// @ingroup utils
		/// @headerfile "Hyperion/Span.h"
		[[nodiscard]] inline constexpr auto is_empty() const noexcept -> bool {
			return size() == 0;
		}

		/// @brief A pointer to the data contained in the `Span`
		///
		/// @return - A pointer to the data
		/// @ingroup utils
		/// @headerfile "Hyperion/Span.h"
		[[nodiscard]] inline constexpr auto data() const noexcept -> T* {
			return m_span_internal.data();
		}

		/// @brief Returns the first element in the `Span`
		///
		/// @return - The first element
		/// @ingroup utils
		/// @headerfile "Hyperion/Span.h"
		[[nodiscard]] inline constexpr auto front() const noexcept -> T& {
			return m_span_internal.front();
		}

		/// @brief Returns the last element in the `Span`
		///
		/// @return - The last element
		/// @ingroup utils
		/// @headerfile "Hyperion/Span.h"
		[[nodiscard]] inline constexpr auto back() const noexcept -> T& {
			return m_span_internal.back();
		}

		/// @brief Returns an iterator at the beginning of the span
		///
		/// @return an iterator at the beginning of the span
		/// @ingroup utils
		/// @headerfile "Hyperion/Span.h"
		[[nodiscard]] inline constexpr auto begin() noexcept -> Iterator {
			return m_span_internal.begin();
		}

		/// @brief Returns an iterator at the beginning of the span
		///
		/// @return an iterator at the beginning of the span
		/// @ingroup utils
		/// @headerfile "Hyperion/Span.h"
		[[nodiscard]] inline constexpr auto begin() const noexcept -> Iterator {
			return m_span_internal.begin();
		}

		/// @brief Returns an iterator at the end of the span
		///
		/// @return an iterator at the end of the span
		/// @ingroup utils
		/// @headerfile "Hyperion/Span.h"
		[[nodiscard]] inline constexpr auto end() noexcept -> Iterator {
			return m_span_internal.end();
		}

		/// @brief Returns an iterator at the end of the span
		///
		/// @return an iterator at the end of the span
		/// @ingroup utils
		/// @headerfile "Hyperion/Span.h"
		[[nodiscard]] inline constexpr auto end() const noexcept -> Iterator {
			return m_span_internal.end();
		}

		/// @brief Returns an iterator at the beginning of the reversed iteration of the span
		///
		/// @return an iterator at the beginning of the reversed iteration of the span
		/// @ingroup utils
		/// @headerfile "Hyperion/Span.h"
		[[nodiscard]] inline constexpr auto rbegin() noexcept -> ReverseIterator {
			return m_span_internal.rbegin();
		}

		/// @brief Returns an iterator at the beginning of the reversed iteration of the span
		///
		/// @return an iterator at the beginning of the reversed iteration of the span
		/// @ingroup utils
		/// @headerfile "Hyperion/Span.h"
		[[nodiscard]] inline constexpr auto rbegin() const noexcept -> ReverseIterator {
			return m_span_internal.rbegin();
		}

		/// @brief Returns an iterator at the end of the reversed iteration of the span
		///
		/// @return an iterator at the end of the reversed iteration of the span
		/// @ingroup utils
		/// @headerfile "Hyperion/Span.h"
		[[nodiscard]] inline constexpr auto rend() noexcept -> ReverseIterator {
			return m_span_internal.rend();
		}

		/// @brief Returns an iterator at the end of the reversed iteration of the span
		///
		/// @return an iterator at the end of the reversed iteration of the span
		/// @ingroup utils
		/// @headerfile "Hyperion/Span.h"
		[[nodiscard]] inline constexpr auto rend() const noexcept -> ReverseIterator {
			return m_span_internal.rend();
		}

#if HYPERION_PLATFORM_COMPILER_MSVC
		[[nodiscard]] inline constexpr auto _Unchecked_begin() const noexcept -> T* {
			return m_span_internal._Unchecked_begin();
		}

		[[nodiscard]] inline constexpr auto _Unchecked_end() const noexcept -> T* {
			return m_span_internal._Unchecked_end();
		}
#endif // HYPERION_PLATFORM_COMPILER_MSVC

		/// @brief Returns the element at the given `index`
		///
		/// @param index - The index of the desired element
		/// @return - The element at `index`
		/// @ingroup utils
		/// @headerfile "Hyperion/Span.h"
		[[nodiscard]] inline constexpr auto operator[](usize index) noexcept -> T& {
			return this->at(index);
		}

		constexpr auto operator=(const Span<T, Size>& span) noexcept -> Span<T, Size>& = default;
		constexpr auto operator=(Span<T, Size>&& span) noexcept -> Span<T, Size>& = default;

	  private:
		gsl::span<T, Size> m_span_internal = gsl::span<T, Size>();
	};

	IGNORE_UNUSED_TEMPLATES_START

	/// @brief Creates a `Span` from the given arguments
	///
	/// @tparam T - The type stored in the array
	///
	/// @param array - The array to get a `Span` over
	/// @param size - The size of the array
	///
	/// @return a `Span` over the given array
	/// @ingroup utils
	/// @headerfile "Hyperion/Span.h"
	template<typename T>
	[[nodiscard]] inline static constexpr auto make_span(T* array, usize size) noexcept -> Span<T> {
		return Span(gsl::make_span(array, static_cast<typename gsl::span<T>::size_type>(size)));
	}

	/// @brief Creates a `Span` from the given arguments
	///
	/// @tparam T - The type stored in the array
	///
	/// @param first - Pointer to the first element in the array
	/// @param last - Pointer to the last element in the array
	///
	/// @return a `Span` over the given array
	/// @ingroup utils
	/// @headerfile "Hyperion/Span.h"
	template<typename T>
	[[nodiscard]] inline static constexpr auto make_span(T* first, T* last) noexcept -> Span<T> {
		return Span(gsl::make_span(first, last));
	}

	/// @brief Creates a `Span` from the given arguments
	///
	/// @tparam T - The type stored in the array
	/// @tparam Size - The size of the array
	///
	/// @param array - The array to get a `Span` over
	///
	/// @return a `Span` over the given array
	/// @ingroup utils
	/// @headerfile "Hyperion/Span.h"
	template<typename T, usize Size>
	[[nodiscard]] inline static constexpr auto
	make_span(T (&array)[Size]) noexcept -> Span<T, Size> { // NOLINT
		return Span(gsl::make_span(array));
	}

	/// @brief Creates a `Span` from the given arguments
	///
	/// @tparam Collection - The collection type to get a `Span` over
	///
	/// @param collection - The collection to get a `Span` over
	///
	/// @return a `Span` over the given collection
	/// @ingroup utils
	/// @headerfile "Hyperion/Span.h"
	template<typename Collection>
	[[nodiscard]] inline static constexpr auto
	make_span(Collection& collection) noexcept -> Span<typename Collection::value_type> {
		return Span(gsl::make_span(collection));
	}

	/// @brief Creates a `Span` from the given arguments
	///
	/// @tparam Collection - The collection type to get a `Span` over
	///
	/// @param collection - The collection to get a `Span` over
	///
	/// @return a `Span` over the given collection
	/// @ingroup utils
	/// @headerfile "Hyperion/Span.h"
	template<typename Collection>
	[[nodiscard]] inline static constexpr auto make_span(const Collection& collection) noexcept
		-> Span<const typename Collection::value_type> {
		return Span(gsl::make_span(collection));
	}

	/// @brief Creates a `Span` from the given arguments
	///
	/// @tparam SmartPtr - The smart pointer type wrapping the array
	///
	/// @param array - The smart pointer wrapping the array to get a `Span` over
	/// @param size - The size of the array
	///
	/// @return a `Span` over the given array
	/// @ingroup utils
	/// @headerfile "Hyperion/Span.h"
	template<typename SmartPtr>
	[[nodiscard]] inline static constexpr auto
	make_span(SmartPtr& array, usize size) noexcept -> Span<typename SmartPtr::element_type> {
		return Span(gsl::make_span(array, size));
	}

	/// @brief Creates a `Span` from the given arguments
	///
	/// @tparam T - The type stored in the array
	/// @tparam Size - The size of the array
	///
	/// @param array - The array to get a `Span` over
	///
	/// @return a `Span` over the given array
	/// @ingroup utils
	/// @headerfile "Hyperion/Span.h"
	template<typename T, usize Size>
	[[nodiscard]] inline static constexpr auto
	make_span(std::array<T, Size> array) noexcept -> Span<T, Size> {
		return Span(gsl::make_span(array));
	}

	IGNORE_UNUSED_TEMPLATES_STOP
} // namespace hyperion
