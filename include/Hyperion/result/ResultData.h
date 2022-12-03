/// @file ResultData.h
/// @author Braxton Salyer <braxtonsalyer@gmail.com>
/// @brief Implementation of `Result`'s storage representation
/// @version 0.1
/// @date 2021-11-04
///
/// MIT License
/// @copyright Copyright (c) 2021 Braxton Salyer <braxtonsalyer@gmail.com>
///
/// Permission is hereby granted, free of charge, to any person obtaining a copy
/// of this software and associated documentation files (the "Software"), to
/// deal in the Software without restriction, including without limitation the
/// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
/// sell copies of the Software, and to permit persons to whom the Software is
/// furnished to do so, subject to the following conditions:
///
/// The above copyright notice and this permission notice shall be included in
/// all copies or substantial portions of the Software.
///
/// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
/// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
/// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
/// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
/// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
/// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
/// IN THE SOFTWARE.
#pragma once

#include <Hyperion/Concepts.h>
#include <Hyperion/option/None.h>
#include <Hyperion/result/Err.h>
#include <Hyperion/result/Ok.h>
#include <functional>

namespace hyperion::result {
	IGNORE_PADDING_START

	/// @brief `ResultData` is the storage implementation backing `Result`
	/// It acts as a tagged union, supports references, and abstracts away
	/// the storage details of `Result`'s implementation
	template<typename T, typename E>
	struct ResultData;

	template<concepts::NotReference T, concepts::NotReference E>
	struct ResultData<T, E> {
		using ok_type = T;
		using ok_reference = std::add_lvalue_reference_t<ok_type>;
		using ok_const_reference = std::add_lvalue_reference_t<std::add_const_t<ok_type>>;
		using ok_rvalue_reference = std::add_rvalue_reference_t<ok_type>;
		using ok_extracted = ok_rvalue_reference;
		using ok_pointer = std::conditional_t<concepts::Pointer<T>, T, std::add_pointer_t<T>>;
		using ok_pointer_to_const = std::add_pointer_t<std::add_const_t<std::remove_pointer_t<T>>>;

		using err_type = E;
		using err_reference = std::add_lvalue_reference_t<err_type>;
		using err_const_reference = std::add_lvalue_reference_t<std::add_const_t<err_type>>;
		using err_rvalue_reference = std::add_rvalue_reference_t<err_type>;
		using err_extracted = err_rvalue_reference;
		using err_pointer = std::conditional_t<concepts::Pointer<E>, E, std::add_pointer_t<E>>;
		using err_pointer_to_const = std::add_pointer_t<std::add_const_t<std::remove_pointer_t<E>>>;

		using ok_storage_type = T;
		using err_storage_type = E;

		union {
			ok_storage_type m_ok;
			err_storage_type m_err;
			option::None m_none;
		};

		enum class Active {
			Ok,
			Err,
			None
		} m_active = Active::None;

		constexpr ResultData() noexcept : m_none() {
		}
		// NOLINTNEXTLINE(readability-identifier-length)
		explicit constexpr ResultData(ok_const_reference ok) noexcept(
			concepts::NoexceptCopyConstructible<ok_storage_type>)
		requires concepts::CopyConstructible<ok_storage_type>
		: m_ok(ok),
		  m_active(Active::Ok) {
		}
		// NOLINTNEXTLINE(readability-identifier-length)
		explicit constexpr ResultData(ok_rvalue_reference ok) noexcept(
			concepts::NoexceptMoveConstructible<ok_storage_type>)
		requires concepts::MoveConstructible<ok_storage_type>
		: m_ok(std::move(ok)),
		  m_active(Active::Ok) {
		}
		template<typename... Args>
		requires concepts::ConstructibleFrom<T, Args...>
		explicit constexpr ResultData(Args&&... args) noexcept(
			concepts::NoexceptConstructibleFrom<T, Args...>)
			: m_ok(std::forward<Args>(args)...), m_active(Active::Ok) {
		}
		explicit constexpr ResultData(err_const_reference err) noexcept(
			concepts::NoexceptCopyConstructible<err_storage_type>)
		requires concepts::CopyConstructible<err_storage_type>
		: m_err(err),
		  m_active(Active::Err) {
		}
		explicit constexpr ResultData(err_rvalue_reference err) noexcept(
			concepts::NoexceptMoveConstructible<err_storage_type>)
		requires concepts::MoveConstructible<err_storage_type>
		: m_err(std::move(err)),
		  m_active(Active::Err) {
		}
		template<typename... Args>
		requires concepts::ConstructibleFrom<E, Args...>
		explicit constexpr ResultData(Args&&... args) noexcept(
			concepts::NoexceptConstructibleFrom<E, Args...>)
			: m_err(std::forward<Args>(args)...), m_active(Active::Err) {
		}
		explicit constexpr ResultData(option::None none) noexcept
		requires concepts::NotSame<T, option::None>
		: m_none(none) {
		}
		constexpr ResultData(const ResultData& data) noexcept(
			concepts::NoexceptCopyConstructible<ok_storage_type>&&
				concepts::NoexceptCopyConstructible<E>)
		requires concepts::CopyConstructible<ok_storage_type>
				 && concepts::CopyConstructible<err_storage_type>
		{
			if(data.m_active == Active::Ok) {
				// NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
				std::construct_at(std::addressof(m_ok), data.m_ok);
				m_active = Active::Ok;
			}
			else if(data.m_active == Active::Err) {
				// NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
				std::construct_at(std::addressof(m_err), data.m_err);
				m_active = Active::Err;
			}
		}
		constexpr ResultData(ResultData&& data) noexcept(
			concepts::NoexceptMoveConstructible<ok_storage_type>&&
				concepts::NoexceptMoveConstructible<E>)
		requires concepts::MoveConstructible<ok_storage_type>
				 && concepts::MoveConstructible<err_storage_type>
		{
			if(data.m_active == Active::Ok) {
				// NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
				std::construct_at(std::addressof(m_ok), std::move(data.m_ok));
				m_active = Active::Ok;
			}
			else if(data.m_active == Active::Err) {
				// NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
				std::construct_at(std::addressof(m_err), std::move(data.m_err));
				m_active = Active::Err;
			}
			data.m_active = Active::None;
		}
		constexpr ~ResultData() noexcept(concepts::NoexceptDestructible<ok_storage_type>&&
											 concepts::NoexceptDestructible<err_storage_type>) {
			if(m_active == Active::Ok) {
				if constexpr(!std::is_trivially_destructible_v<ok_storage_type>) {
					// NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
					std::destroy_at(std::addressof(m_ok));
				}
			}
			else if(m_active == Active::Err) {
				if constexpr(!std::is_trivially_destructible_v<err_storage_type>) {
					// NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
					std::destroy_at(std::addressof(m_err));
				}
			}
		}

