/// @brief `Result` represents the outcome of an operation that can fail recoverably
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

	using concepts::Same, concepts::NotReference, concepts::CopyConstructible,
		concepts::MoveConstructible, concepts::CopyAssignable, concepts::MoveAssignable,
		concepts::NotMoveAssignable, concepts::NotMoveConstructible, concepts::ConstructibleFrom,
		concepts::Pointer, concepts::NotPointer, concepts::DefaultConstructible,
		concepts::ErrorType, concepts::Invocable, concepts::InvocableR, concepts::Passable,
		concepts::Destructible, concepts::InvocableRConst, concepts::InvocableRMut;

	template<NotReference T>
	class Option;

	IGNORE_PADDING_START
	/// @brief Represents the outcome of an operation that can fail recoverably
	///
	/// Every `Result` is either `Ok`, indicating success and containing a value
	/// or `Err`, indicating failure and containing an `Error` type
	/// `T` can be any type or pointer
	/// `E` can be any `ErrorType`, a type deriving from `hyperion::Error`
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
	template<NotReference T, ErrorType E>
	requires NotReference<E>
	class [[nodiscard]] Result {
	  public:
		/// @brief Default Constructor
		constexpr Result() noexcept = default;
		/// @brief Copy Constructor is deleted. `Result`s cannot be copied
		constexpr Result(const Result& result) = delete;
		/// @brief Move Constructor
		/// Moving a `Result` consumes it, leaving a disengaged (valueless) `Result` in its place
		constexpr Result(Result&& result) noexcept requires MoveAssignable<T> && MoveAssignable<E> {
			this->m_handled = result.m_handled;
			result.m_handled = true;
			this->m_is_ok = result.m_is_ok;
			if(result.m_engaged) {
				if(result.m_is_ok) {
					result.m_is_ok = false;
					if constexpr(Pointer<T>) {
						result.m_engaged = false;
						m_data.m_ok = result.m_data.m_ok;
						result.m_data.m_ok = nullptr;
					}
					else {
						// we don't actually disengage in the non-pointer case, so that the
						// destructor of the moved from `ok` can be run
						m_data.m_ok = std::move(result.m_data.m_ok);
					}
				}
				else {
					if constexpr(Pointer<E>) {
						result.m_engaged = false;
						m_data.m_err = result.m_data.m_err;
						result.m_data.m_err = nullptr;
					}
					else {
						// we don't actually disengage in the non-pointer case, so that the
						// destructor of the moved from `ok` can be run
						m_data.m_err = std::move(result.m_data.m_err);
					}
				}
				m_engaged = true;
			}
		}
		/// @brief Constructs a `Result` from an `Err`
		///
		/// @param error - The error indicating failure
		Result(const hyperion::Err<E>& error) noexcept // NOLINT
			: m_data(error.m_error), m_engaged(true) {
		}
		/// @brief Constructs a `Result` from an `Err`
		///
		/// @param error - The error indicating failure
		Result(hyperion::Err<E>&& error) noexcept // NOLINT
			: m_data(error.m_error), m_engaged(true) {
		}
		/// @brief Constructs a `Result` from an `Ok`
		///
		/// @param ok - The value indicating success
		constexpr Result(const hyperion::Ok<T>& ok) noexcept // NOLINT
			: m_data(ok.m_ok), m_is_ok(true), m_engaged(true) {
		}
		/// @brief Constructs a `Result` from an `Ok`
		///
		/// @param ok - The value indicating success
		constexpr Result(hyperion::Ok<T>&& ok) noexcept // NOLINT
			: m_data(std::move(ok.m_ok)), m_is_ok(true), m_engaged(true) {
		}

		/// @brief Destructor
		~Result() noexcept { // NOLINT
			if(m_engaged) {
				if(m_is_ok) {
					if constexpr(Pointer<T>) {
						m_data.m_ok = nullptr;
					}
					else {
						if(Destructible<T>) {
							m_data.m_ok.~T();
						}
					}
				}
				else {
					if constexpr(Pointer<E>) {
						m_data.m_err = nullptr;
					}
					else {
						if(Destructible<E>) {
							m_data.m_err.~E();
						}
					}
				}
			}
			if(!m_handled) {
				fmt::print(stderr,
						   "Unhandled Result that must be handled being destroyed, terminating\n");
				std::fflush(stderr);
				std::terminate();
			}
		}

		/// @brief Constructs a `Result` as the `Ok` variant, containing `ok`
		///
		/// @param ok - the success value
		///
		/// @return `Ok`
		[[nodiscard]] constexpr static inline auto Ok(const T& ok) noexcept -> Result {
			return Result(ok);
		}

		/// @brief Constructs a `Result` as the `Ok` variant, containing `ok`
		///
		/// @param ok - the success value
		///
		/// @return `Ok`
		[[nodiscard]] constexpr static inline auto Ok(T&& ok) noexcept -> Result {
			return Result(std::forward<T>(ok));
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
			return Result(std::forward<T>(ok));
		}

		/// @brief Constructs a `Result` as the `Err` variant, containing `err`
		///
		/// @param err - the failure value
		///
		/// @return `Err`
		[[nodiscard]] constexpr static inline auto Err(const E& err) noexcept -> Result<T, E> {
			return Result(err);
		}

		/// @brief Constructs a `Result` as the `Err` variant, containing `err`
		///
		/// @param err - the failure value
		///
		/// @return `Err`
		[[nodiscard]] constexpr static inline auto Err(E&& err) noexcept -> Result<T, E> {
			return Result(std::forward<E>(err));
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
			return Result(std::forward<hyperion::Err<E>>(err));
		}

		/// @brief Returns whether this `Result` is the `Ok` variant
		///
		/// @return true if this is `Ok`, otherwise `false`
		[[nodiscard]] constexpr auto is_ok() const noexcept -> bool {
			m_handled = true;
			return m_is_ok;
		}

		/// @brief Returns whether this `Result` is the `Err` variant
		///
		/// @return true if this is `Err`, otherwise `false`
		[[nodiscard]] constexpr inline auto is_err() const noexcept -> bool {
			m_handled = true;
			return !m_is_ok;
		}

		/// @brief Similar to `unwrap`, except doesn't consume this `Result`.
		/// Returns a pointer to the non-const `Ok` value if this is `Ok`, otherwise calls
		/// `std::terminate`
		///
		/// @note If `T` is a pointer, then this returns a pointer to the underlying type of `T`,
		/// ie, it just returns `T`
		///
		/// @return A pointer to non-const `T`
		[[nodiscard]] constexpr inline auto as_mut() noexcept {
			m_handled = true;
			if(m_is_ok) {
				if constexpr(Pointer<T>) {
					return m_data.m_ok;
				}
				else {
					return &(m_data.m_ok);
				}
			}
			else {
				fmt::print(stderr, "as_mut called on an Error result, terminating\n");
				std::fflush(stderr);
				std::terminate();
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
		[[nodiscard]] constexpr inline auto
		as_const() const noexcept -> const T* requires NotPointer<T> {
			m_handled = true;
			if(m_is_ok) {
				return &(m_data.m_ok);
			}
			else {
				fmt::print(stderr, "as_const called on an Error result, terminating\n");
				std::fflush(stderr);
				std::terminate();
			}
		}

		/// @brief Similar to `unwrap`, except doesn't consume this `Result`.
		/// Returns a pointer to the const `T` if this is `Ok`, otherwise calls
		/// `std::terminate`
		///
		/// @note If `T` is a pointer, then this returns a pointer to const of the underlying type
		/// of `T`, ie, it just returns `const T`
		///
		/// @return A pointer (or reference if `T` is a reference) to const `T`
		[[nodiscard]] constexpr inline auto
		as_const() const noexcept -> T const requires Pointer<T> {
			m_handled = true;
			if(m_is_ok) {
				return m_data.m_ok;
			}
			else {
				fmt::print(stderr, "as_const called on an Error result, terminating\n");
				std::fflush(stderr);
				std::terminate();
			}
		}

		/// @brief Returns the contained `T`, consuming this `Result`.
		/// If this is not `Ok`, then `std::terminate` is called
		///
		/// @return The contained `T`
		[[nodiscard]] constexpr inline auto unwrap() noexcept -> T {
			m_handled = true;
			if(m_is_ok) {
				m_is_ok = false;
				m_engaged = false;
				if constexpr(Pointer<T>) {
					auto _ok = m_data.m_ok;
					m_data.m_ok = nullptr;
					return _ok;
				}
				else if constexpr(CopyConstructible<T> && NotMoveConstructible<T>) {
					return m_data.m_ok;
				}
				else {
					return std::move(m_data.m_ok);
				}
			}
			else {
				fmt::print(stderr, "unwrap called on an Error result, terminating\n");
				std::fflush(stderr);
				std::terminate();
			}
		}

		/// @brief Returns the contained `T` if this is `Ok`, consuming this `Result`
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

		/// @brief Returns the contained `T` if this is `Ok`, consuming this `Result`
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
		requires InvocableR<T, F>
		[[nodiscard]] inline auto unwrap_or_else(F&& default_generator) noexcept -> T {
			m_handled = true;
			if(m_is_ok) {
				return unwrap();
			}
			else {
				return std::forward<F>(default_generator)();
			}
		}

		/// @brief Returns the contained `E` if this is `Err`, consuming this `Result`.
		/// If this is not `Err`, then `std::terminate` is called
		///
		/// @return The contained `E`
		[[nodiscard]] constexpr inline auto unwrap_err() noexcept -> E {
			m_handled = true;
			if(!m_is_ok) {
				m_engaged = false;
				if constexpr(Pointer<E>) {
					auto* _err = m_data.m_err;
					m_data.m_err = nullptr;
					return _err;
				}
				else if constexpr(CopyConstructible<E> && NotMoveConstructible<E>) {
					return m_data.m_err;
				}
				else {
					return std::move(m_data.m_err);
				}
			}
			else {
				fmt::print(stderr, "unwrap_err called on an Ok result, terminating\n");
				std::fflush(stderr);
				std::terminate();
			}
		}

		/// @brief Converts this `Result` to an `Option<T>`,
		/// consuming this `Result` and discarding the error, if any
		///
		/// @return `Option<T>`
		[[nodiscard]] constexpr inline auto ok() noexcept -> Option<T> {
			m_handled = true;
			if(m_is_ok) {
				m_is_ok = false;
				m_engaged = false;
				if constexpr(Pointer<T>) {
					auto* _ok = m_data.m_ok;
					m_data.m_ok = nullptr;
					return Some(_ok);
				}
				else if constexpr(CopyConstructible<T> && NotMoveConstructible<T>) {
					return Some(m_data.m_ok);
				}
				else {
					return Some(std::move(m_data.m_ok));
				}
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
			if(!m_is_ok) {
				m_engaged = false;
				if constexpr(Pointer<E>) {
					auto* _err = m_data.m_err;
					m_data.m_err = nullptr;
					return Some(_err);
				}
				else if constexpr(CopyConstructible<E> && NotMoveConstructible<E>) {
					return Some(m_data.m_err);
				}
				else {
					return Some(std::move(m_data.m_err));
				}
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
		template<typename F, NotReference U = decltype(std::declval<F>()(std::declval<const T&>()))>
		requires InvocableRConst<U, F, T>
		[[nodiscard]] inline auto map(F&& map_func) const noexcept -> Result<U, E> {
			// the invocable checks above are probably redundant because of the inferred template
			// parameters, but we'll keep them for completeness sake and clarity of requirements
			m_handled = true;
			if(m_is_ok) {
				return hyperion::Ok(std::forward<F>(map_func)(m_data.m_ok));
			}
			else {
				return hyperion::Err(m_data.m_err);
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
		requires InvocableRConst<U, F, T>
		[[nodiscard]] inline auto map_or(F&& map_func, U&& default_value) const noexcept -> U {
			m_handled = true;
			if(m_is_ok) {
				return std::forward<F>(map_func)(m_data.m_ok);
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
				 typename U = decltype(std::declval<F>()(std::declval<const T&>())),
				 typename V = decltype(std::declval<G>()())>
		requires Same<U, V> && InvocableRConst<U, F, T> && InvocableR<V, G>
		[[nodiscard]] inline auto
		map_or_else(F&& map_func, G&& default_generator) const noexcept -> U {
			// the invocable checks above are probably redundant because of the inferred template
			// parameters, but we'll keep them for completeness sake and clarity of requirements
			m_handled = true;
			if(m_is_ok) {
				return std::forward<F>(map_func)(m_data.m_ok);
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
		template<typename F, NotReference U = decltype(std::declval<F>()(std::declval<const E&>()))>
		requires InvocableRConst<U, F, E> && ErrorType<U>
		[[nodiscard]] inline auto map_err(F&& map_func) const noexcept -> Result<T, U> {
			// the invocable checks above are probably redundant because of the inferred template
			// parameters, but we'll keep them for completeness sake and clarity of requirements
			m_handled = true;
			if(!m_is_ok) {
				return hyperion::Err(std::forward<F>(map_func)(m_data.m_err));
			}
			else {
				return hyperion::Ok(m_data.m_ok);
			}
		}

		/// @brief Returns `result` if this is the `Ok` variant, otherwise returns the `Err` value
		/// contained in this.
		///
		/// @tparam U - The type of the `Ok` variant of `result`
		/// @param result - The next `Result` to potentially use
		///
		/// @return `result` if this is `Ok`, `Err(E)` otherwise
		template<NotReference U>
		[[nodiscard]] inline auto and_then(Result<U, E>&& result) const noexcept -> Result<U, E> {
			m_handled = true;
			if(m_is_ok) {
				return std::forward<Result<U, E>>(result);
			}
			else {
				return hyperion::Err(m_data.m_err);
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
		template<
			typename F,
			NotReference U
			= decltype(std::declval<decltype(std::declval<F>()(std::declval<T&>()))>().unwrap())>
		requires InvocableRMut<Result<U, E>, F, T>
		[[nodiscard]] inline auto and_then(F&& func) noexcept -> Result<U, E> {
			// the invocable checks above are probably redundant because of the inferred template
			// parameters, but we'll keep them for completeness sake and clarity of requirements
			m_handled = true;
			if(m_is_ok) {
				return std::forward<F>(func)(m_data.m_ok);
			}
			else {
				return hyperion::Err(m_data.m_err);
			}
		}

		/// @brief Returns `result` if this is the `Err` variant, otherwise returns the `Ok` value
		/// contained in this.
		///
		/// @tparam F - The type of the `Err` variant of `result`
		/// @param result - The next `Result` to potentially use
		///
		/// @return `result` if this is `Err`, `Ok(T)` otherwise
		template<ErrorType F>
		requires NotReference<F>
		[[nodiscard]] inline auto or_else(Result<T, F>&& result) const noexcept -> Result<T, F> {
			m_handled = true;
			if(m_is_ok) {
				return Ok(m_data.m_ok);
			}
			else {
				return std::forward<Result<T, F>>(result);
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
				 NotReference U
				 = decltype(std::declval<decltype(std::declval<F>()(std::declval<E&>()))>()
								.unwrap_err())>
		requires InvocableRMut<Result<T, U>, F, E>
		[[nodiscard]] inline auto or_else(F&& func) noexcept -> Result<T, U> {
			// the invocable checks above are probably redundant because of the inferred template
			// parameters, but we'll keep them for completeness sake and clarity of requirements
			m_handled = true;
			if(m_is_ok) {
				return hyperion::Ok(m_data.m_ok);
			}
			else {
				return std::forward<F>(func)(m_data.m_err);
			}
		}

		/// @brief Boolean conversion operator. Returns true if this is the `Ok` variant
		///
		/// @return true if this is `Ok`, false otherwise
		constexpr operator bool() const noexcept { // NOLINT
			m_handled = true;
			return m_is_ok && m_engaged;
		}

		/// @brief Deleted copy assignment operator. `Result`s cannot be copied.
		auto operator=(const Result& result) -> Result& = delete;

		/// @brief Move assignment operator.
		/// Moving a `Result` consumes it, leaving a disengaged (valueless) `Result` in its place
		constexpr auto operator=(Result&& result) noexcept
			-> Result& requires MoveAssignable<T> && MoveAssignable<E> {
			this->m_handled = result.m_handled;
			result.m_handled = true;
			this->m_is_ok = result.m_is_ok;
			if(result.m_engaged) {
				if(result.m_is_ok) {
					result.m_is_ok = false;
					if constexpr(Pointer<T>) {
						result.m_engaged = false;
						m_data.m_ok = result.m_data.m_ok;
						result.m_data.m_ok = nullptr;
					}
					else {
						// we don't actually disengage in the non-pointer case, so that the
						// destructor of the moved from `ok` can be run
						m_data.m_ok = std::move(result.m_data.m_ok);
					}
				}
				else {
					if constexpr(Pointer<E>) {
						result.m_engaged = false;
						m_data.m_err = result.m_data.m_err;
						result.m_data.m_err = nullptr;
					}
					else {
						// we don't actually disengage in the non-pointer case, so that the
						// destructor of the moved from `ok` can be run
						m_data.m_err = std::move(result.m_data.m_err);
					}
				}
				m_engaged = true;
			}
			return *this;
		}

	  private :
		  /// lvalue constructor
		  constexpr explicit Result(const T& ok) noexcept
		  : m_data(ok), m_is_ok(true), m_engaged(true) {
		}

		/// rvalue contructor
		constexpr explicit Result(T&& ok) noexcept
			: m_data(std::forward<T>(ok)), m_is_ok(true), m_engaged(true) {
		}

		/// lvalue constructor
		constexpr explicit Result(const E& err) noexcept : m_data(err), m_engaged(true) {
		}

		/// rvalue contructor
		constexpr explicit Result(E&& err) noexcept
			: m_data(std::forward<E>(err)), m_engaged(true) {
		}

		union Data { // NOLINT
			T m_ok;
			E m_err;
			int m_disengaged = 0;

			explicit constexpr Data(const T& ok) noexcept requires CopyConstructible<T> : m_ok(ok) {
			}
			explicit constexpr Data(T&& ok) noexcept requires MoveConstructible<T>
				: m_ok(std::forward<T>(ok)) {
			}
			template<typename... Args>
			requires ConstructibleFrom<T, Args...>
			explicit constexpr Data(Args&&... args) noexcept : m_ok(std::forward<Args>(args)...) {
			}
			explicit constexpr Data(const E& err) noexcept requires CopyConstructible<E>
				: m_err(err) {
			}
			explicit constexpr Data(E&& err) noexcept requires MoveConstructible<E>
				: m_err(std::forward<E>(err)) {
			}
			template<typename... Args>
			requires ConstructibleFrom<E, Args...>
			explicit constexpr Data(Args&&... args) noexcept : m_err(std::forward<Args>(args)...) {
			}
			constexpr ~Data() noexcept { // NOLINT
			}
		} m_data;

		/// whether this is `Ok` (`true`) or `Err` (`false`)
		bool m_is_ok = false;
		/// whether this `Result` has a value (true) or is valueless (false)
		bool m_engaged = false;
		/// whether this `Result` has been handled
		mutable bool m_handled = false;
	};
	IGNORE_PADDING_STOP

} // namespace hyperion
