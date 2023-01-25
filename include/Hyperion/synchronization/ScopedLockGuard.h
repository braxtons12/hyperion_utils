/// @file ScopedLockGuard.h
/// @author Braxton Salyer <braxtonsalyer@gmail.com>
/// @brief This file includes a unique lock guard for automatic unlocking of a typed lock
/// @version 0.1
/// @date 2023-01-25
///
/// MIT License
/// @copyright Copyright (c) 2023 Braxton Salyer <braxtonsalyer@gmail.com>
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
#include <functional>
#include <memory>
#include <mutex>
#include <shared_mutex>

namespace hyperion {

	/// @brief A `LockType` is a `std::unique_lock` or `std::shared_lock`
	/// @ingroup synchronization
	/// @headerfile "Hyperion/synchronization/ScopedLockGuard.h"
	template<template<typename> typename T>
	concept LockType = concepts::Same<T<std::shared_mutex>, std::unique_lock<std::shared_mutex>>
					   || concepts::Same<T<std::shared_mutex>, std::shared_lock<std::shared_mutex>>;

	/// @brief Usable by an owning lock type to provide scoped synchronized access to the protected
	/// data.
	///
	/// If `Lock` is `std::unique_lock`, this provides exclusive read/write access to the protected
	/// data and is not copyable. If `Lock` is std::shared_lock`, this provides shared read-only
	/// access to the protected data and is copyable.
	///
	/// @tparam T - The type of the data to provide access to
	/// @tparam Lock - The `LockType` determining the type of access this `ScopedLockGuard` provides
	/// @ingroup synchronization
	/// @headerfile "Hyperion/synchronization/ScopedLockGuard.h"
	template<concepts::NotReference T, template<typename> typename Lock>
	requires LockType<Lock>
	class [[nodiscard("Discarding a ScopedLockGuard defeats the purpose of locking the protected "
					  "data")]] ScopedLockGuard;

