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
#include <variant>

namespace hyperion::result {
	IGNORE_PADDING_START

	/// @brief `ResultData` is the storage implementation backing `Result`
	/// It acts as a tagged union, supports references, and abstracts away
	/// the storage details of `Result`'s implementation
	template<typename T, typename E>
	struct ResultData;

	template<concepts::NotReference T, concepts::NotReference E>
	struct ResultData<T, E> : private std::variant<T, E, option::None> {
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

		using rep = std::variant<T, E, option::None>;

		static constexpr auto OK_INDEX = 0_usize;
		static constexpr auto ERR_INDEX = 1_usize;
		using ok_storage_type = T;
		using err_storage_type = E;

		constexpr ResultData() noexcept : rep(option::None()) {
		}
		explicit constexpr ResultData(ok_const_reference ok) noexcept(
			concepts::NoexceptCopyConstructible<T>) requires concepts::CopyConstructible<T>
			: rep(ok) {
		}
		explicit constexpr ResultData(ok_rvalue_reference ok) noexcept(
			concepts::NoexceptMoveConstructible<T>) requires concepts::MoveConstructible<T>
			: rep(std::move(ok)) {
		}
		template<typename... Args>
		requires concepts::ConstructibleFrom<T, Args...>
		explicit constexpr ResultData(Args&&... args) noexcept(
			concepts::NoexceptConstructibleFrom<T, Args...>)
			: rep(std::in_place_type_t<T>{}, std::forward<Args>(args)...) {
		}
		explicit constexpr ResultData(err_const_reference err) noexcept(
			concepts::NoexceptCopyConstructible<E>) requires concepts::CopyConstructible<E>
			: rep(err) {
		}
		explicit constexpr ResultData(err_rvalue_reference err) noexcept(
			concepts::NoexceptMoveConstructible<E>) requires concepts::MoveConstructible<E>
			: rep(std::move(err)) {
		}
		template<typename... Args>
		requires concepts::ConstructibleFrom<E, Args...>
		explicit constexpr ResultData(Args&&... args) noexcept(
			concepts::NoexceptConstructibleFrom<E, Args...>)
			: rep(std::in_place_type_t<E>{}, std::forward<Args>(args)...) {
		}
		explicit constexpr ResultData(option::None none) noexcept : rep(none) {
		}
		constexpr ResultData(const ResultData& data) noexcept(
			concepts::NoexceptCopyConstructible<rep>) requires concepts::CopyConstructible<rep>
			: rep(static_cast<const rep&>(data)) {
		}
		constexpr ResultData(ResultData&& data) noexcept(
			concepts::NoexceptMoveConstructible<rep>) requires concepts::MoveConstructible<rep>
			: rep(static_cast<rep&&>(data)) {
		}
		constexpr ~ResultData() noexcept(concepts::NoexceptDestructible<rep>) = default;

		/// @brief Returns whether this is currently the `Ok` variant
		[[nodiscard]] inline constexpr auto has_ok() const noexcept -> bool {
			return this->index() == OK_INDEX;
		}

		/// @brief Returns whether this is currently the `Err` variant
		[[nodiscard]] inline constexpr auto has_err() const noexcept -> bool {
			return this->index() == ERR_INDEX;
		}

		/// @brief Returns whether this is currently disengaged (it's not in the `Ok` nor the `Err`
		/// variant; it has been moved out of)
		[[nodiscard]] inline constexpr auto is_empty() const noexcept -> bool {
			return !(has_ok() || has_err());
		}

		/// @brief Returns a const reference to this as the `Ok` variant
		[[nodiscard]] inline constexpr auto get() const noexcept -> ok_const_reference {
			return std::get<OK_INDEX>(*this);
		}

		/// @brief Returns a reference to this as the `Ok` variant
		[[nodiscard]] inline constexpr auto get() noexcept -> ok_reference {
			return std::get<OK_INDEX>(*this);
		}

		/// @brief Returns a const reference to this as the `Err` variant
		[[nodiscard]] inline constexpr auto get_err() const noexcept -> err_const_reference {
			return std::get<ERR_INDEX>(*this);
		}

		/// @brief Returns a reference to this as the `Err` variant
		[[nodiscard]] inline constexpr auto get_err() noexcept -> err_reference {
			return std::get<ERR_INDEX>(*this);
		}

