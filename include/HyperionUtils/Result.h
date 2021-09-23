/// @brief `Result` represents the outcome of an operation that can fail recoverably
#pragma once

#include "Concepts.h"
#include "Error.h"
#include "FmtIO.h"
#include "HyperionDef.h"
#include "Ignore.h"
#include "option/None.h"
#include "option/SomeFWD.h"
#include "result/Err.h"
#include "result/Ok.h"
#include "result/ResultData.h"

#ifndef HYPERION_RESULT_PANICS_ON_DESTRUCTION_IF_UNHANDLED
	#define HYPERION_RESULT_PANICS_ON_DESTRUCTION_IF_UNHANDLED false
#endif // HYPERION_RESULT_PANICS_ON_DESTRUCTION_IF_UNHANDLED

namespace hyperion {
	template<typename T>
	class Option;

	using option::None;

	IGNORE_PADDING_START
	/// @brief Represents the outcome of an operation that can fail recoverably
	///
	/// Every `Result` is either `Ok`, indicating success and containing a value
	/// or `Err`, indicating failure and containing an `Error` type
	/// `T` can be any type or pointer
	/// `E` can be any `ErrorType`, a type deriving from `error::ErrorBase`
	///
	/// @note While any combination of `T` and `E` can be pointers, `Result` will
	/// __NOT__ take ownership of any pointer it contains, and as such it is the
	/// responsibility of the user to call the destructor on a pointer contained in a
	/// `Result`. To have a `Result` take ownership of a pointer, use `std::unique_ptr` instead of a
	/// raw pointer.
	/// @note `T` and `E` cannot be references. To make a `Result` store a reference, use
	/// `std::reference_wrapper` instead.
	///
	/// @tparam T - the type contained in the case of success
	/// @tparam E - the `Error` type contained in the case of failure
	template<typename T, typename E = error::SystemError>
	class [[nodiscard]] Result final : private result::ResultData<T, E> {
	  public:
		using ResultData = result::ResultData<T, E>;
		using ok_type = typename ResultData::ok_type;
		using ok_pointer = typename ResultData::ok_pointer;
		using ok_pointer_to_const = typename ResultData::ok_pointer_to_const;
		using ok_reference = typename ResultData::ok_reference;
		using ok_const_reference = typename ResultData::ok_const_reference;
		using ok_rvalue_reference = typename ResultData::ok_extracted;
		using err_type = typename ResultData::err_type;
		using err_pointer = typename ResultData::err_pointer;
		using err_pointer_to_const = typename ResultData::err_pointer_to_const;
		using err_reference = typename ResultData::err_reference;
		using err_const_reference = typename ResultData::err_const_reference;
		using err_rvalue_reference = typename ResultData::err_extracted;

		/// @brief Default Constructor
		constexpr Result() noexcept = default;
		// clang-format off

		/// lvalue constructor
		constexpr explicit Result(const T& ok) noexcept
			requires concepts::NoexceptCopyConstructible<T>
			: ResultData(ok) {
		}
		/// rvalue contructor
		constexpr explicit Result(T&& ok) noexcept
			requires concepts::NoexceptMoveConstructible<T>
			: ResultData(std::move(ok)) {
		}
		template<typename... Args>
		requires concepts::ConstructibleFrom<T, Args...> && concepts::NotReference<T>
		constexpr explicit Result(Args&&... args) noexcept // NOLINT
			: ResultData(std::forward<Args>(args)...) {
		}
		/// lvalue constructor
		constexpr explicit Result(const E& err) noexcept
			requires concepts::NoexceptCopyConstructible<E>
			: ResultData(err) {
		}
		/// rvalue contructor
		constexpr explicit Result(E&& err) noexcept
			requires concepts::NoexceptMoveConstructible<E>
			: ResultData(std::move(err)) {
		}
		template<typename... Args>
		requires concepts::ConstructibleFrom<E, Args...> && concepts::NotReference<E>
		constexpr explicit Result(Args&&... args) noexcept // NOLINT
			: ResultData(std::forward<Args>(args)...) {
		}
		// clang-format on

