/// @brief `Option` represents an optional value
#pragma once

#include "Concepts.h"
#include "Error.h"
#include "FmtIO.h"
#include "HyperionDef.h"
#include "Ignore.h"
#include "option/OptionData.h"
#include "option/SomeFWD.h"
#include "result/Err.h"
#include "result/Ok.h"

namespace hyperion {
	template<typename T, typename E>
	class [[nodiscard]] Result;

	using option::None;

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
	/// responsibility of the user to call the destructor on a pointer contained in
	/// an `Option`. To have an `Option` take ownership of a pointer, use
	/// `std::unique_ptr` instead of a raw pointer.
	///
	/// @tparam T - The type of the potentially-contained value
	IGNORE_PADDING_START
	template<typename T>
	class [[nodiscard]] Option final : private option::OptionData<T> {
	  public:
		using OptionData = option::OptionData<T>;
		using type = typename OptionData::type;
		using pointer = typename OptionData::pointer;
		using pointer_to_const = typename OptionData::pointer_to_const;
		using reference = typename OptionData::reference;
		using const_reference = typename OptionData::const_reference;
		using rvalue_reference = typename OptionData::extracted;

		/// @brief Default Constructor
		constexpr Option() noexcept = default;
		// clang-format off

		/// lvalue constructor
		constexpr explicit Option(const T& some) noexcept
			requires concepts::NoexceptCopyConstructible<T>
			: OptionData(some)
		{
		}
		/// rvalue constructor
		constexpr explicit Option(T&& some) noexcept
			requires concepts::NoexceptMoveConstructible<T> && concepts::NotReference<T>
			: OptionData(std::move(some))
		{
		}
		/// @brief emplace constructor
		///
		/// @tparam Args - The types of the arguments to construct the `T` from
		/// @param args - The arguments to construct the `T` from
		template<typename... Args>
		requires concepts::ConstructibleFrom<T, Args...> && concepts::NotReference<T>
		constexpr explicit Option(Args&&... args) noexcept // NOLINT
			: OptionData(std::forward<Args>(args)...) {
		}
		// clang-format on

		/// @brief Constructs an `Option` as a `None`
		constexpr Option(None none) noexcept { // NOLINT
			ignore(none);
		}
		/// @brief Copy Constructor
		constexpr Option(const Option& option) noexcept(
			concepts::NoexceptCopyConstructible<T>) requires concepts::CopyConstructible<T>
			: OptionData(static_cast<const OptionData&>(option)) {
		}
		/// @brief Move Constructor
		constexpr Option(Option&& option) noexcept(
			concepts::NoexceptMoveConstructible<T>) requires concepts::MoveConstructible<T>
			: OptionData(static_cast<OptionData&&>(option)) {
		}

		/// @brief Destructor
		constexpr ~Option() noexcept(concepts::NoexceptDestructible<T>) = default;

		/// @brief Constructs an `Option` as the `Some` variant containing `some`
		///
		/// @param some - The value to store in this `Option`
		///
		/// @return `Some(some)`
		[[nodiscard]] constexpr static inline auto
		Some(const T& some) noexcept -> Option requires concepts::NotReference<T> {
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
		/// Constructs a `T` in place in the `Option` from the provided arguments,
		/// `args`.
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
			return this->has_value();
		}

		/// @brief Returns whether this `Option` is the `None` variant
		///
		/// @return true if this is `None`, false otherwise
		[[nodiscard]] constexpr inline auto is_none() const noexcept -> bool {
			return !is_some();
		}

		/// @brief Maps this `Option` to another one, with a potentially different
		/// `Some` type.
		///
		/// If this is the `Some` variant, returns `Some(map_func(T))`.
		/// Otherwise, this returns `None`
		///
		/// @tparam F - The type of the invocable mapping `T`
		/// @tparam U - The type that `F` maps `T` to. This is deduced. Do not
		/// explicitly provide this.
		/// @param map_func - The invocable that performs the mapping
		///
		/// @return `Some(U)` if this is Some, or `None` if this is `None`
		template<typename F,
				 typename U = decltype(std::declval<F>()(std::declval<const_reference>()))>
		requires concepts::InvocableRConst<U, F, T>
		[[nodiscard]] inline auto map(F&& map_func) const noexcept -> Option<U> {
			// the invocable checks above are probably redundant because of the inferred
			// template parameters, but we'll keep them for completeness sake and
			// clarity of requirements
			if(is_some()) {
				return Option<U>::Some(std::forward<F>(map_func)(this->get()));
			}
			else {
				return Option<U>::None();
			}
		}

