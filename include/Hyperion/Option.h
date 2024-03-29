/// @file Option.h
/// @author Braxton Salyer <braxtonsalyer@gmail.com>
/// @brief A monadic type representing an optional value
/// @version 0.1
/// @date 2022-12-05
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
#include <Hyperion/Error.h>
#include <Hyperion/FmtIO.h>
#include <Hyperion/HyperionDef.h>
#include <Hyperion/Ignore.h>
#include <Hyperion/option/OptionData.h>
#include <Hyperion/option/SomeFWD.h>
#include <Hyperion/result/Err.h>
#include <Hyperion/result/Ok.h>

/// @ingroup error
/// @{
///	@defgroup option Option
/// `Option` is a monadic type for representing an optional value
///
/// Example:
/// @code {.cpp}
/// #include <Hyperion/Option.h>
///
/// using namespace hyperion;
///
/// inline auto checked_add(u32 left, u32 right) noexcept -> Option<u32> {
/// 	return left <= std::numeric_limits<u32>::max() - right ? Some(left + right) : None();
/// }
///
/// auto checked_sum = checked_add(25_u32, 120939_u32);
/// if(checked_sum) {
///		println("sum is: {}", checked_sum.unwrap());
/// }
/// @endcode
/// @headerfile "Hyperion/Option.h"
/// @}

#ifndef HYPERION_OPTION
	#define HYPERION_OPTION

namespace hyperion {
	#ifndef HYPERION_RESULT
	template<typename T, typename E>
	class [[nodiscard("Results of fallible operations should not be ignored")]] Result;
	#endif // HYPERION_RESULT

	using option::None;

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
	/// Example:
	/// @code {.cpp}
	/// #include <Hyperion/Option.h>
	///
	/// using namespace hyperion;
	///
	/// inline auto checked_add(u32 left, u32 right) noexcept -> Option<u32> {
	/// 	return left <= std::numeric_limits<u32>::max() - right ? Some(left + right) : None();
	/// }
	///
	/// auto checked_sum = checked_add(25_u32, 120939_u32);
	/// if(checked_sum) {
	///		println("sum is: {}", checked_sum.unwrap());
	/// }
	/// @endcode
	///
	/// @note While `T` can be a pointer, `Option` will __NOT__ take ownership of any pointer it
	/// contains, and as such it is the responsibility of the user to delete a pointer contained in
	/// an `Option`. To have an `Option` take ownership of a pointer, use a smart-pointer like
	/// `hyperion::UniquePtr` or `std::shared_ptr` instead of a raw pointer.
	///
	/// @tparam T - The type of the potentially-contained value
	/// @ingroup option
	/// @headerfile "Hyperion/Option.h"
	template<typename T>
	class Option final : private option::OptionData<T> {
	  public:
		using OptionData = option::OptionData<T>;
		/// @brief The type stored in the `Option`
		/// @ingroup option
		/// @headerfile "Hyperion/Option.h"
		using type = typename OptionData::type;
		/// @brief Pointer to the type stored in the `Option`
		/// @ingroup option
		/// @headerfile "Hyperion/Option.h"
		using pointer = typename OptionData::pointer;
		/// @brief Pointer-to-const the type stored in the `Option`
		/// @ingroup option
		/// @headerfile "Hyperion/Option.h"
		using pointer_to_const = typename OptionData::pointer_to_const;
		/// @brief Reference to the type stored in the `Option`
		/// @ingroup option
		/// @headerfile "Hyperion/Option.h"
		using reference = typename OptionData::reference;
		/// @brief Reference-to-const the type stored in the `Option`
		/// @ingroup option
		/// @headerfile "Hyperion/Option.h"
		using const_reference = typename OptionData::const_reference;
		/// @brief Rvalue reference (or equivalent) of the type stored in the `Option`
		///
		/// Rvalue reference of `type` or equivalent. E.g., for an `Option` storing a pointer, this
		/// would be the same as `type`, but for a bare type, this would be `type&&`.
		/// @ingroup option
		/// @headerfile "Hyperion/Option.h"
		using rvalue_reference = typename OptionData::extracted;