		/// @brief Constructs a `Result` from an `Err`
		///
		/// @param error - The error indicating failure
		Result(const hyperion::Err<E>& error) noexcept // NOLINT
			requires concepts::NoexceptCopyConstructible<E> : ResultData(error.m_error) {
		}
		/// @brief Constructs a `Result` from an `Err`
		///
		/// @param error - The error indicating failure
		Result(hyperion::Err<E>&& error) noexcept // NOLINT
			requires concepts::NoexceptMoveConstructible<E> : ResultData(std::move(error.m_error)) {
		}
		/// @brief Constructs a `Result` from an `Ok`
		///
		/// @param ok - The value indicating success
		constexpr Result(const hyperion::Ok<T>& ok) noexcept // NOLINT
			requires concepts::NoexceptCopyConstructible<T> : ResultData(ok.m_ok) {
		}
		/// @brief Constructs a `Result` from an `Ok`
		///
		/// @param ok - The value indicating success
		constexpr Result(hyperion::Ok<T>&& ok) noexcept // NOLINT
			requires concepts::NoexceptMoveConstructible<T> : ResultData(std::move(ok.m_ok)) {
		}
		/// @brief Copy Constructor
		constexpr Result(const Result& result) noexcept(
			concepts::NoexceptCopyConstructible<ResultData>) requires
			concepts::CopyConstructible<ResultData>
			: ResultData(static_cast<const ResultData&>(result)),
			  m_handled(result.m_handled) {
			result.m_handled = true;
		}
		/// @brief Move Constructor
		/// Moving a `Result` consumes it, leaving a disengaged (valueless) `Result` in its place
		constexpr Result(Result&& result) noexcept(
			concepts::NoexceptMoveConstructible<ResultData>) requires
			concepts::MoveConstructible<ResultData> : ResultData(static_cast<ResultData&&>(result)),
													  m_handled(result.m_handled) {
			result.m_handled = true;
			static_cast<ResultData&>(result) = None();
		}
#if HYPERION_RESULT_PANICS_ON_DESTRUCTION_IF_UNHANDLED
		/// @brief Destructor
		constexpr ~Result() noexcept(concepts::NoexceptDestructible<ResultData>) {
			if(!m_handled && !this->is_empty()) {
				m_handled = true;
				panic("Unhandled Result that must be handled being destroyed, terminating");
			}
		}
#else
		constexpr ~Result() noexcept(concepts::NoexceptDestructible<ResultData>) = default;
#endif

		/// @brief Constructs a `Result` as the `Ok` variant, containing `ok`
		///
		/// @param ok - the success value
		///
		/// @return `Ok`
		[[nodiscard]] constexpr static inline auto
		Ok(const T& ok) noexcept -> Result requires concepts::NotReference<T> {
			return Result(ok);
		}

		/// @brief Constructs a `Result` as the `Ok` variant, containing `ok`
		///
		/// @param ok - the success value
		///
		/// @return `Ok`
		[[nodiscard]] constexpr static inline auto Ok(T&& ok) noexcept -> Result {
			return Result(std::move(ok));
		}

		/// @brief Constructs a `Result` as the `Ok` variant, containing `ok`
		///
		/// @param ok - the success value
		///
		/// @return `Ok`
		[[nodiscard]] constexpr static inline auto
		Ok(const hyperion::Ok<T>& ok) noexcept -> Result {
			return Result(ok);
		}

		/// @brief Constructs a `Result` as the `Ok` variant, containing `ok`
		///
		/// @param ok - the success value
		///
		/// @return `Ok`
		[[nodiscard]] constexpr static inline auto Ok(hyperion::Ok<T>&& ok) noexcept -> Result {
			return Result(std::move(ok));
		}