		/// @brief Extracts the `Ok` variant out of this
		[[nodiscard]] inline constexpr auto extract() noexcept(concepts::NoexceptMovable<T>)
			-> ok_extracted requires concepts::Movable<T> {
			return std::get<OK_INDEX>(std::move(*this));
		}

		/// @brief Extracts the `Err` variant out of this
		[[nodiscard]] inline constexpr auto extract_err() noexcept(concepts::NoexceptMovable<E>)
			-> err_extracted requires concepts::Movable<E> {
			return std::get<ERR_INDEX>(std::move(*this));
		}

		constexpr auto
		operator=(const ResultData& data) noexcept(concepts::NoexceptCopyAssignable<rep>)
			-> ResultData& requires concepts::CopyAssignable<rep> {
			if(this == &data) {
				return *this;
			}

			rep::operator=(static_cast<const rep&>(data));
			return *this;
		}
		constexpr auto operator=(ResultData&& data) noexcept(concepts::NoexceptMoveAssignable<rep>)
			-> ResultData& requires concepts::MoveAssignable<rep> {
			if(this == &data) {
				return *this;
			}

			rep::operator=(static_cast<rep&&>(data));
			return *this;
		}
		constexpr auto operator=(const option::None& none) noexcept -> ResultData& {
			rep::operator=(none);
			return *this;
		}
		constexpr auto operator=(option::None&& none) noexcept -> ResultData& {
			rep::operator=(none);
			return *this;
		}
	};

	template<concepts::Reference T, concepts::NotReference E>
	struct ResultData<T, E>
		: private std::
			  variant<std::reference_wrapper<std::remove_reference_t<T>>, E, option::None> {
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

		using rep = std::variant<T, E, option::None>;

		static constexpr auto OK_INDEX = 0_usize;
		static constexpr auto ERR_INDEX = 1_usize;
		using ok_storage_type = std::reference_wrapper<ok_type>;
		using err_storage_type = E;

		constexpr ResultData() noexcept : rep(option::None()) {
		}
		explicit constexpr ResultData(ok_const_reference ok) noexcept requires std::is_const_v<T>
			: rep(ref(ok)) {
		}
		explicit constexpr ResultData(ok_reference ok) noexcept : rep(ref(ok)) {
		}
		explicit constexpr ResultData(err_const_reference err) noexcept(
			concepts::NoexceptCopyConstructible<E>) requires concepts::CopyConstructible<E>
			: rep(err) {
		}
		explicit constexpr ResultData(err_rvalue_reference err) noexcept(
			concepts::NoexceptMoveConstructible<E>) requires concepts::MoveConstructible<E>
			: rep(std::move(err)) {
		}
		template<typename... Args>
		requires concepts::ConstructibleFrom<E, Args...>
		explicit constexpr ResultData(Args&&... args) noexcept(
			concepts::NoexceptConstructibleFrom<E, Args...>)
			: rep(std::in_place_type_t<E>{}, std::forward<Args>(args)...) {
		}
		explicit constexpr ResultData(option::None none) noexcept : rep(none) {
		}
		constexpr ResultData(const ResultData& data) noexcept(
			concepts::NoexceptCopyConstructible<rep>) requires concepts::CopyConstructible<rep>
			: rep(static_cast<const rep&>(data)) {
		}
		constexpr ResultData(ResultData&& data) noexcept(
			concepts::NoexceptMoveConstructible<rep>) requires concepts::MoveConstructible<rep>
			: rep(static_cast<rep&&>(data)) {
		}
		constexpr ~ResultData() noexcept(concepts::NoexceptDestructible<rep>) = default;

		/// @brief Returns whether this is currently the `Ok` variant
		[[nodiscard]] inline constexpr auto has_ok() const noexcept -> bool {
			return this->index() == OK_INDEX;
		}

		/// @brief Returns whether this is currently the `Err` variant
		[[nodiscard]] inline constexpr auto has_err() const noexcept -> bool {
			return this->index() == ERR_INDEX;
		}

		/// @brief Returns whether this is currently disengaged (it's not in the `Ok` nor the `Err`
		/// variant; it has been moved out of)
		[[nodiscard]] inline constexpr auto is_empty() const noexcept -> bool {
			return !has_ok() && !has_err();
		}

		/// @brief Returns a const reference to this as the `Ok` variant
		[[nodiscard]] inline constexpr auto get() const noexcept -> ok_const_reference {
			return std::get<OK_INDEX>(*this);
		}

		/// @brief Returns a reference to this as the `Ok` variant
		[[nodiscard]] inline constexpr auto get() noexcept -> ok_storage_type& {
			return std::get<OK_INDEX>(*this);
		}

		/// @brief Returns a const reference to this as the `Err` variant
		[[nodiscard]] inline constexpr auto get_err() const noexcept -> err_const_reference {
			return std::get<ERR_INDEX>(*this);
		}

		/// @brief Returns a reference to this as the `Err` variant
		[[nodiscard]] inline constexpr auto get_err() noexcept -> err_reference {
			return std::get<ERR_INDEX>(*this);
		}

		/// @brief Extracts the `Ok` variant out of this
		[[nodiscard]] inline constexpr auto extract() noexcept -> ok_extracted {
			return std::get<OK_INDEX>(std::move(*this)).get();
		}

		/// @brief Extracts the `Err` variant out of this
		[[nodiscard]] inline constexpr auto extract_err() noexcept(concepts::NoexceptMovable<E>)
			-> err_extracted requires concepts::Movable<E> {
			return std::get<ERR_INDEX>(std::move(*this));
		}

		constexpr auto
		operator=(const ResultData& data) noexcept(concepts::NoexceptCopyAssignable<rep>)
			-> ResultData& requires concepts::CopyAssignable<rep> {
			if(this == &data) {
				return *this;
			}

			rep::operator=(static_cast<const rep&>(data));
			return *this;
		}
		constexpr auto operator=(ResultData&& data) noexcept(concepts::NoexceptMoveAssignable<rep>)
			-> ResultData& requires concepts::MoveAssignable<rep> {
			if(this == &data) {
				return *this;
			}

			rep::operator=(static_cast<rep&&>(data));
			return *this;
		}
		constexpr auto operator=(const option::None& none) noexcept -> ResultData& {
			rep::operator=(none);
			return *this;
		}
		constexpr auto operator=(option::None&& none) noexcept -> ResultData& {
			rep::operator=(none);
			return *this;
		}
		// clang-format off

	  private:
		template<typename U>
		inline static constexpr auto ref(U&& u) noexcept {
			if constexpr(std::is_const_v<T>) {
				return std::cref(std::forward<U>(u));
			}
			else {
				return std::ref(std::forward<U>(u));
			}
		}
		// clang-format on
	};

