/// @file UniquePtr.h
/// @author Braxton Salyer <braxtonsalyer@gmail.com>
/// @brief This includes Hyperion's `constexpr` equivalents and extensions to the C++ standard
/// library's `std::unique_ptr<T, Deleter>`
/// @version 0.1
/// @date 2021-08-27
///
/// MIT License
/// @copyright Copyright (c) 2021 Braxton Salyer <braxtonsalyer@gmail.com>
///
/// Permission is hereby granted, free of charge, to any person obtaining a copy
/// of this software and associated documentation files (the "Software"), to deal
/// in the Software without restriction, including without limitation the rights
/// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
/// copies of the Software, and to permit persons to whom the Software is
/// furnished to do so, subject to the following conditions:
///
/// The above copyright notice and this permission notice shall be included in all
/// copies or substantial portions of the Software.
///
/// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
/// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
/// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
/// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
/// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
/// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
/// SOFTWARE.
#pragma once

#include <gsl/gsl>

#include "../Concepts.h"
#include "../HyperionDef.h"
#include "CompressedPair.h"

/// @ingroup memory
/// @{
///	@defgroup UniquePtr UniquePtr
/// Hyperion provides a `constexpr` equivalent to the C++ standard library's
/// `std::unique_ptr<T, Deleter>` in our `UniquePtr`, with additional functionality like allocator
/// aware factory functions (`allocate_unique`).
///
/// Example:
///
/// auto ptr = hyperion::make_unique<i32>(42);
/// ptr.reset(24);
///
/// auto ptr2 = hyperion::allocate_unique<i32>(std::allocator<i32>(), 42);
/// ptr2.reset(24);
/// @headerfile "HyperionUtils/Memory.h"
/// @}

namespace hyperion {

	/// @brief Default Deleter type for Hyperion smart pointers
	/// This is the default deleter type used by Hyperion smart pointers
	/// such as `hyperion::UniquePtr<T, Deleter>`
	///
	/// @tparam T The type to handle deletion for (eg, `i32`, `std::string`)
	///
	/// # Requirements
	/// - `concepts::Deletable<T>`: T must be a deletable type
	/// - `concepts::NotFunction<T>`: T must NOT be a function type
	/// @ingroup memory
	/// @headerfile "HyperionUtils/Memory.h"
	template<typename T>
	requires concepts::Deletable<T> && concepts::NotFunction<T>
	struct DefaultDeleter {

		/// @brief Default constructs a `DefaultDeleter`
		/// @ingroup memory
		constexpr DefaultDeleter() noexcept = default;
		/// @brief Copy-constructs a `DefaultDeleter` from the given one
		/// @ingroup memory
		constexpr DefaultDeleter(const DefaultDeleter&) noexcept = default;
		/// @brief Move-constructs a `DefaultDeleter` from the given one
		/// @ingroup memory
		constexpr DefaultDeleter(DefaultDeleter&&) noexcept = default;
		/// @brief Destructs a `DefaultDeleter`
		/// @ingroup memory
		constexpr ~DefaultDeleter() noexcept = default;
		/// @brief Constructs a `DefaultDeleter<T>` from a `DefaultDeleter<U>`
		///
		/// @tparam U - The type managed by the given deleter
		///
		/// @param deleter - The deleter to construct this from
		///
		/// # Requirements
		/// - `concepts::Convertible<U*, T*>`: The pointer type of `deleter` must be convertible two
		/// the pointer type of `DefaultDeleter<T>` in order to construct a `DefaultDeleter` from it
		/// @ingroup memory
		template<typename U>
		requires concepts::Convertible<U*, T*>
		explicit DefaultDeleter([[maybe_unused]] const DefaultDeleter<U>& deleter) noexcept {
		}

		/// @brief Deletes the data at the given pointer
		///
		/// @param ptr - The pointer to the data to delete
		/// @ingroup memory
		constexpr auto
		operator()(gsl::owner<T*> ptr) const noexcept(concepts::NoexceptDeletable<T>) {
			delete ptr;
		}

		/// @brief Copy-assigns this `DefaultDeleter` from the given one
		/// @ingroup memory
		constexpr auto operator=(const DefaultDeleter&) noexcept -> DefaultDeleter& = default;
		/// @brief Move-assigns this `DefaultDeleter` from the given one
		/// @ingroup memory
		constexpr auto operator=(DefaultDeleter&&) noexcept -> DefaultDeleter& = default;
	};

	template<typename T>
	requires concepts::Deletable<T[]> && concepts::NotFunction<T>
	struct DefaultDeleter<T[]> { // NOLINT
		constexpr DefaultDeleter() noexcept = default;
		constexpr DefaultDeleter(const DefaultDeleter&) noexcept = default;
		constexpr DefaultDeleter(DefaultDeleter&&) noexcept = default;
		constexpr ~DefaultDeleter() noexcept = default;
		template<typename U>
		requires concepts::Convertible<U (*)[], T (*)[]>							 // NOLINT
		explicit DefaultDeleter([[maybe_unused]] const DefaultDeleter<U[]>& deleter) // NOLINT
			noexcept {
		}

		constexpr auto
		operator()(gsl::owner<T*> ptr) const noexcept(concepts::NoexceptDeletable<T[]>) // NOLINT
		{
			delete[] ptr;
		}

		constexpr auto operator=(const DefaultDeleter&) noexcept -> DefaultDeleter& = default;
		constexpr auto operator=(DefaultDeleter&&) noexcept -> DefaultDeleter& = default;
	};

	template<typename T, typename Deleter>
	concept ForwardableDeleter
		= (std::is_rvalue_reference_v<T> ?
			   (concepts::NoexceptMoveConstructible<Deleter> && concepts::NotReference<Deleter>) :
				 concepts::NoexceptCopyConstructible<Deleter>);