		/// @brief Returns whether this is currently the `Ok` variant
		[[nodiscard]] inline constexpr auto has_ok() const noexcept -> bool {
			return m_active == Active::Ok;
		}

		/// @brief Returns whether this is currently the `Err` variant
		[[nodiscard]] inline constexpr auto has_err() const noexcept -> bool {
			return m_active == Active::Err;
		}

		/// @brief Returns whether this is currently disengaged (it's not in the `Ok` nor the `Err`
		/// variant; it has been moved out of)
		[[nodiscard]] inline constexpr auto is_empty() const noexcept -> bool {
			return !has_ok() && !has_err();
		}

		/// @brief Returns a const reference to this as the `Ok` variant
		[[nodiscard]] inline constexpr auto get() const noexcept -> ok_const_reference {
			// NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
			return m_ok;
		}

		/// @brief Returns a reference to this as the `Ok` variant
		[[nodiscard]] inline constexpr auto get() noexcept -> ok_reference {
			// NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
			return m_ok;
		}

		/// @brief Returns a const reference to this as the `Err` variant
		[[nodiscard]] inline constexpr auto get_err() const noexcept -> err_const_reference {
			// NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
			return m_err;
		}

		/// @brief Returns a reference to this as the `Err` variant
		[[nodiscard]] inline constexpr auto get_err() noexcept -> err_reference {
			// NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
			return m_err;
		}

		[[nodiscard]] inline constexpr auto
		extract() noexcept(concepts::NoexceptMovable<T>) -> ok_extracted {
			// NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
			return std::move(m_ok);
		}

		[[nodiscard]] inline constexpr auto
		extract_err() noexcept(concepts::NoexceptMovable<E>) -> err_extracted {
			// NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
			return std::move(m_err);
		}

		constexpr auto operator=(const ResultData& data) noexcept(
			concepts::NoexceptCopyable<T>&& concepts::NoexceptCopyable<E>) -> ResultData&
		requires concepts::Copyable<T> && concepts::Copyable<E>
		{
			if(this == &data) {
				return *this;
			}

			if(data.m_active == Active::Ok) {
				*this = data.m_ok;
			}
			else if(data.m_active == Active::Err) {
				*this = data.m_err;
			}
			else {
				if(m_active == Active::Ok) {
					if constexpr(!std::is_trivially_destructible_v<ok_storage_type>) {
						// NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
						std::destroy_at(std::addressof(m_ok));
					}
				}
				else if(m_active == Active::Err) {
					if constexpr(!std::is_trivially_destructible_v<err_storage_type>) {
						// NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
						std::destroy_at(std::addressof(m_err));
					}
				}
				m_active = Active::None;
			}
			return *this;
		}
		constexpr auto
		operator=(ResultData&& data) noexcept(concepts::NoexceptMovable<T>&& concepts::Movable<E>)
			-> ResultData&
		requires concepts::Movable<T> && concepts::Movable<E>
		{
			if(this == &data) {
				return *this;
			}

			if(data.m_active == Active::Ok) {
				data.m_active = Active::None;
				*this = std::move(data.m_ok);
			}
			else if(data.m_active == Active::Err) {
				data.m_active = Active::None;
				*this = std::move(data.m_err);
			}
			else {
				if(m_active == Active::Ok) {
					if constexpr(!std::is_trivially_destructible_v<ok_storage_type>) {
						// NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
						std::destroy_at(std::addressof(m_ok));
					}
				}
				else if(m_active == Active::Err) {
					if constexpr(!std::is_trivially_destructible_v<err_storage_type>) {
						// NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
						std::destroy_at(std::addressof(m_err));
					}
				}
				m_active = Active::None;
			}
			return *this;
		}

		// NOLINTNEXTLINE(readability-identifier-length)
		constexpr auto operator=(ok_const_reference t) noexcept -> ResultData& {
			if(m_active == Active::Ok) {
				// NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
				m_ok = t;
			}
			else {
				if(m_active == Active::Err) {
					if constexpr(!std::is_trivially_destructible_v<err_storage_type>) {
						// NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
						std::destroy_at(std::addressof(m_err));
					}
				}
				else {
					// NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
					std::destroy_at(std::addressof(m_none));
				}

				// NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
				std::construct_at(std::addressof(m_ok), t);
			}
			m_active = Active::Ok;
			return *this;
		}
		// NOLINTNEXTLINE(readability-identifier-length)
		constexpr auto operator=(ok_rvalue_reference t) noexcept -> ResultData& {
			if(m_active == Active::Ok) {
				// NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
				m_ok = std::move(t);
			}
			else {
				if(m_active == Active::Err) {
					if constexpr(!std::is_trivially_destructible_v<err_storage_type>) {
						// NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
						std::destroy_at(std::addressof(m_err));
					}
				}
				else {
					// NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
					std::destroy_at(std::addressof(m_none));
				}

				// NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
				std::construct_at(std::addressof(m_ok), std::move(t));
			}
			m_active = Active::Ok;
			return *this;
		}

		// NOLINTNEXTLINE(readability-identifier-length)
		constexpr auto operator=(err_const_reference e) noexcept -> ResultData& {
			if(m_active == Active::Err) {
				// NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
				m_err = e;
			}
			else {
				if(m_active == Active::Ok) {
					if constexpr(!std::is_trivially_destructible_v<ok_storage_type>) {
						// NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
						std::destroy_at(std::addressof(m_ok));
					}
				}
				else {
					// NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
					std::destroy_at(std::addressof(m_none));
				}

				// NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
				std::construct_at(std::addressof(m_err), e);
			}
			m_active = Active::Err;
			return *this;
		}
		// NOLINTNEXTLINE(readability-identifier-length)
		constexpr auto operator=(err_rvalue_reference e) noexcept -> ResultData& {
			if(m_active == Active::Err) {
				// NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
				m_err = std::move(e);
			}
			else {
				if(m_active == Active::Ok) {
					if constexpr(!std::is_trivially_destructible_v<ok_storage_type>) {
						// NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
						std::destroy_at(std::addressof(m_ok));
					}
				}
				else {
					// NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
					std::destroy_at(std::addressof(m_none));
				}

				// NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
				std::construct_at(std::addressof(m_err), std::move(e));
			}
			m_active = Active::Err;
			return *this;
		}