	template<concepts::NotReference T, concepts::Reference E>
	struct ResultData<T, E>
		: private std::variant<std::reference_wrapper<std::remove_reference_t<T>>,
							   std::reference_wrapper<std::remove_reference_t<E>>,
							   option::None> {
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

		using rep = std::variant<T, E, option::None>;

		static constexpr auto OK_INDEX = 0_usize;
		static constexpr auto ERR_INDEX = 1_usize;
		using ok_storage_type = T;
		using err_storage_type = std::reference_wrapper<err_type>;

		constexpr ResultData() noexcept : rep(option::None()) {
		}
		explicit constexpr ResultData(ok_const_reference ok) noexcept(
			concepts::NoexceptCopyConstructible<T>) requires concepts::CopyConstructible<T>
			: rep(ok) {
		}
		explicit constexpr ResultData(ok_rvalue_reference ok) noexcept(
			concepts::NoexceptMoveConstructible<T>) requires concepts::MoveConstructible<T>
			: rep(std::move(ok)) {
		}
		template<typename... Args>
		requires concepts::ConstructibleFrom<T, Args...>
		explicit constexpr ResultData(Args&&... args) noexcept(
			concepts::NoexceptConstructibleFrom<T, Args...>)
			: rep(std::in_place_type_t<T>{}, std::forward<Args>(args)...) {
		}
		explicit constexpr ResultData(err_const_reference err) noexcept requires std::is_const_v<E>
			: rep(ref(err)) {
		}
		explicit constexpr ResultData(err_reference err) noexcept : rep(ref(err)) {
		}
		explicit constexpr ResultData(option::None none) noexcept : rep(none) {
		}
		constexpr ResultData(const ResultData& data) noexcept(
			concepts::NoexceptCopyConstructible<rep>) requires concepts::CopyConstructible<rep>
			: rep(static_cast<const rep&>(data)) {
		}
		constexpr ResultData(ResultData&& data) noexcept(
			concepts::NoexceptMoveConstructible<rep>) requires concepts::MoveConstructible<rep>
			: rep(static_cast<rep&&>(data)) {
		}
		constexpr ~ResultData() noexcept(concepts::NoexceptDestructible<rep>) = default;

		/// @brief Returns whether this is currently the `Ok` variant
		[[nodiscard]] inline constexpr auto has_ok() const noexcept -> bool {
			return this->index() == OK_INDEX;
		}

		/// @brief Returns whether this is currently the `Err` variant
		[[nodiscard]] inline constexpr auto has_err() const noexcept -> bool {
			return this->index() == ERR_INDEX;
		}

		/// @brief Returns whether this is currently disengaged (it's not in the `Ok` nor the `Err`
		/// variant; it has been moved out of)
		[[nodiscard]] inline constexpr auto is_empty() const noexcept -> bool {
			return !has_ok() && !has_err();
		}

		/// @brief Returns a const reference to this as the `Ok` variant
		[[nodiscard]] inline constexpr auto get() const noexcept -> ok_const_reference {
			return std::get<OK_INDEX>(*this);
		}

		/// @brief Returns a reference to this as the `Ok` variant
		[[nodiscard]] inline constexpr auto get() noexcept -> ok_reference {
			return std::get<OK_INDEX>(*this);
		}

		/// @brief Returns a const reference to this as the `Err` variant
		[[nodiscard]] inline constexpr auto get_err() const noexcept -> err_const_reference {
			return std::get<ERR_INDEX>(*this);
		}

		/// @brief Returns a reference to this as the `Err` variant
		[[nodiscard]] inline constexpr auto get_err() noexcept -> err_storage_type& {
			return std::get<ERR_INDEX>(*this);
		}

		/// @brief Extracts the `Ok` variant out of this
		[[nodiscard]] inline constexpr auto extract() noexcept -> ok_extracted {
			return std::get<OK_INDEX>(std::move(*this));
		}

		/// @brief Extracts the `Err` variant out of this
		[[nodiscard]] inline constexpr auto extract_err() noexcept -> err_extracted {
			return std::get<ERR_INDEX>(std::move(*this)).get();
		}

		constexpr auto
		operator=(const ResultData& data) noexcept(concepts::NoexceptCopyAssignable<rep>)
			-> ResultData& requires concepts::CopyAssignable<rep> {
			if(this == &data) {
				return *this;
			}

			rep::operator=(static_cast<const rep&>(data));
			return *this;
		}
		constexpr auto operator=(ResultData&& data) noexcept(concepts::NoexceptMoveAssignable<rep>)
			-> ResultData& requires concepts::MoveAssignable<rep> {
			if(this == &data) {
				return *this;
			}

			rep::operator=(static_cast<rep&&>(data));
			return *this;
		}
		constexpr auto operator=(const option::None& none) noexcept -> ResultData& {
			rep::operator=(none);
			return *this;
		}
		constexpr auto operator=(option::None&& none) noexcept -> ResultData& {
			rep::operator=(none);
			return *this;
		}
		// clang-format off

	  private:
		template<typename U>
		inline static constexpr auto ref(U&& u) noexcept {
			if constexpr(std::is_const_v<T>) {
				return std::cref(std::forward<U>(u));
			}
			else {
				return std::ref(std::forward<U>(u));
			}
		}
		// clang-format on
	};

