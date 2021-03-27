#pragma once

#include <cstdlib>
#include <functional>
#include <iostream>
#include <type_traits>
#include <utility>

#include "Concepts.h"
#include "Error.h"
#include "Ignore.h"
#include "Macros.h"
#include "TypeTraits.h"

namespace hyperion::utils {

	using concepts::Passable, concepts::ErrorType, concepts::DefaultConstructible,
		concepts::Reference, concepts::NotReference, concepts::Pointer, concepts::NotPointer,
		concepts::Copyable, concepts::Movable, concepts::CopyOrMovable, concepts::NotMovable;

	template<Passable T, ErrorType E = Error>
	requires DefaultConstructible<T> && NotReference<T> && DefaultConstructible<E> && NotReference<
		E>
	class [[nodiscard]] Result;

	IGNORE_UNUSED_MACROS_START
	IGNORE_UNUSED_TEMPLATES_START
	IGNORE_PADDING_START

#ifndef OPTION
	#define OPTION

	struct NoneType {
		explicit constexpr NoneType(int) { // NOLINT
		}
		constexpr NoneType(const NoneType& none) noexcept = default;
		constexpr NoneType(NoneType&& none) noexcept = default;
		constexpr auto operator=(const NoneType& none) noexcept -> NoneType& = default;
		constexpr auto operator=(NoneType&& none) noexcept -> NoneType& = default;
	};

	inline constexpr NoneType none_t{0};

	/// @brief Represents an optional value.
	/// Every `Option` is either `Some` and contains a value, or `None`, and does
	/// not. Useful for things such as:
	/// * * Optional members/fields
	/// * * Members/fields that can be loaned or "taken"
	/// * * Optional function arguments
	/// * * Nullable types/pointers
	/// * * Return values for functions not defined over their entire input range
	///
	/// @tparam T - The type of the potentially-contained value
	template<Passable T>
	requires NotReference<T>
	class [[nodiscard]] Option {
	  public:
		constexpr Option(NoneType none) noexcept { // NOLINT
			ignore(none);
		}
		constexpr Option(const Option& option) noexcept requires Copyable<T>
		= default;
		constexpr Option(Option&& option) noexcept requires Movable<T> {
			this->m_is_some = option.m_is_some;
			if(option.m_is_some) {
				option.m_is_some = false;
				if constexpr(std::is_pointer_v<T>) {
					this->m_some = option.m_some;
				}
				else if constexpr(std::is_move_constructible_v<T>) {
					this->m_some = std::move(option.m_some);
				}
			}
		}

		~Option() noexcept = default;

		/// @brief Constructs an `Option<T>` containing `some`, aka a `Some` variant
		/// containing `some` aka `Some(some)`
		///
		/// @param some - The value to store in this `Option<T>`
		///
		/// @return `Some(some)`
		[[nodiscard]] constexpr static inline auto Some(const T& some) noexcept -> Option {
			return Option(some);
		}

		/// @brief Constructs an `Option<T>` containing `some`, aka a `Some` variant
		/// containing `some` aka `Some(some)`
		///
		/// @param some - The value to store in this `Option<T>`
		///
		/// @return `Some(some)`
		[[nodiscard]] constexpr static inline auto Some(T&& some) noexcept -> Option {
			return Option(std::forward<T>(some));
		}

		/// @brief Constructs an empty `Option<T>`, aka a `None`
		///
		/// @return `None`
		[[nodiscard]] constexpr static inline auto None(NoneType none) noexcept -> Option {
			return Option(none);
		}

		/// @brief Returns `true` if this is `Some`, `false` if this is `None`
		///
		/// @return Whether this is `Some`
		[[nodiscard]] constexpr inline auto is_some() const noexcept -> bool {
			return m_is_some;
		}

		/// @brief Returns `true` if this is `None`, `false` if this is `Some`
		///
		/// @return Whether this is `None`
		[[nodiscard]] constexpr inline auto is_none() const noexcept -> bool {
			return !m_is_some;
		}

		/// @brief Maps this `Option<T>` to an `Option<U>`,
		/// returning `Some(U)` if this is Some, or `None` if this is `None`
		///
		/// @tparam U - The type to map to
		/// @param map_func - The function to perform the mapping
		///
		/// @return `Some(U)` if this is Some, or `None` if this is `None`
		template<Passable U>
		[[nodiscard]] inline auto
		map(std::function<U(const T&)> map_func) const noexcept -> Option<U>
		requires NotPointer<T> {
			if(m_is_some) {
				return Option<U>::Some(map_func(m_some));
			}
			else {
				return Option<U>::None(none_t);
			}
		}

		/// @brief Maps this `Option<T>` to an `Option<U>`,
		/// returning `Some(U)` if this is Some, or `None` if this is `None`
		///
		/// @tparam U - The type to map to
		/// @param map_func - The function to perform the mapping
		///
		/// @return `Some(U)` if this is Some, or `None` if this is `None`
		template<Passable U>
		[[nodiscard]] inline auto
		map(std::function<U(const T)> map_func) const noexcept -> Option<U>
		requires Pointer<T> {
			if(m_is_some) {
				return Option<U>::Some(map_func(m_some));
			}
			else {
				return Option<U>::None(none_t);
			}
		}

		/// @brief Maps this `Option<T>` to a `U`,
		/// returning `U` (mapped by `map_func`) if this is `Some`,
		/// or `default_value` if this is `None`
		///
		/// @tparam U - The type to map to
		/// @param map_func - The function to perform the mapping
		/// @param default_value - The default value
		///
		/// @return The result of the mapping if this is `Some`, or `default_value`
		template<Passable U>
		[[nodiscard]] inline auto
		map_or(std::function<U(const T&)> map_func, const U& default_value) const noexcept
			-> U requires NotPointer<T> {
			if(m_is_some) {
				return map_func(m_some);
			}
			else {
				return default_value;
			}
		}