		constexpr auto operator=([[maybe_unused]] const option::None& none) noexcept -> ResultData&
		requires concepts::NotSame<T, option::None>
		{
			if(m_active == Active::Ok) {
				if constexpr(!std::is_trivially_destructible_v<ok_storage_type>) {
					// NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
					std::destroy_at(std::addressof(m_ok));
				}
			}
			else if(m_active == Active::Err) {
				if constexpr(!std::is_trivially_destructible_v<err_storage_type>) {
					// NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
					std::destroy_at(std::addressof(m_err));
				}
			}
			m_active = Active::None;
			return *this;
		}
		constexpr auto operator=([[maybe_unused]] option::None&& none) noexcept -> ResultData&
		requires concepts::NotSame<T, option::None>
		{
			if(m_active == Active::Ok) {
				if constexpr(!std::is_trivially_destructible_v<ok_storage_type>) {
					// NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
					std::destroy_at(std::addressof(m_ok));
				}
			}
			else if(m_active == Active::Err) {
				if constexpr(!std::is_trivially_destructible_v<err_storage_type>) {
					// NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
					std::destroy_at(std::addressof(m_err));
				}
			}
			m_active = Active::None;
			return *this;
		}
	};

	template<concepts::Reference T, concepts::NotReference E>
	struct ResultData<T, E> {
		using ok_type = std::remove_reference_t<T>;
		using ok_reference = std::add_lvalue_reference_t<std::remove_const_t<ok_type>>;
		using ok_const_reference = std::add_lvalue_reference_t<std::add_const_t<ok_type>>;
		using ok_rvalue_reference
			= std::conditional_t<std::is_const_v<ok_type>, ok_const_reference, ok_reference>;
		using ok_extracted = ok_rvalue_reference;
		using ok_pointer = std::add_pointer_t<std::remove_const_t<ok_type>>;
		using ok_pointer_to_const = std::add_pointer_t<std::add_const_t<ok_type>>;

		using err_type = E;
		using err_reference = std::add_lvalue_reference_t<err_type>;
		using err_const_reference = std::add_lvalue_reference_t<std::add_const_t<err_type>>;
		using err_rvalue_reference = std::add_rvalue_reference_t<err_type>;
		using err_extracted = err_rvalue_reference;
		using err_pointer = std::conditional_t<concepts::Pointer<E>, E, std::add_pointer_t<E>>;
		using err_pointer_to_const = std::add_pointer_t<std::add_const_t<std::remove_pointer_t<E>>>;

		using ok_storage_type = std::reference_wrapper<ok_type>;
		using err_storage_type = E;

		union {
			ok_storage_type m_ok;
			err_storage_type m_err;
			option::None m_none;
		};

		enum class Active {
			Ok,
			Err,
			None
		} m_active = Active::None;

		constexpr ResultData() noexcept : m_none() {
		}
		// NOLINTNEXTLINE(readability-identifier-length)
		explicit constexpr ResultData(ok_rvalue_reference ok) noexcept
			: m_ok(ref(ok)), m_active(Active::Ok) {
		}
		explicit constexpr ResultData(err_const_reference err) noexcept(
			concepts::NoexceptCopyConstructible<err_storage_type>)
		requires concepts::CopyConstructible<err_storage_type>
		: m_err(err),
		  m_active(Active::Err) {
		}
		explicit constexpr ResultData(err_rvalue_reference err) noexcept(
			concepts::NoexceptMoveConstructible<err_storage_type>)
		requires concepts::MoveConstructible<err_storage_type>
		: m_err(std::move(err)),
		  m_active(Active::Err) {
		}
		template<typename... Args>
		requires concepts::ConstructibleFrom<E, Args...>
		explicit constexpr ResultData(Args&&... args) noexcept(
			concepts::NoexceptConstructibleFrom<E, Args...>)
			: m_err(std::forward<Args>(args)...), m_active(Active::Err) {
		}
		explicit constexpr ResultData(option::None none) noexcept
		requires concepts::NotSame<T, option::None>
		: m_none(none) {
		}
		constexpr ResultData(const ResultData& data) noexcept(
			concepts::NoexceptCopyConstructible<ok_storage_type>&&
				concepts::NoexceptCopyConstructible<err_storage_type>)
		requires concepts::CopyConstructible<ok_storage_type>
				 && concepts::NoexceptCopyConstructible<err_storage_type>
		{
			if(data.m_active == Active::Ok) {
				// NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
				std::construct_at(std::addressof(m_ok), data.m_ok);
				m_active = Active::Ok;
			}
			else if(data.m_active == Active::Err) {
				// NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
				std::construct_at(std::addressof(m_err), data.m_err);
				m_active = Active::Err;
			}
		}
		constexpr ResultData(ResultData&& data) noexcept(
			concepts::NoexceptMoveConstructible<ok_storage_type>&&
				concepts::NoexceptMoveConstructible<err_storage_type>)
		requires concepts::MoveConstructible<ok_storage_type>
				 && concepts::MoveConstructible<err_storage_type>
		{
			if(data.m_active == Active::Ok) {
				// NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
				std::construct_at(std::addressof(m_ok), std::move(data.m_ok));
				m_active = Active::Ok;
			}
			else if(data.m_active == Active::Err) {
				// NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
				std::construct_at(std::addressof(m_err), std::move(data.m_err));
				m_active = Active::Err;
			}
			data.m_active = Active::None;
		}
		constexpr ~ResultData() noexcept(concepts::NoexceptDestructible<ok_storage_type>&&
											 concepts::NoexceptDestructible<err_storage_type>) {
			if(m_active == Active::Ok) {
				if constexpr(!std::is_trivially_destructible_v<ok_storage_type>) {
					// NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
					std::destroy_at(std::addressof(m_ok));
				}
			}
			else if(m_active == Active::Err) {
				if constexpr(!std::is_trivially_destructible_v<err_storage_type>) {
					// NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
					std::destroy_at(std::addressof(m_err));
				}
			}
		}

		/// @brief Returns whether this is currently the `Ok` variant
		[[nodiscard]] inline constexpr auto has_ok() const noexcept -> bool {
			return m_active == Active::Ok;
		}

