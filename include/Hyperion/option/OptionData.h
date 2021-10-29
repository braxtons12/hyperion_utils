/// @file OptionData.h
/// @author Braxton Salyer <braxtonsalyer@gmail.com>
/// @brief Implementation of `Option`'s storage representation
/// @version 0.1
/// @date 2021-10-19
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
#include <variant>

namespace hyperion::option {
	IGNORE_PADDING_START

	/// @brief `OptionData` is the storage implementation backing `Option`
	/// It acts as a tagged union, supports references (unlike `std::optional`), and abstracts away
	/// the storage details of `Option`'s implementation
	template<typename T>
	struct OptionData;

	template<concepts::NotReference T>
	struct OptionData<T> : private std::variant<T, None> {
		using type = T;
		using reference = std::add_lvalue_reference_t<type>;
		using const_reference = std::add_lvalue_reference_t<std::add_const_t<type>>;
		using rvalue_reference = std::add_rvalue_reference_t<type>;
		using extracted = rvalue_reference;
		using pointer = std::conditional_t<concepts::Pointer<T>, T, std::add_pointer_t<T>>;
		using pointer_to_const = std::add_pointer_t<std::add_const_t<std::remove_pointer_t<type>>>;
		using rep = std::variant<T, None>;

		static constexpr auto T_INDEX = 0_usize;
		using storage_type = T;

		constexpr OptionData() noexcept : rep(None()) {
		}
		explicit constexpr OptionData(const_reference t) noexcept(
			concepts::NoexceptCopyConstructible<T>) requires concepts::CopyConstructible<T>
			: rep(t) {
		}
		explicit constexpr OptionData(rvalue_reference t) noexcept(
			concepts::NoexceptMoveConstructible<T>) requires concepts::MoveConstructible<T>
			: rep(std::move(t)) {
		}
		/// @brief Constructs an `OptionData` by constructing the `T` in place in it
		/// @tparam Args - The types of the arguments to pass to `T`'s constructor
		/// @param args  - The arguments to pass to `T`'s constructor
		template<typename... Args>
		requires concepts::ConstructibleFrom<T, Args...>
		explicit constexpr OptionData(Args&&... args) noexcept(
			concepts::NoexceptConstructibleFrom<T, Args...>)
			: rep(std::forward<Args>(args)...) {
		}
		explicit constexpr OptionData(None n) noexcept : rep(n) {
		}
		constexpr OptionData(const OptionData& data) noexcept(
			concepts::NoexceptCopyConstructible<T>) requires concepts::CopyConstructible<T>
			: rep(static_cast<const rep&>(data)) {
		}
		constexpr OptionData(OptionData&& data) noexcept(
			concepts::NoexceptMoveConstructible<T>) requires concepts::MoveConstructible<T>
			: rep(static_cast<rep&&>(data)) {
		}
		constexpr ~OptionData() noexcept(concepts::NoexceptDestructible<T>) = default;

		/// @brief Returns whether this currently contains an active `T`
		[[nodiscard]] inline constexpr auto has_value() const noexcept -> bool {
			return this->index() == T_INDEX;
		}

		/// @brief Returns a const reference to the contained data
		[[nodiscard]] inline constexpr auto get() const noexcept -> const_reference {
			return std::get<T_INDEX>(*this);
		}

		/// @brief Returns a reference to the contained data
		[[nodiscard]] inline constexpr auto get() noexcept -> reference {
			return std::get<T_INDEX>(*this);
		}

		/// @brief Extracts the contained data out of this
		[[nodiscard]] inline constexpr auto extract() noexcept(concepts::NoexceptMovable<T>)
			-> extracted requires concepts::Movable<T> {
			return std::get<T_INDEX>(std::move(*this));
		}

		constexpr auto
		operator=(const OptionData& data) noexcept(concepts::NoexceptCopyAssignable<T>)
			-> OptionData& requires concepts::CopyAssignable<T> {
			if(this == &data) {
				return *this;
			}

			rep::operator=(static_cast<const rep&>(data));
			return *this;
		}
		constexpr auto operator=(OptionData&& data) noexcept(concepts::NoexceptMoveAssignable<T>)
			-> OptionData& requires concepts::MoveAssignable<T> {
			if(this == &data) {
				return *this;
			}

			rep::operator=(static_cast<rep&&>(data));
			return *this;
		}
	};

	template<concepts::Reference T>
	struct OptionData<T> : std::variant<std::reference_wrapper<std::remove_reference_t<T>>, None> {
		using type = std::remove_reference_t<T>;
		using reference = std::add_lvalue_reference_t<std::remove_const_t<type>>;
		using const_reference = std::add_lvalue_reference_t<std::add_const_t<type>>;
		using rvalue_reference
			= std::conditional_t<std::is_const_v<type>, const_reference, reference>;
		using extracted = rvalue_reference;
		using pointer = std::add_pointer_t<std::remove_const_t<type>>;
		using pointer_to_const = std::add_pointer_t<std::add_const_t<type>>;
		using rep = std::variant<std::reference_wrapper<type>, None>;

		static constexpr auto T_INDEX = 0_usize;
		using storage_type = std::reference_wrapper<type>;

		constexpr OptionData() noexcept : rep(None()) {
		}
		explicit constexpr OptionData(const_reference t) noexcept requires std::is_const_v<T>
			: rep(ref(t)) {
		}
		explicit constexpr OptionData(reference t) noexcept : rep(ref(t)) {
		}
		explicit constexpr OptionData(None n) noexcept : rep(n) {
		}
		constexpr OptionData(const OptionData& data) noexcept : rep(static_cast<const rep&>(data)) {
		}
		constexpr OptionData(OptionData&& data) noexcept : rep(static_cast<rep&&>(data)) {
		}
		constexpr ~OptionData() noexcept = default;

		[[nodiscard]] inline constexpr auto has_value() const noexcept -> bool {
			return this->index() == T_INDEX;
		}

		/// @brief Returns a const reference to the contained data
		[[nodiscard]] inline constexpr auto get() const noexcept -> const_reference {
			return std::get<T_INDEX>(*this).get();
		}

		/// @brief Returns a reference to the contained data
		[[nodiscard]] inline constexpr auto get() noexcept -> storage_type& {
			return std::get<T_INDEX>(*this);
		}

		/// @brief Extracts the contained data out of this
		[[nodiscard]] inline constexpr auto extract() noexcept -> extracted {
			return std::get<T_INDEX>(std::move(*this)).get();
		}

		constexpr auto operator=(const OptionData& data) noexcept -> OptionData& {
			if(this == &data) {
				return *this;
			}

			rep::operator=(static_cast<const rep&>(data));
			return *this;
		}
		constexpr auto operator=(OptionData&& data) noexcept -> OptionData& {
			if(this == &data) {
				return *this;
			}

			rep::operator=(static_cast<rep&&>(data));
			return *this;
		}

	  private:
		template<typename U>
		[[nodiscard]] inline static constexpr auto ref(U&& u) noexcept {
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