		/// @brief Default Constructor
		/// @ingroup option
		/// @headerfile "Hyperion/Option.h"
		constexpr Option() noexcept = default;

		/// @brief Constructs an `Option<T>` from the given `T`
		/// @ingroup option
		/// @headerfile "Hyperion/Option.h"
		constexpr Option(const T& some) // NOLINT (explicit)
			noexcept(concepts::NoexceptCopyConstructible<T>)
		requires concepts::CopyConstructible<T>
			: OptionData(some) {
		}
		/// @brief Constructs an `Option<T>` from the given `T`
		/// @ingroup option
		/// @headerfile "Hyperion/Option.h"
		constexpr Option(T&& some) // NOLINT (explicit)
			noexcept(concepts::NoexceptMoveConstructible<T>)
		requires concepts::MoveConstructible<T> && concepts::NotReference<T>
			: OptionData(std::move(some)) {
		}
		/// @brief Constructs an `Option<T>` by constructing the `T` in place in the `Option`
		///
		/// @tparam Args - The types of the arguments to pass to `T`'s constructor
		/// @param args - The arguments to pass to `T`'s constructor
		/// @ingroup option
		/// @headerfile "Hyperion/Option.h"
		template<typename... Args>
		requires concepts::ConstructibleFrom<T, Args...> && concepts::NotReference<T>
		constexpr explicit Option(Args&&... args) // NOLINT
			noexcept(concepts::NoexceptConstructibleFrom<T, Args...>)
			: OptionData(std::forward<Args>(args)...) {
		}

		/// @brief Constructs an empty `Option`, aka an `Option` as the `None` variant
		/// @ingroup option
		/// @headerfile "Hyperion/Option.h"
		constexpr Option(None none) noexcept { // NOLINT
			ignore(none);
		}
		/// @brief Copy Constructor
		/// @ingroup option
		/// @headerfile "Hyperion/Option.h"
		constexpr Option(const Option& option) noexcept(concepts::NoexceptCopyConstructible<T>)
		requires concepts::CopyConstructible<T>
			: OptionData(static_cast<const OptionData&>(option)) {
		}
		/// @brief Converting Copy Constructor from `Option<U>` where `T != U`
		/// @ingroup option
		/// @headerfile "Hyperion/Option.h"
		template<typename U>
		requires concepts::ConstructibleFrom<T, U> && concepts::NotSame<T, U>
		constexpr Option(const Option<U>& option) // NOLINT
			noexcept(concepts::NoexceptCopyConstructible<T>) {
			if(option.is_some()) {
				static_cast<OptionData&>(*this) = option.as_cref();
			}
		}
		/// @brief Move Constructor
		/// @ingroup option
		/// @headerfile "Hyperion/Option.h"
		constexpr Option(Option&& option) noexcept(concepts::NoexceptMoveConstructible<T>)
		requires concepts::MoveConstructible<T>
			: OptionData(static_cast<OptionData&&>(option)) {
			option = None();
		}
		/// @brief Converting Move Constructor from `Option<U>` where `T != U`
		/// @ingroup option
		/// @headerfile "Hyperion/Option.h"
		template<typename U>
		requires concepts::ConstructibleFrom<T, U> && concepts::NotSame<T, U>
		constexpr Option(Option<U>&& option) // NOLINT
			noexcept(concepts::NoexceptCopyConstructible<T>) {
			if(option.is_some()) {
				static_cast<OptionData&>(*this) = option.unwrap();
			}
		}

		/// @brief Destructor
		/// @ingroup option
		/// @headerfile "Hyperion/Option.h"
		constexpr ~Option() noexcept(concepts::NoexceptDestructible<T>) = default;

		/// @brief Returns whether this `Option` is the `Some` variant
		///
		/// @return true if this is `Some`, false otherwise
		/// @ingroup option
		/// @headerfile "Hyperion/Option.h"
		[[nodiscard]] inline constexpr auto is_some() const noexcept -> bool {
			return this->has_value();
		}

