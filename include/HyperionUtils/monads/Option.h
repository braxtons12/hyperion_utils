/// @brief `Option` represents an optional value
#pragma once

#include "../Concepts.h"
#include "../Error.h"
#include "../Ignore.h"
#include "../Macros.h"
#include "../logging/fmtIncludes.h"
#include "Err.h"
#include "None.h"
#include "Ok.h"

namespace hyperion {
	using concepts::NotReference, concepts::CopyConstructible, concepts::MoveConstructible,
		concepts::NotMoveConstructible, concepts::CopyAssignable, concepts::MoveAssignable,
		concepts::NotMoveAssignable, concepts::ConstructibleFrom, concepts::Pointer,
		concepts::NotPointer, concepts::ErrorType, concepts::Invocable, concepts::InvocableR,
		concepts::Passable, concepts::Destructible, concepts::Same, concepts::InvocableRConst,
		concepts::InvocableRMut;

	template<NotReference T, ErrorType E = Error>
	requires NotReference<E>
	class [[nodiscard]] Result;

	IGNORE_PADDING_START
	/// @brief Represents an optional value.
	///
	/// Every `Option` is either `Some` and contains a value, or `None`, and does
	/// not. Useful for things such as:
	/// * * Optional members/fields
	/// * * Members/fields that can be loaned or "taken"
	/// * * Optional function arguments
	/// * * Nullable types/pointers
	/// * * Return values for functions not defined over their entire input range
	///
	/// @note While `T` can be a pointer, `Option` will
	/// __NOT__ take ownership of any pointer it contains, and as such it is the
	/// responsibility of the user to call the destructor on a pointer contained in an
	/// `Option`. To have an `Option` take ownership of a pointer, use `std::unique_ptr` instead of
	/// a raw pointer.
	/// @note `T` cannot be a reference. To make an `Option` store a reference, use
	/// `std::reference_wrapper` instead.
	///
	/// @tparam T - The type of the potentially-contained value
	template<NotReference T>
	class [[nodiscard]] Option {
	  public:
		/// @brief Default Constructor
		constexpr Option() noexcept = default;
		/// @brief Constructs an `Option` as a `None`
		constexpr Option(None none) noexcept { // NOLINT
			ignore(none);
		}
		/// @brief Copy Constructor
		constexpr Option(const Option& option) noexcept requires CopyAssignable<T> {
			if(option.m_is_some) {
				m_data.m_some = option.m_data.m_some;
				m_is_some = true;
			}
		}
		/// @brief Move Constructor
		constexpr Option(Option&& option) noexcept requires MoveAssignable<T> {
			this->m_is_some = option.m_is_some;
			if(option.m_is_some) {
				option.m_is_some = false;
				if constexpr(Pointer<T>) {
					m_data.m_some = option.m_data.m_some;
					option.m_data.m_none = 0;
				}
				else if constexpr(MoveConstructible<T>) {
					m_data.m_some = std::move(option.m_data.m_some);
					option.m_data.m_none = 0;
				}
			}
		}

		/// @brief Destructor
		~Option() noexcept {
			if(m_is_some) {
				if(Destructible<T> && NotPointer<T>) {
					m_data.m_some.~T();
				}
			}
		}

		/// @brief Constructs an `Option` as the `Some` variant containing `some`
		///
		/// @param some - The value to store in this `Option`
		///
		/// @return `Some(some)`
		[[nodiscard]] constexpr static inline auto Some(const T& some) noexcept -> Option {
			return Option(some);
		}

		/// @brief Constructs an `Option` as the `Some` variant containing `some`
		///
		/// @param some - The value to store in this `Option`
		///
		/// @return `Some(some)`
		[[nodiscard]] constexpr static inline auto Some(T&& some) noexcept -> Option {
			return Option(std::forward<T>(some));
		}

		/// @brief Constructs an `Option` as the `Some` variant.
		/// Constructs a `T` in place in the `Option` from the provided arguments, `args`.
		///
		/// @tparam Args - The types of the arguments to pass to the `T` constructor
		/// @param args - The arguments to pass to the `T` constructor
		///
		/// @return `Some(T)`
		template<typename... Args>
		[[nodiscard]] constexpr static inline auto Some(Args&&... args) noexcept -> Option {
			return Option(std::forward<Args>(args)...);
		}

		/// @brief Constructs an `Option` as the `None` variant
		///
		/// @return `None`
		[[nodiscard]] constexpr static inline auto None(None none) noexcept -> Option {
			return Option(none);
		}