	/// @brief Specialization of `ScopedLockGuard<T, Lock>` for `std::unique_lock`.
	/// Provides exclusive access to the protected data
	///
	/// @tparam T - The type of the data to provide access to
	/// @ingroup synchronization
	/// @headerfile "Hyperion/synchronization/ScopedLockGuard.h"
	template<concepts::NotReference T>
	class [[nodiscard("Discarding a ScopedLockGuard defeats the purpose of locking the protected "
					  "data")]] ScopedLockGuard<T, std::unique_lock> {
	  public:
		using pointer = T*;
		using pointer_to_const = const T*;
		using reference = T&;
		using const_reference = const T&;
		using lock_type = std::unique_lock<std::shared_mutex>;

		/// @brief An exclusive-access `ScopedLockGuard` can't be default-constructed
		/// @ingroup synchronization
		ScopedLockGuard() = delete;
		/// @brief An exclusive-access `ScopedLockGuard` can't be copied
		/// @ingroup synchronization
		ScopedLockGuard(const ScopedLockGuard& guard) = delete;

		/// @brief Move constructs an exclusive access `ScopedLockGuard` from the given one
		///
		/// @param guard - The `ScopedLockGuard` to move
		/// @ingroup synchronization
		ScopedLockGuard(ScopedLockGuard && guard) noexcept
			: m_data(guard.m_data), m_lock(std::move(guard.m_lock)) {
		}

		/// @brief Constructs a `ScopedLockGuard` with the given data and lock
		///
		/// @param lock - The `std::unique_lock` providing synchronization
		/// @param data - A reference to the protected data
		/// @ingroup synchronization
		ScopedLockGuard(lock_type && lock, reference data) noexcept
			: m_data(data), m_lock(std::move(lock)) {
		}

		~ScopedLockGuard() noexcept = default;

		/// @brief Writes the given value to the protected data
		///
		/// @param newValue - The new value to write to the data
		/// @ingroup synchronization
		inline auto write(const T& newValue) noexcept(concepts::NoexceptCopyAssignable<T>)
			->void
		requires concepts::CopyAssignable<T>
		{
			m_data = newValue;
		}

		/// @brief Writes the given value to the protected data
		///
		/// @param newValue - The new value to write to the data
		/// @ingroup synchronization
		inline auto write(T && newValue) noexcept(concepts::NoexceptMoveAssignable<T>)
			->void
		requires concepts::MoveAssignable<T>
		{
			m_data = std::move(newValue);
		}

		/// @brief Provides a mutable reference to the protected data.
		///
		/// The returned reference must not outlive the `ScopedLockGuard` that provided it. To do so
		/// results in undefined behavior.
		///
		/// @return a mutable reference to the protected data
		/// @ingroup synchronization
		inline auto write() noexcept -> reference {
			return m_data;
		}

		/// @brief Provides a const reference to the protected data.
		///
		/// The returned reference must not outlive the `ScopedLockGuard` that provided it. To do so
		/// results in undefined behavior.
		///
		/// @return a const reference to the protected data
		/// @ingroup synchronization
		[[nodiscard]] inline auto read() const noexcept -> const_reference {
			return m_data;
		}

		/// @brief Provides a mutable reference to the protected data.
		///
		/// The returned reference must not outlive the `ScopedLockGuard` that provided it. To do so
		/// results in undefined behavior.
		///
		/// @return a mutable reference to the protected data
		/// @ingroup synchronization
		[[nodiscard]] inline auto operator*() noexcept -> reference {
			return m_data;
		}

		/// @brief Provides a const reference to the protected data.
		///
		/// The returned reference must not outlive the `ScopedLockGuard` that provided it. To do so
		/// results in undefined behavior.
		///
		/// @return a const reference to the protected data
		/// @ingroup synchronization
		[[nodiscard]] inline auto operator*() const noexcept -> const_reference {
			return m_data;
		}

		[[nodiscard]] inline auto operator->() noexcept -> pointer {
			return &m_data;
		}

		[[nodiscard]] inline auto operator->() const noexcept -> pointer_to_const {
			return &m_data;
		}

		[[nodiscard]] explicit inline operator T()
			const noexcept(concepts::NoexceptCopyConstructible<T>) {
			return m_data;
		}

		/// @brief Exclusive-access `ScopedLockGuard`s can't be copy-assigned to
		/// @ingroup synchronization
		auto operator=(const ScopedLockGuard& guard)->ScopedLockGuard& = delete;
		/// @brief Exclusive-access `ScopedLockGuard`s can't be move-assigned to
		/// @ingroup synchronization
		auto operator=(ScopedLockGuard&& guard)->ScopedLockGuard& = delete;

		/// @brief Copy assigns the given value to the protected data
		///
		/// @param value - The value to copy-assign to the protected data
		///
		/// # Requirements
		/// - `concepts::CopyAssignable<T>`: `T` must be copy assignable to copy-assign to the
		/// protected data
		/// @ingroup synchronization
		auto operator=(const T& value) noexcept(concepts::NoexceptCopyAssignable<T>)
			->ScopedLockGuard&
		requires concepts::CopyAssignable<T>
		{
			m_data = value;
			return *this;
		}
		/// @brief Move assigns the given value to the protected data
		///
		/// @param value - The value to move-assign to the protected data
		///
		/// # Requirements
		/// - `concepts::MoveAssignable<T>`: `T` must be move assignable to move-assign to the
		/// protected data
		/// @ingroup synchronization
		auto operator=(T&& value) noexcept(concepts::NoexceptMoveAssignable<T>)
			->ScopedLockGuard&
		requires concepts::MoveAssignable<T>
		{
			m_data = std::move(value);
			return *this;
		}

		// clang-format off
	  private:
		reference m_data;
		lock_type m_lock;
		// clang-format on
	};