		/// @brief Returns whether this is currently the `Err` variant
		[[nodiscard]] inline constexpr auto has_err() const noexcept -> bool {
			return m_active == Active::Err;
		}

		/// @brief Returns whether this is currently disengaged (it's not in the `Ok` nor the `Err`
		/// variant; it has been moved out of)
		[[nodiscard]] inline constexpr auto is_empty() const noexcept -> bool {
			return !has_ok() && !has_err();
		}

		/// @brief Returns a const reference to this as the `Ok` variant
		[[nodiscard]] inline constexpr auto get() const noexcept -> ok_const_reference {
			// NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
			return m_ok.get();
		}

		/// @brief Returns a reference to this as the `Ok` variant
		[[nodiscard]] inline constexpr auto get() noexcept -> ok_storage_type& {
			// NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
			return m_ok.get();
		}

		/// @brief Returns a const reference to this as the `Err` variant
		[[nodiscard]] inline constexpr auto get_err() const noexcept -> err_const_reference {
			// NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
			return m_err;
		}

		/// @brief Returns a reference to this as the `Err` variant
		[[nodiscard]] inline constexpr auto get_err() noexcept -> err_reference {
			// NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
			return m_err;
		}

		/// @brief Extracts the `Ok` variant out of this
		[[nodiscard]] inline constexpr auto extract() noexcept -> ok_extracted {
			// NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
			return std::move(m_ok).get();
		}

		/// @brief Extracts the `Err` variant out of this
		[[nodiscard]] inline constexpr auto
		extract_err() noexcept(concepts::NoexceptMovable<E>) -> err_extracted
		requires concepts::Movable<E>
		{
			// NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
			return std::move(m_err);
		}

		constexpr auto
		operator=(const ResultData& data) noexcept(concepts::NoexceptCopyable<ok_storage_type>&&
													   concepts::NoexceptCopyable<err_storage_type>)
			-> ResultData&
		requires concepts::Copyable<ok_storage_type> && concepts::Copyable<err_storage_type>
		{
			if(this == &data) {
				return *this;
			}

			if(data.m_active == Active::Ok) {
				*this = data.m_ok;
			}
			else if(data.m_active == Active::Err) {
				*this = data.m_err;
			}
			else {
				if(m_active == Active::Ok) {
					if constexpr(!std::is_trivially_destructible_v<ok_storage_type>) {
						// NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
						std::destroy_at(std::addressof(m_ok));
					}
				}
				else if(m_active == Active::Err) {
					if constexpr(!std::is_trivially_destructible_v<err_storage_type>) {
						// NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
						std::destroy_at(std::addressof(m_err));
					}
				}
				m_active = Active::None;
			}
			return *this;
		}
		constexpr auto
		operator=(ResultData&& data) noexcept(concepts::NoexceptMovable<ok_storage_type>&&
												  concepts::NoexceptMovable<err_storage_type>)
			-> ResultData&
		requires concepts::Movable<ok_storage_type> && concepts::Movable<err_storage_type>
		{
			if(this == &data) {
				return *this;
			}

			if(data.m_active == Active::Ok) {
				data.m_active = Active::None;
				*this = std::move(data.m_ok);
			}
			else if(data.m_active == Active::Err) {
				data.m_active = Active::None;
				*this = std::move(data.m_err);
			}
			else {
				if(m_active == Active::Ok) {
					if constexpr(!std::is_trivially_destructible_v<ok_storage_type>) {
						// NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
						std::destroy_at(std::addressof(m_ok));
					}
				}
				else if(m_active == Active::Err) {
					if constexpr(!std::is_trivially_destructible_v<err_storage_type>) {
						// NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
						std::destroy_at(std::addressof(m_err));
					}
				}
				m_active = Active::None;
			}
			return *this;
		}

		// NOLINTNEXTLINE(readability-identifier-length)
		constexpr auto operator=(ok_rvalue_reference t) noexcept -> ResultData& {
			if(m_active == Active::Ok) {
				if constexpr(std::is_trivially_copy_assignable_v<T>) {
					// NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
					m_ok = ref(t);
				}
				else {
					// NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
					m_ok = std::move(t);
				}
			}
			else {
				if(m_active == Active::Err) {
					if constexpr(!std::is_trivially_destructible_v<err_storage_type>) {
						// NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
						std::destroy_at(std::addressof(m_err));
					}
				}
				else {
					// NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
					std::destroy_at(std::addressof(m_none));
				}

				if constexpr(std::is_trivially_copy_assignable_v<T>) {
					// NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
					std::construct_at(std::addressof(m_ok), ref(t));
				}
				else {
					// NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
					std::construct_at(std::addressof(m_ok), std::move(t));
				}
			}
			m_active = Active::Ok;
			return *this;
		}

		// NOLINTNEXTLINE(readability-identifier-length)
		constexpr auto operator=(err_const_reference e) noexcept -> ResultData& {
			if(m_active == Active::Err) {
				// NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
				m_err = e;
			}
			else {
				if(m_active == Active::Ok) {
					if constexpr(!std::is_trivially_destructible_v<ok_storage_type>) {
						// NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
						std::destroy_at(std::addressof(m_ok));
					}
				}
				else {
					// NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
					std::destroy_at(std::addressof(m_none));
				}

				// NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
				std::construct_at(std::addressof(m_err), e);
			}
			m_active = Active::Err;
			return *this;
		}
		// NOLINTNEXTLINE(readability-identifier-length)
		constexpr auto operator=(err_rvalue_reference e) noexcept -> ResultData& {
			if(m_active == Active::Err) {
				// NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
				m_err = std::move(e);
			}
			else {
				if(m_active == Active::Ok) {
					if constexpr(!std::is_trivially_destructible_v<ok_storage_type>) {
						// NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
						std::destroy_at(std::addressof(m_ok));
					}
				}
				else {
					// NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
					std::destroy_at(std::addressof(m_none));
				}

				// NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
				std::construct_at(std::addressof(m_err), std::move(e));
			}
			m_active = Active::Err;
			return *this;
		}
		constexpr auto operator=([[maybe_unused]] const option::None& none) noexcept -> ResultData&
		requires concepts::NotSame<T, option::None>
		{
			if(m_active == Active::Ok) {
				if constexpr(!std::is_trivially_destructible_v<ok_storage_type>) {
					// NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
					std::destroy_at(std::addressof(m_ok));
				}
			}
			else if(m_active == Active::Err) {
				if constexpr(!std::is_trivially_destructible_v<err_storage_type>) {
					// NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
					std::destroy_at(std::addressof(m_err));
				}
			}
			m_active = Active::None;
			return *this;
		}
		constexpr auto operator=([[maybe_unused]] option::None&& none) noexcept -> ResultData&
		requires concepts::NotSame<T, option::None>
		{
			if(m_active == Active::Ok) {
				if constexpr(!std::is_trivially_destructible_v<ok_storage_type>) {
					// NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
					std::destroy_at(std::addressof(m_ok));
				}
			}
			else if(m_active == Active::Err) {
				if constexpr(!std::is_trivially_destructible_v<err_storage_type>) {
					// NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
					std::destroy_at(std::addressof(m_err));
				}
			}
			m_active = Active::None;
			return *this;
		}

