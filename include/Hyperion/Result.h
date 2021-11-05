/// @file Result.h
/// @author Braxton Salyer <braxtonsalyer@gmail.com>
/// @brief `Result` represents the outcome of an operation that can fail recoverably
/// @version 0.1
/// @date 2021-11-05
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
#include <Hyperion/Result.h>
#include <Hyperion/option/None.h>
#include <Hyperion/option/SomeFWD.h>
#include <Hyperion/result/Err.h>
#include <Hyperion/result/ResultData.h>

/// @ingroup error
/// @{
///	@defgroup result Result
/// `Result` is a monadic type for representing the outcome of a fallible operation
///
/// Example:
/// @code {.cpp}
/// #include <Hyperion/Result.h>
///
/// using namespace hyperion;
///
/// inline auto checked_add(u32 left, u32 right) noexcept -> Result<u32> {
/// 	return left <= std::numeric_limits<u32>::max() - right ? Ok(left + right)
/// 														   : Err(SystemError(EDOM));
/// }
///
/// checked_add(25_u32, 120939_u32).match([](u32 value) {
/// 		println("sum is: {}", value)
/// 	},
/// 	[](auto err) {
///			eprintln("failed to add values: {}", err.message());
/// 	});
/// @endcode
/// @headerfile "Hyperion/Result.h"
/// @}

/// @def HYPERION_RESULT_PANICS_ON_DESTRUCTION_IF_UNHANDLED
/// @brief Configures `Result` to panic on destruction if the `Result` was not handled
///
/// `Result` can be configured to panic on destruction if it was not handled prior to the destructor
/// call. This makes enforcing error handling and catching unhandled errors easier, because it
/// results in a hard failure instead of hard to trace bugs. The downside is this adds additional
/// storage and run-time costs to `Result`. Currently this also does not work well in a
/// multithreaded context.
///
/// By default, this is disabled. To enable it, define
/// `HYPERION_RESULT_PANICS_ON_DESTRUCTION_IF_UNHANDLED` to `true` prior to including any `Hyperion`
/// headers.
/// @ingroup result
/// @headerfile "Hyperion/Result.h"
#ifndef HYPERION_RESULT_PANICS_ON_DESTRUCTION_IF_UNHANDLED
	// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
	#define HYPERION_RESULT_PANICS_ON_DESTRUCTION_IF_UNHANDLED false
#endif // HYPERION_RESULT_PANICS_ON_DESTRUCTION_IF_UNHANDLED

namespace hyperion {
	template<typename T>
	class Option;

	using option::None;

	IGNORE_PADDING_START
	/// @brief A `Result` represents the outcome of an operation that can fail recoverably.
	///
	/// Every `Result` is either `Ok`, indicating success and containing a desired value
	/// or `Err`, indicating failure and containing an `Error` type
	/// `T` can be any type or pointer
	/// `E` can be any type, although it is preferred to be an `error::Error` or `error::AnyError`,
	/// and would ideally be at least an `error::ErrorType` (a type deriving from
	/// `error::ErrorBase`)
	///
	/// Example:
	/// @code {.cpp}
	/// #include <Hyperion/Result.h>
	///
	/// using namespace hyperion;
	///
	/// inline auto checked_add(u32 left, u32 right) noexcept -> Result<u32> {
	/// 	return left <= std::numeric_limits<u32>::max() - right ? Ok(left + right)
	/// 														   : Err(SystemError(EDOM));
	/// }
	///
	/// checked_add(25_u32, 120939_u32).match([](u32 value) {
	/// 		println("sum is: {}", value);
	/// 	},
	/// 	[](auto&& err) {
	///			eprintln("failed to add values: {}", err.message());
	/// 	});
	/// @endcode
	///
	/// @note While any combination of `T` and `E` can be pointers, `Result` will __NOT__ take
	/// ownership of any pointer it contains, and as such it is the responsibility of the user to
	/// delete a pointer contained in a `Result`. To have a `Result` take ownership of a pointer,
	/// use a smart-pointer instead such as `hyperion::UniquePtr` or `std::shared_ptr` instead of a
	/// raw pointer.
	///
	/// @tparam T - the type indicating the outcome of a successful operation in the case of success
	/// @tparam E - the type indicating failure in the case of an error
	/// @ingroup result
	/// @headerfile "Hyperion/Result.h"
	template<typename T = bool, typename E = error::SystemError>
	class [[nodiscard]] Result final : private result::ResultData<T, E> {
	  public:
		using ResultData = result::ResultData<T, E>;
		/// @brief The type of the `Ok` variant
		/// @ingroup result
		/// @headerfile "Hyperion/Result.h"
		using ok_type = typename ResultData::ok_type;
		/// @brief Pointer to the type of the `Ok` variant
		/// @ingroup result
		/// @headerfile "Hyperion/Result.h"
		using ok_pointer = typename ResultData::ok_pointer;
		/// @brief Pointer-to-const to the type of the `Ok` variant
		/// @ingroup result
		/// @headerfile "Hyperion/Result.h"
		using ok_pointer_to_const = typename ResultData::ok_pointer_to_const;
		/// @brief Reference to the type of the `Ok` variant
		/// @ingroup result
		/// @headerfile "Hyperion/Result.h"
		using ok_reference = typename ResultData::ok_reference;
		/// @brief Const reference to the type of the `Ok` variant
		/// @ingroup result
		/// @headerfile "Hyperion/Result.h"
		using ok_const_reference = typename ResultData::ok_const_reference;
		/// @brief Rvalue reference (or equivalent) of the type of the `Ok` variant
		///
		/// Rvalue reference of `ok_type` or equivalent. E.g., for a `Result` storing a pointer,
		/// this would be the same as `ok_type`, but for a bare type, this would be `ok_type&&`.
		/// @ingroup result
		/// @headerfile "Hyperion/Result.h"
		using ok_rvalue_reference = typename ResultData::ok_extracted;