	/// @brief Specialization of `ScopedLockGuard<T, Lock>` for `std::shared_lock`.
	/// Provides shared read-only access to the protected data
	///
	/// @tparam T - The type of the data to provide access to
	/// @ingroup synchronization
	/// @headerfile "Hyperion/synchronization/ScopedLockGuard.h"
	template<concepts::NotReference T>
	class [[nodiscard("Discarding a ScopedLockGuard defeats the purpose of locking the protected "
					  "data")]] ScopedLockGuard<T, std::shared_lock> {
	  public:
		using pointer = T*;
		using pointer_to_const = const T*;
		using reference = T&;
		using const_reference = const T&;
		using lock_type = std::shared_lock<std::shared_mutex>;

		/// @brief A shared read-only `ScopedLockGuard` can't be default-constructed
		/// @ingroup synchronization
		ScopedLockGuard() = delete;
		/// @brief Copy constructs a shared read-only access `ScopedLockGuard` from the given one
		///
		/// @param guard - The `ScopedLockGuard` to copy
		/// @ingroup synchronization
		ScopedLockGuard(const ScopedLockGuard& guard) noexcept
			: m_data(guard.m_data), m_lock(*guard.m_lock.mutex()) {
		}
		/// @brief Move constructs a shared read-only access `ScopedLockGuard` from the given one
		///
		/// @param guard - The `ScopedLockGuard` to move
		/// @ingroup synchronization
		ScopedLockGuard(ScopedLockGuard && guard) noexcept
			: m_data(guard.m_data), m_lock(std::move(guard.m_lock)) {
		}

		/// @brief Constructs a `ScopedLockGuard` with the given data and lock
		///
		/// @param lock - The `std::shared_lock` providing synchronization
		/// @param data - A reference to the protected data
		/// @ingroup synchronization
		ScopedLockGuard(lock_type && lock, const_reference data) noexcept
			: m_data(data), m_lock(std::move(lock)) {
		}

		~ScopedLockGuard() noexcept = default;

		/// @brief Provides a const reference to the protected data.
		///
		/// The returned reference must not outlive the `ScopedLockGuard` that provided it. To do so
		/// results in undefined behavior.
		///
		/// @return a const reference to the protected data
		/// @ingroup synchronization
		[[nodiscard]] inline auto read() const noexcept -> const_reference {
			return m_data;
		}

		/// @brief Provides a const reference to the protected data.
		///
		/// The returned reference must not outlive the `ScopedLockGuard` that provided it. To do so
		/// results in undefined behavior.
		///
		/// @return a const reference to the protected data
		/// @ingroup synchronization
		[[nodiscard]] inline auto operator*() const noexcept -> const_reference {
			return m_data;
		}

		[[nodiscard]] inline auto operator->() const noexcept -> pointer_to_const {
			return &m_data;
		}

		[[nodiscard]] explicit inline operator T()
			const noexcept(concepts::NoexceptCopyConstructible<T>) {
			return m_data;
		}

		/// @brief Shared read-only access `ScopedLockGuard`s can't be copy-assigned to
		/// @ingroup synchronization
		auto operator=(const ScopedLockGuard& guard)->ScopedLockGuard& = delete;
		/// @brief Shared read-only access `ScopedLockGuard`s can't be move-assigned to
		/// @ingroup synchronization
		auto operator=(ScopedLockGuard&& guard)->ScopedLockGuard& = delete;

	  private:
		const_reference m_data;
		lock_type m_lock;
	};

    /// @brief Convience alias for an exclusive-access lock guard,
    /// aka `ScopedLockGuard<T, std::unique_lock>`
    /// @ingroup synchronization
	template<typename T>
	using WriteLockGuard = ScopedLockGuard<T, std::unique_lock>;

    /// @brief Convience alias for a shared-access lock guard,
    /// aka `ScopedLockGuard<T, std::shared_lock>`
    /// @ingroup synchronization
	template<typename T>
	using ReadLockGuard = ScopedLockGuard<T, std::shared_lock>;
} // namespace hyperion
