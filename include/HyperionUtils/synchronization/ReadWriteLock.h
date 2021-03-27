#pragma once

#include <atomic>
#include <memory>

#include "../Macros.h"
#include "../OptionAndResult.h"
#include "ScopedLockGuard.h"

namespace hyperion::utils {
	/// @brief Enum containing the possible errors that can occur when locking a ReadWriteLock
	enum class ReadWriteLockErrors
	{
		AlreadyLocked
	};

	IGNORE_PADDING_START
	/// @brief The error that can occur when locking a ReadWriteLock
	///
	/// @tparam Type - The type of error
	template<ReadWriteLockErrors Type>
	class ReadWriteLockError final : public Error {
	  public:
		ReadWriteLockError() noexcept : Error("Lock Failure: Lock has already been acquired") {
		}
		~ReadWriteLockError() noexcept final = default;
	};

	/// @brief Basic Read/Write Lock for synchronizing a single piece of data
	///
	/// @tparam T - The type of the value to be guarded/synchronized
	template<NotReference T>
	requires DefaultConstructible<T>
	class ReadWriteLock {
	  public:
		using LockError = ReadWriteLockError<ReadWriteLockErrors::AlreadyLocked>;
		using LockGuard = ScopedLockGuard<T>;
		using LockResult = Result<LockGuard, LockError>;

		/// @brief Constructs a default `ReadWriteLock`
		ReadWriteLock() noexcept = default;

		/// @brief Copy constructs a `ReadWriteLock`
		///
		/// @param lock - The `ReadWriteLock` to copy
		ReadWriteLock(const ReadWriteLock<T>& lock) noexcept = default;

		/// @brief Move constructs the given `ReadWriteLock`
		///
		/// @param lock - The `ReadWriteLock` to move
		ReadWriteLock(ReadWriteLock<T>&& lock) noexcept = default;

		/// @brief Constructs a `ReadWriteLock` with the given initial data
		///
		/// @param data - The data to guard
		explicit ReadWriteLock(const T& data) noexcept
			: m_data(std::make_shared<T>(data)), m_cached(data) {
		}

		/// @brief Constructs a `ReadWriteLock` with the given initial data
		///
		/// @param data - The data to guard
		explicit ReadWriteLock(T&& data) noexcept
			: m_data(std::make_shared<T>(data)), m_cached(data) {
		}
		~ReadWriteLock() noexcept = default;

		/// @brief Returns the current cached value of the data. This will be up to date with the
		/// most recent unlock, but will not reflect changes made by an active locked access
		///
		/// @return - The current data
		[[nodiscard]] inline auto read() const noexcept -> const T& {
			return m_cached;
		}

		/// @brief Tries to lock this for mutable access. Nonblocking. If locking is successful,
		/// returns an `Ok(ScopedLockGuard<T>)`, otherwise, returns an `Err`
		///
		/// @return - `Ok(ScopedLockGuard<T>)` if successful, otherwise, `Err(ReadWriteLockError)`
		[[nodiscard]] inline auto try_lock() noexcept -> LockResult {
			bool not_locked = false;
			if(m_locked.compare_exchange_strong(not_locked, true, std::memory_order_seq_cst)) {
				return Ok(std::move(ScopedLockGuard<T>(m_data, [this]() { this->unlock(); })));
			}
			else {
				return Err(LockError());
			}
		}

		/// @brief Locks this for mutable access. If this is currently locked, this call will block
		/// until it is unlocked, then return a scoped lock guard
		///
		/// @return `ScopedLockGuard<T>` - The lock guard for the data
		[[nodiscard]] inline auto lock() noexcept -> ScopedLockGuard<T> {
			bool not_locked = false;
			while(!m_locked.compare_exchange_strong(not_locked, true, std::memory_order_seq_cst)) {
				not_locked = false;
			}
			return ScopedLockGuard<T>(m_data, [this]() { this->unlock(); });
		}

		auto operator=(const ReadWriteLock<T>& lock) noexcept -> ReadWriteLock<T>& = default;
		auto operator=(ReadWriteLock<T>&& lock) noexcept -> ReadWriteLock<T>& = default;

	  protected:
		/// @brief Called by the scoped lock guard to update the data and unlock this
		inline auto unlock() noexcept -> void {
			m_cached = *m_data;
			m_locked.store(false);
		}

	  private:
		T m_cached = T();
		std::shared_ptr<T> m_data = std::make_shared<T>(m_cached);
		std::atomic_bool m_locked = std::atomic_bool(false);
	};
	IGNORE_PADDING_STOP
} // namespace hyperion::utils