		/// @brief Constructs an `Option` as the `None` variant
		///
		/// @return `None`
		[[nodiscard]] constexpr static inline auto None() noexcept -> Option {
			return Option(hyperion::None());
		}

		/// @brief Returns whether this `Option` is the `Some` variant
		///
		/// @return true if this is `Some`, false otherwise
		[[nodiscard]] constexpr inline auto is_some() const noexcept -> bool {
			return m_is_some;
		}

		/// @brief Returns whether this `Option` is the `None` variant
		///
		/// @return true if this is `None`, false otherwise
		[[nodiscard]] constexpr inline auto is_none() const noexcept -> bool {
			return !m_is_some;
		}

		/// @brief Maps this `Option` to another one, with a potentially different `Some` type.
		///
		/// If this is the `Some` variant, returns `Some(map_func(T))`.
		/// Otherwise, this returns `None`
		///
		/// @tparam F - The type of the invocable mapping `T`
		/// @tparam U - The type that `F` maps `T` to. This is deduced. Do not explicitly provide
		/// this.
		/// @param map_func - The invocable that performs the mapping
		///
		/// @return `Some(U)` if this is Some, or `None` if this is `None`
		template<typename F, typename U = decltype(std::declval<F>()(std::declval<const T&>()))>
		requires InvocableRConst<U, F, T>
		[[nodiscard]] inline auto map(F&& map_func) const noexcept -> Option<U> {
			// the invocable checks above are probably redundant because of the inferred template
			// parameters, but we'll keep them for completeness sake and clarity of requirements
			if(m_is_some) {
				return Option<U>::Some(std::forward<F>(map_func)(m_data.m_some));
			}
			else {
				return Option<U>::None(none_t);
			}
		}

		/// @brief Maps this `Option` to a `U`
		///
		/// If this is the `Some` variant, returns `map_func(T)`.
		/// Otherwise, this returns `default_value`
		//
		/// @tparam F - The type of the invocable mapping `T`
		/// @tparam U - The type that `F` maps `T` to. This is deduced. Do not explicitly provide
		/// this.
		/// @param map_func - The invocable that performs the mapping
		/// @param default_value - The default value
		///
		/// @return The result of the mapping if this is `Some`, or `default_value`
		template<typename F, typename U>
		requires InvocableRConst<U, F, T>
		[[nodiscard]] inline auto map_or(F&& map_func, U&& default_value) const noexcept -> U {
			if(m_is_some) {
				return std::forward<F>(map_func)(m_data.m_some);
			}
			else {
				return std::forward<U>(default_value);
			}
		}

		/// @brief Maps this `Option` to a `U`
		///
		/// If this is the `Some` variant, returns `map_func(T)`.
		/// Otherwise, this returns `default_generator()`
		//
		/// @tparam F - The type of the invocable mapping `T`
		/// @tparam G - The type of the invocable that generates the default value
		/// @tparam U - The type that `F` maps `T` to. This is deduced. Do not explicitly provide
		/// this.
		/// @tparam V - The type that `G` generates. This is deduced. Do not explicitly provide
		/// this.
		/// @param map_func - The function to perform the mapping
		/// @param default_generator - The function to generate the default value
		///
		/// @note `map_func` and `default_generator` must return the same type
		///
		/// @return The result of the mapping if this is `Some`,
		///			or the value returned by `default_generator` if this is
		///`None`
		template<typename F,
				 typename G,
				 typename U = decltype(std::declval<F>()(std::declval<const T&>())),
				 typename V = decltype(std::declval<G>()())>
		requires Same<U, V> && InvocableRConst<U, F, T> && InvocableR<V, G>
		[[nodiscard]] inline auto
		map_or_else(F&& map_func, G&& default_generator) const noexcept -> U {
			// the invocable checks above are probably redundant because of the inferred template
			// parameters, but we'll keep them for completeness sake and clarity of requirements
			if(m_is_some) {
				return std::forward<F>(map_func)(m_data.m_some);
			}
			else {
				return std::forward<G>(default_generator)();
			}
		}

		/// @brief Returns `option` if this is the `Some` variant, otherwise returns `None`
		///
		/// @tparam U - The type of the `Some` variant of `option`
		/// @param option - The next `Option` to potentially use
		///
		/// @return `option` if this is `Some`, `None` otherwise
		template<NotReference U>
		[[nodiscard]] inline auto and_then(const Option<U>& option) const noexcept -> Option<U> {
			if(m_is_some) {
				return option;
			}
			else {
				return hyperion::None();
			}
		}