		/// @brief Maps this `Option<T>` to a `U`,
		/// returning `U` (mapped by `map_func`) if this is `Some`,
		/// or `default_value` if this is `None`
		///
		/// @tparam U - The type to map to
		/// @param map_func - The function to perform the mapping
		/// @param default_value - The default value
		///
		/// @return The result of the mapping if this is `Some`, or `default_value`
		template<Passable U>
		[[nodiscard]] inline auto
		map_or(std::function<U(const T)> map_func, const U& default_value) const noexcept
			-> U requires Pointer<T> {
			if(m_is_some) {
				return map_func(m_some);
			}
			else {
				return default_value;
			}
		}

		/// @brief Maps this `Option<T>` to a `U`,
		/// returning `U` (mapped by `map_func`) if this is `Some`,
		/// or `default_value` if this is `None`
		///
		/// @tparam U - The type to map to
		/// @param map_func - The function to perform the mapping
		/// @param default_value - The default value
		///
		/// @return The result of the mapping if this is `Some`, or `default_value`
		template<Passable U>
		[[nodiscard]] inline auto
		map_or(std::function<U(const T&)> map_func, U&& default_value) const noexcept
			-> U requires NotPointer<T> {
			if(m_is_some) {
				return map_func(m_some);
			}
			else {
				return std::forward<U>(default_value);
			}
		}

		/// @brief Maps this `Option<T>` to a `U`,
		/// returning `U` (mapped by `map_func`) if this is `Some`,
		/// or `default_value` if this is `None`
		///
		/// @tparam U - The type to map to
		/// @param map_func - The function to perform the mapping
		/// @param default_value - The default value
		///
		/// @return The result of the mapping if this is `Some`, or `default_value`
		template<Passable U>
		[[nodiscard]] inline auto
		map_or(std::function<U(const T)> map_func, U&& default_value) const noexcept
			-> U requires Pointer<T> {
			if(m_is_some) {
				return map_func(m_some);
			}
			else {
				return std::forward<U>(default_value);
			}
		}

		/// @brief Maps this `Option<T>` to a `U`,
		/// returning `U` (mapped by `map_func`) if this is `Some`,
		/// or `U` (the default value returned by `default_generator`)
		/// if this is `None`
		///
		/// @tparam U - The type to map to
		/// @param map_func - The function to perform the mapping
		/// @param default_generator - The function to generate the default value
		///
		/// @return The result of the mapping if this is `Some`,
		///			or the value returned by `default_generator` if this is
		///`None`
		template<Passable U>
		[[nodiscard]] inline auto map_or_else(std::function<U(const T&)> map_func,
											  std::function<U()> default_generator) const noexcept
			-> U requires NotPointer<T> {
			if(m_is_some) {
				return map_func(m_some);
			}
			else {
				return default_generator();
			}
		}

		/// @brief Maps this `Option<T>` to a `U`,
		/// returning `U` (mapped by `map_func`) if this is `Some`,
		/// or `U` (the default value returned by `default_generator`)
		/// if this is `None`
		///
		/// @tparam U - The type to map to
		/// @param map_func - The function to perform the mapping
		/// @param default_generator - The function to generate the default value
		///
		/// @return The result of the mapping if this is `Some`,
		///			or the value returned by `default_generator` if this is
		///`None`
		template<Passable U>
		[[nodiscard]] inline auto
		map_or_else(std::function<U(const T)> map_func,
					std::function<U()> default_generator) const noexcept -> U requires Pointer<T> {
			if(m_is_some) {
				return map_func(m_some);
			}
			else {
				return default_generator();
			}
		}

		/// @brief Converts this `Option<T>` to a `Result()`, consuming this
		/// `Option<T>`. Returns `Ok(T)` if this is `Some` or `Err(error)` if this is
		/// `None`
		///
		/// @tparam E - The type to return if this is `None`.
		///				Must be an `Error` type (`hyperion::utils::Error`)
		/// @param error - The `Error` to return if this `Option<T>` is `None`
		///
		/// @return `Ok(T)` if this is `Some`, `Err(error)` if this is `None`
		[[nodiscard]] constexpr inline auto
		ok_or(const ErrorType auto& error) noexcept -> Result<T, decltype(error)>
		requires NotReference<std::remove_reference_t<decltype(error)>>;

		/// @brief Converts this `Option<T>` to a `Result()`, consuming this
		/// `Option<T>`. Returns `Ok(T)` if this is `Some` or `Err(error)` if this is
		/// `None`
		///
		/// @tparam E - The type to return if this is `None`.
		///				Must be an `Error` type (`hyperion::utils::Error`)
		/// @param error - The `Error` to return if this `Option<T>` is `None`
		///
		/// @return `Ok(T)` if this is `Some`, `Err(error)` if this is `None`
		[[nodiscard]] constexpr inline auto ok_or(ErrorType auto&& error) noexcept
			-> Result<T, std::remove_reference_t<decltype(error)>>;

		/// @brief Converts this `Option<T>` to a `Result()<T, E>`, consuming this
		/// `Option<T>`. Returns `Ok(T)` if this is `Some`, or `Err(E)` (where E is
		/// generated by `error_generator`) if this is `None`
		///
		/// @tparam E - The type to return if this is `None`.
		///				Must be an `Error` type (`hyperion::utils::Error`)
		/// @param error_generator - The function to generate the `Error` value
		///
		/// @return `Ok(T)` if this is `Some`, `Err(E)` if this is `None`
		template<ErrorType E = Error>
		requires NotReference<E>
		[[nodiscard]] inline auto
		ok_or_else(std::function<E()> error_generator) noexcept -> Result<T, E>;

		/// @brief Returns the contained `T`, consuming this `Option`.
		/// If this is not `Some`, then `std::terminate` is called
		///
		/// @return The contained `T`
		[[nodiscard]] constexpr inline auto
		unwrap() noexcept -> T requires Movable<T> && NotPointer<T> {
			if(m_is_some) {
				auto some_ = std::move(m_some);
				m_is_some = false;
				return some_;
			}
			else {
				std::cerr << "unwrap called on a None, terminating" << std::endl;
				std::terminate();
			}
		}