	/// @brief `UniquePtr<T, Deleter>` is Hyperion's `constexpr` equivalent to the standard
	/// library's `std::unique_ptr<T, Deleter>`
	///
	/// `UniquePtr<T, Deleter>` is a `constexpr` implementation of a unique owning pointer, with
	/// almost complete semantic equivalence with `std::unique_ptr<T, Deleter>`.
	/// It is a drop-in replacement in almost all situations(1).
	/// Hyperion also provides extended functionality for `UniquePtr<T, Deleter>`
	/// over `std::unique_ptr<T, Deleter>`, eg: an allocator-aware factory function set in
	/// `allocate_unqiue`
	///
	/// @tparam T - The type to store in the `UniquePtr`
	/// @tparam Deleter - The deleter type to handle deletion of the stored `T`, by default, this is
	/// `hyperion::DefaultDeleter<T>`. This type only needs to be explicitly provided when custom
	/// deletion or resource-freeing strategies are required.
	///
	/// # Requirements
	/// - `!std::is_rvalue_reference_v<Deleter>`: `Deleter` cannot be an r-value reference type
	///
	/// @note(1): The exception is in situations where destructor ordering is required and code was
	/// compiled with Clang. `UniquePtr<T, Deleter>` makes use of Clang's `trivial_abi` attribute,
	/// which allows `UniquePtr<T, Deleter>` to be passed via register in function calls
	/// (and thus removes its overhead compared to a raw pointer), but moves the call to its
	/// destructor from the calling code to the callee. The result is that the destructor for
	/// `UniquePtr<T, Deleter>` will not nest in the same way as `std::unique_ptr<T, Deleter>`, and
	/// code relying on that behavior may experience issues. This is identical behavior to
	/// `std::unique_ptr<T, Deleter>` when `_LIBCPP_ABI_ENABLE_UNIQUE_PTR_TRIVIAL_ABI` is defined
	/// @ingroup UniquePtr
	/// @headerfile "HyperionUtils/Memory.h"
	template<typename T, typename Deleter = DefaultDeleter<T>>
	requires concepts::NotRValueReference<Deleter>
	class HYPERION_TRIVIAL_ABI UniquePtr {
	  public:
		/// @brief The element type allocated in the `UniquePtr`
		/// @ingroup UniquePtr
		using element_type = T;
		/// @brief The deleter type used by the `UniquePtr`
		/// @ingroup UniquePtr
		using deleter_type = Deleter;
		/// @brief The pointer type stored in the `UniquePtr`
		/// @ingroup UniquePtr
		using pointer = T*;
		/// @brief The pointer type stored in the `UniquePtr`, as a pointer to const
		/// @ingroup UniquePtr
		using pointer_to_const = const T*;

		/// @brief Constructs a default `UniquePtr<T, Deleter>`, managing no pointer
		///
		/// # Requirements
		/// - `concepts::NoexceptDefaultConstructible<deleter_type>`: `deleter_type` must be
		/// noexcept default constructible in order to default construct a `UniquePtr`
		/// - `concepts::NotPointer<deleter_type>`: `deleter_type` must not be a pointer
		/// in order to default construct a `UniquePtr`
		/// @ingroup UniquePtr
		constexpr UniquePtr() noexcept requires concepts::NoexceptDefaultConstructible<
			deleter_type> && concepts::NotPointer<deleter_type>
			: m_ptr(pointer(), DefaultInitTag<deleter_type>()) {
		}
		/// @brief Constructs a `UniquePtr<T, Deleter>` managing no pointer
		///
		/// @param ptr - The `nullptr` signaling this `UniquePtr` should manage no pointer
		///
		/// # Requirements
		/// - `concepts::NoexceptDefaultConstructible<deleter_type>`: `deleter_type` must be
		/// noexcept default constructible in order to construct a `UniquePtr` from only a `nullptr`
		/// - `concepts::NotPointer<deleter_type>`: `deleter_type` must not be a pointer
		/// in order to construct a `UniquePtr` from only a `nullptr`
		/// @ingroup UniquePtr
		constexpr UniquePtr(std::nullptr_t ptr) noexcept // NOLINT
			requires concepts::NoexceptDefaultConstructible<deleter_type> && concepts::NotPointer<
				deleter_type> : m_ptr(ptr, DefaultInitTag<deleter_type>()) {
		}
		/// @brief Constructs a `UniquePtr<T, Deleter>` managing the given pointer
		///
		/// @param ptr - The pointer to manage with this `UniquePtr`
		///
		/// # Requirements
		/// - `concepts::NoexceptDefaultConstructible<deleter_type>`: `deleter_type` must be
		/// noexcept default constructible in order to construct a `UniquePtr` from only a `pointer`
		/// - `concepts::NotPointer<deleter_type>`: `deleter_type` must not be a pointer
		/// in order to construct a `UniquePtr` from only a `pointer`
		/// @ingroup UniquePtr
		explicit constexpr UniquePtr(pointer ptr) noexcept requires concepts::
			NoexceptDefaultConstructible<deleter_type> && concepts::NotPointer<deleter_type>
			: m_ptr(ptr, DefaultInitTag<deleter_type>()) {
		}
		/// @brief Constructs a `UniquePtr<T, Deleter>` managing the given pointer
		///
		/// @tparam U - The type of the pointer to manage in this `UniquePtr`
		///
		/// @param ptr - The pointer to manage with this `UniquePtr`
		///
		/// # Requirements
		/// - `concepts::Convertible<U, pointer>`: `U` must be convertible to `pointer` to construct
		/// a `UniquePtr` from it
		/// - `concepts::NoexceptDefaultConstructible<deleter_type>`: `deleter_type` must be
		/// noexcept default constructible in order to construct a `UniquePtr` from only a `pointer`
		/// - `concepts::NotPointer<deleter_type>`: `deleter_type` must not be a pointer
		/// in order to construct a `UniquePtr` from only a `pointer`
		/// @ingroup UniquePtr
		template<typename U>
		requires concepts::Convertible<U, pointer> && concepts::NoexceptDefaultConstructible<
			deleter_type> && concepts::NotPointer<deleter_type>
		explicit constexpr UniquePtr(U ptr) noexcept : m_ptr(ptr, DefaultInitTag<deleter_type>()) {
		}
		/// @brief Constructs a `UniquePtr<T, Deleter>` managing the given pointer
		///
		/// @param ptr - The pointer to manage with this `UniquePtr`
		/// @param deleter - The deleter to delete `ptr` with when the destructor of this
		/// `UniquePtr` is called
		///
		/// @ingroup UniquePtr
		constexpr UniquePtr(pointer ptr, ForwardableDeleter<deleter_type> auto&& deleter) noexcept
			: m_ptr(ptr, std::forward<deleter_type>(deleter)) {
		}
		/// @brief Constructs a `UniquePtr<T, Deleter>` managing the given pointer
		///
		/// @tparam U - The type of the pointer to manage in this `UniquePtr`
		///
		/// @param ptr - The pointer to manage with this `UniquePtr`
		/// @param deleter - The deleter to delete `ptr` with when the destructor of this
		/// `UniquePtr` is called
		///
		/// # Requirements
		/// - `concepts::Convertible<U, pointer>`: `U` must be convertible to `pointer` to construct
		/// a `UniquePtr` from it
		/// @ingroup UniquePtr
		template<typename U>
		requires concepts::Convertible<U, pointer>
		constexpr UniquePtr(U ptr, ForwardableDeleter<deleter_type> auto&& deleter) noexcept
			: m_ptr(ptr, std::forward<deleter_type>(deleter)) {
		}
		/// @brief Constructs a `UniquePtr<T, Deleter>` managing no pointer
		///
		/// @param ptr - The `nullptr` signaling this `UniquePtr` should manage no pointer
		/// @param deleter - The deleter to delete a managed pointer with when the destructor of
		/// this `UniquePtr` is called
		///
		/// @ingroup UniquePtr
		constexpr UniquePtr(std::nullptr_t ptr,
							ForwardableDeleter<deleter_type> auto&& deleter) noexcept
			: m_ptr(ptr, std::forward<deleter_type>(deleter)) {
		}
		/// @brief Constructs a `UniquePtr<T, Deleter>` from the given moved `UniquePtr<U, D>`
		///
		/// @param ptr - The `UniquePtr` to construct this one from
		///
		/// # Requirements
		/// - `concepts::NoexceptConstructibleFrom<deleter_type,
		/// decltype((std::forward<D>(ptr.get_deleter())))`: `deleter_type` must be noexcept
		/// constructible from the deleter type returned by `ptr.get_deleter()`
		/// - `concepts::Convertible<typename UniquePtr<U, D>::pointer, pointer>`: The pointer type
		/// of `UniquePtr<U, D>` must be convertible to `pointer`
		/// - `concepts::Convertible<typename UniquePtr<U, D>::deleter_type, deleter_type>`: The
		/// deleter type of `UniquePtr<U, D>` must be convertible to `deleter_type`
		/// @ingroup UniquePtr
		template<typename U, typename D>
		explicit constexpr UniquePtr(UniquePtr<U, D>&& ptr) noexcept requires concepts::
			NoexceptConstructibleFrom<deleter_type,
									  decltype((std::forward<D>(ptr.get_deleter())))> && concepts::
				Convertible<typename UniquePtr<U, D>::pointer, pointer> && concepts::
					Convertible<typename UniquePtr<U, D>::deleter_type, deleter_type>
			: m_ptr(ptr.release(), std::forward<D>(ptr.get_deleter())) {
		}
		/// @brief `UniquePtr` cannot be copied
		/// @ingroup UniquePtr
		UniquePtr(const UniquePtr&) = delete;
		/// @brief Move Constructs a `UniquePtr` from the given one
		///
		/// @param ptr - The `UniquePtr` to move
		///
		/// # Requirements
		/// - `concepts::NoexceptMoveConstructible<deleter_type>`: `deleter_type` must be noexcept
		/// move constructible in order to move construct a `UniquePtr`
		/// @ingroup UniquePtr
		constexpr UniquePtr(
			UniquePtr&& ptr) noexcept requires concepts::NoexceptMoveConstructible<deleter_type>
			: m_ptr(ptr.release(), std::forward<deleter_type>(ptr.get_deleter())) {
		}
		/// @brief `UniquePtr` destructor
		///
		/// # Requirements
		/// - `noexcept(std::declval<deleter_type>()(std::declval<pointer>()))`: Deleting the
		/// managed pointer via the associated `deleter_type`'s call operator must be noexcept
		/// @ingroup UniquePtr
		constexpr ~UniquePtr() noexcept
			requires(noexcept(std::declval<deleter_type>()(std::declval<pointer>()))) // NOLINT
		{
			reset();
		}