	  private:
		template<typename U>
			// NOLINTNEXTLINE(readability-identifier-length)
		static inline constexpr auto ref(U&& u) noexcept {
			if constexpr(std::is_const_v<T>) {
				return std::cref(std::forward<U>(u));
			}
			else {
				return std::ref(std::forward<U>(u));
			}
		}
	};

	template<concepts::NotReference T, concepts::Reference E>
	struct ResultData<T, E> {
		using ok_type = T;
		using ok_reference = std::add_lvalue_reference_t<ok_type>;
		using ok_const_reference = std::add_lvalue_reference_t<std::add_const_t<ok_type>>;
		using ok_rvalue_reference = std::add_rvalue_reference_t<ok_type>;
		using ok_extracted = ok_rvalue_reference;
		using ok_pointer = std::conditional_t<concepts::Pointer<T>, T, std::add_pointer_t<T>>;
		using ok_pointer_to_const = std::add_pointer_t<std::add_const_t<std::remove_pointer_t<T>>>;

		using err_type = std::remove_reference_t<E>;
		using err_reference = std::add_lvalue_reference_t<std::remove_const_t<err_type>>;
		using err_const_reference = std::add_lvalue_reference_t<std::add_const_t<err_type>>;
		using err_rvalue_reference
			= std::conditional_t<std::is_const_v<err_type>, err_const_reference, err_reference>;
		using err_extracted = err_rvalue_reference;
		using err_pointer = std::add_pointer_t<std::remove_const_t<err_type>>;
		using err_pointer_to_const = std::add_pointer_t<std::add_const_t<err_type>>;

		using ok_storage_type = T;
		using err_storage_type = std::reference_wrapper<err_type>;

		union {
			ok_storage_type m_ok;
			err_storage_type m_err;
			option::None m_none;
		};

		enum class Active {
			Ok,
			Err,
			None
		} m_active = Active::None;

		constexpr ResultData() noexcept : m_none() {
		}
		// NOLINTNEXTLINE(readability-identifier-length)
		explicit constexpr ResultData(ok_const_reference ok) noexcept(
			concepts::NoexceptCopyConstructible<ok_storage_type>)
		requires concepts::CopyConstructible<ok_storage_type>
		: m_ok(ok),
		  m_active(Active::Ok) {
		}
		// NOLINTNEXTLINE(readability-identifier-length)
		explicit constexpr ResultData(ok_rvalue_reference ok) noexcept(
			concepts::NoexceptMoveConstructible<ok_storage_type>)
		requires concepts::MoveConstructible<ok_storage_type>
		: m_ok(std::move(ok)),
		  m_active(Active::Ok) {
		}
		template<typename... Args>
		requires concepts::ConstructibleFrom<T, Args...>
		explicit constexpr ResultData(Args&&... args) noexcept(
			concepts::NoexceptConstructibleFrom<T, Args...>)
			: m_ok(std::forward<Args>(args)...), m_active(Active::Ok) {
		}
		explicit constexpr ResultData(err_rvalue_reference err) noexcept
			: m_err(ref(err)), m_active(Active::Err) {
		}
		explicit constexpr ResultData(option::None none) noexcept
		requires concepts::NotSame<T, option::None>
		: m_none(none) {
		}
		constexpr ResultData(const ResultData& data) noexcept(
			concepts::NoexceptCopyConstructible<ok_storage_type>&&
				concepts::NoexceptCopyConstructible<err_storage_type>)
		requires concepts::CopyConstructible<ok_storage_type>
				 && concepts::CopyConstructible<err_storage_type>
		{
			if(data.m_active == Active::Ok) {
				// NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
				std::construct_at(std::addressof(m_ok), data.m_ok);
				m_active = Active::Ok;
			}
			else if(data.m_active == Active::Err) {
				// NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
				std::construct_at(std::addressof(m_err), data.m_err);
				m_active = Active::Err;
			}
		}
		constexpr ResultData(ResultData&& data) noexcept(
			concepts::NoexceptMoveConstructible<ok_storage_type>&&
				concepts::NoexceptMoveConstructible<err_storage_type>)
		requires concepts::MoveConstructible<ok_storage_type>
				 && concepts::MoveConstructible<err_storage_type>
		{
			if(data.m_active == Active::Ok) {
				// NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
				std::construct_at(std::addressof(m_ok), std::move(data.m_ok));
				m_active = Active::Ok;
			}
			else if(data.m_active == Active::Err) {
				// NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
				std::construct_at(std::addressof(m_err), std::move(data.m_err));
				m_active = Active::Err;
			}
			data.m_active = Active::None;
		}
		constexpr ~ResultData() noexcept(concepts::NoexceptDestructible<ok_storage_type>&&
											 concepts::NoexceptDestructible<err_storage_type>) {
			if(m_active == Active::Ok) {
				if constexpr(!std::is_trivially_destructible_v<ok_storage_type>) {
					// NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
					std::destroy_at(std::addressof(m_ok));
				}
			}
			else if(m_active == Active::Err) {
				if constexpr(!std::is_trivially_destructible_v<err_storage_type>) {
					// NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
					std::destroy_at(std::addressof(m_err));
				}
			}
		}

		/// @brief Returns whether this is currently the `Ok` variant
		[[nodiscard]] inline constexpr auto has_ok() const noexcept -> bool {
			return m_active == Active::Ok;
		}

		/// @brief Returns whether this is currently the `Err` variant
		[[nodiscard]] inline constexpr auto has_err() const noexcept -> bool {
			return m_active == Active::Err;
		}