		/// @brief Returns the contained `T`, consuming this `Option`.
		/// If this is not `Some`, then `std::terminate` is called
		///
		/// @return The contained `T`
		[[nodiscard]] constexpr inline auto unwrap() noexcept -> T requires Pointer<T> {
			if(m_is_some) {
				auto some = m_some;
				m_is_some = false;
				m_some = nullptr;
				return some;
			}
			else {
				std::cerr << "unwrap called on a None, terminating" << std::endl;
				std::terminate();
			}
		}

		/// @brief Returns the contained `T`, consuming this `Option`.
		/// If this is not `Some`, then `std::terminate` is called
		///
		/// @return The contained `T`
		[[nodiscard]] constexpr inline auto
		unwrap() noexcept -> T requires NotMovable<T> && NotPointer<T> {
			if(m_is_some) {
				auto some = m_some;
				m_is_some = false;
				return some;
			}
			else {
				std::cerr << "unwrap called on a None, terminating" << std::endl;
				std::terminate();
			}
		}

		/// @brief Returns the contained `T` if this is `Some`, consuming this
		/// `Option`. If this is not `Some`, then returns `default_value`.
		///
		/// @param default_value - The value to return if this is `None`
		///
		/// @return The contained `T` if this is `Some`, or `default_value`
		[[nodiscard]] constexpr inline auto unwrap_or(const T& default_value) noexcept -> T {
			if(m_is_some) {
				return unwrap();
			}
			else {
				return default_value;
			}
		}

		/// @brief Returns the contained `T` if this is `Some`, consuming this
		/// `Option`. If this is not `Some`, then returns `default_value`.
		///
		/// @param default_value - The value to return if this is `None`
		///
		/// @return The contained `T` if this is `Some`, or `default_value`
		[[nodiscard]] constexpr inline auto unwrap_or(T&& default_value) noexcept -> T {
			if(m_is_some) {
				return unwrap();
			}
			else {
				return std::forward<T>(default_value);
			}
		}

		/// @brief Returns the contained `T` if this is `Some`, consuming this
		/// `Option<T>`. If this is not `Some`, then returns the value generated by
		/// `default_generator`.
		///
		/// @param default_generator - The function to generate the value returned if
		/// this is `None`
		///
		/// @return The contained `T` if this is `Some`, or the value generated by
		/// `default_generator`
		[[nodiscard]] inline auto
		unwrap_or_else(std::function<T()> default_generator) noexcept -> T {
			if(m_is_some) {
				return unwrap();
			}
			else {
				return default_generator();
			}
		}

		/// @brief Similar to `unwrap`, except doesn't consume this `Option`.
		/// Returns a pointer to the mutable `T` if this is `Some`, otherwise calls
		/// `std::terminate`
		///
		/// @return A pointer (or reference if `T` is a reference) to `T`
		[[nodiscard]] constexpr inline auto get_mut() noexcept {
			if(m_is_some) {
				if constexpr(std::is_pointer_v<T>) {
					return m_some;
				}
				else {
					return &m_some;
				}
			}
			else {
				std::cerr << "get_mut called on a None, terminating" << std::endl;
				std::terminate();
			}
		}

		/// @brief Similar to `unwrap`, except doesn't consume this `Option`.
		/// Returns a pointer to the const `T` if this is `Some`, otherwise calls
		/// `std::terminate`
		///
		/// @return A pointer (or reference if `T` is a reference) to const `T`
		[[nodiscard]] constexpr inline auto get_const() const noexcept {
			if(m_is_some) {
				if constexpr(std::is_pointer_v<T>) {
					return m_some;
				}
				else {
					return &m_some;
				}
			}
			else {
				std::cerr << "get_const called on a None, terminating" << std::endl;
				std::terminate();
			}
		}

		explicit constexpr operator bool() const noexcept {
			return m_is_some;
		}

		constexpr auto operator=(const Option& option) -> Option& requires Copyable<T>
		= default;
		constexpr auto operator=(Option&& option) noexcept -> Option& requires Movable<T> {
			this->m_is_some = option.m_is_some;
			if(option.m_is_some) {
				option.m_is_some = false;
				if constexpr(std::is_pointer_v<T>) {
					this->m_some = option.m_some;
				}
				else if constexpr(std::is_move_constructible_v<T>) {
					this->m_some = std::move(option.m_some);
				}
			}
			return *this;
		}

	  private :
		  /// value type constructor
		  constexpr explicit Option(const T& some) noexcept requires Movable<T> : m_some(some),
																				  m_is_some(true) {
		}

		/// rvalue constructor
		constexpr explicit Option(T&& some) noexcept : m_some(some), m_is_some(true) {
		}

		constexpr Option() = default;

		/// The contained value
		T m_some;
		/// Whether this is `Some`
		bool m_is_some = false;

		APEX_DECLARE_NON_HEAP_ALLOCATABLE()
	};

	/// @brief Convenience shorthand for `Option<T>::Some`
	///
	/// @param some - The value to store in the `Option`
	template<Passable T>
	requires DefaultConstructible<T> && NotReference<T>
	inline constexpr auto Some(const T& some) noexcept -> Option<T> {
		return Option<T>::Some(some);
	}

	/// @brief Convenience shorthand for `Option<T>::Some`
	///
	/// @param some - The value to store in the `Option`
	template<Passable T>
	requires DefaultConstructible<T> && NotReference<T>
	inline constexpr auto Some(T&& some) noexcept -> Option<T> {
		return Option<T>::Some(std::forward<T>(some));
	}

	/// @brief Convenience shorthand for `Option<T>::None`
	inline constexpr auto None() noexcept -> NoneType {
		return none_t;
	}
#endif // OPTION

#ifndef RESULT
	#define RESULT

