/// @brief This is a simple change-of-value detector.
///
/// This is useful for when you need to store a value and track whether writes to the value are
/// actually changes
#pragma once

#include <utility>

#include "Concepts.h"

namespace hyperion {

	/// @brief Stores a value and detects if an updated value is different than the previous one
	///
	/// @tparam T - The type to store and check for equality. T must be default constructible and
	/// have an inequality operator
	template<typename T>
	requires concepts::DefaultConstructible<T> && concepts::DerefInequalityComparable<T>
	class ChangeDetector {
	  public:
		/// @brief Create a default `ChangeDetector`
		constexpr ChangeDetector() noexcept(concepts::NoexceptDefaultConstructible<T>) = default;

		/// @brief Create a `ChangeDetector` with the given initial value
		///
		/// @param initialValue - The initial value to store in the detector
		explicit constexpr ChangeDetector(const T& initialValue) noexcept(
			concepts::NoexceptCopyConstructible<T>)
			: mPreviousValue(initialValue) {
		}

		/// @brief Create a `ChangeDetector` with the given initial value
		///
		/// @param initialValue - The initial value to store in the detector
		explicit constexpr ChangeDetector(T&& initialValue) noexcept(
			concepts::NoexceptMoveConstructible<T>)
			: mPreviousValue(std::move(initialValue)) {
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
		/// compared, NOT the pointers themselves. If `newValue` is not `nullptr`, the stored
		/// pointer will be replaced with `newValue`. If  `newValue` is `nullptr`, false will always
		/// be returned.
		///
		/// @param newValue - The new value to store and check for equality
		///
		/// @return Whether the new value was different than the old one
		constexpr inline auto
		changed(const T& newValue) noexcept(concepts::NoexceptCopyAssignable<T>)
			-> bool requires concepts::CopyAssignable<T> {
			bool returnVal = false;
			if constexpr(std::is_pointer_v<T>) {
				if(newValue != nullptr) {
					if(mPreviousValue == nullptr) {
						returnVal = true;
					}
					else {
						returnVal = *mPreviousValue != *newValue;
					}
				}
				mPreviousValue = newValue;
			}
			else {
				returnVal = mPreviousValue != newValue;
				mPreviousValue = newValue;
			}
			return returnVal;
		}

		/// @brief Updates the stored value and returns if the new value
		/// is different than the previous one.
		///
		/// @note In the case that T is a pointer, the values located AT the pointers will be
		/// compared, NOT the pointers themselves. If `newValue` is not `nullptr`, the stored
		/// pointer will be replaced with `newValue`. If  `newValue` is `nullptr`, false will always
		/// be returned.
		///
		/// @param newValue - The new value to store and check for equality
		///
		/// @return Whether the new value was different than the old one
		constexpr inline auto changed(T&& newValue) noexcept(concepts::NoexceptMoveAssignable<T>)
			-> bool requires concepts::MoveAssignable<T> {
			bool returnVal = false;
			if constexpr(std::is_pointer_v<T>) {
				if(newValue != nullptr) {
					if(mPreviousValue == nullptr) {
						returnVal = true;
					}
					else {
						returnVal = *mPreviousValue != *newValue;
					}
				}
				mPreviousValue = std::move(newValue);
			}
			else {
				returnVal = mPreviousValue != newValue;
				mPreviousValue = std::move(newValue);
			}
			return returnVal;
		}

		/// @brief Returns the currently contained value
		///
		/// @return the current value
		constexpr inline auto
		value() const noexcept(concepts::NoexceptCopyable<T>) -> T requires concepts::Copyable<T> {
			return mPreviousValue;
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
		T mPreviousValue = T();
	};

} // namespace hyperion