		/// @brief The type of the `Err` variant
		/// @ingroup result
		/// @headerfile "Hyperion/Result.h"
		using err_type = typename ResultData::err_type;
		/// @brief Pointer to the type of the `Err` variant
		/// @ingroup result
		/// @headerfile "Hyperion/Result.h"
		using err_pointer = typename ResultData::err_pointer;
		/// @brief Pointer-to-const to the type of the `Err` variant
		/// @ingroup result
		/// @headerfile "Hyperion/Result.h"
		using err_pointer_to_const = typename ResultData::err_pointer_to_const;
		/// @brief Reference to the type of the `Err` variant
		/// @ingroup result
		/// @headerfile "Hyperion/Result.h"
		using err_reference = typename ResultData::err_reference;
		/// @brief Const reference to the type of the `Err` variant
		/// @ingroup result
		/// @headerfile "Hyperion/Result.h"
		using err_const_reference = typename ResultData::err_const_reference;
		/// @brief Rvalue reference (or equivalent) of the type of the `Err` variant
		///
		/// Rvalue reference of `err_type` or equivalent. E.g., for a `Result` storing a pointer,
		/// this would be the same as `err_type`, but for a bare type, this would be `err_type&&`.
		/// @ingroup result
		/// @headerfile "Hyperion/Result.h"
		using err_rvalue_reference = typename ResultData::err_extracted;

		/// @brief Default Constructor
		/// @ingroup result
		/// @headerfile "Hyperion/Result.h"
		constexpr Result() noexcept = default;
		// clang-format off

		/// @brief Constructs a `Result` from the given `T`
		/// @ingroup result
		/// @headerfile "Hyperion/Result.h"
		constexpr explicit Result(const T& ok) noexcept
			requires concepts::NoexceptCopyConstructible<T>
			: ResultData(ok) {
		}
		/// @brief Constructs a `Result` from the given `T`
		/// @ingroup result
		/// @headerfile "Hyperion/Result.h"
		constexpr explicit Result(T&& ok) noexcept
			requires concepts::NoexceptMoveConstructible<T>
			: ResultData(std::move(ok)) {
		}
		/// @brief Constructs a `Result` by constructing the `T` in place in the `Result`
		///
		/// @tparam Args - The types of the arguments to pass to `T`'s constructor
		/// @param args - The arguments to pass to `T`'s constructor
		/// @ingroup result
		/// @headerfile "Hyperion/Result.h"
		template<typename... Args>
		requires concepts::ConstructibleFrom<T, Args...> && concepts::NotReference<T>
		constexpr explicit Result(Args&&... args) noexcept // NOLINT
			: ResultData(std::forward<Args>(args)...) {
		}
		/// @brief Constructs a `Result` from the given `E`
		/// @ingroup result
		/// @headerfile "Hyperion/Result.h"
		constexpr explicit Result(const E& err) noexcept
			requires concepts::NoexceptCopyConstructible<E>
			: ResultData(err) {
		}
		/// @brief Constructs a `Result` from the given `E`
		/// @ingroup result
		/// @headerfile "Hyperion/Result.h"
		constexpr explicit Result(E&& err) noexcept
			requires concepts::NoexceptMoveConstructible<E>
			: ResultData(std::move(err)) {
		}
		/// @brief Constructs a `Result` by constructing the `E` in place in the `Result`
		///
		/// @tparam Args - The types of the arguments to pass to `E`'s constructor
		/// @param args - The arguments to pass to `E`'s constructor
		/// @ingroup result
		/// @headerfile "Hyperion/Result.h"
		template<typename... Args>
		requires concepts::ConstructibleFrom<E, Args...> && concepts::NotReference<E>
		constexpr explicit Result(Args&&... args) noexcept // NOLINT
			: ResultData(std::forward<Args>(args)...) {
		}
		// clang-format on