		/// @brief Returns whether this `Option` is the `None` variant
		///
		/// @return true if this is `None`, false otherwise
		/// @ingroup option
		/// @headerfile "Hyperion/Option.h"
		[[nodiscard]] inline constexpr auto is_none() const noexcept -> bool {
			return !is_some();
		}

		/// @brief Maps this `Option` to another one, with a potentially different
		/// `Some` type.
		///
		/// If this is the `Some` variant, invokes `map_func` with a const reference to the
		/// contained value, returning the result as a new `Option`. Otherwise, returns `None`.
		///
		/// @tparam F - The type of the invocable mapping `T`
		/// @tparam U - The type that `F` maps `T` to. This is deduced. Do not
		/// explicitly provide this.
		/// @param map_func - The invocable that performs the mapping
		///
		/// @return The result of `map_func` wrapped in an `Option` if this is `Some`, `None`
		/// otherwise
		/// @ingroup option
		/// @headerfile "Hyperion/Option.h"
		template<typename F,
				 typename U = decltype(std::declval<F>()(std::declval<const_reference>()))>
		requires concepts::InvocableWithReturn<U, F, const_reference>
		[[nodiscard]] inline auto map(F&& map_func) const noexcept -> Option<U> {
			// the invocable checks above are probably redundant because of the inferred
			// template parameters, but we'll keep them for completeness’ sake and
			// clarity of requirements
			if(!is_some()) {
				return hyperion::None();
			}

			return Some(std::forward<F>(map_func)(this->get()));
		}

		/// @brief Maps this `Option` to a `U`
		///
		/// If this is the `Some` variant, returns the result of invoking `map_func` with a const
		/// reference to the contained value. Otherwise, returns `default_value`.
		//
		/// @tparam F - The type of the invocable mapping `T`
		/// @tparam U - The type that `F` maps `T` to. This is deduced. Do not
		/// explicitly provide this.
		/// @param map_func - The invocable that performs the mapping
		/// @param default_value - The default value
		///
		/// @return The result of the mapping if this is `Some`, `default_value` otherwise
		/// @ingroup option
		/// @headerfile "Hyperion/Option.h"
		template<typename F, typename U>
		requires concepts::InvocableWithReturn<U, F, const_reference>
		[[nodiscard]] inline auto map_or(F&& map_func, U&& default_value) const noexcept -> U {
			if(!is_some()) {
				return std::forward<U>(default_value);
			}

			return std::forward<F>(map_func)(this->get());
		}

		/// @brief Maps this `Option` to a `U`
		///
		/// If this is the `Some` variant, returns the result of invoking `map_func` with a const
		/// reference to the contained value. Otherwise, returns the result of invoking
		/// `default_generator`.
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
		/// @return The result of the mapping if this is `Some`, or the value returned by
		/// `default_generator` if this is `None`
		/// @ingroup option
		/// @headerfile "Hyperion/Option.h"
		template<typename F,
				 concepts::Invocable G,
				 typename U = decltype(std::declval<F>()(std::declval<const_reference>())),
				 typename V = decltype(std::declval<G>()())>
		requires concepts::Same<U, V>
				 && concepts::InvocableWithReturn<U, F, const_reference>
				 [[nodiscard]] inline auto
				 map_or_else(F&& map_func, G&& default_generator) const noexcept -> U {
			// the invocable checks above are probably redundant because of the inferred
			// template parameters, but we'll keep them for completeness’ sake and
			// clarity of requirements
			if(!is_some()) {
				return std::forward<G>(default_generator)();
			}

			return std::forward<F>(map_func)(this->get());
		}