		/// @brief Releases ownership of the managed pointer and returns it
		///
		/// @return The managed pointer
		/// @ingroup UniquePtr
		[[nodiscard]] constexpr inline auto release() noexcept -> pointer {
			auto* ptr = m_ptr.first();
			m_ptr.first() = pointer();
			return ptr;
		}

		/// @brief Deletes the currently managed pointer, if any, and begins managing the given one
		///
		/// @param ptr - The new pointer to manage with this `UniquePtr`
		///
		/// # Requirements
		/// - `noexcept(std::declval<deleter_type>()(std::declval<pointer>()))`: Deleting the
		/// managed pointer via the associated `deleter_type`'s call operator must be noexcept
		constexpr inline auto reset(pointer ptr = pointer()) noexcept -> void requires(
			noexcept(std::declval<deleter_type>()(std::declval<pointer>()))) // NOLINT
		{
			gsl::owner<pointer> tmp = m_ptr.first(); // NOLINT
			m_ptr.first() = ptr;
			m_ptr.second()(tmp);
		}

		/// @brief Deletes the currently managed pointer, if any, and begins managing the given one
		///
		/// @tparam U - The type of the new pointer to manage
		///
		/// @param ptr - The new pointer to manage with this `UniquePtr`
		///
		/// # Requirements
		/// - `concepts::Convertible<U, pointer>`: `U` must be convertible to `pointer` in order for
		/// this `UniquePtr` to manage a `U`
		/// - `noexcept(std::declval<deleter_type>()(std::declval<pointer>()))`: Deleting the
		/// managed pointer via the associated `deleter_type`'s call operator must be noexcept
		/// @ingroup UniquePtr
		template<typename U>
		requires concepts::Convertible<U, pointer>
		constexpr inline auto reset(U ptr) noexcept -> void requires(
			noexcept(std::declval<deleter_type>()(std::declval<pointer>()))) // NOLINT
		{
			gsl::owner<pointer> tmp = m_ptr.first(); // NOLINT
			m_ptr.first() = ptr;
			m_ptr.second()(tmp);
		}

		/// @brief Swaps the managed pointer and deleter of this `UniquePtr` with the given one
		///
		/// @param ptr - The `UniquePtr` to swap with
		///
		/// # Requirements
		/// - `std::is_nothrow_swappable_v<CompressedPair<pointer, deleter_type>>`:
		/// `CompressedPair<pointer, deleter_type> must be noexcept swappable in order to
		/// swap two `UniquePtr`s. (`CompressedPair` is used internally to store the managed pointer
		/// and deleter, to allow for
		/// [Empty Base Class Optimization](https://en.cppreference.com/w/cpp/language/ebo) )
		/// @ingroup UniquePtr
		constexpr inline auto swap(UniquePtr& ptr) noexcept
			-> void requires concepts::NoexceptSwappable<CompressedPair<pointer, deleter_type>> {
			m_ptr.swap(ptr.m_ptr);
		}

		/// @brief Returns the managed pointer
		///
		/// Returns the managed pointer. This does not release ownership of the pointer, it only
		/// provides unmanaged access to it. Use of the returned pointer after the lifetime of this
		/// `UniquePtr` has ended results in undefined behavior.
		///
		/// @return The managed pointer
		/// @ingroup UniquePtr
		[[nodiscard]] constexpr inline auto get() noexcept -> pointer {
			return m_ptr.first();
		}

		/// @brief Returns the managed pointer
		///
		/// Returns the managed pointer. This does not release ownership of the pointer, it only
		/// provides unmanaged access to it. Use of the returned pointer after the lifetime of this
		/// `UniquePtr` has ended results in undefined behavior.
		///
		/// @return The managed pointer
		/// @ingroup UniquePtr
		[[nodiscard]] constexpr inline auto get() const noexcept -> pointer_to_const {
			return m_ptr.first();
		}

