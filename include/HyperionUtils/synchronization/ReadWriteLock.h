/// @file ReadWriteLock.h
/// @author Braxton Salyer <braxtonsalyer@gmail.com>
/// @brief This file includes a reader-write lock type that manages access to a single instance of
/// a type
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

#include <memory>
#include <shared_mutex>

#include "../HyperionDef.h"
#include "../Memory.h"
#include "../Result.h"
#include "ScopedLockGuard.h"

namespace hyperion {
	/// @brief Enum containing the possible errors that can occur when locking a ReadWriteLock
	enum class ReadWriteLockErrorCategory : i8
	{
		Success = 0,
		AlreadyLocked,
		Unknown = -1
	};

	class ReadWriteLockErrorDomain {
	  public:
		using value_type = ReadWriteLockErrorCategory;
		using ReadWriteLockStatusCode = error::StatusCode<ReadWriteLockErrorDomain>;
		using ReadWriteLockErrorCode = error::ErrorCode<ReadWriteLockErrorDomain>;

		static constexpr const char (&UUID)[error::num_chars_in_uuid] // NOLINT
			= "1bab4a0f-c777-4da7-a90c-45a9bb2fe429";

		static constexpr u64 ID = error::parse_uuid_from_string(UUID);

		constexpr ReadWriteLockErrorDomain() noexcept = default;
		explicit constexpr ReadWriteLockErrorDomain(u64 uuid) noexcept : m_uuid(uuid) {
		}
		explicit constexpr ReadWriteLockErrorDomain(const error::UUIDString auto& uuid) noexcept
			: m_uuid(error::parse_uuid_from_string(uuid)) {
		}
		constexpr ReadWriteLockErrorDomain(const ReadWriteLockErrorDomain&) noexcept = default;
		constexpr ReadWriteLockErrorDomain(ReadWriteLockErrorDomain&&) noexcept = default;
		constexpr ~ReadWriteLockErrorDomain() noexcept = default;

		[[nodiscard]] constexpr inline auto id() const noexcept -> u64 {
			return m_uuid;
		}

		[[nodiscard]] constexpr inline auto name() const noexcept -> std::string_view { // NOLINT
			return "ReadWriteLockErrorDomain";
		}

		[[nodiscard]] constexpr inline auto message(value_type code) // NOLINT
			const noexcept -> std::string_view {
			if(code == value_type::Success) {
				return "Success";
			}
			else {
				return "ReadWriteLock Already Locked.";
			}
		}

		[[nodiscard]] constexpr inline auto message(const ReadWriteLockStatusCode& code) // NOLINT
			const noexcept -> std::string_view {
			return message(code.code());
		}

		[[nodiscard]] constexpr inline auto is_error(const ReadWriteLockStatusCode& code) // NOLINT
			const noexcept -> bool {
			return code.code() != value_type::Success;
		}

		[[nodiscard]] constexpr inline auto
		is_success(const ReadWriteLockStatusCode& code) // NOLINT
			const noexcept -> bool {
			return code.code() == value_type::Success;
		}

		template<typename Domain2>
		[[nodiscard]] constexpr inline auto
		are_equivalent(const ReadWriteLockStatusCode& lhs,
					   const error::StatusCode<Domain2>& rhs) const noexcept -> bool {
			if constexpr(concepts::Same<ReadWriteLockStatusCode, error::StatusCode<Domain2>>) {
				return lhs.code() == rhs.code();
			}
			else {
				return false;
			}
		}

		[[nodiscard]] constexpr inline auto
		as_generic_code(const ReadWriteLockStatusCode& code) // NOLINT
			const noexcept -> error::GenericStatusCode {
			if(code.code() == value_type::Success || code.code() == value_type::Unknown) {
				return make_status_code(static_cast<error::Errno>(code.code()));
			}
			else {
				return make_status_code(error::Errno::Unknown);
			}
		}

		[[nodiscard]] constexpr inline auto success_value() const noexcept -> value_type { // NOLINT
			return value_type::Success;
		}

		template<typename Domain>
		friend constexpr inline auto
		operator==(const ReadWriteLockErrorDomain& lhs, const Domain& rhs) noexcept -> bool {
			return rhs.id() == lhs.id();
		}

		template<typename Domain>
		friend constexpr inline auto
		operator!=(const ReadWriteLockErrorDomain& lhs, const Domain& rhs) noexcept -> bool {
			return rhs.id() != lhs.id();
		}

		constexpr auto
		operator=(const ReadWriteLockErrorDomain&) noexcept -> ReadWriteLockErrorDomain& = default;
		constexpr auto
		operator=(ReadWriteLockErrorDomain&&) noexcept -> ReadWriteLockErrorDomain& = default;

	  private:
		u64 m_uuid = ID;
	};

	using ReadWriteLockStatusCode = ReadWriteLockErrorDomain::ReadWriteLockStatusCode;
	using ReadWriteLockErrorCode = ReadWriteLockErrorDomain::ReadWriteLockErrorCode;
	using ReadWriteLockError = error::Error<ReadWriteLockErrorDomain>;
} // namespace hyperion

template<>
constexpr inline auto make_status_code_domain<hyperion::ReadWriteLockErrorDomain>() noexcept
	-> hyperion::ReadWriteLockErrorDomain {
	return {};
}