		/// @brief Matches this `Option` to a consuming function, depending on whether it is the
		/// `Some` or `None` variant.
		///
		/// If this is the `Some` variant, the contained `T` is moved out of this `Option` and
		/// `some_func` is invoked with it, returning the result.
		/// Otherwise, the result of invoking `none_func` is returned.
		///
		/// @tparam SomeFunc - The type of the function to consume the contained value with if this
		/// is `Some`
		/// @tparam NoneFunc - The type of the function to call if this is `None`
		/// @tparam R1 - The return type of `SomeFunc`. This is deduced. Do not explicitly provide
		/// this.
		/// @tparam R2 - The return type of `NoneFunc`. This is deduced. Do not explicitly provide
		/// this.
		///
		/// @param some_func - The function to consume the contained value with if this is `Some`
		/// @param none_func - The function to call if this is `None`
		/// @return The result of invoking `some_func` with the contained value if this is `Some`,
		/// otherwise the result of invoking `none_func`.
		/// @note The returned type of `some_func` and `none_func` must be the same
		/// @ingroup option
		/// @headerfile "Hyperion/Option.h"
		template<typename SomeFunc,
				 concepts::Invocable NoneFunc,
				 typename R1 = decltype(std::declval<SomeFunc>()(std::declval<rvalue_reference>())),
				 typename R2 = decltype(std::declval<NoneFunc>()())>
		requires concepts::Same<R1, R2>
				 && concepts::InvocableWithReturn<R1, SomeFunc, rvalue_reference>
				 inline auto match(SomeFunc&& some_func, NoneFunc&& none_func) noexcept -> R1 {
			if(!is_some()) {
				return std::forward<NoneFunc>(none_func)();
			}

			return std::forward<SomeFunc>(some_func)(this->extract());
		}

	  private:
		template<typename F>
		using and_then_result = decltype(std::declval<F>()(std::declval<rvalue_reference>()));

	  public:
		// clang-format off

		/// @brief Continues control flow into `func` if this is the `Some` variant,
		/// otherwise returns `None`.
		///
		/// If this is the `Some` variant, returns the result of invoking `func` with the contained
		/// value moved out of this. Otherwise, returns `None`. Generally used to chain multiple
		/// operations that return `Option`s together, forwarding control flow through each link in
		/// the chain. Requires that the return type of `func` is also an `Option` (possibly
		/// containing a different type).
		///
		/// @tparam F - The type of invocable to call if this is `Some`
		/// @tparam U - The type of the `Some` variant of the `Option` returned by
		/// `F`. This is deduced. Don't explicitly provide this.
		/// @param func - The invocable to call if this is `Some`
		///
		/// @return The result of invoking `func()` if this is `Some`, otherwise `None`
		/// @note `func` must return an `Option`
		/// @ingroup option
		/// @headerfile "Hyperion/Option.h"
		template<typename F,
				 typename U = typename and_then_result<F>::rvalue_reference,
				 typename R = std::conditional_t<std::is_rvalue_reference_v<U>,
                                                 std::remove_reference_t<U>,
                                U>>
		requires concepts::InvocableWithReturn<Option<R>, F, rvalue_reference>
		[[nodiscard]] inline auto and_then(F&& func) noexcept -> Option<R> {
			// clang-format on
			// the invocable checks above are probably redundant because of the inferred
			// template
			if(!is_some()) {
				return hyperion::None();
			}

			return std::forward<F>(func)(this->extract());
		}

		/// @brief If this is the `Some` variant, forwards this. Otherwise returns `option`.
		///
		/// @param option - The next option to potentially use
		///
		/// @return this if this is `Some`, otherwise `option`
		/// @ingroup option
		/// @headerfile "Hyperion/Option.h"
		[[nodiscard]] inline auto or_else(const Option& option) noexcept -> const Option& {
			if(!is_some()) {
				return option;
			}

			return *this;
		}

		/// @brief If this is the `Some` variant, forwards this. Otherwise returns `option`.
		///
		/// @param option - The next option to potentially use
		///
		/// @return this if this is `Some`, otherwise `option`
		/// @ingroup option
		/// @headerfile "Hyperion/Option.h"
		[[nodiscard]] inline auto or_else(Option&& option) noexcept -> Option {
			if(!is_some()) {
				return std::move(option);
			}

			return std::move(*this);
		}