		/// @brief Returns the associated deleter
		///
		/// @return The associated deleter
		/// @ingroup UniquePtr
		[[nodiscard]] constexpr inline auto get_deleter() const noexcept -> const deleter_type& {
			return m_ptr.second();
		}

		/// @brief Returns the associated deleter
		///
		/// @return The associated deleter
		/// @ingroup UniquePtr
		[[nodiscard]] constexpr inline auto get_deleter() noexcept -> deleter_type& {
			return m_ptr.second();
		}

		/// @brief Converts this `UniquePtr` to a `bool`
		///
		/// Returns `true` if the managed pointer is not null, `false` otherwise.
		/// @return this, as a `bool`
		/// @ingroup UniquePtr
		explicit constexpr operator bool() const noexcept {
			return m_ptr.first() != nullptr;
		}

		constexpr auto operator*() const -> typename std::add_lvalue_reference_t<T> {
			return *(m_ptr.first());
		}

		constexpr auto operator->() const noexcept -> pointer {
			return m_ptr.first();
		}

		constexpr inline auto operator==(const UniquePtr& ptr) const noexcept -> bool {
			return m_ptr.first() == ptr.first();
		}

		constexpr inline auto operator==(std::nullptr_t) const noexcept -> bool {
			return m_ptr.first() == nullptr;
		}

		auto operator=(const UniquePtr&) -> UniquePtr& = delete;
		/// @brief Move-assigns this `UniquePtr` from the given one
		///
		/// @param ptr - The `UniquePtr` to move into this one
		///
		/// # Requirements
		/// - `concepts::NoexceptMoveAssignable<deleter_type>`: `deleter_type` must be noexcept move
		/// assignable in order to move assign a `UniquePtr`
		/// - `noexcept(std::declval<deleter_type>()(std::declval<pointer>()))`: Deleting the
		/// managed pointer via the associated `deleter_type`'s call operator must be noexcept
		///
		/// @return this
		/// @ingroup UniquePtr
		constexpr auto operator=(UniquePtr&& ptr) noexcept
			-> UniquePtr& requires concepts::NoexceptMoveAssignable<deleter_type> &&(
				noexcept(std::declval<deleter_type>()(std::declval<pointer>()))) // NOLINT
		{
			if(this == &ptr) {
				return *this;
			}

			reset(ptr.release());
			m_ptr.second() = std::forward<deleter_type>(ptr.get_deleter());

			return *this;
		}

		/// @brief Move-assigns this `UniquePtr` from the given one
		///
		/// @tparam U - The managed type of the given `UniquePtr`
		/// @tparam D - The deleter type of the given `UniquePtr`
		///
		/// @param ptr - The `UniquePtr<U, D>` to move into this one
		///
		/// # Requirements
		/// - `concepts::NoexceptAssignable<deleter_type,
		/// decltype(std::forward<D>(ptr.get_deleter()))>`: `deleter_type` must be assignable with
		/// the deleter type returned by `ptr`'s `get_deleter()` member function
		/// - `concepts::Convertible<typename UniquePtr<U, D>::pointer, pointer>`: The pointer type
		/// of `ptr` must be convertible to the pointer type of `this` in order to assign from
		/// it
		/// - `concepts::Convertible<typename UniquePtr<U, D>::deleter_type, deleter_type>`:
		/// The deleter type of `ptr` must be convertible to the pointer type of `this` in order to
		/// assign from it
		/// - `noexcept(std::declval<deleter_type>()(std::declval<pointer>()))`: Deleting the
		/// managed pointer via the associated `deleter_type`'s call operator must be noexcept
		///
		/// @return this
		/// @ingroup UniquePtr
		template<typename U, typename D>
		constexpr auto operator=(UniquePtr<U, D>&& ptr) noexcept -> UniquePtr& requires
			concepts::NoexceptAssignable<deleter_type,
										 decltype(std::forward<D>(ptr.get_deleter()))> && concepts::
				Convertible<typename UniquePtr<U, D>::pointer, pointer> && concepts::
					Convertible<typename UniquePtr<U, D>::deleter_type, deleter_type> &&(
						noexcept(std::declval<deleter_type>()(std::declval<pointer>()))) // NOLINT
		{
			reset(ptr.release());
			m_ptr.second() = std::forward<D>(ptr.get_deleter());
			return *this;
		}

		/// @brief Assigns this `UniquePtr` with `nullptr`
		///
		/// Calls the deleter on the managed pointer and replaces it with `nullptr`
		///
		/// # Requirements
		/// - `noexcept(std::declval<deleter_type>()(std::declval<pointer>()))`: Deleting the
		/// managed pointer via the associated `deleter_type`'s call operator must be noexcept
		///
		/// @return this
		/// @ingroup UniquePtr
		constexpr auto operator=(std::nullptr_t) noexcept -> UniquePtr& requires(
			noexcept(std::declval<deleter_type>()(std::declval<pointer>()))) // NOLINT
		{
			reset();
			return *this;
		}

	  private : template<typename T_, typename Deleter_>
				using CompressedPair
				= CompressedPair<T_, Deleter_>;
		template<typename T_>
		using DefaultInitTag = DefaultInitTag<T_>;

		CompressedPair<pointer, deleter_type> m_ptr;
	};

	template<typename T, typename Deleter>
	requires concepts::NotRValueReference<Deleter>
	class HYPERION_TRIVIAL_ABI UniquePtr<T[], Deleter> { // NOLINT
	  public:
		using element_type = T;
		using deleter_type = Deleter;
		using pointer = T*;
		using pointer_to_const = const T*;

		constexpr UniquePtr() noexcept requires concepts::NoexceptDefaultConstructible<
			deleter_type> && concepts::NotPointer<deleter_type>
			: m_ptr(pointer(), DefaultInitTag<deleter_type>()) {
		}