		/// @brief Returns whether this is currently disengaged (it's not in the `Ok` nor the `Err`
		/// variant; it has been moved out of)
		[[nodiscard]] inline constexpr auto is_empty() const noexcept -> bool {
			return !has_ok() && !has_err();
		}

		/// @brief Returns a const reference to this as the `Ok` variant
		[[nodiscard]] inline constexpr auto get() const noexcept -> ok_const_reference {
			// NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
			return m_ok;
		}

		/// @brief Returns a reference to this as the `Ok` variant
		[[nodiscard]] inline constexpr auto get() noexcept -> ok_reference {
			// NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
			return m_ok;
		}

		/// @brief Returns a const reference to this as the `Err` variant
		[[nodiscard]] inline constexpr auto get_err() const noexcept -> err_const_reference {
			// NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
			return m_err.get();
		}

		/// @brief Returns a reference to this as the `Err` variant
		[[nodiscard]] inline constexpr auto get_err() noexcept -> err_storage_type& {
			// NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
			return m_err.get();
		}

		/// @brief Extracts the `Ok` variant out of this
		[[nodiscard]] inline constexpr auto extract() noexcept -> ok_extracted {
			// NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
			return std::move(m_ok);
		}

		/// @brief Extracts the `Err` variant out of this
		[[nodiscard]] inline constexpr auto extract_err() noexcept -> err_extracted {
			// NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
			return std::move(m_err).get();
		}

		constexpr auto
		operator=(const ResultData& data) noexcept(concepts::NoexceptCopyable<ok_storage_type>&&
													   concepts::NoexceptCopyable<err_storage_type>)
			-> ResultData&
		requires concepts::Copyable<ok_storage_type> && concepts::Copyable<err_storage_type>
		{
			if(this == &data) {
				return *this;
			}

			if(data.m_active == Active::Ok) {
				*this = data.m_ok;
			}
			else if(data.m_active == Active::Err) {
				*this = data.m_err;
			}
			else {
				if(m_active == Active::Ok) {
					if constexpr(!std::is_trivially_destructible_v<ok_storage_type>) {
						// NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
						std::destroy_at(std::addressof(m_ok));
					}
				}
				else if(m_active == Active::Err) {
					if constexpr(!std::is_trivially_destructible_v<err_storage_type>) {
						// NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
						std::destroy_at(std::addressof(m_err));
					}
				}
				m_active = Active::None;
			}
			return *this;
		}
		constexpr auto
		operator=(ResultData&& data) noexcept(concepts::NoexceptMovable<ok_storage_type>&&
												  concepts::NoexceptMovable<err_storage_type>)
			-> ResultData&
		requires concepts::Movable<ok_storage_type> && concepts::Movable<err_storage_type>
		{
			if(this == &data) {
				return *this;
			}

			if(data.m_active == Active::Ok) {
				data.m_active = Active::None;
				*this = std::move(data.m_ok);
			}
			else if(data.m_active == Active::Err) {
				data.m_active = Active::None;
				*this = std::move(data.m_err);
			}
			else {
				if(m_active == Active::Ok) {
					if constexpr(!std::is_trivially_destructible_v<ok_storage_type>) {
						// NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
						std::destroy_at(std::addressof(m_ok));
					}
				}
				else if(m_active == Active::Err) {
					if constexpr(!std::is_trivially_destructible_v<err_storage_type>) {
						// NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
						std::destroy_at(std::addressof(m_err));
					}
				}
				m_active = Active::None;
			}
			return *this;
		}

		// NOLINTNEXTLINE(readability-identifier-length)
		constexpr auto operator=(ok_const_reference t) noexcept -> ResultData& {
			if(m_active == Active::Ok) {
				// NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
				m_ok = t;
			}
			else {
				if(m_active == Active::Err) {
					if constexpr(!std::is_trivially_destructible_v<err_storage_type>) {
						// NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
						std::destroy_at(std::addressof(m_err));
					}
				}
				else {
					// NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
					std::destroy_at(std::addressof(m_none));
				}

				// NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
				std::construct_at(std::addressof(m_ok), t);
			}
			m_active = Active::Ok;
			return *this;
		}
		// NOLINTNEXTLINE(readability-identifier-length)
		constexpr auto operator=(ok_rvalue_reference t) noexcept -> ResultData& {
			if(m_active == Active::Ok) {
				// NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
				m_ok = std::move(t);
			}
			else {
				if(m_active == Active::Err) {
					if constexpr(!std::is_trivially_destructible_v<err_storage_type>) {
						// NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
						std::destroy_at(std::addressof(m_err));
					}
				}
				else {
					// NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
					std::destroy_at(std::addressof(m_none));
				}

				// NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
				std::construct_at(std::addressof(m_ok), std::move(t));
			}
			m_active = Active::Ok;
			return *this;
		}

		// NOLINTNEXTLINE(readability-identifier-length)
		constexpr auto operator=(err_rvalue_reference e) noexcept -> ResultData& {
			if(m_active == Active::Err) {
				if constexpr(std::is_trivially_copy_assignable_v<E>) {
					// NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
					m_err = ref(e);
				}
				else {
					// NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
					m_err = std::move(e);
				}
			}
			else {
				if(m_active == Active::Ok) {
					if constexpr(!std::is_trivially_destructible_v<ok_storage_type>) {
						// NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
						std::destroy_at(std::addressof(m_ok));
					}
				}
				else {
					// NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
					std::destroy_at(std::addressof(m_none));
				}

				if constexpr(std::is_trivially_copy_assignable_v<E>) {
					// NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
					std::construct_at(std::addressof(m_err), ref(e));
				}
				else {
					// NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
					std::construct_at(std::addressof(m_err), std::move(e));
				}
			}
			m_active = Active::Err;
			return *this;
		}

		constexpr auto operator=([[maybe_unused]] const option::None& none) noexcept -> ResultData&
		requires concepts::NotSame<T, option::None>
		{
			if(m_active == Active::Ok) {
				if constexpr(!std::is_trivially_destructible_v<ok_storage_type>) {
					// NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
					std::destroy_at(std::addressof(m_ok));
				}
			}
			else if(m_active == Active::Err) {
				if constexpr(!std::is_trivially_destructible_v<err_storage_type>) {
					// NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
					std::destroy_at(std::addressof(m_err));
				}
			}
			m_active = Active::None;
			return *this;
		}
		constexpr auto operator=([[maybe_unused]] option::None&& none) noexcept -> ResultData&
		requires concepts::NotSame<T, option::None>
		{
			if(m_active == Active::Ok) {
				if constexpr(!std::is_trivially_destructible_v<ok_storage_type>) {
					// NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
					std::destroy_at(std::addressof(m_ok));
				}
			}
			else if(m_active == Active::Err) {
				if constexpr(!std::is_trivially_destructible_v<err_storage_type>) {
					// NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
					std::destroy_at(std::addressof(m_err));
				}
			}
			m_active = Active::None;
			return *this;
		}