	template<ErrorType E = Error>
	requires NotReference<E>
	struct ErrorWrapper {
		explicit ErrorWrapper(const E& error) : m_error(error) {
		}
		explicit ErrorWrapper(E&& error) : m_error(std::forward<E>(error)) {
		}
		ErrorWrapper(const ErrorWrapper& error) noexcept requires Copyable<E>
		= default;
		ErrorWrapper(ErrorWrapper&& error) noexcept requires Movable<E>
		= default;
		auto operator=(const ErrorWrapper& error) noexcept -> ErrorWrapper& requires Copyable<E>
		= default;
		auto operator=(ErrorWrapper&& error) noexcept -> ErrorWrapper& requires Movable<E>
		= default;

		E m_error;
	};

	template<Passable T>
	requires NotReference<T> && DefaultConstructible<T>
	struct OkWrapper {
		explicit constexpr OkWrapper(const T& ok) : m_ok(ok) {
		}
		explicit constexpr OkWrapper(T&& ok) : m_ok(std::forward<T>(ok)) {
		}
		constexpr OkWrapper(const OkWrapper& ok) noexcept requires Copyable<T>
		= default;
		constexpr OkWrapper(OkWrapper&& ok) noexcept requires Movable<T>
		= default;
		constexpr auto operator=(const OkWrapper& ok) noexcept -> OkWrapper& requires Copyable<T>
		= default;
		constexpr auto operator=(OkWrapper&& ok) noexcept -> OkWrapper& requires Movable<T>
		= default;

		T m_ok;
	};

	/// @brief Represents the result of an operation that can fail.
	/// Every `Result()` is either `Ok`, indicating success and containing a value
	/// or `Err`, indicating failure and containing an `Error` type
	///
	/// @tparam T - the type contained in the case of success
	/// @tparam E - the `Error` type contained in the case of failure
	template<Passable T, ErrorType E>
	requires DefaultConstructible<T> && NotReference<T> && DefaultConstructible<E> && NotReference<
		E>
	class [[nodiscard]] Result {
	  public:
		Result(const ErrorWrapper<E>& error) noexcept : m_err(error.m_error) { // NOLINT
		}
		Result(ErrorWrapper<E>&& error) noexcept : m_err(error.m_error) { // NOLINT
		}
		constexpr Result(const OkWrapper<T>& ok) noexcept : m_is_ok(true), m_ok(ok.m_ok) { // NOLINT
		}
		constexpr Result(OkWrapper<T>&& ok) noexcept // NOLINT
			: m_is_ok(true), m_ok(std::move(ok.m_ok)) {
		}
		constexpr Result() noexcept = delete;
		constexpr Result(Result& result) = delete;
		constexpr Result(const Result& result) = delete;
		constexpr Result(Result&& result) noexcept requires Movable<T> && Movable<E> {
			this->m_handled = result.m_handled;
			result.m_handled = true;
			this->m_is_ok = result.m_is_ok;
			if(result.m_is_ok) {
				if constexpr(std::is_move_constructible_v<T>) {
					this->m_ok = std::move(result.m_ok);
				}
				else {
					this->m_ok = result.m_ok;
				}
			}
			else {
				if constexpr(std::is_move_constructible_v<E>) {
					this->m_err = std::move(result.m_err);
				}
				else {
					this->m_err = result.m_err;
				}
			}
		}

		~Result() noexcept {
			if constexpr(std::is_pointer_v<T>) {
				if(!m_handled) {
					if(m_is_ok) {
						m_ok = nullptr;
					}
					std::cerr
						<< "Unhandled Result() that must be handled being destroyed, terminating"
						<< std::endl;
					std::terminate();
				}
				else if(m_is_ok && m_ok != nullptr) {
					m_ok = nullptr;
				}
			}
			else if(!m_handled) {
				std::cerr << "Unhandled Result() that must be handled being destroyed, terminating"
						  << std::endl;
				std::terminate();
			}
		}

		/// @brief Constructs a `Result()` as the `Ok` variant, containing `ok`
		///
		/// @param ok - the success value
		///
		/// @return `Ok`
		[[nodiscard]] constexpr static inline auto Ok(const T& ok) noexcept -> Result<T, E> {
			return Result(ok);
		}

		/// @brief Constructs a `Result()` as the `Ok` variant, containing `ok`
		///
		/// @param ok - the success value
		///
		/// @return `Ok`
		[[nodiscard]] constexpr static inline auto Ok(T&& ok) noexcept -> Result<T, E> {
			return Result(std::forward<T>(ok));
		}

		/// @brief Constructs a `Result()` as the `Ok` variant, containing `ok`
		///
		/// @param ok - the success value
		///
		/// @return `Ok`
		[[nodiscard]] constexpr static inline auto Ok(OkWrapper<T>&& ok) noexcept -> Result<T, E> {
			return Result(std::forward<T>(ok));
		}

		/// @brief Constructs a `Result()` as the `Ok` variant, containing `ok`
		///
		/// @param ok - the success value
		///
		/// @return `Ok`
		[[nodiscard]] constexpr static inline auto
		Ok(const OkWrapper<T>& ok) noexcept -> Result<T, E> {
			return Result(ok);
		}

		/// @brief Constructs a `Result()` as the `Err` variant, containing `err`
		///
		/// @param err - the failure value
		///
		/// @return `Err`
		[[nodiscard]] constexpr static inline auto Err(const E& err) noexcept -> Result<T, E> {
			return Result(err);
		}

		/// @brief Constructs a `Result()` as the `Err` variant, containing `err`
		///
		/// @param err - the failure value
		///
		/// @return `Err`
		[[nodiscard]] constexpr static inline auto Err(E&& err) noexcept -> Result<T, E> {
			return Result(std::forward<E>(err));
		}

		/// @brief Constructs a `Result()` as the `Err` variant, containing `err`
		///
		/// @param err - the failure value
		///
		/// @return `Err`
		[[nodiscard]] constexpr static inline auto
		Err(const ErrorWrapper<E>& err) noexcept -> Result<T, E> {
			return Result(err);
		}