		constexpr UniquePtr(std::nullptr_t ptr) noexcept // NOLINT
			requires concepts::NoexceptDefaultConstructible<deleter_type> && concepts::NotPointer<
				deleter_type> : m_ptr(ptr, DefaultInitTag<deleter_type>()) {
		}
		explicit constexpr UniquePtr(pointer ptr) noexcept requires concepts::
			NoexceptDefaultConstructible<deleter_type> && concepts::NotPointer<deleter_type>
			: m_ptr(ptr, DefaultInitTag<deleter_type>()) {
		}
		template<typename U>
		requires concepts::Convertible<U, pointer> && concepts::NoexceptDefaultConstructible<
			deleter_type> && concepts::NotPointer<deleter_type>
		explicit constexpr UniquePtr(U ptr) noexcept : m_ptr(ptr, DefaultInitTag<deleter_type>()) {
		}
		constexpr UniquePtr(pointer ptr, ForwardableDeleter<deleter_type> auto&& deleter) noexcept
			: m_ptr(ptr, std::forward<deleter_type>(deleter)) {
		}
		template<typename U>
		requires concepts::Convertible<U, pointer>
		constexpr UniquePtr(U ptr, ForwardableDeleter<deleter_type> auto&& deleter) noexcept
			: m_ptr(ptr, std::forward<deleter_type>(deleter)) {
		}
		constexpr UniquePtr(std::nullptr_t ptr,
							ForwardableDeleter<deleter_type> auto&& deleter) noexcept
			: m_ptr(ptr, std::forward<deleter_type>(deleter)) {
		}
		// clang-format off
		template<typename U, typename D>
		requires std::is_array_v<U>
		explicit constexpr UniquePtr(UniquePtr<U, D>&& ptr)
			noexcept(concepts::NoexceptConstructibleFrom<deleter_type,
														 decltype((std::forward<D>(ptr.get_deleter())))>)
			requires concepts::Convertible<typename UniquePtr<U, D>::element_type(*)[], // NOLINT
										   element_type (*)[] > 						// NOLINT
					 && concepts::Convertible<typename UniquePtr<U, D>::deleter_type, deleter_type>
			: m_ptr(ptr.release(), std::forward<D>(ptr.get_deleter())) {
		}
		// clang-format on
		UniquePtr(const UniquePtr&) = delete;
		constexpr UniquePtr(
			UniquePtr&& ptr) noexcept requires concepts::NoexceptMoveConstructible<deleter_type>
			: m_ptr(ptr.release(), std::forward<deleter_type>(ptr.get_deleter())) {
		}
		constexpr ~UniquePtr() noexcept
			requires(noexcept(std::declval<deleter_type>()(std::declval<pointer>()))) // NOLINT
		{
			reset();
		}

		[[nodiscard]] constexpr inline auto release() noexcept -> pointer {
			auto* ptr = m_ptr.first();
			m_ptr.first() = pointer();
			return ptr;
		}

		constexpr inline auto reset(pointer ptr = pointer()) noexcept -> void requires(
			noexcept(std::declval<deleter_type>()(std::declval<pointer>()))) // NOLINT
		{
			gsl::owner<pointer> tmp = m_ptr.first(); // NOLINT
			m_ptr.first() = ptr;
			m_ptr.second()(tmp);
		}

		template<typename U>
		requires concepts::Convertible<U, pointer>
		constexpr inline auto reset(U ptr) noexcept -> void requires(
			noexcept(std::declval<deleter_type>()(std::declval<pointer>()))) // NOLINT
		{
			gsl::owner<pointer> tmp = m_ptr.first(); // NOLINT
			m_ptr.first() = ptr;
			m_ptr.second()(tmp);
		}

		constexpr inline auto swap(UniquePtr& ptr) noexcept
			-> void requires concepts::NoexceptSwappable<CompressedPair<pointer, deleter_type>> {
			m_ptr.swap(ptr.m_ptr);
		}

		[[nodiscard]] constexpr inline auto get() noexcept -> pointer {
			return m_ptr.first();
		}

		[[nodiscard]] constexpr inline auto get() const noexcept -> pointer_to_const {
			return m_ptr.first();
		}

		[[nodiscard]] constexpr inline auto get_deleter() const noexcept -> const deleter_type& {
			return m_ptr.second();
		}

		[[nodiscard]] constexpr inline auto get_deleter() noexcept -> deleter_type& {
			return m_ptr.second();
		}

		explicit constexpr operator bool() const noexcept {
			return m_ptr.first() != nullptr;
		}

		constexpr inline auto operator==(const UniquePtr& ptr) const noexcept -> bool {
			return m_ptr.first() == ptr.first();
		}

		constexpr inline auto operator==(std::nullptr_t) const noexcept -> bool {
			return m_ptr.first() == nullptr;
		}

		constexpr inline auto operator[](concepts::Integral auto i) const
			-> std::add_const_t<std::add_lvalue_reference_t<element_type>> {
			return m_ptr.first()[i];
		}

		constexpr inline auto
		operator[](concepts::Integral auto i) -> std::add_lvalue_reference_t<element_type> {
			return m_ptr.first()[i];
		}

		auto operator=(const UniquePtr&) -> UniquePtr& = delete;
		constexpr auto operator=(UniquePtr&& ptr) noexcept
			-> UniquePtr& requires concepts::NoexceptMoveAssignable<deleter_type> &&(
				noexcept(std::declval<deleter_type>()(std::declval<pointer>()))) // NOLINT
		{
			if(this == &ptr) {
				return *this;
			}

			reset(ptr.release());
			m_ptr.second() = std::forward<deleter_type>(ptr.get_deleter());

			return *this;
		}
		// clang-format off

		template<typename U, typename D>
		requires std::is_array_v<U>
		constexpr auto operator=(UniquePtr<U, D>&& ptr) noexcept -> UniquePtr&
			requires concepts::NoexceptAssignable<deleter_type,
												  decltype(std::forward<D>(ptr.get_deleter()))>
					 && concepts::Convertible<typename UniquePtr<U, D>::element_type(*)	[], // NOLINT
										   	  element_type(*)[]> 							// NOLINT
					 && concepts::Convertible<typename UniquePtr<U, D>::deleter_type, deleter_type>
					 && (noexcept(std::declval<deleter_type>()(std::declval<pointer>()))) // NOLINT
		{
			reset(ptr.release());
			m_ptr.second() = std::forward<D>(ptr.get_deleter());
			return *this;
		}
		// clang-format on

		constexpr auto operator=(std::nullptr_t) noexcept -> UniquePtr& requires(
			noexcept(std::declval<deleter_type>()(std::declval<pointer>()))) // NOLINT
		{
			reset();
			return *this;
		}

	  private : template<typename T_, typename Deleter_>
				using CompressedPair
				= CompressedPair<T_, Deleter_>;
		template<typename T_>
		using DefaultInitTag = DefaultInitTag<T_>;

		CompressedPair<pointer, deleter_type> m_ptr;
	};
	template<typename T, typename Deleter>
	UniquePtr(T*, Deleter) -> UniquePtr<T, Deleter>;

	/// @brief Swaps the managed pointer and deleter of the two `UniquePtr`s
	///
	/// @tparam T - The type held by the `UniquePtr`s
	/// @tparam Deleter - The deleter type used by the `UniquePtr`s
	///
	/// @param first - The `UniquePtr` to swap to
	/// @param second - The `UniquePtr` to swap from
	/// @ingroup UniquePtr
	template<typename T, typename Deleter>
	requires concepts::Swappable<Deleter>
	constexpr inline auto
	swap(UniquePtr<T, Deleter>& first, UniquePtr<T, Deleter>& second) noexcept -> void {
		first.swap(second);
	}
	// clang-format off

	IGNORE_UNUSED_TEMPLATES_START

