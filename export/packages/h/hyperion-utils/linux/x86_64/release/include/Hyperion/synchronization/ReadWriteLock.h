/// @file ReadWriteLock.h
/// @author Braxton Salyer <braxtonsalyer@gmail.com>
/// @brief This file includes a reader/writer lock type that manages access to a single instance of
/// a type
/// @version 0.1
/// @date 2022-06-04
///
/// MIT License
/// @copyright Copyright (c) 2022 Braxton Salyer <braxtonsalyer@gmail.com>
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

#include <Hyperion/HyperionDef.h>
#include <Hyperion/Platform.h>
#include <Hyperion/Result.h>
#include <Hyperion/synchronization/ScopedLockGuard.h>

#ifndef ERROR_CANT_WAIT
	#define ERROR_CANT_WAIT 0x0000022A // NOLINT(cppcoreguidelines-macro-usage)
#endif								   // ERROR_CANT_WAIT

namespace hyperion {
	IGNORE_PADDING_START
	/// @brief Owning reader/writer lock for synchronizing a single piece of data
	///
	/// Owns an instance of type `T`, and provides synchronized protected access to it
	/// with `ScopedLockGuard<T, LockType>`s. `write()` and `try_write()` provide exclusive
	/// read/write access via `ScopedLockGuard<T, std::unique_lock>`,
	/// while `read()` and `try_read()` provide shared read-only access via
	/// `ScopedLockGuard`<T, std::shared_lock>`
	///
	/// Example:
	/// @code {.cpp}
	/// #include "Hyperion/Synchronization.h"
	/// #include <array>
	/// #include <thread>
	/// // create a `ReadWriteLock<T>` protecting a `std::array`
	/// auto vec = ReadWriteLock(std::array<i32, 10>());
	/// auto previous = 1_i32;
	/// {
	/// 	// get write access and mutate all of the elements in the array
	/// 	auto write_guard = vec.write();
	/// 	for(auto& i: *write_guard) {
	/// 		i = previous + previous;
	/// 		previous = i;
	/// 	}
	/// }
	/// // ^^^ Notice we had to scope `write_guard`'s lifetime so we could get read access below.
	/// // Guards only release ownership when they're destroyed
	///
	/// // get read access to the array and print each element on two separate threads
	/// auto read_guard = vec.read();
	/// auto thread1 = std::jthread([guard = read_guard]() {
	/// 	for(auto i = 0_usize; i < 5_usize; ++i) {
	/// 		println("{}", guard->at(i));
	///     }
	/// });
	/// auto thread1 = std::jthread([guard = read_guard]() {
	/// 	for(auto i = 5_usize; i < 10_usize; ++i) {
	/// 		println("{}", guard->at(i));
	///     }
	/// });
	/// @endcode
	///
	/// @tparam T - The type of the value to be guarded/synchronized
	/// @ingroup synchronization
	/// @headerfile "Hyperion/synchronization/ReadWriteLock.h"
	template<concepts::NotReference T>
	class ReadWriteLock {
	  public:
		/// @brief Constructs a default `ReadWriteLock<T>`
		///
		/// # Requirements
		/// - `concepts::DefaultConstructible<T>`: The protected `T` must be default constructible
		/// in order to default construct a `ReadWriteLock<T>`
		/// @ingroup synchronization
		ReadWriteLock() noexcept(concepts::NoexceptDefaultConstructible<T>)
		requires concepts::DefaultConstructible<T>
		= default;

		/// @brief `ReadWriteLock<T>`s are not copyable
		/// @ingroup synchronization
		ReadWriteLock(const ReadWriteLock& lock) = delete;

		/// @brief Move constructs the given `ReadWriteLock<T>` from the given one
		///
		/// Acquires exclusive access to the data in the given `lock` and moves its data into this
		/// one.
		///
		/// @param lock - The `ReadWriteLock<T>` to move
		///
		/// # Requirements
		/// - `concepts::MoveAssignable<T>`: The protected `T` must be move assignable in order to
		/// move construct a `ReadWriteLock<T>`
		/// @ingroup synchronization
		ReadWriteLock(ReadWriteLock&& lock) noexcept(concepts::NoexceptMoveAssignable<T>)
		requires concepts::MoveAssignable<T>
		{
			auto _lock = std::unique_lock(lock.m_mutex);
			// NOLINTNEXTLINE(cppcoreguidelines-prefer-member-initializer)
			m_data = std::move(lock.m_data);
		}

		/// @brief Constructs a `ReadWriteLock<T>` with the given initial data
		///
		/// @param data - The data to guard
		///
		/// # Requirements
		/// - `concepts::CopyConstructible<T>`: The protected `T` must be copy constructible in
		/// order to construct a `ReadWriteLock<T>` initialized with a copy of the given `data`
		/// @ingroup synchronization
		ReadWriteLock(const T& data) // NOLINT(google-explicit-constructor,
									 // hicpp-explicit-conversions)
			noexcept(concepts::NoexceptCopyConstructible<T>)
		requires concepts::CopyConstructible<T>
		: m_data(data) {
		}

