/// @file OptionData.h
/// @author Braxton Salyer <braxtonsalyer@gmail.com>
/// @brief Implementation of `Option`'s storage representation
/// @version 0.1
/// @date 2021-11-03
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
#include <Hyperion/HyperionDef.h>
#include <Hyperion/option/None.h>
#include <functional>

namespace hyperion::option {
	IGNORE_PADDING_START

	/// @brief `OptionData` is the storage implementation backing `Option`
	/// It acts as a tagged union, supports references (unlike `std::optional`), and abstracts away
	/// the storage details of `Option`'s implementation
	template<typename T>
	struct OptionData;

	template<concepts::NotReference T>
	struct OptionData<T> {
		using type = T;
		using reference = std::add_lvalue_reference_t<type>;
		using const_reference = std::add_lvalue_reference_t<std::add_const_t<type>>;
		using rvalue_reference = std::add_rvalue_reference_t<type>;
		using extracted = rvalue_reference;
		using pointer = std::conditional_t<concepts::Pointer<T>, T, std::add_pointer_t<T>>;
		using pointer_to_const = std::add_pointer_t<std::add_const_t<std::remove_pointer_t<type>>>;

		using storage_type = T;

		union {
			storage_type m_some;
			None m_none;
		};

		bool m_is_some = false;

		constexpr OptionData() noexcept : m_none() {
		}
		// NOLINTNEXTLINE(readability-identifier-length)
		explicit constexpr OptionData(const_reference t) noexcept(
			concepts::NoexceptCopyConstructible<storage_type>) requires
			concepts::CopyConstructible<storage_type> : m_some(t),
														m_is_some(true) {
		}
		// NOLINTNEXTLINE(readability-identifier-length)
		explicit constexpr OptionData(rvalue_reference t) noexcept(
			concepts::NoexceptMoveConstructible<storage_type>) requires
			concepts::MoveConstructible<storage_type> : m_some(std::move(t)),
														m_is_some(true) {
		}
		/// @brief Constructs an `OptionData` by constructing the `T` in place in it
		/// @tparam Args - The types of the arguments to pass to `T`'s constructor
		/// @param args  - The arguments to pass to `T`'s constructor
		template<typename... Args>
		requires concepts::ConstructibleFrom<T, Args...>
		explicit constexpr OptionData(Args&&... args) noexcept(
			concepts::NoexceptConstructibleFrom<T, Args...>)
			: m_some(std::forward<Args>(args)...), m_is_some(true) {
		}
		explicit constexpr OptionData(const None& n) noexcept : m_none(n) {
		}
		explicit constexpr OptionData(None&& n) noexcept : m_none(n) {
		}
		constexpr OptionData(const OptionData& data) noexcept(
			concepts::NoexceptCopyConstructible<storage_type>) requires
			concepts::CopyConstructible<storage_type> {
			if(data.m_is_some) {
				// NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
				std::construct_at(std::addressof(m_some), data.m_some);
				m_is_some = true;
			}
		}
		constexpr OptionData(OptionData&& data) noexcept(
			concepts::NoexceptMoveConstructible<storage_type>) requires
			concepts::MoveConstructible<storage_type> {
			if(data.m_is_some) {
				// NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
				std::construct_at(std::addressof(m_some), std::move(data.m_some));
				m_is_some = true;
				data.m_is_some = false;
			}
		}
		template<typename U>
		requires concepts::Same<storage_type, std::remove_const_t<std::remove_reference_t<U>>>
		constexpr OptionData(const OptionData<U>& data) // NOLINT
			noexcept(concepts::NoexceptCopyConstructible<storage_type>) requires
			concepts::CopyConstructible<storage_type> && concepts::Reference<U> {
			if(data.m_is_some) {
				// NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
				std::construct_at(std::addressof(m_some), data.get());
				m_is_some = true;
			}
		}
		constexpr ~OptionData() noexcept(concepts::NoexceptDestructible<T>) {
			if constexpr(!std::is_trivially_destructible_v<T>) {
				if(m_is_some) {
					// NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
					std::destroy_at(std::addressof(m_some));
				}
			}
		}

		/// @brief Returns whether this currently contains an active `T`
		[[nodiscard]] inline constexpr auto has_value() const noexcept -> bool {
			return m_is_some;
		}

		/// @brief Returns a const reference to the contained data
		[[nodiscard]] inline constexpr auto get() const noexcept -> const_reference {
			// NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
			return m_some;
		}

		/// @brief Returns a reference to the contained data
		[[nodiscard]] inline constexpr auto get() noexcept -> reference {
			// NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
			return m_some;
		}

		/// @brief Extracts the contained data out of this
		[[nodiscard]] inline constexpr auto
		extract() noexcept(concepts::NoexceptMovable<storage_type>)
			-> type requires concepts::Movable<storage_type> {
			// NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
			return std::move(m_some);
		}