		/// @brief Constructs a `Result` from an `Err`
		///
		/// @param error - The error indicating failure
		/// @ingroup result
		/// @headerfile "Hyperion/Result.h"
		Result(const hyperion::Err<E>& error) noexcept // NOLINT
			requires concepts::NoexceptCopyConstructible<E> : ResultData(error.m_error) {
		}
		/// @brief Constructs a `Result` from an `Err`
		///
		/// @param error - The error indicating failure
		/// @ingroup result
		/// @headerfile "Hyperion/Result.h"
		Result(hyperion::Err<E>&& error) noexcept // NOLINT
			requires concepts::NoexceptMoveConstructible<E> : ResultData(std::move(error.m_error)) {
		}
		/// @brief Constructs a `Result` from an `Ok`
		///
		/// @param ok - The value indicating success
		/// @ingroup result
		/// @headerfile "Hyperion/Result.h"
		constexpr Result(const hyperion::Ok<T>& ok) noexcept // NOLINT
			requires concepts::NoexceptCopyConstructible<T> : ResultData(ok.m_ok) {
		}
		/// @brief Constructs a `Result` from an `Ok`
		///
		/// @param ok - The value indicating success
		/// @ingroup result
		/// @headerfile "Hyperion/Result.h"
		constexpr Result(hyperion::Ok<T>&& ok) noexcept // NOLINT
			requires concepts::NoexceptMoveConstructible<T> : ResultData(std::move(ok.m_ok)) {
		}
		/// @brief Copy Constructor
		/// @ingroup result
		/// @headerfile "Hyperion/Result.h"
		constexpr Result(const Result& result) noexcept(
			concepts::NoexceptCopyConstructible<ResultData>) requires
			concepts::CopyConstructible<ResultData>
			: ResultData(static_cast<const ResultData&>(result))
#if HYPERION_RESULT_PANICS_ON_DESTRUCTION_IF_UNHANDLED
			,
			  m_handled(result.m_handled)
#endif // HYPERION_RESULT_PANICS_ON_DESTRUCTION_IF_UNHANDLED
		{
#if HYPERION_RESULT_PANICS_ON_DESTRUCTION_IF_UNHANDLED
			result.m_handled = true;
#endif // HYPERION_RESULT_PANICS_ON_DESTRUCTION_IF_UNHANDLED
		}
		/// @brief Move Constructor
		/// Moving a `Result` consumes it, leaving a disengaged (valueless) `Result` in its place
		/// @ingroup result
		/// @headerfile "Hyperion/Result.h"
		constexpr Result(Result&& result) noexcept(
			concepts::NoexceptMoveConstructible<ResultData>) requires
			concepts::MoveConstructible<ResultData> : ResultData(static_cast<ResultData&&>(result))
#if HYPERION_RESULT_PANICS_ON_DESTRUCTION_IF_UNHANDLED
			,
													  m_handled(result.m_handled)
#endif // HYPERION_RESULT_PANICS_ON_DESTRUCTION_IF_UNHANDLED
		{
#if HYPERION_RESULT_PANICS_ON_DESTRUCTION_IF_UNHANDLED
			result.m_handled = true;
#endif // HYPERION_RESULT_PANICS_ON_DESTRUCTION_IF_UNHANDLED
			static_cast<ResultData&>(result) = ResultData(None());
		}
#if HYPERION_RESULT_PANICS_ON_DESTRUCTION_IF_UNHANDLED
		/// @brief Destructor
		/// @ingroup result
		/// @headerfile "Hyperion/Result.h"
		constexpr ~Result() noexcept(concepts::NoexceptDestructible<ResultData>) {
			if(!m_handled && !this->is_empty()) {
				m_handled = true;
				panic("Unhandled Result that must be handled being destroyed, terminating");
			}
		}
#else
		/// @brief Destructor
		/// @ingroup result
		/// @headerfile "Hyperion/Result.h"
		constexpr ~Result() noexcept(concepts::NoexceptDestructible<ResultData>) = default;
#endif

		/// @brief Returns whether this `Result` is the `Ok` variant
		///
		/// @return true if this is `Ok`, otherwise `false`
		/// @ingroup result
		/// @headerfile "Hyperion/Result.h"
		[[nodiscard]] constexpr auto is_ok() const noexcept -> bool {
#if HYPERION_RESULT_PANICS_ON_DESTRUCTION_IF_UNHANDLED
			m_handled = true;
#endif // HYPERION_RESULT_PANICS_ON_DESTRUCTION_IF_UNHANDLED
			return this->has_ok();
		}

		/// @brief Returns whether this `Result` is the `Err` variant
		///
		/// @return true if this is `Err`, otherwise `false`
		/// @ingroup result
		/// @headerfile "Hyperion/Result.h"
		[[nodiscard]] inline constexpr auto is_err() const noexcept -> bool {
#if HYPERION_RESULT_PANICS_ON_DESTRUCTION_IF_UNHANDLED
			m_handled = true;
#endif // HYPERION_RESULT_PANICS_ON_DESTRUCTION_IF_UNHANDLED
			return this->has_err();
		}

