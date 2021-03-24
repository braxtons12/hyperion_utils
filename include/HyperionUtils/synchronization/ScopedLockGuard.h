#pragma once
#include <functional>
#include <memory>

namespace hyperion::utils {
	/// @brief Basic scoped lock guard that wraps a single value of the template type
	/// Returned by other synchronization mechanisms as a way of ensuring RAII managed locking and
	/// unlocking
	///
	/// @tparam T - The type of the value to wrap in the guard
	template<typename T>
	class [[nodiscard]] ScopedLockGuard {
	  public:
		ScopedLockGuard() = default;
		ScopedLockGuard(const ScopedLockGuard<T>& guard) = delete;
		ScopedLockGuard(ScopedLockGuard<T>&& guard) noexcept = default;

		/// @brief Constructs a `ScopedLockGuard` with the given data and unlocker function
		///
		/// @param data - The data to be guarded
		/// @param unlocker - The unlocker to call upon destruction, updating the data in the Lock
		/// and unlocking it
		explicit ScopedLockGuard(std::shared_ptr<T> data,
								 std::function<void(void)> unlocker) noexcept
			: m_data(std::move(data)), m_unlocker(std::move(unlocker)) {
		}

		~ScopedLockGuard() noexcept {
			m_unlocker();
		}

		/// @brief Writes the given value to the data
		///
		/// @param newValue - The new value to write to the data
		inline auto write(T newValue) noexcept -> void {
			*m_data = newValue;
		}

		/// @brief Returns the current value of the data
		///
		/// @return - The current data
		[[nodiscard]] inline auto read() const noexcept -> T {
			return *m_data;
		}

		auto operator=(const ScopedLockGuard<T>& guard) -> ScopedLockGuard<T>& = delete;
		auto operator=(ScopedLockGuard<T>&& guard) noexcept -> ScopedLockGuard<T>& = default;

	  private:
		std::shared_ptr<T> m_data;
		std::function<void(void)> m_unlocker;
	};
} // namespace hyperion::utils