		/// @brief Returns `option` if this is the `Some` variant, otherwise returns `None`
		///
		/// @tparam U - The type of the `Some` variant of `option`
		/// @param option - The next `Option` to potentially use
		///
		/// @return `option` if this is `Some`, `None` otherwise
		template<NotReference U>
		[[nodiscard]] inline auto and_then(Option<U>&& option) const noexcept -> Option<U> {
			if(m_is_some) {
				return option;
			}
			else {
				return hyperion::None();
			}
		}

		/// @brief Continues control flow into `func` if this is the `Some` variant, otherwise
		/// returns `None`.
		///
		/// @tparam F - The type of invocable to call if this is `Some`
		/// @tparam U - The type of the `Some` variant of the `Option` returned by `F`. This is
		/// deduced. Don't explicitly provide this.
		/// @param func - The invocable to call if this is `Some`
		///
		/// @return `func()` if this is `Some`, otherwise `None`
		template<typename F,
				 NotReference U
				 = decltype(std::declval<decltype(std::declval<F>(std::declval<T&>()))>().unwrap())>
		requires InvocableRMut<Option<U>, F, T>
		[[nodiscard]] inline auto and_then(F&& func) noexcept -> Option<U> {
			// the invocable checks above are probably redundant because of the inferred template
			if(m_is_some) {
				return std::forward<F>(func)(m_data.m_some);
			}
			else {
				return hyperion::None();
			}
		}

		/// @brief Returns the value contained in this `Option` if this is the `Some` variant,
		/// otherwise returns `option`
		///
		/// @param option - The next option to potentially use
		///
		/// @return `Some(T)` if this is `Some`, otherwise `option`
		[[nodiscard]] inline auto or_else(const Option& option) const noexcept -> Option {
			if(m_is_some) {
				return Option<T>::Some(m_data.m_some);
			}
			else {
				return option;
			}
		}

		/// @brief Returns the value contained in this `Option` if this is the `Some` variant,
		/// otherwise returns `option`
		///
		/// @param option - The next option to potentially use
		///
		/// @return `Some(T)` if this is `Some`, otherwise `option`
		[[nodiscard]] inline auto or_else(Option&& option) const noexcept -> Option {
			if(m_is_some) {
				return Option<T>::Some(m_data.m_some);
			}
			else {
				return option;
			}
		}

		/// @brief Continues control flow into `func` if this is the `None` variant, otherwise
		/// returns the value contained in this `Option`
		///
		/// @tparam F - The type of the invocable to call if this is `None`
		/// @param func - The invocable to call if this is `None`
		///
		/// @return `func()` if this is `None`, otherwise `Some(T)`
		template<typename F>
		requires InvocableR<Option<T>, F>
		[[nodiscard]] inline auto or_else(F&& func) const noexcept -> Option {
			if(m_is_some) {
				return Option<T>::Some(m_data.m_some);
			}
			else {
				return std::forward<F>(func)();
			}
		}

		/// @brief Converts this `Option` to a `Result` consuming this `Option`.
		///
		/// If this is the `Some` variant, returns `Ok(T)`.
		/// Otherwise, this returns `Err(error)`.
		///
		/// @tparam E - The type to return if this is `None`.
		///				Must be an `ErrorType` (`hyperion::Error` or derived from it)
		/// @param error - The error to return if this `Option<T>` is `None`
		///
		/// @return `Ok(T)` if this is `Some`, `Err(error)` if this is `None`
		[[nodiscard]] constexpr inline auto
		ok_or(ErrorType auto&& error) noexcept -> Result<T, std::decay_t<decltype(error)>> {
			using E = std::decay_t<decltype(error)>;
			if(m_is_some) {
				return Ok(unwrap());
			}
			else {
				return Err(std::forward<E>(error));
			}
		}

		/// @brief Converts this `Option` to a `Result` consuming this `Option`.
		///
		/// If this is the `Some` variant, returns `Ok(T)`.
		/// Otherwise, this constructs an `E` in place as an `Err` from the provided arguments.
		///
		/// @tparam E - The type to return if this is `None`.
		///				Must be an `ErrorType` (`hyperion::Error` or derived from it)
		/// @tparam Args - The types of the arguments from which to contruct the `E`
		/// @param args - The arguments to construct the `E` from
		///
		/// @return `Ok(T)` if this is `Some`, `Err(E(args))` if this is `None`
		template<ErrorType E, typename... Args>
		requires ConstructibleFrom<E, Args...>
		[[nodiscard]] constexpr inline auto ok_or(Args&&... args) noexcept -> Result<T, E> {
			if(m_is_some) {
				return Ok(unwrap());
			}
			else {
				return Err<E>(std::forward<Args>(args)...);
			}
		}

