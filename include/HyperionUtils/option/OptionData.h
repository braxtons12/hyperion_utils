#pragma once

#include <functional>
#include <variant>

#include "../Concepts.h"
#include "../HyperionDef.h"
#include "None.h"

namespace hyperion::option {
	IGNORE_PADDING_START
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

		[[nodiscard]] constexpr inline auto has_value() const noexcept -> bool {
			return this->index() == T_INDEX;
		}

		/// @brief Returns a const reference to the contained data
		[[nodiscard]] constexpr inline auto get() const noexcept -> const_reference {
			return std::get<T_INDEX>(*this);
		}

		/// @brief Returns a reference to the contained data
		[[nodiscard]] constexpr inline auto get() noexcept -> reference {
			return std::get<T_INDEX>(*this);
		}

		/// @brief Extracts the contained data out of this
		[[nodiscard]] constexpr inline auto extract() noexcept(concepts::NoexceptMovable<T>)
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

		[[nodiscard]] constexpr inline auto has_value() const noexcept -> bool {
			return this->index() == T_INDEX;
		}

		/// @brief Returns a const reference to the contained data
		[[nodiscard]] constexpr inline auto get() const noexcept -> const_reference {
			return std::get<T_INDEX>(*this).get();
		}

		/// @brief Returns a reference to the contained data
		[[nodiscard]] constexpr inline auto get() noexcept -> storage_type& {
			return std::get<T_INDEX>(*this);
		}

		/// @brief Extracts the contained data out of this
		[[nodiscard]] constexpr inline auto extract() noexcept -> extracted {
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