		constexpr auto
		operator=(const OptionData& data) noexcept(concepts::NoexceptCopyable<storage_type>)
			-> OptionData& requires concepts::Copyable<storage_type> {
			if(this == &data) {
				return *this;
			}

			if(data.m_is_some) {
				*this = data.m_some;
			}
			else if(m_is_some) {
				if constexpr(!std::is_trivially_destructible_v<storage_type>) {
					// NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
					std::destroy_at(std::addressof(m_some));
				}
				m_is_some = false;
			}
			return *this;
		}
		// clang-format off


		template<typename U>
		requires concepts::Same<storage_type, std::remove_reference_t<U>>
		constexpr auto operator=(const OptionData& data)
			noexcept(concepts::NoexceptCopyable<storage_type>)
			-> OptionData&
			requires concepts::Copyable<storage_type> && concepts::Reference<U>
		{
			// clang-format on

			if(data.m_is_some) {
				*this = data.get();
			}
			else if(m_is_some) {
				if constexpr(!std::is_trivially_destructible_v<storage_type>) {
					// NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
					std::destroy_at(std::addressof(m_some));
				}
				m_is_some = false;
			}
			return *this;
		}
		constexpr auto
		operator=(OptionData&& data) noexcept(concepts::NoexceptMovable<storage_type>)
			-> OptionData& requires concepts::Movable<storage_type> {
			if(this == &data) {
				return *this;
			}

			if(data.m_is_some) {
				data.m_is_some = false;
				*this = std::move(data.m_some);
			}
			else if(m_is_some) {
				if constexpr(!std::is_trivially_destructible_v<storage_type>) {
					// NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
					std::destroy_at(std::addressof(m_some));
				}
				m_is_some = false;
			}
			return *this;
		}
		constexpr auto
		// NOLINTNEXTLINE(readability-identifier-length)
		operator=(const_reference t) noexcept(concepts::NoexceptCopyable<storage_type>)
			-> OptionData& requires concepts::Copyable<storage_type> {
			if(m_is_some) {
				// NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
				m_some = t;
			}
			else {
				// NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
				std::construct_at(std::addressof(m_some), t);
				m_is_some = true;
			}
			return *this;
		}
		constexpr auto
		// NOLINTNEXTLINE(readability-identifier-length)
		operator=(rvalue_reference t) noexcept(concepts::NoexceptMovable<storage_type>)
			-> OptionData& requires concepts::Movable<storage_type> {
			if constexpr(std::is_trivially_copy_assignable_v<T>) {
				if(m_is_some) {
					// NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
					m_some = t;
				}
				else {
					// NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
					std::construct_at(std::addressof(m_some), t);
					m_is_some = true;
				}
			}
			else {
				if(m_is_some) {
					// NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
					m_some = std::move(t);
				}
				else {
					// NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
					std::construct_at(std::addressof(m_some), std::move(t));
					m_is_some = true;
				}
			}
			return *this;
		}
		constexpr auto operator=([[maybe_unused]] const None& n) noexcept -> OptionData& {
			if(m_is_some) {
				if constexpr(!std::is_trivially_destructible_v<storage_type>) {
					// NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
					std::destroy_at(std::addressof(m_some));
				}
				m_is_some = false;
			}
			return *this;
		}
		constexpr auto operator=([[maybe_unused]] None&& n) noexcept -> OptionData& {
			if(m_is_some) {
				if constexpr(!std::is_trivially_destructible_v<storage_type>) {
					// NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
					std::destroy_at(std::addressof(m_some));
				}
				m_is_some = false;
			}
			return *this;
		}
	};

	template<concepts::Reference T>
	struct OptionData<T> {
		using type = std::remove_reference_t<T>;
		using reference = std::add_lvalue_reference_t<std::remove_const_t<type>>;
		using const_reference = std::add_lvalue_reference_t<std::add_const_t<type>>;
		using rvalue_reference
			= std::conditional_t<std::is_const_v<type>, const_reference, reference>;
		using extracted = rvalue_reference;
		using pointer = std::add_pointer_t<std::remove_const_t<type>>;
		using pointer_to_const = std::add_pointer_t<std::add_const_t<type>>;

		using storage_type = std::reference_wrapper<type>;

		union {
			storage_type m_some;
			None m_none;
		};

		bool m_is_some = false;

		constexpr OptionData() noexcept : m_none() {
		}
		// clang-format off
		// NOLINTNEXTLINE(readability-identifier-length)
		explicit constexpr OptionData(const_reference t) noexcept
			requires std::is_const_v<std::remove_reference_t<T>>
			: m_some(ref(t)),
			  m_is_some(true) {
		}
		// clang-format on