		/// @brief Constructs a `Result()` as the `Err` variant, containing `err`
		///
		/// @param err - the failure value
		///
		/// @return `Err`
		[[nodiscard]] constexpr static inline auto
		Err(ErrorWrapper<E>&& err) noexcept -> Result<T, E> {
			return Result(std::forward<ErrorWrapper<E>>(err));
		}

		/// @brief Returns `true` if this is `Ok`, `false` if this is `Err`
		///
		/// @return Whether this is `Ok`
		[[nodiscard]] constexpr auto is_ok() const noexcept -> bool {
			m_handled = true;
			return m_is_ok;
		}

		/// @brief Returns `true` if this is `Err`, `false` if this is `Ok`
		///
		/// @return Whether this is `Err`
		[[nodiscard]] constexpr inline auto is_err() const noexcept -> bool {
			m_handled = true;
			return !m_is_ok;
		}

		/// @brief Similar to `unwrap`, except doesn't consume this `Result()`.
		/// Returns a pointer to the mutable `T` if this is `Ok`, otherwise calls
		/// `std::terminate`
		///
		/// @return A pointer (or reference if `T` is a reference) to `T`
		[[nodiscard]] constexpr inline auto get_mut() noexcept {
			m_handled = true;
			if(m_is_ok) {
				if constexpr(std::is_pointer_v<T>) {
					return m_ok;
				}
				else {
					return &m_ok;
				}
			}
			else {
				std::cerr << "get_mut called on an Error result, terminating" << std::endl;
				std::terminate();
			}
		}

		/// @brief Similar to `unwrap`, except doesn't consume this `Result()`.
		/// Returns a pointer to the const `T` if this is `Ok`, otherwise calls
		/// `std::terminate`
		///
		/// @return A pointer (or reference if `T` is a reference) to const `T`
		[[nodiscard]] constexpr inline auto get_const() const noexcept {
			m_handled = true;
			if(m_is_ok) {
				if constexpr(std::is_pointer_v<T>) {
					return m_ok;
				}
				else {
					return &m_ok;
				}
			}
			else {
				std::cerr << "get_const called on an Error result, terminating" << std::endl;
				std::terminate();
			}
		}

		/// @brief Returns the contained `T`, consuming this `Result()`.
		/// If this is not `Ok`, then `std::terminate` is called
		///
		/// @return The contained `T`
		[[nodiscard]] constexpr inline auto
		unwrap() noexcept -> T requires Movable<T> && NotPointer<T> {
			m_handled = true;
			if(m_is_ok) {
				auto ok_ = std::move(m_ok);
				m_is_ok = false;
				return ok_;
			}
			else {
				std::cerr << "unwrap called on an Error result, terminating" << std::endl;
				std::terminate();
			}
		}

		/// @brief Returns the contained `T`, consuming this `Result()`.
		/// If this is not `Ok`, then `std::terminate` is called
		///
		/// @return The contained `T`
		[[nodiscard]] constexpr inline auto unwrap() noexcept -> T requires Pointer<T> {
			m_handled = true;
			if(m_is_ok) {
				auto _ok = m_ok;
				m_is_ok = false;
				m_ok = nullptr;
				return _ok;
			}
			else {
				std::cerr << "unwrap called on an Error result, terminating" << std::endl;
				std::terminate();
			}
		}

		/// @brief Returns the contained `T`, consuming this `Result()`.
		/// If this is not `Ok`, then `std::terminate` is called
		///
		/// @return The contained `T`
		[[nodiscard]] constexpr inline auto
		unwrap() noexcept -> T requires NotMovable<T> && NotPointer<T> {
			m_handled = true;
			if(m_is_ok) {
				auto _ok = m_ok;
				m_is_ok = false;
				return _ok;
			}
			else {
				std::cerr << "unwrap called on an Error result, terminating" << std::endl;
				std::terminate();
			}
		}

		/// @brief Returns the contained `T` if this is `Ok`, consuming this `Result()`
		/// If this is not `Ok`, then returns `default_value`
		///
		/// @param default_value - The value to return if this is `Err`
		///
		/// @return The contained `T` if this is `Ok`, or `default_value`
		[[nodiscard]] constexpr inline auto unwrap_or(const T& default_value) noexcept -> T {
			m_handled = true;
			if(m_is_ok) {
				return unwrap();
			}
			else {
				return default_value;
			}
		}

		/// @brief Returns the contained `T` if this is `Ok`, consuming this `Result()`
		/// If this is not `Ok`, then returns `default_value`
		///
		/// @param default_value - The value to return if this is `Err`
		///
		/// @return The contained `T` if this is `Ok`, or `default_value`
		[[nodiscard]] constexpr inline auto unwrap_or(T&& default_value) noexcept -> T {
			m_handled = true;
			if(m_is_ok) {
				return unwrap();
			}
			else {
				return std::forward<T>(default_value);
			}
		}

		/// @brief Returns the contained `T` if this is `Ok`, consuming this `Result()`.
		/// If this is not `Ok`, then returns the value generated by `default_generator`
		///
		/// @param default_generator - The function to generate the value returned if this is
		/// `Err`
		///
		/// @return  The contained `T` if this is `Ok`, or the value generated by
		/// `default_generator`
		[[nodiscard]] inline auto
		unwrap_or_else(std::function<T()> default_generator) noexcept -> T {
			m_handled = true;
			if(m_is_ok) {
				return unwrap();
			}
			else {
				return default_generator();
			}
		}

		/// @brief Returns the contained `E` if this is `Err`, consuming this `Result()`.
		/// If this is not `Err`, then `std::terminate` is called
		///
		/// @return The contained `E`
		[[nodiscard]] constexpr inline auto
		unwrap_err() noexcept -> E requires Movable<E> && NotPointer<E> {
			m_handled = true;
			if(!m_is_ok) {
				auto err_ = std::move(m_err);
				return err_;
			}
			else {
				std::cerr << "unwrap_err called on an Ok result, terminating" << std::endl;
				std::terminate();
			}
		}