		/// @brief Constructs a `Result` as the `Err` variant, containing `err`
		///
		/// @param err - the failure value
		///
		/// @return `Err`
		[[nodiscard]] constexpr static inline auto Err(const E& err) noexcept -> Result<T, E>
		requires concepts::NotReference<E> {
			return Result(err);
		}

		/// @brief Constructs a `Result` as the `Err` variant, containing `err`
		///
		/// @param err - the failure value
		///
		/// @return `Err`
		[[nodiscard]] constexpr static inline auto Err(E&& err) noexcept -> Result<T, E> {
			return Result(std::move(err));
		}

		/// @brief Constructs a `Result` as the `Err` variant, containing `err`
		///
		/// @param err - the failure value
		///
		/// @return `Err`
		[[nodiscard]] constexpr static inline auto
		Err(const hyperion::Err<E>& err) noexcept -> Result<T, E> {
			return Result(err);
		}

		/// @brief Constructs a `Result` as the `Err` variant, containing `err`
		///
		/// @param err - the failure value
		///
		/// @return `Err`
		[[nodiscard]] constexpr static inline auto
		Err(hyperion::Err<E>&& err) noexcept -> Result<T, E> {
			return Result(std::move(err));
		}

		/// @brief Returns whether this `Result` is the `Ok` variant
		///
		/// @return true if this is `Ok`, otherwise `false`
		[[nodiscard]] constexpr auto is_ok() const noexcept -> bool {
			m_handled = true;
			return this->has_ok();
		}

		/// @brief Returns whether this `Result` is the `Err` variant
		///
		/// @return true if this is `Err`, otherwise `false`
		[[nodiscard]] constexpr inline auto is_err() const noexcept -> bool {
			m_handled = true;
			return this->has_err();
		}