	/// @brief Constructs a `UniquePtr<T, Deleter>` from the given arguments
	///
	/// Constructs a `UniquePtr<T, Deleter>` from the given arguments, passing them directly
	/// to `T`'s constructor
	///
	/// @tparam T - The type to hold in the `UniquePtr`
	/// @tparam Deleter - The deleter type to free the resources associated with `T`
	/// @tparam Args - The types of the arguments to pass to `T`'s constructor
	///
	/// @param args - The arguments to pass to `T`'s constructor
	///
	/// # Requirements
	/// - `concepts::ConstructibleFrom<T, Args...>`: `T` must be constructible from the given
	/// arguments in order to make a `UniquePtr` from them
	/// - `!std::is_array_v<T>`: Cannot make a `UniquePtr` holding an array type from a set of
	/// constructor arguments. Use the overload for arrays to make a `UniquePtr<T[]>`.
	///
	/// @return a `UniquePtr<T, Deleter>` with the `T` constructed from the given arguments
	/// @ingroup UniquePtr
	template<typename T, typename Deleter = DefaultDeleter<T>, typename... Args>
	requires concepts::ConstructibleFrom<T, Args...> && (!std::is_unbounded_array_v<T>)
	static constexpr inline auto make_unique(Args&&... args)
		noexcept(concepts::NoexceptConstructibleFrom<T, Args...>)
		-> UniquePtr<T, Deleter>
	{
		// NOLINTNEXTLINE(modernize-use-auto, hicpp-use-auto)
		gsl::owner<T*> ptr = new T(std::forward<Args>(args)...);
		return UniquePtr<T, Deleter>(ptr);
	}
	// clang-format on

	/// @brief Constructs a `UniquePtr` holding array type `T` of size `N`
	///
	/// Constructs a `UniquePtr<T, Deleter>`, where `T` is an unbounded array type with initial
	/// size of `N`.
	///
	/// @tparam T - The array type to hold in the `UniquePtr`
	/// @tparam Deleter - The deleter type to free the resources associated with `T`
	/// @tparam ElementType - The element type of the array
	///
	/// @param N - The initial size of the array managed by the `UniquePtr`
	///
	/// # Requirements
	/// - `std::is_unbounded_array_v<T>`: `T` must be an unbounded array type to make a `UniquePtr`
	/// managing an array.
	///
	/// @return a `UniquePtr<T, Deleter>` managing an array of `N` `ElementType`s
	/// @note The array managed by the returned `UniquePtr` will be uninitialized, and thus each
	/// element must be appropriately initialized before use. Otherwise, the result is undefined
	/// behavior.
	/// @ingroup UniquePtr
	template<typename T,
			 typename Deleter = DefaultDeleter<T>,
			 typename ElementType = std::remove_extent_t<T>> // NOLINT
	requires std::is_unbounded_array_v<T>
	static constexpr inline auto make_unique(usize N) noexcept -> UniquePtr<T, Deleter> {
		// NOLINTNEXTLINE(modernize-use-auto, hicpp-use-auto)
		gsl::owner<ElementType*> ptr = new ElementType[N];
		return UniquePtr<T, Deleter>(ptr);
	}
	IGNORE_UNUSED_TEMPLATES_STOP

	IGNORE_PADDING_START
	/// @brief An allocator-aware deleter type for Hyperion smart pointers
	///
	/// `AllocatorAwareDeleter` is useful for using smart pointers in situations that require custom
	/// allocation strategies. It allows use of smart pointers in an allocator-aware fashion.
	///
	/// @tparam T - The type to manage deletion for
	/// @tparam Allocator - The allocator type allocation was performed with
	/// @tparam ElementType - The element type of the managed deletion, in the case that `T` is an
	/// array
	///
	/// # Requirements
	/// - `concepts::Allocatable<ElementType, Allocator>`: `AllocatorAwareDeleter` can't be
	/// instantiated for an `ElementType`-`Allocator` pair where the element type `ElementType`
	/// isn't allocatable by the associated allocator type, `Allocator`
	/// @note In the case that `T` is an array type, `AllocatorAwareDeleter` requires that the
	/// managed array is fully-initialized and the same size as when the `AllocatorAwareDeleter`
	/// was constructed with it (by a call to one of the factory functions, `allocate_unique`, or
	/// `allocate_shared`) when its call operator is used to free the resouces associated with the
	/// array.
	/// @note If `T` is an array type and the array is resized, the deleter associated with it in
	/// the owning smart pointer must be set to a new one to match the changed state.
	/// @note If `T` is an array type and the members of the array are not all initiaized when the
	/// call operator of this deleter is used to free the resources associated with it, the result
	/// is undefined behavior.
	/// @ingroup memory
	template<typename T,
			 typename Allocator = std::allocator<T>,
			 typename ElementType = std::remove_cv_t<std::remove_all_extents_t<T>>>
	requires concepts::Allocatable<ElementType, Allocator>
	class AllocatorAwareDeleter {
	  public:
		/// @brief The rebound allocator type for this deleter
		/// @ingroup memory
		using Alloc = typename std::allocator_traits<Allocator>::template rebind_alloc<ElementType>;
		/// @brief The `std::allocator_traits` type for this deleter
		/// @ingroup memory
		using Traits = std::allocator_traits<Alloc>;
		/// @brief The pointer type for the associated allocator traits for this deleter
		/// @ingroup memory
		using pointer = typename Traits::pointer;

		/// @brief Default-Constructs an `AllocatorAwareDeleter`
		///
		/// # Requirements
		/// - `concepts::NoexceptDefaultConstructible<Alloc>`: The associated allocator type must be
		/// noexcept default constructible in order to construct an `AllocatorAwareDeleter` with one
		/// @ingroup memory
		constexpr AllocatorAwareDeleter() noexcept requires
			concepts::NoexceptDefaultConstructible<Alloc> : m_allocator() {
		}
		/// @brief Constructs an `AllocatorAwareDeleter` from the given allocator
		///
		/// @param alloc - The allocator to construct the allocator associated with this from
		///
		/// # Requirements
		/// - `concepts::NoexceptConstructibleFrom<Alloc, decltype(alloc)>`: The associated
		/// allocator type must be noexcept constructible from the given allocator in order to
		/// construct an `AllocatorAwareDeleter` from it
		/// @ingroup memory
		explicit constexpr AllocatorAwareDeleter(const Allocator& alloc) noexcept requires
			concepts::NoexceptConstructibleFrom<Alloc, decltype(alloc)> : m_allocator(alloc) {
		}
		/// @brief Copy-Constructs an `AllocatorAwareDeleter` from the given one
		///
		/// # Requirements
		/// - `concepts::NoexceptCopyConstructible<Alloc>`: The associated allocator type must be
		/// noexcept copy constructible in order to copy construct an `AllocatorAwareDeleter`
		/// @ingroup memory
		constexpr AllocatorAwareDeleter(const AllocatorAwareDeleter&) noexcept requires
			concepts::NoexceptCopyConstructible<Alloc>
		= default;
		/// @brief Move-Constructs an `AllocatorAwareDeleter` from the given one
		///
		/// # Requirements
		/// - `concepts::NoexceptMoveConstructible<Alloc>`: The associated allocator type must be
		/// noexcept move constructible in order to copy construct an `AllocatorAwareDeleter`
		/// @ingroup memory
		constexpr AllocatorAwareDeleter(
			AllocatorAwareDeleter&&) noexcept requires concepts::NoexceptMoveConstructible<Alloc>
		= default;