	template<concepts::Reference T, concepts::Reference E>
	struct ResultData<T, E>
		: private std::variant<std::reference_wrapper<std::remove_reference_t<T>>,
							   std::reference_wrapper<std::remove_reference_t<E>>,
							   option::None> {
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

		using rep = std::variant<T, E, option::None>;

		static constexpr auto OK_INDEX = 0_usize;
		static constexpr auto ERR_INDEX = 1_usize;
		using ok_storage_type = std::reference_wrapper<ok_type>;
		using err_storage_type = std::reference_wrapper<err_type>;

		constexpr ResultData() noexcept : rep(option::None()) {
		}
		explicit constexpr ResultData(ok_const_reference ok) noexcept requires std::is_const_v<T>
			: rep(ref(ok)) {
		}
		explicit constexpr ResultData(ok_reference ok) noexcept : rep(ref(ok)) {
		}
		explicit constexpr ResultData(err_const_reference err) noexcept requires std::is_const_v<E>
			: rep(ref(err)) {
		}
		explicit constexpr ResultData(err_reference err) noexcept : rep(ref(err)) {
		}
		explicit constexpr ResultData(option::None none) noexcept : rep(none) {
		}
		constexpr ResultData(const ResultData& data) noexcept(
			concepts::NoexceptCopyConstructible<rep>) requires concepts::CopyConstructible<rep>
			: rep(static_cast<const rep&>(data)) {
		}
		constexpr ResultData(ResultData&& data) noexcept(
			concepts::NoexceptMoveConstructible<rep>) requires concepts::MoveConstructible<rep>
			: rep(static_cast<rep&&>(data)) {
		}
		constexpr ~ResultData() noexcept(concepts::NoexceptDestructible<rep>) = default;

		/// @brief Returns whether this is currently the `Ok` variant
		[[nodiscard]] inline constexpr auto has_ok() const noexcept -> bool {
			return this->index() == OK_INDEX;
		}

		/// @brief Returns whether this is currently the `Err` variant
		[[nodiscard]] inline constexpr auto has_err() const noexcept -> bool {
			return this->index() == ERR_INDEX;
		}

		/// @brief Returns whether this is currently disengaged (it's not in the `Ok` nor the `Err`
		/// variant; it has been moved out of)
		[[nodiscard]] inline constexpr auto is_empty() const noexcept -> bool {
			return !has_ok() && !has_err();
		}

		/// @brief Returns a const reference to this as the `Ok` variant
		[[nodiscard]] inline constexpr auto get() const noexcept -> ok_const_reference {
			return std::get<OK_INDEX>(*this);
		}

		/// @brief Returns a reference to this as the `Ok` variant
		[[nodiscard]] inline constexpr auto get() noexcept -> ok_storage_type& {
			return std::get<OK_INDEX>(*this);
		}

		/// @brief Returns a const reference to this as the `Err` variant
		[[nodiscard]] inline constexpr auto get_err() const noexcept -> err_const_reference {
			return std::get<ERR_INDEX>(*this);
		}

		/// @brief Returns a reference to this as the `Err` variant
		[[nodiscard]] inline constexpr auto get_err() noexcept -> err_storage_type& {
			return std::get<ERR_INDEX>(*this);
		}

		/// @brief Extracts the `Ok` variant out of this
		[[nodiscard]] inline constexpr auto extract() noexcept -> ok_extracted {
			return std::get<OK_INDEX>(std::move(*this)).get();
		}

		/// @brief Extracts the `Err` variant out of this
		[[nodiscard]] inline constexpr auto extract_err() noexcept -> err_extracted {
			return std::get<ERR_INDEX>(std::move(*this)).get();
		}

		constexpr auto
		operator=(const ResultData& data) noexcept(concepts::NoexceptCopyAssignable<rep>)
			-> ResultData& requires concepts::CopyAssignable<rep> {
			if(this == &data) {
				return *this;
			}

			rep::operator=(static_cast<const rep&>(data));
			return *this;
		}
		constexpr auto operator=(ResultData&& data) noexcept(concepts::NoexceptMoveAssignable<rep>)
			-> ResultData& requires concepts::MoveAssignable<rep> {
			if(this == &data) {
				return *this;
			}

			rep::operator=(static_cast<rep&&>(data));
			return *this;
		}
		constexpr auto operator=(const option::None& none) noexcept -> ResultData& {
			rep::operator=(none);
			return *this;
		}
		constexpr auto operator=(option::None&& none) noexcept -> ResultData& {
			rep::operator=(none);
			return *this;
		}
		// clang-format off

	  private:
		template<typename U>
		inline static constexpr auto ref(U&& u) noexcept {
			if constexpr(std::is_const_v<T>) {
				return std::cref(std::forward<U>(u));
			}
			else {
				return std::ref(std::forward<U>(u));
			}
		}
		// clang-format on
	};
	IGNORE_PADDING_STOP
} // namespace hyperion::result