		/// @brief Similar to `unwrap`, except doesn't consume this `Result`.
		/// Returns a pointer to the non-const `Ok` value if this is `Ok`, otherwise calls
		/// `std::terminate`
		///
		/// @note If `T` is a pointer, then this returns a pointer to the underlying type of `T`,
		/// ie, it just returns `T`
		///
		/// @return A pointer to non-const `T`
		[[nodiscard]] constexpr inline auto as_mut() noexcept -> ok_reference {
			m_handled = true;
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

		/// @brief Similar to `unwrap`, except doesn't consume this `Result`.
		/// Returns a pointer to the const `Ok` value if this is `Ok`, otherwise calls
		/// `std::terminate`
		///
		/// @note If `T` is a pointer, then this returns a pointer to const of the underlying type
		/// of `T`, ie, it just returns `const T`
		///
		/// @return A pointer to const `T`
		[[nodiscard]] constexpr inline auto as_const() const noexcept -> ok_const_reference {
			m_handled = true;
			if(is_ok()) {
				return this->get();
			}
			else {
				panic("Result::as_const called on an Error result, terminating");
			}
		}

		/// @brief Returns the contained `T`, consuming this `Result`.
		/// If this is not `Ok`, then `std::terminate` is called
		///
		/// @return The contained `T`
		[[nodiscard]] constexpr inline auto
		unwrap() noexcept -> ok_rvalue_reference requires concepts::NoexceptMovable<T> {
			m_handled = true;
			if(is_ok()) {
				return this->extract();
			}
			else {
				panic("Result::unwrap called on an Error result, terminating");
			}
		}

		/// @brief Returns the contained `T` if this is `Ok`, consuming this `Result`
		/// If this is not `Ok`, then returns `default_value`
		///
		/// @param default_value - The value to return if this is `Err`
		///
		/// @return The contained `T` if this is `Ok`, or `default_value`
		[[nodiscard]] constexpr inline auto unwrap_or(T& default_value) noexcept
			-> ok_rvalue_reference requires concepts::NotReference<T> {
			m_handled = true;
			if(is_ok()) {
				return unwrap();
			}
			else {
				return std::move(default_value);
			}
		}

		/// @brief Returns the contained `T` if this is `Ok`, consuming this `Result`
		/// If this is not `Ok`, then returns `default_value`
		///
		/// @param default_value - The value to return if this is `Err`
		///
		/// @return The contained `T` if this is `Ok`, or `default_value`
		[[nodiscard]] constexpr inline auto
		unwrap_or(T&& default_value) noexcept -> ok_rvalue_reference {
			m_handled = true;
			if(is_ok()) {
				return unwrap();
			}
			else {
				return std::move(default_value);
			}
		}

		/// @brief Returns the contained `T` if this is `Ok`, consuming this `Result`.
		/// If this is not `Ok`, then returns the value generated by `default_generator`
		///
		/// @tparam F - The type of the `default_generator` invocable
		/// @param default_generator - The function to generate the value returned if this is
		/// `Err`
		///
		/// @return  The contained `T` if this is `Ok`, or the value generated by
		/// `default_generator`
		template<typename F>
		requires concepts::InvocableR<T, F>
		[[nodiscard]] inline auto unwrap_or_else(F&& default_generator) noexcept -> T {
			m_handled = true;
			if(is_ok()) {
				return unwrap();
			}
			else {
				return std::forward<F>(default_generator)();
			}
		}

		/// @brief Returns the contained `T` if this is `Ok`, consuming this `Result`.
		///
		/// If this is not the `Ok` variant, invokes a panic with the given `panic_message`.
		///
		/// @param panic_message - The panic message to print if this is not the `Ok` variant
		///
		/// @return The contained `T` if this is `Ok`
		template<typename U>
		requires concepts::Convertible<U, std::string> || concepts::Convertible<U, std::string_view>
		[[nodiscard]] inline auto expect(U&& panic_message) noexcept -> ok_rvalue_reference {
			m_handled = true;
			if(is_ok()) {
				return unwrap();
			}
			else {
				panic("{}", std::forward<U>(panic_message));
			}
		}

		/// @brief Returns the contained `E` if this is `Err`, consuming this `Result`.
		/// If this is not `Err`, then `std::terminate` is called
		///
		/// @return The contained `E`
		[[nodiscard]] constexpr inline auto unwrap_err() noexcept -> err_rvalue_reference {
			m_handled = true;
			if(is_err()) {
				return this->extract_err();
			}
			else {
				panic("Result::unwrap_err called on an Ok result, terminating");
			}
		}

		/// @brief Converts this `Result` to an `Option<T>`,
		/// consuming this `Result` and discarding the error, if any
		///
		/// @return `Option<T>`
		[[nodiscard]] constexpr inline auto ok() noexcept -> Option<T> {
			m_handled = true;
			if(is_ok()) {
				return Some(this->extract());
			}
			else {
				return None();
			}
		}

		/// @brief Converts this `Result` to an `Option<E>`,
		/// consuming this `Result` and discarding the success value, if any
		///
		/// @return `Option<E>`
		[[nodiscard]] constexpr inline auto err() noexcept -> Option<E> {
			m_handled = true;
			if(is_err()) {
				return Some(this->extract_err());
			}
			else {
				return None();
			}
		}

		/// @brief Maps this `Result` another one with a potentially different `Ok` type.
		///
		/// If this is `Ok`, returns `Ok(map_func(T))`.
		/// If this is `Err`, returns `Err(E)`.
		///
		/// @tparam F - The type of the invocable that maps `T`
		/// @tparam U - The type that the invocable `F` maps `T` to. This will be deduced. Don't
		/// explicitly provide this
		/// @param map_func - The invocable to perform the mapping
		///
		/// @return `Ok(map_func(T))` if this is `Ok`, otherwise `Err(E)`
		template<typename F,
				 typename U = decltype(std::declval<F>()(std::declval<ok_const_reference>()))>
		requires concepts::InvocableRConst<U, F, T>
		[[nodiscard]] inline auto map(F&& map_func) const noexcept -> Result<U, E> {
			// the invocable checks above are probably redundant because of the inferred template
			// parameters, but we'll keep them for completeness sake and clarity of requirements
			m_handled = true;
			if(is_ok()) {
				return hyperion::Ok<U>(std::forward<F>(map_func)(this->get()));
			}
			else {
				return hyperion::Err<E>(this->get_err());
			}
		}

		/// @brief Maps this `Result` to a `U`,
		/// returning `U` (mapped by `map_func`) if this is `Ok`,
		/// or `default_value` if this is `Err`
		///
		/// @tparam F - The type of the invocaable that maps `T` to `U`
		/// @tparam U - The type to map to
		/// @param map_func - The invocable to perform the mapping
		/// @param default_value - The default value
		///
		/// @return The result of the mapping if this is `Ok`, or `default_value`
		template<typename F, typename U>
		requires concepts::InvocableRConst<U, F, T>
		[[nodiscard]] inline auto map_or(F&& map_func, U&& default_value) const noexcept -> U {
			m_handled = true;
			if(is_ok()) {
				return std::forward<F>(map_func)(this->get());
			}
			else {
				return std::forward<U>(default_value);
			}
		}

		/// @brief Maps the value of this `Result` to another value.
		///
		/// If this is `Ok`, the returned value is the one generated by `map_func`.
		/// In the case this is `Err`, the returned value is the one generated by
		/// `default_generator`.
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
		/// @return `map_func(T)` if this is `Ok`, otherwise `default_generator()`
		template<typename F,
				 typename G,
				 typename U = decltype(std::declval<F>()(std::declval<ok_const_reference>())),
				 typename V = decltype(std::declval<G>()())>
		requires concepts::Same<U, V> && concepts::InvocableRConst<U, F, T> && concepts::
			InvocableR<V, G>
		[[nodiscard]] inline auto
		map_or_else(F&& map_func, G&& default_generator) const noexcept -> U {
			// the invocable checks above are probably redundant because of the inferred template
			// parameters, but we'll keep them for completeness sake and clarity of requirements
			m_handled = true;
			if(is_ok()) {
				return std::forward<F>(map_func)(this->get());
			}
			else {
				return std::forward<G>(default_generator)();
			}
		}

		/// @brief Maps this `Result` to a another one, with a potentially different `Error` type.
		///
		/// If this is `Ok`, this returns `Ok(T)`, otherwise, this returns `Err(map_func(E))`
		///
		/// @tparam F - The type of the invocable that maps `E`
		/// @tparam U - The type that the invocable `F` maps `E` to. This will be deduced. Don't
		/// explicitly provide this.
		/// @param map_func - The function to perform the mapping
		///
		/// @return `Ok(T)` if this is `Ok`, or `Err(map_func(E))` if this is `Err`
		template<typename F,
				 typename U = decltype(std::declval<F>()(std::declval<err_const_reference>()))>
		requires concepts::InvocableRConst<U, F, err_const_reference>
		[[nodiscard]] inline auto map_err(F&& map_func) const noexcept -> Result<T, U>
		requires concepts::NoexceptCopyable<T> {
			// the invocable checks above are probably redundant because of the inferred template
			// parameters, but we'll keep them for completeness sake and clarity of requirements
			m_handled = true;
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
		///
		/// @param ok_func - The function to consume the contained value with if this is `Ok`
		/// @param err_func - The function to consume the error value with if this is `Err`
		/// @return The return value of `ok_func` if this is `Ok`, otherwise the return value
		/// of `err_func`
		/// @note The returned type of `ok_func` and `err_func` must be the same
		template<
			typename OkFunc,
			typename ErrFunc,
			typename R1 = decltype(std::declval<OkFunc>()(std::declval<ok_rvalue_reference>())),
			typename R2 = decltype(std::declval<ErrFunc>()(std::declval<err_rvalue_reference>()))>
		requires concepts::Same<R1, R2> && concepts::InvocableR<
			void,
			OkFunc,
			ok_rvalue_reference> && concepts::InvocableR<void, ErrFunc, err_rvalue_reference>
		inline auto match(OkFunc&& ok_func, ErrFunc&& err_func) noexcept -> R1 {
			m_handled = true;
			if(is_ok()) {
				return std::forward<OkFunc>(ok_func)(this->extract());
			}
			else {
				return std::forward<ErrFunc>(err_func)(this->extract_err());
			}
		}

		/// @brief Continues control flow into `func` if this is the `Ok` variant, otherwise returns
		/// the `Err` value contained in this.
		///
		/// @tparam F - The type of invocable to call if this is `Ok`
		/// @tparam U - The type of the `Ok` variant of the `Result` returned by `F`. This is
		/// deduced. Don't explicitly provide this.
		/// @param func - The invocable to call if this is `Ok`
		///
		/// @return `func()` if this is `Ok`, otherwise `Err`
		template<typename F,
				 typename U = typename decltype(std::declval<F>()(
					 std::declval<ok_type>()))::ok_rvalue_reference,
				 typename R
				 = std::conditional_t<std::is_rvalue_reference_v<U>, std::remove_reference_t<U>, U>>
		requires concepts::InvocableRMut<Result<R, E>, F, ok_type>
		[[nodiscard]] inline auto and_then(F&& func) noexcept -> Result<R, E> {
			// the invocable checks above are probably redundant because of the inferred template
			// parameters, but we'll keep them for completeness sake and clarity of requirements
			m_handled = true;
			if(is_ok()) {
				return std::forward<F>(func)(this->extract());
			}
			else {
				return hyperion::Err<E>(this->extract_err());
			}
		}

		/// @brief Continues control flow into `func` if this is the `Ok` variant, otherwise returns
		/// the `Err` value contained in this.
		///
		/// @tparam F - The type of invocable to call if this is `Ok`
		/// @tparam U - The type of the `Ok` variant of the `Result` returned by `F`. This is
		/// deduced. Don't explicitly provide this.
		/// @param func - The invocable to call if this is `Ok`
		///
		/// @return `func()` if this is `Ok`, otherwise `Err`
		template<typename F,
				 typename U = typename decltype(std::declval<F>()(
					 std::declval<ok_reference>()))::ok_rvalue_reference,
				 typename R
				 = std::conditional_t<std::is_rvalue_reference_v<U>, std::remove_reference_t<U>, U>>
		requires concepts::InvocableRMut<Result<R, E>, F, ok_reference>
		[[nodiscard]] inline auto and_then(F&& func) noexcept -> Result<R, E> {
			// the invocable checks above are probably redundant because of the inferred template
			// parameters, but we'll keep them for completeness sake and clarity of requirements
			m_handled = true;
			if(is_ok()) {
				return std::forward<F>(func)(this->extract());
			}
			else {
				return hyperion::Err<E>(this->extract_err());
			}
		}

		/// @brief Continues control flow into `func` if this is the `Ok` variant, otherwise returns
		/// the `Err` value contained in this.
		///
		/// @tparam F - The type of invocable to call if this is `Ok`
		/// @tparam U - The type of the `Ok` variant of the `Result` returned by `F`. This is
		/// deduced. Don't explicitly provide this.
		/// @param func - The invocable to call if this is `Ok`
		///
		/// @return `func()` if this is `Ok`, otherwise `Err`
		template<typename F,
				 typename U = typename decltype(std::declval<F>()(
					 std::declval<ok_rvalue_reference>()))::ok_rvalue_reference,
				 typename R
				 = std::conditional_t<std::is_rvalue_reference_v<U>, std::remove_reference_t<U>, U>>
		requires concepts::InvocableRMut<Result<R, E>, F, ok_rvalue_reference>
		[[nodiscard]] inline auto and_then(F&& func) noexcept -> Result<R, E> {
			// the invocable checks above are probably redundant because of the inferred template
			// parameters, but we'll keep them for completeness sake and clarity of requirements
			m_handled = true;
			if(is_ok()) {
				return std::forward<F>(func)(this->extract());
			}
			else {
				return hyperion::Err<E>(this->extract_err());
			}
		}

		/// @brief Returns `result` if this is the `Err` variant, otherwise returns the `Ok` value
		/// contained in this.
		///
		/// @tparam F - The type of the `Err` variant of `result`
		/// @param result - The next `Result` to potentially use
		///
		/// @return `result` if this is `Err`, `Ok(T)` otherwise
		template<typename F>
		[[nodiscard]] inline auto or_else(Result<T, F>&& result) const noexcept -> Result<T, F> {
			m_handled = true;
			if(is_ok()) {
				return hyperion::Ok<T>(this->extract());
			}
			else {
				return std::move(result);
			}
		}

		/// @brief Continues control flow into `func` if this is the `Err` variant, otherwise
		/// returns the `Ok` value contained in this.
		///
		/// @tparam F - The type of invocable to call if this is `Err`
		/// @tparam U - The type of the `Err` variant of the `Result` returned by `F`. This is
		/// deduced. Don't explicitly provide this.
		/// @param func - The invocable to call if this is `Err`
		///
		/// @return `func()` if this is `Err`, otherwise `Ok(T)`
		template<typename F,
				 typename U = decltype(std::declval<decltype(std::declval<F>()(
										   std::declval<err_reference>()))>()
										   .unwrap_err())>
		requires concepts::InvocableRMut<Result<T, U>, F, E>
		[[nodiscard]] inline auto or_else(F&& func) noexcept -> Result<T, U> {
			// the invocable checks above are probably redundant because of the inferred template
			// parameters, but we'll keep them for completeness sake and clarity of requirements
			m_handled = true;
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
		constexpr operator bool() const noexcept { // NOLINT
			m_handled = true;
			return is_ok();
		}

		/// @brief Deleted copy assignment operator. `Result`s cannot be copied.
		constexpr auto
		operator=(const Result& result) noexcept(concepts::NoexceptCopyAssignable<ResultData>)
			-> Result& requires concepts::CopyAssignable<ResultData> {
			if(this == &result) {
				return *this;
			}

			ResultData::operator=(static_cast<const ResultData&>(result));

			this->m_handled = result.m_handled;
			result.m_handled = true;
			return *this;
		}

		/// @brief Move assignment operator.
		/// Moving a `Result` consumes it, leaving a disengaged (valueless) `Result` in its place
		constexpr auto
		operator=(Result&& result) noexcept(concepts::NoexceptMoveAssignable<ResultData>)
			-> Result& requires concepts::MoveAssignable<ResultData> {
			if(this == &result) {
				return *this;
			}

			ResultData::operator=(static_cast<ResultData&&>(result));

			this->m_handled = result.m_handled;
			result.m_handled = true;
			static_cast<ResultData&>(result) = None();
			return *this;
		}

	  private :
		  /// whether this `Result` has been handled
		  mutable bool m_handled
		  = false;
	};
	IGNORE_PADDING_STOP

	template<typename T, typename E = error::SystemError, typename... Args>
	requires concepts::ConstructibleFrom<T, Args...>
	[[nodiscard]] constexpr inline auto
	make_result(Args&&... args) noexcept(concepts::NoexceptConstructibleFrom<T, Args...>)
		-> Result<T, E> {
		return Result<T, E>(std::forward<Args>(args)...);
	}

	template<typename T, typename E = error::SystemError, typename... Args>
	requires concepts::ConstructibleFrom<E, Args...>
	[[nodiscard]] constexpr inline auto
	make_result(Args&&... args) noexcept(concepts::NoexceptConstructibleFrom<E, Args...>)
		-> Result<T, E> {
		return Result<T, E>(std::forward<Args>(args)...);
	}
} // namespace hyperion