		/// @brief If this is the `Some` variant, forwards this. Otherwise returns the result of
		/// invoking `func`.
		///
		/// @tparam F - The type of the invocable to call if this is `None`
		/// @param func - The invocable to call if this is `None`
		///
		/// @return this if this is `Some`, otherwise the result of invoking `func`
		/// @note `func` must return an `Option` containing the same type as this
		/// @ingroup option
		/// @headerfile "Hyperion/Option.h"
		template<typename F>
		requires concepts::InvocableWithReturn<Option<T>, F>
		[[nodiscard]] inline auto or_else(F&& func) noexcept -> Option {
			if(!is_some()) {
				return std::forward<F>(func)();
			}

			return std::move(*this);
		}

		/// @brief Converts this `Option` into a `Result` consuming this `Option`.
		///
		/// If this is the `Some` variant, moves the contained value out of this and returns
		/// it as an `Ok` variant `Result`. Otherwise, returns `Err(error)`.
		///
		/// @tparam E - The type to return if this is `None`
		/// @param error - The error to return if this is `None`
		///
		/// @return `Ok<T>` if this is `Some`, `Err(error)` if this is `None`
		/// @ingroup option
		/// @headerfile "Hyperion/Option.h"
		template<typename E>
		[[nodiscard]] inline constexpr auto
		ok_or(E&& error) noexcept -> Result<T, std::remove_reference_t<E>> {
			if(!is_some()) {
				return Err(std::forward<E>(error));
			}

			return Ok<T>(this->extract());
		}

		/// @brief Converts this `Option` into a `Result` consuming this `Option`.
		///
		/// If this is the `Some` variant, moves the contained value out of this and returns it as
		/// an `Ok` variant `Result`. Otherwise, returns an `Err` variant `Result` by constructing
		/// an `E` in place from the given arguments.
		///
		/// @tparam E - The type to return if this is `None`.
		/// @tparam Args - The types of the arguments to pass to `E`'s constructor
		/// @param args - The arguments to pass to `E`'s constructor
		///
		/// @return `Ok<T>` if this is `Some`, `Err<E>` if this is `None`
		/// @ingroup option
		/// @headerfile "Hyperion/Option.h"
		template<typename E, typename... Args>
		requires concepts::ConstructibleFrom<E, Args...>
		[[nodiscard]] inline constexpr auto ok_or(Args&&... args) noexcept -> Result<T, E> {
			if(!is_some()) {
				return Err<E>(std::forward<Args>(args)...);
			}

			return Ok<T>(this->extract());
		}

		/// @brief Converts this `Option` into a `Result`, consuming this `Option`.
		///
		/// If this is the `Some` variant, moves the contained value out of this and returns it as
		/// an `Ok` variant `Result`. Otherwise, returns `Err(error_generator())`.
		///
		/// @tparam F - The invocable to generate the error value if this is the
		/// `None` variant
		/// @tparam E - The `error::ErrorType` returned by `F`. This is deduced. Don't
		/// explicitly provide it
		/// @param error_generator - The function to generate the `Error` value
		///
		/// @return `Ok<T>` if this is `Some`, `Err(error_generator())` if this is
		/// `None`
		/// @ingroup option
		/// @headerfile "Hyperion/Option.h"
		template<concepts::Invocable F, typename E = decltype(std::declval<F>()())>
		[[nodiscard]] inline auto ok_or_else(F&& error_generator) noexcept -> Result<T, E> {
			// the invocable checks above are probably redundant because of the inferred
			// template parameters, but we'll keep them for completeness’ sake and
			// clarity of requirements
			if(!is_some()) {
				return Err(std::forward<F>(error_generator)());
			}

			return Ok<T>(this->extract());
		}