		/// @brief Similar to `unwrap`, but doesn't consume this `Result`.
		///
		/// Returns a reference to the `Ok` value if this is `Ok`, otherwise invokes a panic
		///
		/// @return A reference to the `Ok` value
		/// @ingroup result
		/// @headerfile "Hyperion/Result.h"
		[[nodiscard]] inline constexpr auto as_mut() noexcept -> ok_reference {
#if HYPERION_RESULT_PANICS_ON_DESTRUCTION_IF_UNHANDLED
			m_handled = true;
#endif // HYPERION_RESULT_PANICS_ON_DESTRUCTION_IF_UNHANDLED
			if(is_ok()) {
				if constexpr(concepts::Reference<T>) {
					return this->get().get();
				}
				else {
					return this->get();
				}
			}
			else {
				panic("Result::as_mut called on an Error result, terminating");
			}
		}

		/// @brief Similar to `unwrap`, but doesn't consume this `Result`.
		///
		/// Returns a const reference to the `Ok` value if this is `Ok`, otherwise invokes a panic
		///
		/// @return A const reference to the `Ok` value
		/// @ingroup result
		/// @headerfile "Hyperion/Result.h"
		[[nodiscard]] inline constexpr auto as_const() const noexcept -> ok_const_reference {
#if HYPERION_RESULT_PANICS_ON_DESTRUCTION_IF_UNHANDLED
			m_handled = true;
#endif // HYPERION_RESULT_PANICS_ON_DESTRUCTION_IF_UNHANDLED
			if(is_ok()) {
				return this->get();
			}
			else {
				panic("Result::as_const called on an Error result, terminating");
			}
		}

		/// @brief Returns the contained `T`, consuming this `Result`.
		///
		/// If this is the `Ok` variant, moves the `Ok` value out of this and returns it. Otherwise,
		/// invokes a panic.
		///
		/// @return The contained `T`
		/// @ingroup result
		/// @headerfile "Hyperion/Result.h"
		[[nodiscard]] inline constexpr auto
		unwrap() noexcept -> ok_type requires concepts::NoexceptMovable<T> {
#if HYPERION_RESULT_PANICS_ON_DESTRUCTION_IF_UNHANDLED
			m_handled = true;
#endif // HYPERION_RESULT_PANICS_ON_DESTRUCTION_IF_UNHANDLED
			if(is_ok()) {
				return this->extract();
			}
			else {
				panic("Result::unwrap called on an Error result, terminating");
			}
		}

		/// @brief Returns the contained `T` if this is the `Ok` variant, consuming this `Result`.
		///
		/// If this is not the `Ok` variant, then returns `default_value`.
		///
		/// @param default_value - The value to return if this is `Err`
		///
		/// @return The contained `T` if this is `Ok`, or `default_value`
		/// @ingroup result
		/// @headerfile "Hyperion/Result.h"
		[[nodiscard]] inline constexpr auto
		unwrap_or(T& default_value) noexcept -> ok_type requires concepts::NotReference<T> {
#if HYPERION_RESULT_PANICS_ON_DESTRUCTION_IF_UNHANDLED
			m_handled = true;
#endif // HYPERION_RESULT_PANICS_ON_DESTRUCTION_IF_UNHANDLED
			if(is_ok()) {
				return unwrap();
			}
			else {
				return std::move(default_value);
			}
		}

		/// @brief Returns the contained `T` if this is the `Ok` variant, consuming this `Result`.
		///
		/// If this is not the `Ok` variant, then returns `default_value`.
		///
		/// @param default_value - The value to return if this is `Err`
		///
		/// @return The contained `T` if this is `Ok`, or `default_value`
		/// @ingroup result
		/// @headerfile "Hyperion/Result.h"
		[[nodiscard]] inline constexpr auto unwrap_or(T&& default_value) noexcept -> ok_type {
#if HYPERION_RESULT_PANICS_ON_DESTRUCTION_IF_UNHANDLED
			m_handled = true;
#endif // HYPERION_RESULT_PANICS_ON_DESTRUCTION_IF_UNHANDLED
			if(is_ok()) {
				return unwrap();
			}
			else {
				return std::move(default_value);
			}
		}

		/// @brief Returns the contained `T` if this is the `Ok` variant, consuming this `Result`.
		///
		/// If this is not `Ok`, then returns the result of invoking `default_generator`.
		///
		/// @tparam F - The type of the `default_generator` invocable
		/// @param default_generator - The function to generate the value to return if this is
		/// `Err`
		///
		/// @return  The contained `T` if this is `Ok`, or the result of invoking
		/// `default_generator`
		/// @ingroup result
		/// @headerfile "Hyperion/Result.h"
		template<typename F>
		requires concepts::InvocableWithReturn<T, F>
		[[nodiscard]] inline auto unwrap_or_else(F&& default_generator) noexcept -> T {
#if HYPERION_RESULT_PANICS_ON_DESTRUCTION_IF_UNHANDLED
			m_handled = true;
#endif // HYPERION_RESULT_PANICS_ON_DESTRUCTION_IF_UNHANDLED
			if(is_ok()) {
				return unwrap();
			}
			else {
				return std::forward<F>(default_generator)();
			}
		}