		/// @brief Destructs this `AllocatorAwareDeleter`
		/// @ingroup memory
		constexpr ~AllocatorAwareDeleter() noexcept = default;

		/// @brief Frees the resources associated with the pointed-to object
		///
		/// @param p - Pointer to the object to free associated resources of
		/// @ingroup memory
		inline constexpr auto operator()(pointer p) const noexcept {
			Alloc allocator = m_allocator;

			Traits::destroy(allocator, std::addressof(*p));

			Traits::deallocate(allocator, p, 1);
		}

		/// @brief Copy-Assigns this `AllocatorAwareDeleter` from the given one
		///
		/// # Requirements
		/// - `concepts::NoexceptCopyAssignable<Alloc>`: The associated allocator type must be
		/// noexcept copy assignable in order to copy assign this `AllocatorAwareDeleter`
		/// @ingroup memory
		constexpr auto operator=(const AllocatorAwareDeleter&) noexcept
			-> AllocatorAwareDeleter& requires concepts::NoexceptCopyAssignable<Alloc>
		= default;
		/// @brief Move-Assigns this `AllocatorAwareDeleter` from the given one
		///
		/// # Requirements
		/// - `concepts::NoexceptMoveAssignable<Alloc>`: The associated allocator type must be
		/// noexcept move assignable in order to move assign this `AllocatorAwareDeleter`
		/// @ingroup memory
		constexpr auto operator=(AllocatorAwareDeleter&&) noexcept
			-> AllocatorAwareDeleter& requires concepts::NoexceptMoveAssignable<Alloc>
		= default;

	  private:
		Alloc m_allocator;
	};

	template<typename T, typename Allocator, typename ElementType>
	requires concepts::Allocatable<ElementType, Allocator>
	class AllocatorAwareDeleter<T[], Allocator, ElementType> { // NOLINT
	  public:
		using Alloc = typename std::allocator_traits<Allocator>::template rebind_alloc<ElementType>;
		using Traits = std::allocator_traits<Alloc>;
		using pointer = typename Traits::pointer;

		constexpr AllocatorAwareDeleter() noexcept requires
			concepts::NoexceptDefaultConstructible<Alloc> : m_allocator() {
		}
		explicit constexpr AllocatorAwareDeleter(const Allocator& alloc,
												 usize num_elements) noexcept requires
			concepts::NoexceptConstructibleFrom<Alloc, decltype(alloc)>
			: m_allocator(alloc),
			  m_num_elements(num_elements) {
		}
		constexpr AllocatorAwareDeleter(const AllocatorAwareDeleter&) noexcept requires
			concepts::NoexceptCopyConstructible<Alloc>
		= default;
		constexpr AllocatorAwareDeleter(
			AllocatorAwareDeleter&&) noexcept requires concepts::NoexceptMoveConstructible<Alloc>
		= default;
		constexpr ~AllocatorAwareDeleter() noexcept = default;

		inline constexpr auto operator()(pointer p) const noexcept {
			Alloc allocator = m_allocator;

			auto i = m_num_elements >= 1 ? m_num_elements - 1 : 0_usize;
			// we need to use do-while to ensure we destroy the 0th element
			// in a 1 element array
			do {
				Traits::destroy(allocator, std::addressof(*p) + i); // NOLINT
				--i;
			} while(i != 0);

			Traits::deallocate(allocator, p, m_num_elements);
		}

		constexpr auto operator=(const AllocatorAwareDeleter&) noexcept
			-> AllocatorAwareDeleter& requires concepts::NoexceptCopyAssignable<Alloc>
		= default;
		constexpr auto operator=(AllocatorAwareDeleter&&) noexcept
			-> AllocatorAwareDeleter& requires concepts::NoexceptMoveAssignable<Alloc>
		= default;

	  private:
		Alloc m_allocator;
		usize m_num_elements = 1;
	};
	IGNORE_PADDING_STOP

	/// @brief Constructs an allocator-aware `UniquePtr`
	///
	/// Constructs a `UniquePtr<T, AllocatorAwareDeleter<T, Alloc>>`. The managed `T` will be
	/// constructed from the given arguments, `args`, passing them directly to `T`'s constructor.
	/// Allocation of the managed `T` will be performed by the given allocator.
	///
	/// @tparam T - The type to manage in the `UniquePtr`
	/// @tparam Allocator - The type of the allocator to use for allocation and deletion of the `T`
	/// @tparam ElementType - The element type of the array, in the case that `T` is an unbounded
	/// array type
	/// @tparam Alloc - The allocator type `Allocator` rebound to the element type, `ElementType`
	///
	/// @param alloc - The allocator to perform allocation and deallocation with
	/// @param args - The arguments to pass to `T`'s constructor
	///
	/// # Requirements
	/// - `concepts::NoexceptConstructibleFrom<ElementType, Args...>`: `ElementType` must be
	/// noexcept constructible from `args` in order to make a `UniquePtr` from them
	/// - `concepts::Allocatable<ElementType, Alloc>`: `AllocatorAwareDeleter` can't be
	/// instantiated for an `ElementType`-`Alloc` pair where the element type `ElementType`
	/// isn't allocatable by the associated allocator type, `Alloc`
	/// - `!std::is_array_v<T>`: Cannot make a `UniquePtr` holding an array type from a set of
	/// constructor arguments. Use the overload for arrays to make a `UniquePtr<T[]>`.
	/// - `concepts::NoexceptConstructibleFrom<Alloc, decltype(alloc)>`: The associated allocator
	/// type, `Alloc`, must be noexcept constructible from the given allocator in order to create
	/// a `UniquePtr` using it
	/// @ingroup UniquePtr
	template<typename T,
			 typename Allocator = std::allocator<T>,
			 typename ElementType = std::remove_cv_t<std::remove_all_extents_t<T>>,
			 typename Alloc =
				 typename std::allocator_traits<Allocator>::template rebind_alloc<ElementType>,
			 typename... Args>
	requires concepts::NoexceptConstructibleFrom<ElementType, Args...> && concepts::
		Allocatable<ElementType, Alloc> &&(!std::is_unbounded_array_v<T>)
			[[nodiscard]] inline constexpr auto allocate_unique(const Allocator& alloc,
																Args&&... args) noexcept
		-> UniquePtr<T, AllocatorAwareDeleter<T, Alloc>>
	requires concepts::NoexceptConstructibleFrom<Alloc, decltype(alloc)> {
		using Traits = std::allocator_traits<Alloc>;
		using Deleter = AllocatorAwareDeleter<T, Alloc>;

		Alloc allocator(alloc);
		auto* p = Traits::allocate(allocator, 1);
		Traits::construct(allocator, std::addressof(*p), std::forward<Args>(args)...);

		return UniquePtr<T, Deleter>(p, Deleter(allocator));
	}