	  private:
		template<typename U>
			// NOLINTNEXTLINE(readability-identifier-length)
		static inline constexpr auto ref(U&& u) noexcept {
			if constexpr(std::is_const_v<T>) {
				return std::cref(std::forward<U>(u));
			}
			else {
				return std::ref(std::forward<U>(u));
			}
		}
	};

	template<concepts::Reference T, concepts::Reference E>
	struct ResultData<T, E> {
		using ok_type = std::remove_reference_t<T>;
		using ok_reference = std::add_lvalue_reference_t<std::remove_const_t<ok_type>>;
		using ok_const_reference = std::add_lvalue_reference_t<std::add_const_t<ok_type>>;
		using ok_rvalue_reference
			= std::conditional_t<std::is_const_v<ok_type>, ok_const_reference, ok_reference>;
		using ok_extracted = ok_rvalue_reference;
		using ok_pointer = std::add_pointer_t<std::remove_const_t<ok_type>>;
		using ok_pointer_to_const = std::add_pointer_t<std::add_const_t<ok_type>>;

		using err_type = std::remove_reference_t<E>;
		using err_reference = std::add_lvalue_reference_t<std::remove_const_t<err_type>>;
		using err_const_reference = std::add_lvalue_reference_t<std::add_const_t<err_type>>;
		using err_rvalue_reference
			= std::conditional_t<std::is_const_v<err_type>, err_const_reference, err_reference>;
		using err_extracted = err_rvalue_reference;
		using err_pointer = std::add_pointer_t<std::remove_const_t<err_type>>;
		using err_pointer_to_const = std::add_pointer_t<std::add_const_t<err_type>>;

		using ok_storage_type = std::reference_wrapper<ok_type>;
		using err_storage_type = std::reference_wrapper<err_type>;

		union {
			ok_storage_type m_ok;
			err_storage_type m_err;
			option::None m_none;
		};

		enum class Active {
			Ok,
			Err,
			None
		} m_active = Active::None;

		constexpr ResultData() noexcept : m_none() {
		}
		// NOLINTNEXTLINE(readability-identifier-length)
		explicit constexpr ResultData(ok_rvalue_reference ok) noexcept
			: m_ok(ref(ok)), m_active(Active::Ok) {
		}
		explicit constexpr ResultData(err_rvalue_reference err) noexcept
			: m_err(ref(err)), m_active(Active::Err) {
		}
		explicit constexpr ResultData(option::None none) noexcept
		requires concepts::NotSame<T, option::None>
		: m_none(none) {
		}
		constexpr ResultData(const ResultData& data) noexcept(
			concepts::NoexceptCopyConstructible<ok_storage_type>&&
				concepts::NoexceptCopyConstructible<err_storage_type>)
		requires concepts::CopyConstructible<ok_storage_type>
				 && concepts::CopyConstructible<err_storage_type>
		{
			if(data.m_active == Active::Ok) {
				// NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
				std::construct_at(std::addressof(m_ok), data.m_ok);
				m_active = Active::Ok;
			}
			else if(data.m_active == Active::Err) {
				// NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
				std::construct_at(std::addressof(m_err), data.m_err);
				m_active = Active::Err;
			}
		}
		constexpr ResultData(ResultData&& data) noexcept(
			concepts::NoexceptMoveConstructible<ok_storage_type>&&
				concepts::NoexceptMoveConstructible<err_storage_type>)
		requires concepts::MoveConstructible<ok_storage_type>
				 && concepts::MoveConstructible<err_storage_type>
		{
			if(data.m_active == Active::Ok) {
				// NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
				std::construct_at(std::addressof(m_ok), std::move(data.m_ok));
				m_active = Active::Ok;
			}
			else if(data.m_active == Active::Err) {
				// NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
				std::construct_at(std::addressof(m_err), std::move(data.m_err));
				m_active = Active::Err;
			}
			data.m_active = Active::None;
		}
		constexpr ~ResultData() noexcept(concepts::NoexceptDestructible<ok_storage_type>&&
											 concepts::NoexceptDestructible<err_storage_type>) {
			if(m_active == Active::Ok) {
				if constexpr(!std::is_trivially_destructible_v<ok_storage_type>) {
					// NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
					std::destroy_at(std::addressof(m_ok));
				}
			}
			else if(m_active == Active::Err) {
				if constexpr(!std::is_trivially_destructible_v<err_storage_type>) {
					// NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
					std::destroy_at(std::addressof(m_err));
				}
			}
		}

		/// @brief Returns whether this is currently the `Ok` variant
		[[nodiscard]] inline constexpr auto has_ok() const noexcept -> bool {
			return m_active == Active::Ok;
		}

		/// @brief Returns whether this is currently the `Err` variant
		[[nodiscard]] inline constexpr auto has_err() const noexcept -> bool {
			return m_active == Active::Err;
		}

		/// @brief Returns whether this is currently disengaged (it's not in the `Ok` nor the `Err`
		/// variant; it has been moved out of)
		[[nodiscard]] inline constexpr auto is_empty() const noexcept -> bool {
			return !has_ok() && !has_err();
		}

		/// @brief Returns a const reference to this as the `Ok` variant
		[[nodiscard]] inline constexpr auto get() const noexcept -> ok_const_reference {
			// NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
			return m_ok.get();
		}

		/// @brief Returns a reference to this as the `Ok` variant
		[[nodiscard]] inline constexpr auto get() noexcept -> ok_storage_type& {
			// NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
			return m_ok.get();
		}

		/// @brief Returns a const reference to this as the `Err` variant
		[[nodiscard]] inline constexpr auto get_err() const noexcept -> err_const_reference {
			// NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
			return m_err.get();
		}

		/// @brief Returns a reference to this as the `Err` variant
		[[nodiscard]] inline constexpr auto get_err() noexcept -> err_storage_type& {
			// NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
			return m_err.get();
		}