		/// @brief Returns the contained `T` if this is the `Ok` variant, consuming this `Result`.
		///
		/// If this is not the `Ok` variant, invokes a panic with the given `panic_message`.
		///
		/// @param panic_message - The panic message to print if this is not the `Ok` variant
		///
		/// @return The contained `T` if this is `Ok`
		/// @ingroup result
		/// @headerfile "Hyperion/Result.h"
		template<typename U>
		requires concepts::Convertible<U, std::string> || concepts::Convertible<U, std::string_view>
		[[nodiscard]] inline auto expect(U&& panic_message) noexcept -> ok_type {
#if HYPERION_RESULT_PANICS_ON_DESTRUCTION_IF_UNHANDLED
			m_handled = true;
#endif // HYPERION_RESULT_PANICS_ON_DESTRUCTION_IF_UNHANDLED
			if(is_ok()) {
				return unwrap();
			}
			else {
				panic("{}", std::forward<U>(panic_message));
			}
		}

		/// @brief Returns the contained `E` if this is the `Err` variant, consuming this `Result`.
		///
		/// If this is not `Err`, invokes a panic
		///
		/// @return The contained `E`
		/// @ingroup result
		/// @headerfile "Hyperion/Result.h"
		[[nodiscard]] inline constexpr auto
		unwrap_err() noexcept -> err_type requires concepts::NoexceptMovable<E> {
#if HYPERION_RESULT_PANICS_ON_DESTRUCTION_IF_UNHANDLED
			m_handled = true;
#endif // HYPERION_RESULT_PANICS_ON_DESTRUCTION_IF_UNHANDLED
			if(is_err()) {
				return this->extract_err();
			}
			else {
				panic("Result::unwrap_err called on an Ok result, terminating");
			}
		}

		/// @brief Converts this `Result` into an `Option<T>`, consuming this `Result` and
		/// discarding the error, if any.
		///
		/// If this is the `Ok` variant, moves the contained value out of this and returns it as a
		/// `Some` variant `Option`. Otherwise, returns `None`.
		///
		/// @return `Option<T>`
		/// @ingroup result
		/// @headerfile "Hyperion/Result.h"
		[[nodiscard]] inline constexpr auto ok() noexcept -> Option<T>
		requires concepts::NoexceptMovable<T> {
#if HYPERION_RESULT_PANICS_ON_DESTRUCTION_IF_UNHANDLED
			m_handled = true;
#endif // HYPERION_RESULT_PANICS_ON_DESTRUCTION_IF_UNHANDLED
			if(is_ok()) {
				return Some(this->extract());
			}
			else {
				return None();
			}
		}

		/// @brief Converts this `Result` into an `Option<E>`, consuming this `Result` and
		/// discarding the success value, if any.
		///
		/// If this is the `Ok` variant, moves the contained value out of this and returns it as a
		/// `Some` variant `Option`. Otherwise, returns `None`.
		///
		/// @return `Option<E>`
		/// @ingroup result
		/// @headerfile "Hyperion/Result.h"
		[[nodiscard]] inline constexpr auto err() noexcept -> Option<E>
		requires concepts::NoexceptMovable<E> {
#if HYPERION_RESULT_PANICS_ON_DESTRUCTION_IF_UNHANDLED
			m_handled = true;
#endif // HYPERION_RESULT_PANICS_ON_DESTRUCTION_IF_UNHANDLED
			if(is_err()) {
				return Some(this->extract_err());
			}
			else {
				return None();
			}
		}

		/// @brief Maps this `Result` to another one with a potentially different `Ok` type.
		///
		/// If this is the `Ok` variant, invokes `map_func` with a const reference to the contained
		/// value and returns the result in a new `Result`. Otherwise, copies the error into a new
		/// `Result`.
		///
		/// @tparam F - The type of the invocable that maps `T`
		/// @tparam U - The type that the invocable `F` maps `T` to. This will be deduced. Don't
		/// explicitly provide this
		/// @param map_func - The invocable to perform the mapping
		///
		/// @return the result of `map_func` wrapped in another `Result` if this is `Ok`, otherwise
		/// a copy of the error in another `Result`
		/// @ingroup result
		/// @headerfile "Hyperion/Result.h"
		template<typename F,
				 typename U = decltype(std::declval<F>()(std::declval<ok_const_reference>()))>
		requires concepts::InvocableWithReturn<U, F, ok_const_reference> && concepts::
			NoexceptMovable<U> && concepts::NoexceptCopyable<E>
		[[nodiscard]] inline auto map(F&& map_func) const noexcept -> Result<U, E> {
			// the invocable checks above are probably redundant because of the inferred template
			// parameters, but we'll keep them for completeness’ sake and clarity of requirements
#if HYPERION_RESULT_PANICS_ON_DESTRUCTION_IF_UNHANDLED
			m_handled = true;
#endif // HYPERION_RESULT_PANICS_ON_DESTRUCTION_IF_UNHANDLED
			if(is_ok()) {
				return hyperion::Ok<U>(std::forward<F>(map_func)(this->get()));
			}
			else {
				return hyperion::Err<E>(this->get_err());
			}
		}

