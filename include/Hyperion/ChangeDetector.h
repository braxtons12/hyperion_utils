/// @file ChangeDetector.h
/// @author Braxton Salyer <braxtonsalyer@gmail.com>
/// @brief basic value-change detector type. Useful when you need to track when
/// writes to a stored value actually change it semantically
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

#include <Hyperion/Concepts.h>
#include <utility>

namespace hyperion {

	/// @brief Stores a value and detects if an updated value is different than the previous one
	///
	/// @tparam T - The type to store and check for equality.
	///
	/// # Requirements
	/// - `concepts::DefaultConstructible<T>` - `T` must be default constructible to use
	/// `ChangeDetector<T>`
	/// - `concepts::DerefInequalityComparable<T>` - `T` must be inequality comparable if `T` is
	/// __NOT__ a pointer. If `T` is a pointer, then `std::remove_pointer_t<T>` must be inequality
	/// comparable
	/// @ingroup utils
	/// @headerfile "Hyperion/ChangeDetector.h"
	template<typename T>
	requires concepts::DefaultConstructible<T> && concepts::DerefInequalityComparable<T>
	class ChangeDetector {
	  public:
		/// @brief Create a default `ChangeDetector`
		/// @ingroup utils
		/// @headerfile "Hyperion/ChangeDetector.h"
		constexpr ChangeDetector() noexcept(concepts::NoexceptDefaultConstructible<T>) = default;

		/// @brief Create a `ChangeDetector` with the given initial value
		///
		/// @param initial_value - The initial value to store in the detector
		/// @ingroup utils
		/// @headerfile "Hyperion/ChangeDetector.h"
		explicit constexpr ChangeDetector(const T& initial_value) noexcept(
			concepts::NoexceptCopyConstructible<T>)
			: m_previous_value(initial_value) {
		}

		/// @brief Create a `ChangeDetector` with the given initial value
		///
		/// @param initial_value - The initial value to store in the detector
		/// @ingroup utils
		/// @headerfile "Hyperion/ChangeDetector.h"
		explicit constexpr ChangeDetector(T&& initial_value) noexcept(
			concepts::NoexceptMoveConstructible<T>)
			: m_previous_value(std::move(initial_value)) {
		}

		constexpr ChangeDetector(const ChangeDetector& detector) noexcept(
			concepts::NoexceptCopyConstructible<T>) requires concepts::CopyConstructible<T>
		= default;
		constexpr ChangeDetector(ChangeDetector&& detector) noexcept(
			concepts::NoexceptMoveConstructible<T>) requires concepts::MoveConstructible<T>
		= default;
		constexpr ~ChangeDetector() noexcept(concepts::NoexceptDestructible<T>) = default;

		/// @brief Updates the stored value and returns if the new value
		/// is different than the previous one.
		///
		/// @note In the case that T is a pointer, the values located AT the pointers will be
		/// compared, NOT the pointers themselves. If `new_value` is not `nullptr`, the stored
		/// pointer will be replaced with `new_value`.
		///
		/// @param new_value - The new value to store and check for equality
		///
		/// @return Whether the new value was different than the old one
		/// @ingroup utils
		/// @headerfile "Hyperion/ChangeDetector.h"
		inline constexpr auto
		changed(const T& new_value) noexcept(concepts::NoexceptCopyAssignable<T>)
			-> bool requires concepts::CopyAssignable<T> {
			bool returnVal = false;
			if constexpr(std::is_pointer_v<T>) {
				if(new_value != nullptr) {
					if(m_previous_value == nullptr) {
						returnVal = true;
					}
					else {
						returnVal = *m_previous_value != *new_value;
					}
				}
				else if(m_previous_value != nullptr) {
					returnVal = true;
				}
				m_previous_value = new_value;
			}
			else {
				returnVal = m_previous_value != new_value;
				m_previous_value = new_value;
			}
			return returnVal;
		}

		/// @brief Updates the stored value and returns if the new value
		/// is different than the previous one.
		///
		/// @note In the case that T is a pointer, the values located AT the pointers will be
		/// compared, NOT the pointers themselves. If `new_value` is not `nullptr`, the stored
		/// pointer will be replaced with `new_value`.
		///
		/// @param new_value - The new value to store and check for equality
		///
		/// @return Whether the new value was different than the old one
		/// @ingroup utils
		/// @headerfile "Hyperion/ChangeDetector.h"
		inline constexpr auto changed(T&& new_value) noexcept(concepts::NoexceptMoveAssignable<T>)
			-> bool requires concepts::MoveAssignable<T> {
			bool returnVal = false;
			if constexpr(std::is_pointer_v<T>) {
				if(new_value != nullptr) {
					if(m_previous_value == nullptr) {
						returnVal = true;
					}
					else {
						returnVal = *m_previous_value != *new_value;
					}
				}
				else if(m_previous_value != nullptr) {
					returnVal = true;
				}
				m_previous_value = std::move(new_value);
			}
			else {
				returnVal = m_previous_value != new_value;
				m_previous_value = std::move(new_value);
			}
			return returnVal;
		}

		/// @brief Returns the currently contained value
		///
		/// @return the current value
		/// @ingroup utils
		/// @headerfile "Hyperion/ChangeDetector.h"
		inline constexpr auto
		value() const noexcept(concepts::NoexceptCopyable<T>) -> T requires concepts::Copyable<T> {
			return m_previous_value;
		}

		constexpr auto
		operator=(const ChangeDetector& detector) noexcept(concepts::NoexceptCopyAssignable<T>)
			-> ChangeDetector& requires concepts::CopyAssignable<T>
		= default;
		constexpr auto
		operator=(ChangeDetector&& detector) noexcept(concepts::NoexceptMoveAssignable<T>)
			-> ChangeDetector& requires concepts::MoveAssignable<T>
		= default;

	  private:
		T m_previous_value = T();
	};

} // namespace hyperion