		// NOLINTNEXTLINE(readability-identifier-length)
		explicit constexpr OptionData(reference t) noexcept : m_some(ref(t)), m_is_some(true) {
		}
		// NOLINTNEXTLINE(readability-identifier-length)
		explicit constexpr OptionData(None n) noexcept : m_none(n) {
		}
		constexpr OptionData(const OptionData& data) noexcept {
			if(data.m_is_some) {
				// NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
				std::construct_at(std::addressof(m_some), data.m_some);
				m_is_some = true;
			}
		}
		constexpr OptionData(OptionData&& data) noexcept {
			if(data.m_is_some) {
				// NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
				std::construct_at(std::addressof(m_some), std::move(data.m_some));
				m_is_some = true;
				data.m_is_some = false;
			}
		}
		constexpr ~OptionData() noexcept {
			if constexpr(!std::is_trivially_destructible_v<storage_type>) {
				if(m_is_some) {
					// NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
					std::destroy_at(std::addressof(m_some));
				}
			}
		}

		[[nodiscard]] inline constexpr auto has_value() const noexcept -> bool {
			return m_is_some;
		}

		/// @brief Returns a const reference to the contained data
		[[nodiscard]] inline constexpr auto get() const noexcept -> const_reference {
			// NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
			return m_some.get();
		}

		/// @brief Returns a reference to the contained data
		[[nodiscard]] inline constexpr auto get() noexcept -> reference {
			// NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
			return m_some.get();
		}

		/// @brief Extracts the contained data out of this
		[[nodiscard]] inline constexpr auto extract() noexcept -> extracted {
			// NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
			return std::move(m_some).get();
		}

		constexpr auto operator=(const OptionData& data) noexcept -> OptionData& {
			if(this == &data) {
				return *this;
			}

			if(data.m_is_some) {
				*this = data.m_some;
			}
			else {
				if(m_is_some) {
					if constexpr(!std::is_trivially_destructible_v<storage_type>) {
						// NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
						std::destroy_at(std::addressof(m_some));
					}
					m_is_some = false;
				}
			}
			return *this;
		}
		constexpr auto operator=(OptionData&& data) noexcept -> OptionData& {
			if(this == &data) {
				return *this;
			}

			if(data.m_is_some) {
				data.m_is_some = false;
				*this = std::move(data.m_some);
				m_is_some = true;
			}
			else {
				if(m_is_some) {
					m_is_some = false;
					if constexpr(!std::is_trivially_destructible_v<storage_type>) {
						// NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
						std::destroy_at(std::addressof(m_some));
					}
				}
			}
			return *this;
		}

		constexpr auto
		// NOLINTNEXTLINE(readability-identifier-length)
		operator=(const_reference t) noexcept(concepts::NoexceptCopyable<storage_type>)
			-> OptionData& requires concepts::Copyable<storage_type> {
			if(m_is_some) {
				// NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
				m_some = ref(t);
			}
			else {
				// NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
				std::construct_at(std::addressof(m_some), ref(t));
				m_is_some = true;
			}
			return *this;
		}
		constexpr auto
		// NOLINTNEXTLINE(readability-identifier-length)
		operator=(rvalue_reference t) noexcept(concepts::NoexceptMovable<storage_type>)
			-> OptionData& requires concepts::Movable<storage_type> {
			if constexpr(std::is_trivially_copy_assignable_v<T>) {
				if(m_is_some) {
					// NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
					m_some = ref(t);
				}
				else {
					// NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
					std::construct_at(std::addressof(m_some), ref(t));
					m_is_some = true;
				}
			}
			else {
				if(m_is_some) {
					// NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
					m_some = std::move(t);
				}
				else {
					// NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
					std::construct_at(std::addressof(m_some), std::move(t));
					m_is_some = true;
				}
			}
			return *this;
		}
		constexpr auto operator=([[maybe_unused]] const None& n) noexcept -> OptionData& {
			if(m_is_some) {
				if constexpr(!std::is_trivially_destructible_v<storage_type>) {
					// NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
					std::destroy_at(std::addressof(m_some));
				}
				m_is_some = false;
			}
			return *this;
		}
		constexpr auto operator=([[maybe_unused]] None&& n) noexcept -> OptionData& {
			if(m_is_some) {
				if constexpr(!std::is_trivially_destructible_v<storage_type>) {
					// NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
					std::destroy_at(std::addressof(m_some));
				}
				m_is_some = false;
			}
			return *this;
		}

	  private:
		template<typename U>
		// NOLINTNEXTLINE(readability-identifier-length)
		[[nodiscard]] static inline constexpr auto ref(U&& u) noexcept {
			if constexpr(std::is_const_v<T>) {
				return std::cref(std::forward<U>(u));
			}
			else {
				return std::ref(std::forward<U>(u));
			}
		}
	};
	IGNORE_PADDING_STOP

} // namespace hyperion::option