		/// @brief Maps this `Result` to a `U`.
		///
		/// If this is the `Ok` variant, invokes `map_func` with a const reference to the contained
		/// value and returns the result. Otherwise, returns `default_value`.
		///
		/// @tparam F - The type of the invocable that maps `T` to `U`
		/// @tparam U - The type to map to
		/// @param map_func - The invocable to perform the mapping
		/// @param default_value - The default value to return if this is the `Err` variant
		///
		/// @return The result of the mapping if this is `Ok`, otherwise `default_value`
		/// @ingroup result
		/// @headerfile "Hyperion/Result.h"
		template<typename F, typename U>
		requires concepts::InvocableWithReturn<U, F, ok_const_reference>
		[[nodiscard]] inline auto map_or(F&& map_func, U&& default_value) const noexcept -> U {
#if HYPERION_RESULT_PANICS_ON_DESTRUCTION_IF_UNHANDLED
			m_handled = true;
#endif // HYPERION_RESULT_PANICS_ON_DESTRUCTION_IF_UNHANDLED
			if(is_ok()) {
				return std::forward<F>(map_func)(this->get());
			}
			else {
				return std::forward<U>(default_value);
			}
		}

		/// @brief Maps this `Result` to a `U`.
		///
		/// If this is the `Ok` variant, invokes `map_func` with a const reference to the contained
		/// value and returns the result. Otherwise, returns the result of invoking
		/// `default_generator`
		///
		/// @tparam F - The type of the invocable that maps `T`
		/// @tparam G - The type of the invocable that generates the default value
		/// @tparam U - The type that the invocable `F` maps `T` to. This will be deduced. Don't
		/// explicitly provide this.
		/// @tparam V - The type that the invocable `G` generates. This will be deduced. Don't
		/// explicitly provide this.
		/// @param map_func - The invocable to perform the mapping
		/// @param default_generator - The invocable to generate the default value
		///
		/// @note `map_func` and `default_generator` must return the same type
		///
		/// @return The result of the mapping if this is `Ok`, otherwise the result of invoking
		/// `default_generator`
		/// @ingroup result
		/// @headerfile "Hyperion/Result.h"
		template<typename F,
				 concepts::Invocable G,
				 typename U = decltype(std::declval<F>()(std::declval<ok_const_reference>())),
				 typename V = decltype(std::declval<G>()())>
		requires concepts::Same<U, V> && concepts::InvocableWithReturn<U, F, ok_const_reference>
		[[nodiscard]] inline auto
		map_or_else(F&& map_func, G&& default_generator) const noexcept -> U {
			// the invocable checks above are probably redundant because of the inferred template
			// parameters, but we'll keep them for completeness’ sake and clarity of requirements
#if HYPERION_RESULT_PANICS_ON_DESTRUCTION_IF_UNHANDLED
			m_handled = true;
#endif // HYPERION_RESULT_PANICS_ON_DESTRUCTION_IF_UNHANDLED
			if(is_ok()) {
				return std::forward<F>(map_func)(this->get());
			}
			else {
				return std::forward<G>(default_generator)();
			}
		}

		/// @brief Maps this `Result` to a another one, with a potentially different `Error` type.
		///
		/// If this is the `Ok` variant, copies the contained value into a new `Result`. Otherwise,
		/// returns the result of invoking `map_func` with a const reference to the contained error.
		///
		/// @tparam F - The type of the invocable that maps `E`
		/// @tparam U - The type that the invocable `F` maps `E` to. This will be deduced. Don't
		/// explicitly provide this.
		/// @param map_func - The function to perform the mapping
		///
		/// @return the contained value copied into a new `Result` if this is `Ok`. Otherwise,
		/// the result of the mapping
		/// @ingroup result
		/// @headerfile "Hyperion/Result.h"
		template<typename F,
				 typename U = decltype(std::declval<F>()(std::declval<err_const_reference>()))>
		requires concepts::InvocableWithReturn<U, F, err_const_reference> && concepts::
			NoexceptCopyable<T> && concepts::NoexceptCopyable<E>
		[[nodiscard]] inline auto map_err(F&& map_func) const noexcept -> Result<T, U> {
			// the invocable checks above are probably redundant because of the inferred template
			// parameters, but we'll keep them for completeness’ sake and clarity of requirements
#if HYPERION_RESULT_PANICS_ON_DESTRUCTION_IF_UNHANDLED
			m_handled = true;
#endif // HYPERION_RESULT_PANICS_ON_DESTRUCTION_IF_UNHANDLED
			if(is_err()) {
				return hyperion::Err<U>(std::forward<F>(map_func)(this->get_err()));
			}
			else {
				return hyperion::Ok<T>(this->get());
			}
		}

