#pragma once

#include <utility>

#include "Concepts.h"

namespace hyperion::utils {
	using concepts::Passable, concepts::DefaultConstructible, concepts::InequalityComparable,
		concepts::Copyable, concepts::Movable;
	/// @brief Stores a value and detects if an updated value is different than the previous one
	///
	/// @tparam T - The type to store and check for equality. T must be default constructible and
	/// have an inequality operator
	template<Passable T>
	requires DefaultConstructible<T> && InequalityComparable<T>
	class ChangeDetector {
	  public:
		/// @brief Create a default `ChangeDetector`
		ChangeDetector() noexcept = default;

		/// @brief Create a `ChangeDetector` with the given initial value
		///
		/// @param initialValue - The initial value to store in the detector
		explicit ChangeDetector(const T& initialValue) noexcept : mPreviousValue(initialValue) {
		}

		/// @brief Create a `ChangeDetector` with the given initial value
		///
		/// @param initialValue - The initial value to store in the detector
		explicit ChangeDetector(T&& initialValue) noexcept
			: mPreviousValue(std::forward<T>(initialValue)) {
		}

		ChangeDetector(const ChangeDetector& detector) noexcept requires Copyable<T>
		= default;
		ChangeDetector(ChangeDetector&& detector) noexcept requires Movable<T>
		= default;
		~ChangeDetector() noexcept = default;

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
		inline auto changed(const T& newValue) noexcept -> bool {
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
		inline auto changed(T&& newValue) noexcept -> bool {
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
				mPreviousValue = std::forward<T>(newValue);
			}
			else {
				returnVal = mPreviousValue != newValue;
				mPreviousValue = std::forward<T>(newValue);
			}
			return returnVal;
		}

		/// @brief Returns the currently contained value
		///
		/// @return the current value
		inline auto value() const noexcept -> T requires Copyable<T> {
			return mPreviousValue;
		}

		auto
		operator=(const ChangeDetector& detector) noexcept -> ChangeDetector& requires Copyable<T>
		= default;
		auto operator=(ChangeDetector&& detector) noexcept -> ChangeDetector& requires Movable<T>
		= default;

	  private:
		T mPreviousValue = T();
	};

} // namespace hyperion::utils