namespace hyperion {

	static_assert(error::StatusCodeDomain<ReadWriteLockErrorDomain>);

	template<>
	struct error::status_code_enum_info<ReadWriteLockErrorCategory> {
		using domain_type = ReadWriteLockErrorDomain;
		static constexpr bool value = true;
	};

	IGNORE_PADDING_START
	/// @brief Basic Read/Write Lock for synchronizing a single piece of data
	///
	/// @tparam T - The type of the value to be guarded/synchronized
	template<concepts::NotReference T>
	class ReadWriteLock {
	  public:
		using LockError = ReadWriteLockError;
		using LockGuard = ScopedLockGuard<T>;
		using LockResult = Result<LockGuard, LockError>;

		/// @brief Constructs a default `ReadWriteLock`
		ReadWriteLock() noexcept(
			concepts::NoexceptDefaultConstructible<T>) requires concepts::DefaultConstructible<T>
		= default;

		/// @brief Copy constructs a `ReadWriteLock`
		///
		/// @param lock - The `ReadWriteLock` to copy
		ReadWriteLock(const ReadWriteLock<T>& lock) noexcept(
			concepts::NoexceptCopyConstructible<T>) requires concepts::CopyConstructible<T>
		= default;

		/// @brief Move constructs the given `ReadWriteLock`
		///
		/// @param lock - The `ReadWriteLock` to move
		ReadWriteLock(ReadWriteLock<T>&& lock) noexcept(
			concepts::NoexceptMoveConstructible<T>&& concepts::NoexceptCopyConstructible<
				T>) requires concepts::MoveConstructible<T> && concepts::CopyConstructible<T>
		= default;

		/// @brief Constructs a `ReadWriteLock` with the given initial data
		///
		/// @param data - The data to guard
		explicit ReadWriteLock(const T& data) noexcept(
			concepts::NoexceptCopyConstructible<T>) requires concepts::CopyConstructible<T>
			: m_data(data),
			  m_cached(data) {
		}

		/// @brief Constructs a `ReadWriteLock` with the given initial data
		///
		/// @param data - The data to guard
		explicit ReadWriteLock(T&& data) noexcept(
			concepts::NoexceptMoveConstructible<T>&& concepts::NoexceptCopyConstructible<
				T>) requires concepts::MoveConstructible<T> && concepts::CopyConstructible<T>
			: m_data(std::move(data)),
			  m_cached(m_data) {
		}
		~ReadWriteLock() noexcept(concepts::NoexceptDestructible<T>) = default;

		/// @brief Returns the current cached value of the data. This will be up to date with the
		/// most recent unlock, but will not reflect changes made by an active locked access
		///
		/// @return - The current data
		[[nodiscard]] inline auto
		read() noexcept(concepts::NoexceptCopyable<T>) -> T requires concepts::Copyable<T> {
			auto lock = std::shared_lock(m_read_mutex);
			return m_cached;
		}

		/// @brief Tries to lock this for mutable access. Nonblocking. If locking is successful,
		/// returns an `Ok(ScopedLockGuard<T>)`, otherwise, returns an `Err`
		///
		/// @return - `Ok(ScopedLockGuard<T>)` if successful, otherwise, `Err(ReadWriteLockError)`
		[[nodiscard]] inline auto try_lock() noexcept -> LockResult {
			auto lock = std::unique_lock(m_write_mutex, std::defer_lock_t());
			if(lock.try_lock()) {
				return Ok(
					ScopedLockGuard<T>(std::move(lock), m_data, [this]() { this->unlock(); }));
			}
			else {
				return Err(
					ReadWriteLockError(make_error_code(ReadWriteLockErrorCategory::AlreadyLocked)));
			}
		}

		/// @brief Locks this for mutable access. If this is currently locked, this call will block
		/// until it is unlocked, then return a scoped lock guard
		///
		/// @return `ScopedLockGuard<T>` - The lock guard for the data
		[[nodiscard]] inline auto lock() noexcept -> ScopedLockGuard<T> {
			return ScopedLockGuard<T>(std::unique_lock(m_write_mutex), m_data, [this]() {
				this->unlock();
			});
		}

		auto operator=(const ReadWriteLock<T>& lock) noexcept(concepts::NoexceptCopyAssignable<T>)
			-> ReadWriteLock<T>& requires concepts::CopyAssignable<T>
		= default;
		auto operator=(ReadWriteLock<T>&& lock) noexcept(concepts::NoexceptMoveAssignable<T>)
			-> ReadWriteLock<T>& requires concepts::MoveAssignable<T>
		= default;

	  protected:
		/// @brief Called by the scoped lock guard to update the data and unlock this
		inline auto unlock() noexcept(concepts::NoexceptCopyAssignable<T>)
			-> void requires concepts::CopyAssignable<T> {
			auto lock = std::unique_lock(m_read_mutex);
			m_cached = m_data;
		}

	  private : T m_cached = T();
		T m_data = T();
		std::shared_mutex m_read_mutex = std::shared_mutex();
		std::mutex m_write_mutex = std::mutex();
	};
	IGNORE_PADDING_STOP
} // namespace hyperion