	/// @brief Constructs an allocator-aware `UniquePtr`
	///
	/// Constructs a `UniquePtr<T, AllocatorAwareDeleter<T, Alloc>>`, where `T` is an unbounded
	/// array type. The managed array will have its elements default constructed in place.
	/// Allocation of the managed array will be performed by the given allocator.
	///
	/// @tparam T - The type to manage in the `UniquePtr`
	/// @tparam Allocator - The type of the allocator to use for allocation and deletion of the `T`
	/// @tparam ElementType - The element type of the array, in the case that `T` is an unbounded
	/// array type
	/// @tparam Alloc - The allocator type `Allocator` rebound to the element type, `ElementType`
	///
	/// @param alloc - The allocator to perform allocation and deallocation with
	/// @param N - The number of elements to allocate in the array
	///
	/// # Requirements
	/// - `concepts::NoexceptDefaultConstructible<ElementType>`: `ElementType` must be
	/// noexcept default-constructible in order to make a `UniquePtr` managing an array of them
	/// - `concepts::Allocatable<ElementType, Alloc>`: `AllocatorAwareDeleter` can't be
	/// instantiated for an `ElementType`-`Alloc` pair where the element type `ElementType`
	/// isn't allocatable by the associated allocator type, `Alloc`
	/// - `std::is_unbounded_array_v<T>`: `T` must be an unbounded array type to make a `UniquePtr`
	/// managing an array.
	/// - `concepts::NoexceptConstructibleFrom<Alloc, decltype(alloc)>`: The associated allocator
	/// type, `Alloc`, must be noexcept constructible from the given allocator in order to create
	/// a `UniquePtr` using it
	/// @ingroup UniquePtr
	template<typename T,
			 typename Allocator = std::allocator<T>,
			 typename ElementType = std::remove_cv_t<std::remove_all_extents_t<T>>,
			 typename Alloc =
				 typename std::allocator_traits<Allocator>::template rebind_alloc<ElementType>>
	requires concepts::NoexceptDefaultConstructible<
		ElementType> && concepts::Allocatable<ElementType, Alloc> && std::is_unbounded_array_v<T>
	[[nodiscard]] inline constexpr auto allocate_unique(const Allocator& alloc, usize N) noexcept
		-> UniquePtr<T, AllocatorAwareDeleter<T, Alloc>>
	requires concepts::NoexceptConstructibleFrom<Alloc, decltype(alloc)> {
		using Traits = std::allocator_traits<Alloc>;
		using Deleter = AllocatorAwareDeleter<T, Alloc>;

		Alloc allocator(alloc);
		auto* p = Traits::allocate(allocator, N);
		// we need to use do-while so we make sure we construct the 0th element in
		// a one element array
		auto i = 0_usize;
		do {
			Traits::construct(allocator, std::addressof(*p) + i); // NOLINT
			++i;
		} while(i < N);

		return UniquePtr<T, Deleter>(p, Deleter(allocator, N));
	}

	/// @brief Constructs an allocator-aware `UniquePtr`
	///
	/// Constructs a `UniquePtr<T, AllocatorAwareDeleter<T, Alloc>>`, where `T` is an unbounded
	/// array type. The managed array will have its elements default constructed in place.
	/// Allocation of the managed array will be performed by the given allocator.
	///
	/// @tparam T - The type to manage in the `UniquePtr`
	/// @tparam Allocator - The type of the allocator to use for allocation and deletion of the `T`
	/// @tparam ElementType - The element type of the array, in the case that `T` is an unbounded
	/// array type
	/// @tparam Alloc - The allocator type `Allocator` rebound to the element type, `ElementType`
	/// @tparam Args - The types of the arguments to use to default-construct the elements of the
	/// array
	///
	/// @param alloc - The allocator to perform allocation and deallocation with
	/// @param N - The number of elements to allocate in the array
	///
	/// # Requirements
	/// - `concepts::NoexceptDefaultConstructible<ElementType>`: `ElementType` must be
	/// noexcept default-constructible in order to make a `UniquePtr` managing an array of them
	/// - `concepts::Allocatable<ElementType, Alloc>`: `AllocatorAwareDeleter` can't be
	/// instantiated for an `ElementType`-`Alloc` pair where the element type `ElementType`
	/// isn't allocatable by the associated allocator type, `Alloc`
	/// - `std::is_unbounded_array_v<T>`: `T` must be an unbounded array type to make a `UniquePtr`
	/// managing an array.
	/// - `concepts::NoexceptConstructibleFrom<Alloc, decltype(alloc)>`: The associated allocator
	/// type, `Alloc`, must be noexcept constructible from the given allocator in order to create
	/// a `UniquePtr` using it
	/// @ingroup UniquePtr
	template<typename T,
			 typename Allocator = std::allocator<T>,
			 typename ElementType = std::remove_cv_t<std::remove_all_extents_t<T>>,
			 typename Alloc =
				 typename std::allocator_traits<Allocator>::template rebind_alloc<ElementType>,
			 typename... Args>
	requires concepts::NoexceptDefaultConstructible<
		ElementType> && concepts::Allocatable<ElementType, Alloc> && std::is_unbounded_array_v<T>
	[[nodiscard]] inline constexpr auto
	allocate_unique(const Allocator& alloc, usize N, Args&&... args) noexcept
		-> UniquePtr<T, AllocatorAwareDeleter<T, Alloc>>
	requires concepts::NoexceptConstructibleFrom<Alloc, decltype(alloc)> {
		using Traits = std::allocator_traits<Alloc>;
		using Deleter = AllocatorAwareDeleter<T, Alloc>;

		Alloc allocator(alloc);
		auto* p = Traits::allocate(allocator, N);
		// we need to use do-while so we make sure we construct the 0th element in
		// a one element array
		auto i = 0_usize;
		do {
			Traits::construct(allocator,
							  std::addressof(*p) + i,
							  std::forward<Args>(args)...); // NOLINT
			++i;
		} while(i < N);

		return UniquePtr<T, Deleter>(p, Deleter(allocator, N));
	}
} // namespace hyperion