		/// @brief Returns the contained `E` if this is `Err`, consuming this `Result()`.
		/// If this is not `Err`, then `std::terminate` is called
		///
		/// @return The contained `E`
		[[nodiscard]] constexpr inline auto unwrap_err() noexcept -> E requires Pointer<E> {
			m_handled = true;
			if(!m_is_ok) {
				auto _err = m_err;
				m_err = nullptr;
				return _err;
			}
			else {
				std::cerr << "unwrap_err called on an Ok result, terminating" << std::endl;
				std::terminate();
			}
		}

		/// @brief Returns the contained `E` if this is `Err`, consuming this `Result()`.
		/// If this is not `Err`, then `std::terminate` is called
		///
		/// @return The contained `E`
		[[nodiscard]] constexpr inline auto
		unwrap_err() noexcept -> E requires NotMovable<E> && NotPointer<E> {
			m_handled = true;
			if(!m_is_ok) {
				auto _err = m_err;
				return _err;
			}
			else {
				std::cerr << "unwrap_err called on an Ok result, terminating" << std::endl;
				std::terminate();
			}
		}

		/// @brief Converts this `Result()<T, E>` to an `Option<T>`,
		/// consuming this `Result()` and discarding the error, if any
		///
		/// @return `Option<T>`
		[[nodiscard]] constexpr inline auto ok() noexcept -> Option<T>
		requires Movable<T> && NotPointer<T> {
			m_handled = true;
			if(m_is_ok) {
				auto ok_ = std::move(m_ok);
				m_is_ok = false;
				return Some(std::move(ok_));
			}
			else {
				return None();
			}
		}

		/// @brief Converts this `Result()<T, E>` to an `Option<T>`,
		/// consuming this `Result()` and discarding the error, if any
		///
		/// @return `Option<T>`
		[[nodiscard]] constexpr inline auto ok() noexcept -> Option<T>
		requires Pointer<T> {
			m_handled = true;
			if(m_is_ok) {
				auto _ok = m_ok;
				m_is_ok = false;
				m_ok = nullptr;
				return Some(_ok);
			}
			else {
				return None();
			}
		}

		/// @brief Converts this `Result()<T, E>` to an `Option<T>`,
		/// consuming this `Result()` and discarding the error, if any
		///
		/// @return `Option<T>`
		[[nodiscard]] constexpr inline auto ok() noexcept -> Option<T>
		requires NotMovable<T> && NotPointer<T> {
			m_handled = true;
			if(m_is_ok) {
				auto _ok = m_ok;
				m_is_ok = false;
				return Some(_ok);
			}
			else {
				return None();
			}
		}

		/// @brief Converts this `Result()<T, E>` to an `Option<E>`,
		/// consuming this `Result()` and discarding the success value, if any
		///
		/// @return `Option<E>`
		[[nodiscard]] constexpr inline auto err() noexcept -> Option<E>
		requires Movable<E> && NotPointer<E> {
			m_handled = true;
			if(!m_is_ok) {
				auto err_ = std::move(m_err);
				return Some(std::move(err_));
			}
			else {
				m_is_ok = false;
				return None();
			}
		}

		/// @brief Converts this `Result()<T, E>` to an `Option<E>`,
		/// consuming this `Result()` and discarding the success value, if any
		///
		/// @return `Option<E>`
		[[nodiscard]] constexpr inline auto err() noexcept -> Option<E>
		requires Pointer<E> {
			m_handled = true;
			if(!m_is_ok) {
				auto _err = m_err;
				m_err = nullptr;
				return Some(_err);
			}
			else {
				m_is_ok = false;
				return None();
			}
		}

		/// @brief Converts this `Result()<T, E>` to an `Option<E>`,
		/// consuming this `Result()` and discarding the success value, if any
		///
		/// @return `Option<E>`
		[[nodiscard]] constexpr inline auto err() noexcept -> Option<E>
		requires NotMovable<E> && NotPointer<E> {
			m_handled = true;
			if(!m_is_ok) {
				auto _err = m_err;
				return Some(_err);
			}
			else {
				m_is_ok = false;
				return None();
			}
		}

		/// @brief Maps this `Result()<T, E>` to a `Result()<U, E>`,
		/// returning `Ok(U)` (mapped by `map_func`) if this is an `Ok`,
		/// or `Err` if this is `Err`
		///
		/// @tparam U - The type to map to
		/// @param map_func - The function to perform the mapping
		///
		/// @return `Ok(U)` if this is `Ok`, or `Err`
		template<Passable U>
		requires NotReference<U>
		[[nodiscard]] inline auto
		map(std::function<U(const T&)> map_func) const noexcept -> Result<U, E>
		requires NotPointer<T> {
			m_handled = true;
			if(m_is_ok) {
				return Result<U, E>::Ok(map_func(m_ok));
			}
			else {
				return Result<U, E>::Err(m_err);
			}
		}

		/// @brief Maps this `Result()<T, E>` to a `Result()<U, E>`,
		/// returning `Ok(U)` (mapped by `map_func`) if this is an `Ok`,
		/// or `Err` if this is `Err`
		///
		/// @tparam U - The type to map to
		/// @param map_func - The function to perform the mapping
		///
		/// @return `Ok(U)` if this is `Ok`, or `Err`
		template<Passable U>
		requires NotReference<U>
		[[nodiscard]] inline auto
		map(std::function<U(const T)> map_func) const noexcept -> Result<U, E>
		requires Pointer<T> {
			m_handled = true;
			if(m_is_ok) {
				return Result<U, E>::Ok(map_func(m_ok));
			}
			else {
				return Result<U, E>::Err(m_err);
			}
		}