		/// @brief Returns the contained `T`, consuming this `Option`.
		///
		/// If this is not `Some` variant, then this will invoke a panic
		///
		/// @return The contained `T`
		/// @ingroup option
		/// @headerfile "Hyperion/Option.h"
		[[nodiscard]] inline constexpr auto
		unwrap() noexcept(concepts::MoveConstructible<T> ? concepts::NoexceptMoveConstructible<T> :
														   concepts::NoexceptCopyConstructible<T>)
			-> type
		requires(std::is_reference_v<T> || concepts::MoveConstructible<T>
				 || concepts::CopyConstructible<T>)
		{
			if(!is_some()) {
				panic("Option::unwrap called on a None, terminating");
			}

			return this->extract();
		}

		/// @brief Returns the contained `T` if this is `Some`, consuming this
		/// `Option`.
		///
		/// If this is not the `Some` variant, then returns `default_value`.
		///
		/// @param default_value - The value to return if this is `None`
		///
		/// @return The contained `T` if this is `Some`, or `default_value`
		/// @ingroup option
		/// @headerfile "Hyperion/Option.h"
		[[nodiscard]] inline constexpr auto unwrap_or(T& default_value) noexcept -> type
		requires concepts::NotReference<T>
		{
			if(!is_some()) {
				return std::move(default_value);
			}

			return unwrap();
		}

		/// @brief Returns the contained `T` if this is `Some`, consuming this
		/// `Option`.
		///
		/// If this is not the `Some` variant, then returns `default_value`.
		///
		/// @param default_value - The value to return if this is `None`
		///
		/// @return The contained `T` if this is `Some`, or `default_value`
		/// @ingroup option
		/// @headerfile "Hyperion/Option.h"
		[[nodiscard]] inline constexpr auto
		unwrap_or(T&& default_value) noexcept(concepts::MoveConstructible<T> ?
												  concepts::NoexceptMoveConstructible<T> :
												  concepts::NoexceptCopyConstructible<T>) -> type
		requires(std::is_reference_v<T> || concepts::MoveConstructible<T>
				 || concepts::CopyConstructible<T>)
		{
			if(!is_some()) {
				return std::move(default_value);
			}

			return unwrap();
		}

		/// @brief Returns the contained `T` if this is `Some`, consuming this
		/// `Option`.
		///
		/// If this is not the `Some` variant, then returns the result of invoking
		/// `default_generator`.
		///
		/// @param default_generator - The invocable to generate the default value if
		/// this is `None`
		///
		/// @return The contained `T` if this is `Some`, or the result of invoking
		/// `default_generator`
		/// @ingroup option
		/// @headerfile "Hyperion/Option.h"
		template<typename F>
		requires concepts::InvocableWithReturn<T, F>
				 [[nodiscard]] inline auto unwrap_or_else(F&& default_generator) noexcept(
					 concepts::MoveConstructible<T> ? concepts::NoexceptMoveConstructible<T> :
													  concepts::NoexceptCopyConstructible<T>)
					 -> type
				 requires(std::is_reference_v<T> || concepts::MoveConstructible<T>
						  || concepts::CopyConstructible<T>)
		{
			if(!is_some()) {
				return std::forward<F>(default_generator)();
			}

			return unwrap();
		}

		/// @brief Returns the contained `T` if this is `Some`, consuming this `Option`.
		///
		/// If this is not the `Some` variant, invokes a panic with the given `panic_message`.
		///
		/// @param panic_message - The panic message to print if this is not the `Some` variant
		///
		/// @return The contained `T` if this is `Some`
		/// @ingroup option
		/// @headerfile "Hyperion/Option.h"
		template<typename U>
		requires concepts::Convertible<U, std::string>
				 || concepts::Convertible<U, std::string_view>
					[[nodiscard]] inline auto
					expect(U&& panic_message) noexcept(concepts::MoveConstructible<T> ?
														   concepts::NoexceptMoveConstructible<T> :
														   concepts::NoexceptCopyConstructible<T>)
						-> type
					requires(std::is_reference_v<T> || concepts::MoveConstructible<T>
							 || concepts::CopyConstructible<T>)
		{
			if(!is_some()) {
				panic("{}", std::forward<U>(panic_message));
			}

			return unwrap();
		}