		/// @brief Converts this `Option` to a `Result`, consuming this `Option`.
		///
		/// If this is the `Some` variant, returns `Ok(T)`.
		/// Otherwise, returns `Err(error_generator)`
		///
		/// @tparam F - The invocable to generator the error value if this is the `None` variant
		/// @tparam E - The `ErrorType` returns by `F`. This is deduced. Don't explicitly provide it
		/// @param error_generator - The function to generate the `Error` value
		///
		/// @return `Ok(T)` if this is `Some`, `Err(error_generator())` if this is `None`
		template<typename F, typename E = decltype(std::declval<F>()())>
		requires InvocableR<E, F> && ErrorType<E>
		[[nodiscard]] inline auto ok_or_else(F&& error_generator) noexcept -> Result<T, E> {
			// the invocable checks above are probably redundant because of the inferred template
			// parameters, but we'll keep them for completeness sake and clarity of requirements
			if(m_is_some) {
				return Ok(unwrap());
			}
			else {
				return Err(std::forward<F>(error_generator)());
			}
		}

		/// @brief Returns the contained `T`, consuming this `Option`.
		///
		/// If this is not `Some` variant, then `std::terminate` is called
		///
		/// @return The contained `T`
		[[nodiscard]] constexpr inline auto unwrap() noexcept -> T {
			if(m_is_some) {
				m_is_some = false;
				if constexpr(Pointer<T>) {
					auto* some = m_data.m_some;
					m_data.m_some = nullptr;
					return some;
				}
				else if constexpr(CopyConstructible<T> && !MoveConstructible<T>) {
					return m_data.m_some;
				}
				else {
					return std::move(m_data.m_some);
				}
			}
			else {
				fmt::print(stderr, "unwrap called on a None, terminating\n");
				std::fflush(stderr);
				std::terminate();
			}
		}

		/// @brief Returns the contained `T` if this is `Some`, consuming this `Option`.
		///
		/// If this is not the `Some` variant, then returns `default_value`.
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

		/// @brief Returns the contained `T` if this is `Some`, consuming this `Option`.
		///
		/// If this is not the `Some` variant, then returns `default_value`.
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

		/// @brief Returns the contained `T` if this is `Some`, consuming this `Option`.
		///
		/// If this is not the `Some` variant, then returns `default_generator()`.
		///
		/// @param default_generator - The invocable to generate the default value if this is `None`
		///
		/// @return The contained `T` if this is `Some`, or `default_generator()`
		template<typename F>
		requires InvocableR<T, F>
		[[nodiscard]] inline auto unwrap_or_else(F&& default_generator) noexcept -> T {
			if(m_is_some) {
				return unwrap();
			}
			else {
				return std::forward<F>(default_generator)();
			}
		}

		/// @brief Similar to `unwrap`, except doesn't consume this `Option`.
		/// Returns a pointer to the non-const `Some` value if this is `Some`, otherwise calls
		/// `std::terminate`
		///
		/// @note If `T` is a pointer, then this returns a pointer to the underlying type of `T`,
		/// ie, it just returns `T`
		///
		/// @return A pointer to non-const `T`
		[[nodiscard]] constexpr inline auto as_mut() noexcept {
			if(m_is_some) {
				if constexpr(Pointer<T>) {
					return m_data.m_some;
				}
				else {
					return &(m_data.m_some);
				}
			}
			else {
				fmt::print(stderr, "as_mut called on a None, terminating\n");
				std::fflush(stderr);
				std::terminate();
			}
		}

		/// @brief Similar to `unwrap`, except doesn't consume this `Option`.
		/// Returns a pointer to the const `Some` value if this is `Some`, otherwise calls
		/// `std::terminate`
		///
		/// @note If `T` is a pointer, then this returns a pointer to the underlying type of `T`,
		/// ie, it just returns `const T`
		///
		/// @return A pointer to const `T`
		[[nodiscard]] constexpr inline auto
		as_const() const noexcept -> const T* requires NotPointer<T> {
			if(m_is_some) {
				return &m_data.m_some;
			}
			else {
				fmt::print(stderr, "as_const called on a None, terminating\n");
				std::fflush(stderr);
				std::terminate();
			}
		}