		/// @brief Maps this `Result()` to a `U`,
		/// returning `U` (mapped by `map_func`) if this is `Ok`,
		/// or `default_value` if this is `Err`
		///
		/// @tparam U - The type to map to
		/// @param map_func - The function to perform the mapping
		/// @param default_value - The default value
		///
		/// @return The result of the mapping if this is `Ok`, or `default_value`
		template<Passable U>
		[[nodiscard]] inline auto
		map_or(std::function<U(const T&)> map_func, const U& default_value) const noexcept
			-> U requires NotPointer<T> {
			m_handled = true;
			if(m_is_ok) {
				return map_func(m_ok);
			}
			else {
				return default_value;
			}
		}

		/// @brief Maps this `Result()` to a `U`,
		/// returning `U` (mapped by `map_func`) if this is `Ok`,
		/// or `default_value` if this is `Err`
		///
		/// @tparam U - The type to map to
		/// @param map_func - The function to perform the mapping
		/// @param default_value - The default value
		///
		/// @return The result of the mapping if this is `Ok`, or `default_value`
		template<Passable U>
		[[nodiscard]] inline auto
		map_or(std::function<U(const T)> map_func, const U& default_value) const noexcept
			-> U requires Pointer<T> {
			m_handled = true;
			if(m_is_ok) {
				return map_func(m_ok);
			}
			else {
				return default_value;
			}
		}

		/// @brief Maps this `Result()` to a `U`,
		/// returning `U` (mapped by `map_func`) if this is `Ok`,
		/// or `default_value` if this is `Err`
		///
		/// @tparam U - The type to map to
		/// @param map_func - The function to perform the mapping
		/// @param default_value - The default value
		///
		/// @return The result of the mapping if this is `Ok`, or `default_value`
		template<Passable U>
		[[nodiscard]] inline auto
		map_or(std::function<U(const T&)> map_func, U&& default_value) const noexcept
			-> U requires NotPointer<T> {
			m_handled = true;
			if(m_is_ok) {
				return map_func(m_ok);
			}
			else {
				return std::forward<U>(default_value);
			}
		}

		/// @brief Maps this `Result()` to a `U`,
		/// returning `U` (mapped by `map_func`) if this is `Ok`,
		/// or `default_value` if this is `Err`
		///
		/// @tparam U - The type to map to
		/// @param map_func - The function to perform the mapping
		/// @param default_value - The default value
		///
		/// @return The result of the mapping if this is `Ok`, or `default_value`
		template<Passable U>
		[[nodiscard]] inline auto
		map_or(std::function<U(const T)> map_func, U&& default_value) const noexcept
			-> U requires Pointer<T> {
			m_handled = true;
			if(m_is_ok) {
				return map_func(m_ok);
			}
			else {
				return std::forward<U>(default_value);
			}
		}

		/// @brief Maps this `Result()` to a `U`,
		/// returning `U` (mapped by `map_func`) if this is `Ok`,
		/// or the value returned by `default_generator` if this is `Err`
		///
		/// @tparam U - The type to map to
		/// @param map_func - The function to perform the mapping
		/// @param default_generator - The function to generate the default value
		///
		/// @return The result of the mapping if this is `Ok`,
		///			or the value returned by `default_generator`
		template<Passable U>
		[[nodiscard]] inline auto map_or_else(std::function<U(const T&)> map_func,
											  std::function<U()> default_generator) const noexcept
			-> U requires NotPointer<T> {
			m_handled = true;
			if(m_is_ok) {
				return map_func(m_ok);
			}
			else {
				return default_generator();
			}
		}

		/// @brief Maps this `Result()` to a `U`,
		/// returning `U` (mapped by `map_func`) if this is `Ok`,
		/// or the value returned by `default_generator` if this is `Err`
		///
		/// @tparam U - The type to map to
		/// @param map_func - The function to perform the mapping
		/// @param default_generator - The function to generate the default value
		///
		/// @return The result of the mapping if this is `Ok`,
		///			or the value returned by `default_generator`
		template<Passable U>
		[[nodiscard]] inline auto
		map_or_else(std::function<U(const T)> map_func,
					std::function<U()> default_generator) const noexcept -> U requires Pointer<T> {
			m_handled = true;
			if(m_is_ok) {
				return map_func(m_ok);
			}
			else {
				return default_generator();
			}
		}

		/// @brief Maps this `Result()<T, E>` to a `Result()<T, F>`,
		/// returning `Ok` if this is `Ok`,
		/// or `Err(F)` (mapped by `map_func`) if this is `Err`
		///
		/// @tparam F - The type to map to
		///				Must be an `Error` type (`hyperion::utils::Error`)
		/// @param map_func - The function to perform the mapping
		///
		/// @return `Ok` if this is `Ok`, or `Err(F)` if this is `Err`
		template<ErrorType F = Error>
		requires NotReference<F>
		[[nodiscard]] inline auto
		map_err(std::function<F(const E&)> map_func) const noexcept -> Result<T, F>
		requires NotPointer<E> {
			m_handled = true;
			if(!m_is_ok) {
				return Result<T, F>::Err(map_func(m_err));
			}
			else {
				return Result<T, F>::Ok(m_ok);
			}
		}

		/// @brief Maps this `Result()<T, E>` to a `Result()<T, F>`,
		/// returning `Ok` if this is `Ok`,
		/// or `Err(F)` (mapped by `map_func`) if this is `Err`
		///
		/// @tparam F - The type to map to
		///				Must be an `Error` type (`hyperion::utils::Error`)
		/// @param map_func - The function to perform the mapping
		///
		/// @return `Ok` if this is `Ok`, or `Err(F)` if this is `Err`
		template<ErrorType F = Error>
		requires NotReference<F>
		[[nodiscard]] inline auto
		map_err(std::function<F(const E)> map_func) const noexcept -> Result<T, F>
		requires Pointer<E> {
			m_handled = true;
			if(!m_is_ok) {
				return Result<T, F>::Err(map_func(m_err));
			}
			else {
				return Result<T, F>::Ok(m_ok);
			}
		}

		constexpr operator bool() const noexcept { // NOLINT
			m_handled = true;
			return m_is_ok;
		}