		/// @brief Returns a reference to the contained value if this is `Some`, otherwise
		/// invokes a panic
		///
		/// @return A reference to the contained `T`
		/// @ingroup option
		/// @headerfile "Hyperion/Option.h"
		[[nodiscard]] inline constexpr auto as_ref() noexcept -> reference {
			if(!is_some()) {
				panic("Option::as_ref called on a None, terminating");
			}

			if constexpr(concepts::Reference<T>) {
				return this->get().get();
			}
			else {
				return this->get();
			}
		}

		/// @brief Returns a const reference to the contained value if this is `Some`, otherwise
		/// invokes a panic
		///
		/// @return A const reference to the contained `T`
		/// @ingroup option
		/// @headerfile "Hyperion/Option.h"
		[[nodiscard]] inline constexpr auto as_cref() const noexcept -> const_reference {
			if(!is_some()) {
				panic("Option::as_cref called on a None, terminating");
			}

			return this->get();
		}

		/// @brief Equality comparison operator to `None`
		///
		/// @return true if this is `None`, false otherwise
		/// @ingroup option
		/// @headerfile "Hyperion/Option.h"
		constexpr auto operator==(const None& none) const noexcept -> bool {
			ignore(none);
			return is_none();
		}

		/// @brief Inequality comparison operator to `None`
		///
		/// @return true if this is `None`, false otherwise
		/// @ingroup option
		/// @headerfile "Hyperion/Option.h"
		constexpr auto operator!=(const None& none) const noexcept -> bool {
			ignore(none);
			return is_some();
		}

		/// @brief Equality comparison operator to `T`
		///
		/// @return true if this is `None`, false otherwise
		/// @ingroup option
		/// @headerfile "Hyperion/Option.h"
		constexpr auto operator==(const T& some) const noexcept -> bool
		requires concepts::EqualityComparable<T>
		{
			if(!is_some()) {
				return false;
			}

			return this->get() == some;
		}

		/// @brief Inequality comparison operator to `T`
		///
		/// @return true if this is `None`, false otherwise
		/// @ingroup option
		/// @headerfile "Hyperion/Option.h"
		constexpr auto operator!=(const T& some) const noexcept -> bool
		requires concepts::InequalityComparable<T>
		{
			if(!is_some()) {
				return true;
			}

			return this->get() != some;
		}

		/// @brief Boolean conversion operator. Returns true if this is the `Some`
		/// variant.
		///
		/// @return true if this is `Some`, false otherwise
		/// @ingroup option
		/// @headerfile "Hyperion/Option.h"
		explicit constexpr operator bool() const noexcept {
			return is_some();
		}

		/// @brief Copy assignment operator
		/// @ingroup option
		/// @headerfile "Hyperion/Option.h"
		constexpr auto
		operator=(const Option& option) noexcept(concepts::NoexceptCopyAssignable<T>) -> Option&
		requires concepts::CopyAssignable<T>
		{
			if(this == &option) {
				return *this;
			}

			OptionData::operator=(static_cast<const OptionData&>(option));
			return *this;
		}

		// clang-format off

		/// @brief Converting assignment operator from `U`
		/// @ingroup option
		/// @headerfile "Hyperion/Option.h"
        template<typename U>
        constexpr auto operator=(const U& data)
            noexcept(std::is_nothrow_assignable_v<T, decltype(data)>)
            -> Option&
            requires std::is_assignable_v<T, decltype(data)>
        {
            OptionData::operator=(data);
            return *this;
        }

		/// @brief Converting assignment operator from `Option<U>` where U is `T&` or `const T&`
		/// @ingroup option
		/// @headerfile "Hyperion/Option.h"
		template<typename U>
		requires concepts::Same<T, std::remove_reference_t<U>>
                 && concepts::CopyAssignable<T>
                 && concepts::Reference<U>
		constexpr auto operator=(const Option<U>& option)
            noexcept(concepts::NoexceptCopyAssignable<T>)
			-> Option&
		{
			// clang-format on
			OptionData::operator=(static_cast<const OptionData&>(option));
			return *this;
		}