		/// @brief Matches this `Result` to a consuming function, depending on whether it is the
		/// `Ok` or `Err` variant.
		///
		/// If this is the `Ok` variant, the contained `T` is moved out of this `Result` and
		/// `ok_func` is called with it as its parameter.
		/// Otherwise, this is the `Err` variant, and the contained `E` is moved out of this
		/// `Result` and `err_func` is called with it as its parameter.
		///
		/// @tparam OkFunc - The type of the function to consume the contained value with if this
		/// is `Ok`
		/// @tparam ErrFunc - The type of the function to consume the error value with if this is
		/// `Err`
		/// @tparam R1 - The type returned by `ok_func`. This is deduced. Don't explicitly provide
		/// this
		/// @tparam R2 - The type returned by `err_func`. This is deduced. Don't explicitly provide
		/// this
		///
		/// @param ok_func - The function to consume the contained value with if this is `Ok`
		/// @param err_func - The function to consume the error value with if this is `Err`
		/// @return The return value of `ok_func` if this is `Ok`, otherwise the return value
		/// of `err_func`
		/// @note The returned type of `ok_func` and `err_func` must be the same
		/// @ingroup result
		/// @headerfile "Hyperion/Result.h"
		template<
			typename OkFunc,
			typename ErrFunc,
			typename R1 = decltype(std::declval<OkFunc>()(std::declval<ok_rvalue_reference>())),
			typename R2 = decltype(std::declval<ErrFunc>()(std::declval<err_rvalue_reference>()))>
		requires concepts::Same<R1, R2> && concepts::
			InvocableWithReturn<R1, OkFunc, ok_rvalue_reference> && concepts::
				InvocableWithReturn<R2, ErrFunc, err_rvalue_reference> && concepts::NoexceptMovable<
					T> && concepts::NoexceptMovable<E>
		inline auto match(OkFunc&& ok_func, ErrFunc&& err_func) noexcept -> R1 {
#if HYPERION_RESULT_PANICS_ON_DESTRUCTION_IF_UNHANDLED
			m_handled = true;
#endif // HYPERION_RESULT_PANICS_ON_DESTRUCTION_IF_UNHANDLED
			if(is_ok()) {
				return std::forward<OkFunc>(ok_func)(this->extract());
			}
			else {
				return std::forward<ErrFunc>(err_func)(this->extract_err());
			}
		}

		/// @brief Continues control flow into `func` if this is the `Ok` variant, otherwise returns
		/// the `Err` value contained in this. In either case, consumes this `Result`.
		///
		/// @tparam F - The type of invocable to call if this is `Ok`
		/// @tparam U - The rvalue reference type of the `Ok` variant of the `Result` returned by
		/// `func`. This is deduced. Don't explicitly provide this.
		/// @tparam R - The type of the `Ok` variant of the `Result` returned by `func`. This is
		/// deduced. Don't explicitly provide this.
		/// @param func - The invocable to call if this is `Ok`
		///
		/// @return the result of invoking `func` with the contained value, otherwise moves the
		/// error value into a new `Result`
		/// @ingroup result
		/// @headerfile "Hyperion/Result.h"
		template<typename F,
				 typename U = typename decltype(std::declval<F>()(
					 std::declval<ok_rvalue_reference>()))::ok_rvalue_reference,
				 typename R
				 = std::conditional_t<std::is_rvalue_reference_v<U>, std::remove_reference_t<U>, U>>
		requires concepts::InvocableWithReturn<Result<R, E>, F, ok_rvalue_reference> && concepts::
			NoexceptMovable<T> && concepts::NoexceptMovable<E>
		[[nodiscard]] inline auto and_then(F&& func) noexcept -> Result<R, E> {
			// the invocable checks above are probably redundant because of the inferred template
			// parameters, but we'll keep them for completeness’ sake and clarity of requirements
#if HYPERION_RESULT_PANICS_ON_DESTRUCTION_IF_UNHANDLED
			m_handled = true;
#endif // HYPERION_RESULT_PANICS_ON_DESTRUCTION_IF_UNHANDLED
			if(is_ok()) {
				return std::forward<F>(func)(this->extract());
			}
			else {
				return hyperion::Err<E>(this->extract_err());
			}
		}

