/// @file ScopedLockGuard.h
/// @author Braxton Salyer <braxtonsalyer@gmail.com>
/// @brief This file includes a unique lock guard for automatic unlocking of a typed lock
/// @version 0.1
/// @date 2021-08-27
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
#include <functional>
#include <memory>
#include <mutex>

#include "../Concepts.h"

namespace hyperion {
	/// @brief Basic scoped lock guard that wraps a single value of the template type
	/// Returned by other synchronization mechanisms as a way of ensuring RAII managed locking and
	/// unlocking
	///
	/// @tparam T - The type of the value to wrap in the guard
	template<concepts::NotReference T>
	class [[nodiscard]] ScopedLockGuard {
	  public:
		using reference = T&;
		using const_reference = const T&;

		ScopedLockGuard() = delete;
		ScopedLockGuard(const ScopedLockGuard<T>& guard) = delete;
		ScopedLockGuard(ScopedLockGuard<T>&& guard) noexcept
			: m_data(guard.m_data), m_unlocker(std::move(guard.m_locker)),
			  m_lock(std::move(guard.m_lock)) {
		}

		/// @brief Constructs a `ScopedLockGuard` with the given data and unlocker function
		///
		/// @param lock - The lock to be guarded
		/// @param data - The data to be guarded
		/// @param unlocker - The unlocker to call upon destruction, updating the data in the Lock
		/// and unlocking it
		ScopedLockGuard(std::unique_lock<std::mutex>&& lock,
						reference data,
						std::function<void(void)> unlocker) noexcept
			: m_data(data), m_unlocker(std::move(unlocker)), m_lock(std::move(lock)) {
		}

		~ScopedLockGuard() noexcept {
			if(m_lock.owns_lock()) {
				m_unlocker();
			}
		}

		/// @brief Writes the given value to the data
		///
		/// @param newValue - The new value to write to the data
		inline auto write(T newValue) noexcept -> void {
			m_data = newValue;
		}

		inline auto write() noexcept -> reference {
			return m_data;
		}

		/// @brief Returns the current value of the data
		///
		/// @return - The current data
		[[nodiscard]] inline auto read() const noexcept -> const_reference {
			return m_data;
		}

		[[nodiscard]] inline auto operator*() const noexcept -> const_reference {
			return m_data;
		}

		[[nodiscard]] inline auto operator*() noexcept -> reference {
			return m_data;
		}

		auto operator=(const ScopedLockGuard<T>& guard) -> ScopedLockGuard<T>& = delete;
		auto operator=(ScopedLockGuard<T>&& guard) noexcept -> ScopedLockGuard<T>& {
			if(this == &guard) {
				return *this;
			}

			m_data = guard.m_data;
			m_unlocker = std::move(guard.m_unlocker);
			m_lock = std::move(guard.m_lock);

			return *this;
		}

	  private:
		reference m_data;
		std::function<void(void)> m_unlocker;
		std::unique_lock<std::mutex> m_lock;
	};
} // namespace hyperion