		auto operator=(const Result& result) -> Result& = delete;
		constexpr auto
		operator=(Result&& result) noexcept -> Result& requires Movable<T> && Movable<E> {
			this->m_handled = result.m_handled;
			result.m_handled = true;
			this->m_is_ok = result.m_is_ok;
			if(result.m_is_ok) {
				result.m_is_ok = false;
				if constexpr(std::is_move_constructible_v<T>) {
					this->m_ok = std::move(result.m_ok);
				}
				else {
					this->m_ok = result.m_ok;
				}
			}
			else {
				if constexpr(std::is_move_constructible_v<E>) {
					this->m_err = std::move(result.m_err);
				}
				else {
					this->m_err = result.m_err;
				}
			}
			return *this;
		}

	  private :
		  /// value type constructor
		  constexpr explicit Result(const T& ok) noexcept
		  : m_is_ok(true), m_ok(ok) {
		}

		/// rvalue contructor
		constexpr explicit Result(T&& ok) noexcept : m_is_ok(true), m_ok(ok) {
		}

		/// value type constructor
		constexpr explicit Result(const E& err) noexcept : m_err(err) {
		}

		/// rvalue contructor
		constexpr explicit Result(E&& err) noexcept : m_err(err) {
		}

		/// whether this is `Ok` (`true`) or `Err` (`false`)
		bool m_is_ok = false;
		/// whether this `Result()` has been handled
		mutable bool m_handled = false;
		/// the `Ok` value
		T m_ok;
		/// The `Err` value
		E m_err;

		APEX_DECLARE_NON_HEAP_ALLOCATABLE()
	};

	/// @brief Convenience shorthand for `Result()<T, E>::Ok`
	///
	/// @param ok - The value to store in the `Result()` representing success
	template<Passable T>
	requires NotReference<T> && DefaultConstructible<T>
	inline static constexpr auto Ok(const T& ok) noexcept -> OkWrapper<T> {
		return OkWrapper<T>(ok);
	}

	/// @brief Convenience shorthand for `Result()<T, E>::Ok`
	///
	/// @param ok - The value to store in the `Result()` representing success
	template<Passable T>
	requires NotReference<T> && DefaultConstructible<T>
	inline static constexpr auto Ok(T&& ok) noexcept -> OkWrapper<T> {
		return OkWrapper<T>(std::forward<T>(ok));
	}

	/// @brief Convenience shorthand for `Result()<T, E>::Err`
	///
	/// @param err - The value to store in the `Result()` representing failure
	template<ErrorType E = Error>
	requires NotReference<E> && DefaultConstructible<E>
	inline static constexpr auto Err(const E& err) noexcept -> ErrorWrapper<E> {
		return ErrorWrapper<E>(err);
	}

	/// @brief Convenience shorthand for `Result()<T, E>::Err`
	///
	/// @param err - The value to store in the `Result()` representing failure
	template<ErrorType E = Error>
	requires NotReference<E> && DefaultConstructible<E>
	inline static constexpr auto Err(E&& err) noexcept -> ErrorWrapper<E> {
		return ErrorWrapper<E>(std::forward<E>(err));
	}

#endif // RESULT

#ifndef OPTION_IMPL
	#define OPTION_IMPL
	/// @brief Converts this `Option<T>` to a `Result()`, consuming this
	/// `Option<T>`. Returns `Ok(T)` if this is `Some` or `Err(error)` if this is
	/// `None`
	///
	/// @tparam E - The type to return if this is `None`.
	///				Must be an `Error` type (`hyperion::utils::Error`)
	/// @param error - The `Error` to return if this `Option<T>` is `None`
	///
	/// @return `Ok(T)` if this is `Some`, `Err(error)` if this is `None`
	template<Passable T>
	requires NotReference<T>
	[[nodiscard]] constexpr inline auto
	Option<T>::ok_or(const ErrorType auto& error) noexcept -> Result<T, decltype(error)>
	requires NotReference<std::remove_reference_t<decltype(error)>> {
		if(m_is_some) {
			return Ok(unwrap());
		}
		else {
			return Err(error);
		}
	}

	/// @brief Converts this `Option<T>` to a `Result()`, consuming this
	/// `Option<T>`. Returns `Ok(T)` if this is `Some` or `Err(error)` if this is
	/// `None`
	///
	/// @tparam E - The type to return if this is `None`.
	///				Must be an `Error` type (`hyperion::utils::Error`)
	/// @param error - The `Error` to return if this `Option<T>` is `None`
	///
	/// @return `Ok(T)` if this is `Some`, `Err(error)` if this is `None`
	template<Passable T>
	requires NotReference<T>
	[[nodiscard]] constexpr inline auto Option<T>::ok_or(ErrorType auto&& error) noexcept
		-> Result<T, std::remove_reference_t<decltype(error)>> {
		using E = std::remove_reference_t<decltype(error)>;
		if(m_is_some) {
			return Ok(unwrap());
		}
		else {
			return Err(std::forward<E>(error));
		}
	}

	/// @brief Converts this `Option<T>` to a `Result()<T, E>`, consuming this
	/// `Option<T>`. Returns `Ok(T)` if this is `Some`, or `Err(E)` (where E is
	/// generated by `error_generator`) if this is `None`
	///
	/// @tparam E - The type to return if this is `None`.
	///				Must be an `Error` type (`hyperion::utils::Error`)
	/// @param error_generator - The function to generate the `Error` value
	///
	/// @return `Ok(T)` if this is `Some`, `Err(E)` if this is `None`
	template<Passable T>
	requires NotReference<T>
	template<ErrorType E>
	requires NotReference<E>
	[[nodiscard]] inline auto
	Option<T>::ok_or_else(std::function<E()> error_generator) noexcept -> Result<T, E> {
		if(m_is_some) {
			return Ok(unwrap());
		}
		else {
			return Err(error_generator());
		}
	}
#endif // OPTION_IMPL
	IGNORE_UNUSED_MACROS_STOP
	IGNORE_UNUSED_TEMPLATES_STOP
	IGNORE_PADDING_STOP
} // namespace hyperion::utils