		/// @brief Forwards the value contained in this into a new `Result`, consuming this
		/// `Result`.
		///
		/// If this is the `Ok` variant, moves the contained value out of this into a new `Result`
		/// and returns it. Otherwise, returns `result`.
		///
		/// @tparam F - The type of the `Err` variant of `result`
		/// @param result - The next `Result` to potentially use
		///
		/// @return `result` if this is `Err`, the contained value moved into a new `Result`
		/// otherwise
		/// @ingroup result
		/// @headerfile "Hyperion/Result.h"
		template<typename F>
		requires concepts::NoexceptMovable<T>
		[[nodiscard]] inline auto or_else(Result<T, F>&& result) const noexcept -> Result<T, F> {
#if HYPERION_RESULT_PANICS_ON_DESTRUCTION_IF_UNHANDLED
			m_handled = true;
#endif // HYPERION_RESULT_PANICS_ON_DESTRUCTION_IF_UNHANDLED
			if(is_ok()) {
				return hyperion::Ok<T>(this->extract());
			}
			else {
				return std::move(result);
			}
		}

		/// @brief Forwards the value contained in this into a new `Result`, consuming this
		/// `Result`.
		///
		/// If this is the `Ok` variant, moves the contained value out of this into a new `Result`
		/// and returns it. Otherwise, moves the contained error out of this and returns the result
		/// of invoking `func` with it.
		///
		/// @tparam F - The type of invocable to call if this is `Err`
		/// @tparam U - The type of the `Err` variant of the `Result` returned by `F`. This is
		/// deduced. Don't explicitly provide this.
		/// @param func - The invocable to call if this is `Err`
		///
		/// @return the result of invoking func with the contained error if this is `Err`,
		/// otherwise the contained value in a new `Result`
		/// @ingroup result
		/// @headerfile "Hyperion/Result.h"
		template<typename F,
				 typename U = typename decltype(std::declval<F>()(
					 std::declval<err_rvalue_reference>()))::err_rvalue_reference,
				 typename R
				 = std::conditional_t<std::is_rvalue_reference_v<U>, std::remove_reference_t<U>, U>>
		requires concepts::InvocableWithReturn<Result<T, R>, F, err_rvalue_reference> && concepts::
			NoexceptMovable<T> && concepts::NoexceptMovable<E>
		[[nodiscard]] inline auto or_else(F&& func) noexcept -> Result<T, R> {
			// the invocable checks above are probably redundant because of the inferred template
			// parameters, but we'll keep them for completeness’ sake and clarity of requirements
#if HYPERION_RESULT_PANICS_ON_DESTRUCTION_IF_UNHANDLED
			m_handled = true;
#endif // HYPERION_RESULT_PANICS_ON_DESTRUCTION_IF_UNHANDLED
			if(is_ok()) {
				return hyperion::Ok<T>(this->extract());
			}
			else {
				return std::forward<F>(func)(this->extract_err());
			}
		}

		/// @brief Boolean conversion operator. Returns true if this is the `Ok` variant
		///
		/// @return true if this is `Ok`, false otherwise
		/// @ingroup result
		/// @headerfile "Hyperion/Result.h"
		constexpr operator bool() const noexcept { // NOLINT
#if HYPERION_RESULT_PANICS_ON_DESTRUCTION_IF_UNHANDLED
			m_handled = true;
#endif // HYPERION_RESULT_PANICS_ON_DESTRUCTION_IF_UNHANDLED
			return is_ok();
		}

		/// @brief Copy assignment operator
		/// @ingroup result
		/// @headerfile "Hyperion/Result.h"
		constexpr auto
		operator=(const Result& result) noexcept(concepts::NoexceptCopyAssignable<ResultData>)
			-> Result& requires concepts::CopyAssignable<ResultData> {
			if(this == &result) {
				return *this;
			}

			ResultData::operator=(static_cast<const ResultData&>(result));

#if HYPERION_RESULT_PANICS_ON_DESTRUCTION_IF_UNHANDLED
			this->m_handled = result.m_handled;
			result.m_handled = true;
#endif // HYPERION_RESULT_PANICS_ON_DESTRUCTION_IF_UNHANDLED
			return *this;
		}

		/// @brief Move assignment operator.
		/// Moving a `Result` consumes it, leaving a disengaged (valueless) `Result` in its place
		/// @ingroup result
		/// @headerfile "Hyperion/Result.h"
		constexpr auto
		operator=(Result&& result) noexcept(concepts::NoexceptMoveAssignable<ResultData>)
			-> Result& requires concepts::MoveAssignable<ResultData> {
			if(this == &result) {
				return *this;
			}

			ResultData::operator=(static_cast<ResultData&&>(result));

#if HYPERION_RESULT_PANICS_ON_DESTRUCTION_IF_UNHANDLED
			this->m_handled = result.m_handled;
			result.m_handled = true;
#endif // HYPERION_RESULT_PANICS_ON_DESTRUCTION_IF_UNHANDLED
			static_cast<ResultData&>(result) = ResultData(None());
			return *this;
		}

	  private :
#if HYPERION_RESULT_PANICS_ON_DESTRUCTION_IF_UNHANDLED
		  /// whether this `Result` has been handled
		  mutable bool m_handled
		  = false;
#endif // HYPERION_RESULT_PANICS_ON_DESTRUCTION_IF_UNHANDLED
	};
	IGNORE_PADDING_STOP
} // namespace hyperion