		/// @brief Constructs a `ReadWriteLock<T>` with the given initial data
		///
		/// @param data - The data to guard
		///
		/// # Requirements
		/// - `concepts::MoveConstructible<T>`: The protected `T` must be move constructible in
		/// order to construct a `ReadWriteLock<T>` initialized by moving `data` into it
		/// @ingroup synchronization
		ReadWriteLock(T&& data) // NOLINT(google-explicit-constructor, hicpp-explicit-conversions)
			noexcept(concepts::NoexceptMoveConstructible<T>)
		requires concepts::MoveConstructible<T>
		: m_data(std::move(data)) {
		}

		~ReadWriteLock() noexcept(concepts::NoexceptDestructible<T>) = default;

		/// @brief Returns a `ReadLockGuard<T>` providing read-only access to the protected `T`
		///
		/// @return a `ReadLockGuard<T>` to the data
		/// @ingroup synchronization
		[[nodiscard]] inline auto read() const noexcept -> ReadLockGuard<T> {
			// NOLINTNEXTLINE(google-readability-casting)
			return ReadLockGuard<T>(std::shared_lock(m_mutex), m_data);
		}

		/// @brief Attempts to gain shared read-only access to the protected `T`
		///
		/// If access is gained successfully, returns a `ReadLockGuard<T>` to the data,
		/// otherwise returns an error.
		///
		/// @return `Ok(ReadLockGuard<T>)` on success, `Err(error::SystemError)` on failure
		/// @ingroup synchronization
		[[nodiscard]] inline auto try_read() const noexcept -> Result<ReadLockGuard<T>> {
			auto lock = std::shared_lock(m_mutex, std::defer_lock_t());
			if(!lock.try_lock()) {
#if !HYPERION_PLATFORM_WINDOWS || HYPERION_WINDOWS_USES_POSIX_CODES
				return Err(error::SystemError(EWOULDBLOCK));
#else
				return Err(error::SystemError(ERROR_CANT_WAIT));
#endif // !HYPERION_PLATFORM_WINDOWS || HYPERION_WINDOWS_USES_POSIX_CODES
			}

			return Ok(ReadLockGuard<T>(std::move(lock), m_data));
		}

		/// @brief Returns a `WriteLockGuard<T>` providing exclusive read/write access to the
		/// protected `T`
		///
		/// @return a `WriteLockGuard<T>` the the data
		/// @ingroup synchronization
		[[nodiscard]] inline auto write() noexcept -> WriteLockGuard<T> {
			// NOLINTNEXTLINE(google-readability-casting)
			return WriteLockGuard<T>(std::unique_lock(m_mutex), m_data);
		}

		/// @brief Attempts to gain exclusive read/write access to the protected `T`
		///
		/// If access is gained successfully, returns a `WriteLockGuard<T>` to the data,
		/// otherwise returns an error.
		///
		/// @return `Ok(WriteLockGuard<T>)` on success, `Err(error::SystemError)` on failure
		/// @ingroup synchronization
		[[nodiscard]] inline auto try_write() noexcept -> Result<WriteLockGuard<T>> {
			auto lock = std::unique_lock(m_mutex, std::defer_lock_t());
			if(!lock.try_lock()) {
#if !HYPERION_PLATFORM_WINDOWS || HYPERION_WINDOWS_USES_POSIX_CODES
				return Err(error::SystemError(EWOULDBLOCK));
#else
				return Err(error::SystemError(ERROR_CANT_WAIT));
#endif // !HYPERION_PLATFORM_WINDOWS || HYPERION_WINDOWS_USES_POSIX_CODES
			}

			return Ok(WriteLockGuard<T>(std::move(lock), m_data));
		}

		/// @brief `ReadWriteLock<T>` is not copyable
		/// @ingroup synchronization
		auto operator=(const ReadWriteLock& lock) -> ReadWriteLock& = delete;
		/// @brief Move-assigns this `ReadWriteLock<T>` from the given one
		/// Acquires exclusive access to the data in the given `lock` and moves its data into this
		/// one.
		///
		/// @param lock - The `ReadWriteLock<T>` to move
		///
		/// # Requirements
		/// - `concepts::MoveAssignable<T>`: The protected `T` must be move assignable in order to
		/// move construct a `ReadWriteLock<T>`
		/// @ingroup synchronization
		auto operator=(ReadWriteLock&& lock) noexcept(concepts::NoexceptMoveAssignable<T>)
			-> ReadWriteLock&
		requires concepts::MoveAssignable<T>
		{
			if(this == &lock) {
				return *this;
			}

			auto _lock = std::unique_lock(lock);
			m_data = std::move(lock.m_data);

			return *this;
		}

		// clang-format off
	  private:
		T m_data = T();
		mutable std::shared_mutex m_mutex = std::shared_mutex();
		// clang-format on
	};
	template<typename T>
	ReadWriteLock(const T&) -> ReadWriteLock<T>;
	template<typename T>
	ReadWriteLock(T&&) -> ReadWriteLock<T>;

	IGNORE_PADDING_STOP
} // namespace hyperion