		/// @brief Move assignment operator
		/// @ingroup option
		/// @headerfile "Hyperion/Option.h"
		constexpr auto
		operator=(Option&& option) noexcept(concepts::NoexceptMoveAssignable<T>) -> Option&
		requires concepts::MoveAssignable<T> || concepts::Reference<T>
		{

			if(this == &option) {
				return *this;
			}

			OptionData::operator=(static_cast<OptionData&&>(option));
			option = None();
			return *this;
		}

		/// @brief Copy assignment operator from `T`
		/// @ingroup option
		/// @headerfile "Hyperion/Option.h"
		constexpr auto
		operator=(const T& some) noexcept(concepts::NoexceptCopyAssignable<T>) -> Option&
		requires concepts::CopyAssignable<T>
		{
			OptionData::operator=(some);
			return *this;
		}

		/// @brief Move assignment operator from `T`
		/// @ingroup option
		/// @headerfile "Hyperion/Option.h"
		constexpr auto operator=(T&& some) noexcept(concepts::NoexceptMoveAssignable<T>) -> Option&
		requires concepts::MoveAssignable<T>
		{
			OptionData::operator=(std::move(some));
			return *this;
		}

		/// @brief Copy assignment operator from `None`
		/// @ingroup option
		/// @headerfile "Hyperion/Option.h"
		constexpr auto operator=(const None& none) noexcept -> Option& {
			OptionData::operator=(none);
			return *this;
		}

		/// @brief Move assignment operator from `None`
		/// @ingroup option
		/// @headerfile "Hyperion/Option.h"
		constexpr auto operator=(None&& none) noexcept -> Option& {
			OptionData::operator=(none);
			return *this;
		}
	};
	IGNORE_PADDING_STOP

	/// @brief Creates an `Option<T>` by storing the given `T` in the `Option<T>`
	///
	/// @tparam T - The type to store in the option
	///
	/// @param some - The value to store in the `Option<T>`
	///
	/// @return an `Option<T>` containing the given value
	/// @ingroup option
	/// @headerfile "Hyperion/Option.h"
	template<typename T, typename U>
	[[nodiscard]] inline constexpr auto
	Some(T&& some) noexcept(concepts::NoexceptConstructibleFrom<U, T>) -> Option<U> {
		return Option<U>(std::forward<T>(some));
	}

	/// @brief Creates an `Option<T>` by constructing a `T` in place in the `Option<T>`
	///
	/// @tparam T - The type to store in the option
	/// @tparam Args - The types of the arguments to pass to `T`'s constructor
	///
	/// @param args - The arguments to pass to `T`'s constructor
	///
	/// @return an `Option<T>` containing a newly constructed `T`
	/// @ingroup option
	/// @headerfile "Hyperion/Option.h"
	template<typename T, typename... Args>
	requires concepts::ConstructibleFrom<T, Args...>
	[[nodiscard]] inline constexpr auto
	Some(Args&&... args) noexcept(concepts::NoexceptConstructibleFrom<T, Args...>) -> Option<T> {
		return Option<T>(std::forward<Args>(args)...);
	}

	/// @brief Creates an `Option<T>` by storing the given `T` in the `Option<T>`
	///
	/// @tparam T - The type to store in the option
	///
	/// @param some - The value to store in the `Option<T>`
	///
	/// @return an `Option<T>` containing the given value
	/// @ingroup option
	/// @headerfile "Hyperion/Option.h"
	template<typename T>
	[[nodiscard]] inline constexpr auto
	Some(concepts::Convertible<T> auto&& some) noexcept -> Option<T> {
		return Option<T>(T(some));
	}
} // namespace hyperion

#endif // HYPERION_OPTION