		/// @brief Maps this `Option` to a `U`
		///
		/// If this is the `Some` variant, returns `map_func(T)`.
		/// Otherwise, this returns `default_value`
		//
		/// @tparam F - The type of the invocable mapping `T`
		/// @tparam U - The type that `F` maps `T` to. This is deduced. Do not
		/// explicitly provide this.
		/// @param map_func - The invocable that performs the mapping
		/// @param default_value - The default value
		///
		/// @return The result of the mapping if this is `Some`, or `default_value`
		template<typename F, typename U>
		requires concepts::InvocableRConst<U, F, T>
		[[nodiscard]] inline auto map_or(F&& map_func, U&& default_value) const noexcept -> U {
			if(is_some()) {
				return std::forward<F>(map_func)(this->get());
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
		/// @tparam U - The type that `F` maps `T` to. This is deduced. Do not
		/// explicitly provide this.
		/// @tparam V - The type that `G` generates. This is deduced. Do not
		/// explicitly provide this.
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
				 typename U = decltype(std::declval<F>()(std::declval<const_reference>())),
				 typename V = decltype(std::declval<G>()())>
		requires concepts::Same<U, V> && concepts::InvocableRConst<U, F, T> && concepts::
			InvocableR<V, G>
		[[nodiscard]] inline auto
		map_or_else(F&& map_func, G&& default_generator) const noexcept -> U {
			// the invocable checks above are probably redundant because of the inferred
			// template parameters, but we'll keep them for completeness sake and
			// clarity of requirements
			if(is_some()) {
				return std::forward<F>(map_func)(this->get());
			}
			else {
				return std::forward<G>(default_generator)();
			}
		}

		/// @brief Matches this `Option` to a consuming function, depending on whether it is the
		/// `Some` or `None` variant.
		///
		/// If this is the `Some` variant, the contained `T` is moved out of this `Option` and
		/// `some_func` is called with it as its parameter.
		/// Otherwise, `none_func` is called.
		///
		/// @tparam SomeFunc - The type of the function to consume the contained value with if this
		/// is `Some`
		/// @tparam NoneFunc - The type of the function to call if this is `None`
		///
		/// @param some_func - The function to consume the contained value with if this is `Some`
		/// @param none_func - The function to call if this is `None`
		/// @return The return value of `some_func` if this is `Some`, otherwise the return value
		/// of `none_func`
		/// @note The returned type of `some_func` and `none_func` must be the same
		template<typename SomeFunc,
				 typename NoneFunc,
				 typename R1 = decltype(std::declval<SomeFunc>()(std::declval<rvalue_reference>())),
				 typename R2 = decltype(std::declval<NoneFunc>()())>
		requires concepts::Same<R1, R2> && concepts::
			InvocableR<void, SomeFunc, rvalue_reference> && concepts::InvocableR<void, NoneFunc>
		inline auto match(SomeFunc&& some_func, NoneFunc&& none_func) noexcept -> R1 {
			if(is_some()) {
				return std::forward<SomeFunc>(some_func)(std::forward<T>(this->extract()));
			}
			else {
				return std::forward<NoneFunc>(none_func)();
			}
		}

		/// @brief Continues control flow into `func` if this is the `Some` variant,
		/// otherwise returns `None`.
		///
		/// @tparam F - The type of invocable to call if this is `Some`
		/// @tparam U - The type of the `Some` variant of the `Option` returned by
		/// `F`. This is deduced. Don't explicitly provide this.
		/// @param func - The invocable to call if this is `Some`
		///
		/// @return `func()` if this is `Some`, otherwise `None`
		template<typename F,
				 typename U = typename decltype(std::declval<F>(
					 std::declval<rvalue_reference>()))::rvalue_reference,
				 typename R
				 = std::conditional_t<std::is_rvalue_reference_v<U>, std::remove_reference_t<U>, U>>
		requires concepts::InvocableRMut<Option<R>, F, rvalue_reference>
		[[nodiscard]] inline auto and_then(F&& func) noexcept -> Option<R> {
			// the invocable checks above are probably redundant because of the inferred
			// template
			if(is_some()) {
				return std::forward<F>(func)(this->extract());
			}
			else {
				return hyperion::None();
			}
		}

		/// @brief Returns the value contained in this `Option` if this is the `Some`
		/// variant, otherwise returns `option`
		///
		/// @param option - The next option to potentially use
		///
		/// @return `Some(T)` if this is `Some`, otherwise `option`
		[[nodiscard]] inline auto or_else(const Option& option) const noexcept -> Option {
			if(is_some()) {
				return Option<T>::Some(this->extract());
			}
			else {
				return option;
			}
		}

		/// @brief Returns the value contained in this `Option` if this is the `Some`
		/// variant, otherwise returns `option`
		///
		/// @param option - The next option to potentially use
		///
		/// @return `Some(T)` if this is `Some`, otherwise `option`
		[[nodiscard]] inline auto or_else(Option&& option) const noexcept -> Option {
			if(is_some()) {
				return Option<T>::Some(this->extract());
			}
			else {
				return std::move(option);
			}
		}

		/// @brief Continues control flow into `func` if this is the `None` variant,
		/// otherwise returns the value contained in this `Option`
		///
		/// @tparam F - The type of the invocable to call if this is `None`
		/// @param func - The invocable to call if this is `None`
		///
		/// @return `func()` if this is `None`, otherwise `Some(T)`
		template<typename F>
		requires concepts::InvocableR<Option<T>, F>
		[[nodiscard]] inline auto or_else(F&& func) const noexcept -> Option {
			if(is_some()) {
				return Option<T>::Some(this->extract());
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
		///				Must be an `error::ErrorType` (`hyperion::Error`
		/// or derived from it)
		/// @param error - The error to return if this `Option<T>` is `None`
		///
		/// @return `Ok(T)` if this is `Some`, `Err(error)` if this is `None`
		template<typename E>
		[[nodiscard]] constexpr inline auto ok_or(E&& error) noexcept -> Result<T, E> {
			if(is_some()) {
				return Ok(this->extract());
			}
			else {
				return Err(std::forward<E>(error));
			}
		}

		/// @brief Converts this `Option` to a `Result` consuming this `Option`.
		///
		/// If this is the `Some` variant, returns `Ok(T)`.
		/// Otherwise, this constructs an `E` in place as an `Err` from the provided
		/// arguments.
		///
		/// @tparam E - The type to return if this is `None`.
		///				Must be an `error::ErrorType` (`hyperion::Error`
		/// or derived from it)
		/// @tparam Args - The types of the arguments from which to contruct the `E`
		/// @param args - The arguments to construct the `E` from
		///
		/// @return `Ok(T)` if this is `Some`, `Err(E(args))` if this is `None`
		template<typename E, typename... Args>
		requires concepts::ConstructibleFrom<E, Args...>
		[[nodiscard]] constexpr inline auto ok_or(Args&&... args) noexcept -> Result<T, E> {
			if(is_some()) {
				return Ok(this->extract());
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
		/// @tparam F - The invocable to generator the error value if this is the
		/// `None` variant
		/// @tparam E - The `error::ErrorType` returns by `F`. This is deduced. Don't
		/// explicitly provide it
		/// @param error_generator - The function to generate the `Error` value
		///
		/// @return `Ok(T)` if this is `Some`, `Err(error_generator())` if this is
		/// `None`
		template<typename F, typename E = decltype(std::declval<F>()())>
		requires concepts::InvocableR<E, F> && error::ErrorType<E>
		[[nodiscard]] inline auto ok_or_else(F&& error_generator) noexcept -> Result<T, E> {
			// the invocable checks above are probably redundant because of the inferred
			// template parameters, but we'll keep them for completeness sake and
			// clarity of requirements
			if(is_some()) {
				return Ok(this->extract());
			}
			else {
				return Err(std::forward<F>(error_generator)());
			}
		}

		/// @brief Returns the contained `T`, consuming this `Option`.
		///
		/// If this is not `Some` variant, then this will invoke a panic
		///
		/// @return The contained `T`
		[[nodiscard]] constexpr inline auto
		unwrap() noexcept -> rvalue_reference requires concepts::NoexceptMovable<T> {
			if(is_some()) {
				return this->extract();
			}
			else {
				panic("Option::unwrap called on a None, terminating");
			}
		}

		/// @brief Returns the contained `T` if this is `Some`, consuming this
		/// `Option`.
		///
		/// If this is not the `Some` variant, then returns `default_value`.
		///
		/// @param default_value - The value to return if this is `None`
		///
		/// @return The contained `T` if this is `Some`, or `default_value`
		[[nodiscard]] constexpr inline auto unwrap_or(T& default_value) noexcept
			-> rvalue_reference requires concepts::NotReference<T> {
			if(is_some()) {
				return unwrap();
			}
			else {
				return std::move(default_value);
			}
		}

		/// @brief Returns the contained `T` if this is `Some`, consuming this
		/// `Option`.
		///
		/// If this is not the `Some` variant, then returns `default_value`.
		///
		/// @param default_value - The value to return if this is `None`
		///
		/// @return The contained `T` if this is `Some`, or `default_value`
		[[nodiscard]] constexpr inline auto
		unwrap_or(T&& default_value) noexcept -> rvalue_reference {
			if(is_some()) {
				return unwrap();
			}
			else {
				return std::move(default_value);
			}
		}

		/// @brief Returns the contained `T` if this is `Some`, consuming this
		/// `Option`.
		///
		/// If this is not the `Some` variant, then returns `default_generator()`.
		///
		/// @param default_generator - The invocable to generate the default value if
		/// this is `None`
		///
		/// @return The contained `T` if this is `Some`, or `default_generator()`
		template<typename F>
		requires concepts::InvocableR<T, F>
		[[nodiscard]] inline auto unwrap_or_else(F&& default_generator) noexcept -> T {
			if(is_some()) {
				return unwrap();
			}
			else {
				return std::forward<F>(default_generator)();
			}
		}

		/// @brief Returns the contained `T` if this is `Some`, consuming this `Option`.
		///
		/// If this is not the `Some` variant, invokes a panic with the given `panic_message`.
		///
		/// @param panic_message - The panic message to print if this is not the `Some` variant
		///
		/// @return The contained `T` if this is `Some`
		template<typename U>
		requires concepts::Convertible<U, std::string> || concepts::Convertible<U, std::string_view>
		[[nodiscard]] inline auto expect(U&& panic_message) noexcept -> rvalue_reference {
			if(is_some()) {
				return unwrap();
			}
			else {
				panic("{}", std::forward<U>(panic_message));
			}
		}

		/// @brief Returns a pointer to the non-const `Some` value if this is `Some`, otherwise
		/// invokes a panic
		///
		/// @note If `T` is a pointer, then this returns a pointer to the underlying
		/// type of `T`, ie, it just returns `T`
		///
		/// @return A pointer to non-const `T`
		[[nodiscard]] constexpr inline auto as_mut() noexcept -> reference {
			if(is_some()) {
				if constexpr(concepts::Reference<T>) {
					return this->get().get();
				}
				else {
					return this->get();
				}
			}
			else {
				panic("Option::as_mut called on a None, terminating");
			}
		}

		/// @brief Returns a pointer to the const `Some` value if this is `Some`, otherwise invokes
		/// a panic
		///
		/// @note If `T` is a pointer, then this returns a pointer to the underlying
		/// type of `T`, ie, it just returns `const T`
		///
		/// @return A pointer to const `T`
		[[nodiscard]] constexpr inline auto as_const() const noexcept -> const_reference {
			if(is_some()) {
				return this->get();
			}
			else {
				panic("Option::as_const called on a None, terminating");
			}
		}

		/// @brief Boolean conversion operator. Returns true if this is the `Some`
		/// variant.
		///
		/// @return true if this is `Some`, false otherwise
		explicit constexpr operator bool() const noexcept {
			return is_some();
		}

		/// @brief Copy assignment operator
		constexpr auto operator=(const Option& option) noexcept(concepts::NoexceptCopyAssignable<T>)
			-> Option& requires concepts::CopyAssignable<T> {
			if(this == &option) {
				return *this;
			}

			OptionData::operator=(static_cast<const OptionData&>(option));
			return *this;
		}
		/// @brief Move assignment operator
		constexpr auto operator=(Option&& option) noexcept(concepts::NoexceptMoveAssignable<T>)
			-> Option& requires concepts::MoveAssignable<T> || concepts::Reference<T> {

			if(this == &option) {
				return *this;
			}

			OptionData::operator=(static_cast<OptionData&&>(option));
			return *this;
		}
	};
	IGNORE_PADDING_STOP

	/// @brief Convenience shorthand for `Option<T>::Some`
	///
	/// @param some - The value to store in the `Option`
	template<typename T, typename U>
	[[nodiscard]] inline constexpr auto Some(T&& some) noexcept -> Option<U> {
		return Option<U>(std::forward<T>(some));
	}

	/// @brief Convenience shorthand for `Option<T>::Some`. Constructs the `T` in place in the
	/// `Option`
	///
	/// @tparam T - The type to store in the `Option`
	/// @tparam Args - The types of arguments to pass to the `T` constructor
	/// @param args - The arguments to pass to the `T` constructor
	template<typename T, typename... Args>
	requires concepts::ConstructibleFrom<T, Args...>
	[[nodiscard]] inline constexpr auto Some(Args&&... args) noexcept -> Option<T> {
		return Option<T>(std::forward<Args>(args)...);
	}

	template<typename T>
	[[nodiscard]] inline constexpr auto
	Some(concepts::Convertible<T> auto&& some) noexcept -> Option<T> {
		return Option<T>(T(some));
	}

	template<typename T, typename... Args>
	requires concepts::ConstructibleFrom<T, Args...>
	[[nodiscard]] constexpr inline auto
	make_option(Args&&... args) noexcept(concepts::NoexceptConstructibleFrom<T, Args...>)
		-> Option<T> {
		return Option<T>(std::forward<Args>(args)...);
	}
} // namespace hyperion