		/// @brief Extracts the `Ok` variant out of this
		[[nodiscard]] inline constexpr auto extract() noexcept -> ok_extracted {
			// NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
			return std::move(m_ok).get();
		}

		/// @brief Extracts the `Err` variant out of this
		[[nodiscard]] inline constexpr auto extract_err() noexcept -> err_extracted {
			// NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
			return std::move(m_err).get();
		}

		constexpr auto
		operator=(const ResultData& data) noexcept(concepts::NoexceptCopyable<ok_storage_type>&&
													   concepts::NoexceptCopyable<err_storage_type>)
			-> ResultData&
		requires concepts::Copyable<ok_storage_type> && concepts::Copyable<err_storage_type>
		{
			if(this == &data) {
				return *this;
			}

			if(data.m_active == Active::Ok) {
				*this = data.m_ok;
			}
			else if(data.m_active == Active::Err) {
				*this = data.m_err;
			}
			else {
				if(m_active == Active::Ok) {
					if constexpr(!std::is_trivially_destructible_v<ok_storage_type>) {
						// NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
						std::destroy_at(std::addressof(m_ok));
					}
				}
				else if(m_active == Active::Err) {
					if constexpr(!std::is_trivially_destructible_v<err_storage_type>) {
						// NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
						std::destroy_at(std::addressof(m_err));
					}
				}
				m_active = Active::None;
			}
			return *this;
		}
		constexpr auto
		operator=(ResultData&& data) noexcept(concepts::NoexceptMovable<ok_storage_type>&&
												  concepts::NoexceptMovable<err_storage_type>)
			-> ResultData&
		requires concepts::Movable<ok_storage_type> && concepts::Movable<err_storage_type>
		{
			if(this == &data) {
				return *this;
			}

			if(data.m_active == Active::Ok) {
				data.m_active = Active::None;
				*this = std::move(data.m_ok);
			}
			else if(data.m_active == Active::Err) {
				data.m_active = Active::None;
				*this = std::move(data.m_err);
			}
			else {
				if(m_active == Active::Ok) {
					if constexpr(!std::is_trivially_destructible_v<ok_storage_type>) {
						// NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
						std::destroy_at(std::addressof(m_ok));
					}
				}
				else if(m_active == Active::Err) {
					if constexpr(!std::is_trivially_destructible_v<err_storage_type>) {
						// NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
						std::destroy_at(std::addressof(m_err));
					}
				}
				m_active = Active::None;
			}
			return *this;
		}

		// NOLINTNEXTLINE(readability-identifier-length)
		constexpr auto operator=(ok_rvalue_reference t) noexcept -> ResultData& {
			if(m_active == Active::Ok) {
				if constexpr(std::is_trivially_copy_assignable_v<T>) {
					// NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
					m_ok = ref(t);
				}
				else {
					// NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
					m_ok = std::move(t);
				}
			}
			else {
				if(m_active == Active::Err) {
					if constexpr(!std::is_trivially_destructible_v<err_storage_type>) {
						// NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
						std::destroy_at(std::addressof(m_err));
					}
				}
				else {
					// NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
					std::destroy_at(std::addressof(m_none));
				}

				if constexpr(std::is_trivially_copy_assignable_v<T>) {
					// NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
					std::construct_at(std::addressof(m_ok), ref(t));
				}
				else {
					// NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
					std::construct_at(std::addressof(m_ok), std::move(t));
				}
			}
			m_active = Active::Ok;
			return *this;
		}

		// NOLINTNEXTLINE(readability-identifier-length)
		constexpr auto operator=(err_rvalue_reference e) noexcept -> ResultData& {
			if(m_active == Active::Err) {
				if constexpr(std::is_trivially_copy_assignable_v<E>) {
					// NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
					m_err = ref(e);
				}
				else {
					// NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
					m_err = std::move(e);
				}
			}
			else {
				if(m_active == Active::Ok) {
					if constexpr(!std::is_trivially_destructible_v<ok_storage_type>) {
						// NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
						std::destroy_at(std::addressof(m_ok));
					}
				}
				else {
					// NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
					std::destroy_at(std::addressof(m_none));
				}

				if constexpr(std::is_trivially_copy_assignable_v<E>) {
					// NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
					std::construct_at(std::addressof(m_err), ref(e));
				}
				else {
					// NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
					std::construct_at(std::addressof(m_err), std::move(e));
				}
			}
			m_active = Active::Err;
			return *this;
		}

		constexpr auto operator=([[maybe_unused]] const option::None& none) noexcept -> ResultData&
		requires concepts::NotSame<T, option::None>
		{
			if(m_active == Active::Ok) {
				if constexpr(!std::is_trivially_destructible_v<ok_storage_type>) {
					// NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
					std::destroy_at(std::addressof(m_ok));
				}
			}
			else if(m_active == Active::Err) {
				if constexpr(!std::is_trivially_destructible_v<err_storage_type>) {
					// NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
					std::destroy_at(std::addressof(m_err));
				}
			}
			m_active = Active::None;
			return *this;
		}
		constexpr auto operator=([[maybe_unused]] option::None&& none) noexcept -> ResultData&
		requires concepts::NotSame<T, option::None>
		{
			if(m_active == Active::Ok) {
				if constexpr(!std::is_trivially_destructible_v<ok_storage_type>) {
					// NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
					std::destroy_at(std::addressof(m_ok));
				}
			}
			else if(m_active == Active::Err) {
				if constexpr(!std::is_trivially_destructible_v<err_storage_type>) {
					// NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
					std::destroy_at(std::addressof(m_err));
				}
			}
			m_active = Active::None;
			return *this;
		}

	  private:
		template<typename U>
			// NOLINTNEXTLINE(readability-identifier-length)
		static inline constexpr auto ref(U&& u) noexcept {
			if constexpr(std::is_const_v<T>) {
				return std::cref(std::forward<U>(u));
			}
			else {
				return std::ref(std::forward<U>(u));
			}
		}
	};

	template<typename T, typename V = void>
	struct is_result : public std::false_type { };

	template<typename T>
	struct is_result<T, std::void_t<typename T::ok_rvalue_reference>> : public std::true_type { };

	template<typename T>
	static inline constexpr bool is_result_v = is_result<T>::value;

	template<typename T>
	concept IsResult = is_result_v<T>;

	template<typename T>
	concept NotResult = !
	IsResult<T>;
	IGNORE_PADDING_STOP
} // namespace hyperion::result