		/// @brief Similar to `unwrap`, except doesn't consume this `Option`.
		/// Returns a pointer to the const `Some` value if this is `Some`, otherwise calls
		/// `std::terminate`
		///
		/// @note If `T` is a pointer, then this returns a pointer to the underlying type of `T`,
		/// ie, it just returns `const T`
		///
		/// @return A pointer to const `T`
		[[nodiscard]] constexpr inline auto
		as_const() const noexcept -> T const requires Pointer<T> {
			if(m_is_some) {
				return m_data.m_some;
			}
			else {
				fmt::print(stderr, "as_const called on a None, terminating\n");
				std::fflush(stderr);
				std::terminate();
			}
		}

		/// @brief Boolean conversion operator. Returns true if this is the `Some` variant.
		///
		/// @return true if this is `Some`, false otherwise
		explicit constexpr operator bool() const noexcept {
			return m_is_some;
		}

		/// @brief Copy assignment operator
		constexpr auto operator=(const Option& option) -> Option& requires CopyAssignable<T> {
			if(this == &option) {
				return *this;
			}

			if(option.m_is_some) {
				m_is_some = true;
				m_data.m_some = option.m_data.m_some;
			}
			else {
				m_is_some = false;
				m_data.m_none = 0;
			}

			return *this;
		}
		/// @brief Move assignment operator
		constexpr auto operator=(Option&& option) noexcept -> Option& requires MoveAssignable<T> {
			this->m_is_some = option.m_is_some;
			if(option.m_is_some) {
				option.m_is_some = false;
				if constexpr(Pointer<T>) {
					m_data.m_some = option.m_data.m_some;
					option.m_data.m_none = 0;
				}
				else if constexpr(MoveConstructible<T>) {
					m_data.m_some = std::move(option.m_data.m_some);
					option.m_data.m_none = 0;
				}
			}
			return *this;
		}

	  private :
		  /// lvalue constructor
		  constexpr explicit Option(const T& some) noexcept requires CopyConstructible<T>
		  : m_data(some),
			m_is_some(true) {
		}

		/// rvalue constructor
		constexpr explicit Option(T&& some) noexcept requires MoveConstructible<T>
			: m_data(some), m_is_some(true) {
		}

		/// @brief emplace constructor
		///
		/// @tparam Args - The types of the arguments to construct the `T` from
		/// @param args - The arguments to construct the `T` from
		template<typename... Args>
		requires ConstructibleFrom<T, Args...>
		constexpr explicit Option(Args&&... args) noexcept : m_data(std::forward<Args>(args)...) {
		}

		/// The contained value
		union Data { // NOLINT
			T m_some;
			int m_none = 0;

			explicit constexpr Data(const T& some) noexcept requires CopyConstructible<T>
				: m_some(some) {
			}
			explicit constexpr Data(T&& some) noexcept requires MoveConstructible<T>
				: m_some(std::forward<T>(some)) {
			}
			template<typename... Args>
			requires ConstructibleFrom<T, Args...>
			explicit constexpr Data(Args&&... args) noexcept : m_some(std::forward<Args>(args)...) {
			}
			constexpr ~Data() noexcept { // NOLINT
			}
		} m_data;

		/// Whether this is `Some`
		bool m_is_some = false;
	};
	IGNORE_PADDING_STOP

	/// @brief Convenience shorthand for `Option<T>::Some`
	///
	/// @param some - The value to store in the `Option`
	template<NotReference T>
	inline constexpr auto Some(const T& some) noexcept -> Option<T> {
		return Option<T>::Some(some);
	}

	/// @brief Convenience shorthand for `Option<T>::Some`
	///
	/// @param some - The value to store in the `Option`
	template<NotReference T>
	inline constexpr auto Some(T&& some) noexcept -> Option<T> {
		return Option<T>::Some(std::forward<T>(some));
	}

	/// @brief Convenience shorthand for `Option<T>::Some`. Constructs the `T` in place in the
	/// `Option`
	///
	/// @tparam T - The type to store in the `Option`
	/// @tparam Args - The types of arguments to pass to the `T` constructor
	/// @param args - The arguments to pass to the `T` constructor
	template<NotReference T, typename... Args>
	requires ConstructibleFrom<T, Args...>
	inline constexpr auto Some(Args&&... args) noexcept -> Option<T> {
		return Option<T>::Some(std::forward<Args>(args)...);
	}
} // namespace hyperion
