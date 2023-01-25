/// @file UniquePtr.h
/// @author Braxton Salyer <braxtonsalyer@gmail.com>
/// @brief This includes Hyperion's `constexpr` equivalents and extensions to the C++ standard
/// library's `std::unique_ptr<T, Deleter>`
/// @version 0.1
/// @date 2022-12-03
///
/// MIT License
/// @copyright Copyright (c) 2022 Braxton Salyer <braxtonsalyer@gmail.com>
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

#include <Hyperion/Concepts.h>
#include <Hyperion/HyperionDef.h>
#include <Hyperion/memory/CompressedPair.h>
#include <gsl/gsl>

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
/// @headerfile "Hyperion/memory/UniquePtr.h"
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
	/// @headerfile "Hyperion/memory/UniquePtr.h"
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
		explicit DefaultDeleter([[maybe_unused]] DefaultDeleter<U>&& deleter) noexcept {
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
		template<typename U>
		requires concepts::Convertible<U*, T*>
		constexpr auto
		operator=([[maybe_unused]] const DefaultDeleter<U>& deleter) noexcept -> DefaultDeleter& {
			return *this;
			// deleters have no state, so this is a no-op
		}
		template<typename U>
		requires concepts::Convertible<U*, T*>
		constexpr auto
		operator=([[maybe_unused]] DefaultDeleter<U>&& deleter) noexcept -> DefaultDeleter& {
			// deleters have no state, so this is a no-op
			return *this;
		}
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
		template<typename U>
		requires concepts::Convertible<U (*)[], T (*)[]>						// NOLINT
		explicit DefaultDeleter([[maybe_unused]] DefaultDeleter<U[]>&& deleter) // NOLINT
			noexcept {
		}

		constexpr auto
		operator()(gsl::owner<T*> ptr) const noexcept(concepts::NoexceptDeletable<T[]>) // NOLINT
		{
			delete[] ptr;
		}

		constexpr auto operator=(const DefaultDeleter&) noexcept -> DefaultDeleter& = default;
		constexpr auto operator=(DefaultDeleter&&) noexcept -> DefaultDeleter& = default;
		template<typename U>
		requires concepts::Convertible<U (*)[], T (*)[]>							  // NOLINT
		constexpr auto operator=([[maybe_unused]] const DefaultDeleter<U[]>& deleter) // NOLINT
			noexcept -> DefaultDeleter& {
			// deleters have no state, so this is a no-op
		}
		template<typename U>
		requires concepts::Convertible<U (*)[], T (*)[]>						 // NOLINT
		constexpr auto operator=([[maybe_unused]] DefaultDeleter<U[]>&& deleter) // NOLINT
			noexcept -> DefaultDeleter& {
			// deleters have no state, so this is a no-op
		}
	};

	/// @brief `UniquePtr<T, Deleter>` is Hyperion's `constexpr` equivalent to the standard
	/// library's `std::unique_ptr<T, Deleter>`
	///
	/// `UniquePtr<T, Deleter>` is a `constexpr` implementation of a unique owning pointer, with
	/// almost complete semantic equivalence with `std::unique_ptr<T, Deleter>`.
	/// It is a drop-in replacement in almost all situations(1).
	/// Hyperion also provides extended functionality for `UniquePtr<T, Deleter>`
	/// over `std::unique_ptr<T, Deleter>`, eg: an allocator-aware factory function set in
	/// `allocate_unique`
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
	/// @headerfile "Hyperion/memory/UniquePtr.h"
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
		constexpr UniquePtr() noexcept
		requires concepts::NoexceptDefaultConstructible<deleter_type>
				 && concepts::NotPointer<deleter_type>
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
		requires concepts::NoexceptDefaultConstructible<deleter_type>
				 && concepts::NotPointer<deleter_type>
			: m_ptr(ptr, DefaultInitTag<deleter_type>()) {
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
		explicit constexpr UniquePtr(pointer ptr) noexcept
		requires concepts::NoexceptDefaultConstructible<deleter_type>
				 && concepts::NotPointer<deleter_type>
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
		requires concepts::Convertible<U, pointer>
				 && concepts::NoexceptDefaultConstructible<deleter_type>
				 && concepts::NotPointer<deleter_type>
		explicit constexpr UniquePtr(U ptr) noexcept : m_ptr(ptr, DefaultInitTag<deleter_type>()) {
		}
		/// @brief Constructs a `UniquePtr<T, Deleter>` managing the given pointer
		///
		/// @param ptr - The pointer to manage with this `UniquePtr`
		/// @param deleter - The deleter to delete `ptr` with when the destructor of this
		/// `UniquePtr` is called
		///
		/// @ingroup UniquePtr
		constexpr UniquePtr(pointer ptr, const Deleter& deleter) noexcept
		requires concepts::NoexceptCopyConstructible<Deleter>
			: m_ptr(ptr, deleter) {
		}
		/// @brief Constructs a `UniquePtr<T, Deleter>` managing the given pointer
		///
		/// @param ptr - The pointer to manage with this `UniquePtr`
		/// @param deleter - The deleter to delete `ptr` with when the destructor of this
		/// `UniquePtr` is called
		///
		/// @ingroup UniquePtr
		constexpr UniquePtr(pointer ptr, Deleter&& deleter) noexcept
		requires concepts::NoexceptMoveConstructible<Deleter>
				 && concepts::NotRValueReference<Deleter>
			: m_ptr(ptr, std::move(deleter)) {
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
		requires concepts::Convertible<U, pointer> && concepts::NotSame<U, pointer>
				 && concepts::NoexceptCopyConstructible<Deleter>
		constexpr UniquePtr(U ptr, const Deleter& deleter) noexcept : m_ptr(ptr, deleter) {
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
		requires concepts::Convertible<U, pointer> && concepts::NotSame<U, pointer>
				 && concepts::NoexceptMoveConstructible<Deleter>
				 && concepts::NotRValueReference<Deleter>
		constexpr UniquePtr(U ptr, Deleter&& deleter) noexcept : m_ptr(ptr, std::move(deleter)) {
		}
		/// @brief Constructs a `UniquePtr<T, Deleter>` managing no pointer
		///
		/// @param ptr - The `nullptr` signaling this `UniquePtr` should manage no pointer
		/// @param deleter - The deleter to delete a managed pointer with when the destructor of
		/// this `UniquePtr` is called
		///
		/// @ingroup UniquePtr
		constexpr UniquePtr(std::nullptr_t ptr, const Deleter& deleter) noexcept
		requires concepts::NoexceptCopyConstructible<Deleter>
			: m_ptr(ptr, deleter) {
		}
		/// @brief Constructs a `UniquePtr<T, Deleter>` managing no pointer
		///
		/// @param ptr - The `nullptr` signaling this `UniquePtr` should manage no pointer
		/// @param deleter - The deleter to delete a managed pointer with when the destructor of
		/// this `UniquePtr` is called
		///
		/// @ingroup UniquePtr
		constexpr UniquePtr(std::nullptr_t ptr, Deleter&& deleter) noexcept
		requires concepts::NoexceptMoveConstructible<Deleter>
				 && concepts::NotRValueReference<Deleter>
			: m_ptr(ptr, std::move(deleter)) {
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
		constexpr UniquePtr(UniquePtr<U, D>&& ptr) noexcept // NOLINT
		requires concepts::NoexceptConstructibleFrom<deleter_type, D>
				 && concepts::Convertible<typename UniquePtr<U, D>::pointer, pointer>
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
		constexpr UniquePtr(UniquePtr&& ptr) noexcept
		requires concepts::NoexceptMoveConstructible<deleter_type>
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
		[[nodiscard]] inline constexpr auto release() noexcept -> pointer {
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
		inline constexpr auto reset(pointer ptr = pointer()) noexcept -> void
		requires(noexcept(std::declval<deleter_type>()(std::declval<pointer>()))) // NOLINT
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
				 inline constexpr auto reset(U ptr) noexcept -> void
				 requires(noexcept(std::declval<deleter_type>()(std::declval<pointer>()))) // NOLINT
		{
			gsl::owner<pointer> tmp = m_ptr.first(); // NOLINT
			m_ptr.first() = ptr;
			m_ptr.second()(tmp);
		}

		/// @brief Swaps the managed pointer and deleter of this `UniquePtr` with the given
		/// one
		///
		/// @param ptr - The `UniquePtr` to swap with
		///
		/// # Requirements
		/// - `std::is_nothrow_swappable_v<CompressedPair<pointer, deleter_type>>`:
		/// `CompressedPair<pointer, deleter_type> must be noexcept swappable in order to
		/// swap two `UniquePtr`s. (`CompressedPair` is used internally to store the
		/// managed pointer and deleter, to allow for [Empty Base Class
		/// Optimization](https://en.cppreference.com/w/cpp/language/ebo) )
		/// @ingroup UniquePtr
		inline constexpr auto swap(UniquePtr& ptr) noexcept -> void
		requires concepts::NoexceptSwappable<CompressedPair<pointer, deleter_type>>
		{
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
		[[nodiscard]] inline constexpr auto get() noexcept -> pointer {
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
		[[nodiscard]] inline constexpr auto get() const noexcept -> pointer_to_const {
			return m_ptr.first();
		}

		/// @brief Returns the associated deleter
		///
		/// @return The associated deleter
		/// @ingroup UniquePtr
		[[nodiscard]] inline constexpr auto get_deleter() const noexcept -> const deleter_type& {
			return m_ptr.second();
		}

		/// @brief Returns the associated deleter
		///
		/// @return The associated deleter
		/// @ingroup UniquePtr
		[[nodiscard]] inline constexpr auto get_deleter() noexcept -> deleter_type& {
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

		inline constexpr auto operator==(const UniquePtr& ptr) const noexcept -> bool {
			return m_ptr.first() == ptr.first();
		}

		inline constexpr auto operator==(std::nullptr_t) const noexcept -> bool {
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
		constexpr auto operator=(UniquePtr&& ptr) noexcept -> UniquePtr&
		requires concepts::NoexceptMoveAssignable<deleter_type>
				 && (noexcept(std::declval<deleter_type>()(std::declval<pointer>()))) // NOLINT
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
		constexpr auto operator=(UniquePtr<U, D>&& ptr) noexcept -> UniquePtr&
		requires concepts::NoexceptAssignable<deleter_type, D>
				 && concepts::Convertible<typename UniquePtr<U, D>::pointer, pointer>
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
		constexpr auto operator=(std::nullptr_t) noexcept -> UniquePtr&
		requires(noexcept(std::declval<deleter_type>()(std::declval<pointer>()))) // NOLINT
		{
			reset();
			return *this;
		}

		/// @brief Assigns this `UniquePtr` with `ptr`
		///
		/// Calls the deleter on the managed pointer and replaces it with `ptr`
		///
		/// # Requirements
		/// - `noexcept(std::declval<deleter_type>()(std::declval<pointer>()))`: Deleting the
		/// managed pointer via the associated `deleter_type`'s call operator must be noexcept
		///
		/// @return this
		/// @ingroup UniquePtr
		constexpr auto operator=(pointer ptr) noexcept -> UniquePtr&
		requires(noexcept(std::declval<deleter_type>()(std::declval<pointer>()))) // NOLINT
		{
			reset(ptr);
			return *this;
		}

	  private:
		template<typename T_, typename Deleter_>
		using CompressedPair = CompressedPair<T_, Deleter_>;
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
		using pointer = std::add_pointer_t<element_type>;
		using pointer_to_const = std::add_pointer_t<std::add_const_t<element_type>>;

		constexpr UniquePtr() noexcept
		requires concepts::NoexceptDefaultConstructible<deleter_type>
				 && concepts::NotPointer<deleter_type>
			: m_ptr(pointer(), DefaultInitTag<deleter_type>()) {
		}

		constexpr UniquePtr(std::nullptr_t ptr) noexcept // NOLINT
		requires concepts::NoexceptDefaultConstructible<deleter_type>
				 && concepts::NotPointer<deleter_type>
			: m_ptr(ptr, DefaultInitTag<deleter_type>()) {
		}
		explicit constexpr UniquePtr(pointer ptr) noexcept
		requires concepts::NoexceptDefaultConstructible<deleter_type>
				 && concepts::NotPointer<deleter_type>
			: m_ptr(ptr, DefaultInitTag<deleter_type>()) {
		}
		template<typename U>
		requires concepts::Convertible<U, pointer>
				 && concepts::NoexceptDefaultConstructible<deleter_type>
				 && concepts::NotPointer<deleter_type> && concepts::NotSame<U, pointer>
		explicit constexpr UniquePtr(U ptr) noexcept : m_ptr(ptr, DefaultInitTag<deleter_type>()) {
		}
		constexpr UniquePtr(pointer ptr, const Deleter& deleter) noexcept
		requires concepts::NoexceptCopyConstructible<Deleter>
			: m_ptr(ptr, deleter) {
		}
		constexpr UniquePtr(pointer ptr, Deleter&& deleter) noexcept
		requires concepts::NoexceptMoveConstructible<Deleter>
				 && concepts::NotRValueReference<Deleter>
			: m_ptr(ptr, std::move(deleter)) {
		}
		template<typename U>
		requires concepts::Convertible<U, pointer> && concepts::NotSame<U, pointer>
				 && concepts::NoexceptCopyConstructible<Deleter>
		constexpr UniquePtr(U ptr, const Deleter& deleter) noexcept : m_ptr(ptr, deleter) {
		}
		template<typename U>
		requires concepts::Convertible<U, pointer> && concepts::NotSame<U, pointer>
				 && concepts::NoexceptMoveConstructible<Deleter>
				 && concepts::NotRValueReference<Deleter>
		constexpr UniquePtr(U ptr, Deleter&& deleter) noexcept : m_ptr(ptr, std::move(deleter)) {
		}
		constexpr UniquePtr(std::nullptr_t ptr, const Deleter& deleter) noexcept
		requires concepts::NoexceptCopyConstructible<Deleter>
			: m_ptr(ptr, deleter) {
		}
		constexpr UniquePtr(std::nullptr_t ptr, Deleter&& deleter) noexcept
		requires concepts::NoexceptMoveConstructible<Deleter>
				 && concepts::NotRValueReference<Deleter>
			: m_ptr(ptr, std::move(deleter)) {
		}
		// clang-format off
		template<typename U, typename D>
		requires std::is_array_v<U>
		constexpr UniquePtr(UniquePtr<U, D>&& ptr) // NOLINT
			requires concepts::NoexceptConstructibleFrom<deleter_type, D>
					 && concepts::Convertible<typename UniquePtr<U, D>::element_type(*)[], // NOLINT
										   	  element_type (*)[] > 						   // NOLINT
			: m_ptr(ptr.release(), std::forward<D>(ptr.get_deleter())) {
		}
		// clang-format on
		UniquePtr(const UniquePtr&) = delete;
		constexpr UniquePtr(UniquePtr&& ptr) noexcept
		requires concepts::NoexceptMoveConstructible<deleter_type>
			: m_ptr(ptr.release(), std::forward<deleter_type>(ptr.get_deleter())) {
		}
		constexpr ~UniquePtr() noexcept
		requires(noexcept(std::declval<deleter_type>()(std::declval<pointer>()))) // NOLINT
		{
			reset();
		}

		[[nodiscard]] inline constexpr auto release() noexcept -> pointer {
			auto* ptr = m_ptr.first();
			m_ptr.first() = pointer();
			return ptr;
		}

		inline constexpr auto reset(pointer ptr = pointer()) noexcept -> void
		requires(noexcept(std::declval<deleter_type>()(std::declval<pointer>()))) // NOLINT
		{
			gsl::owner<pointer> tmp = m_ptr.first(); // NOLINT
			m_ptr.first() = ptr;
			m_ptr.second()(tmp);
		}

		template<typename U>
		requires concepts::Convertible<U, pointer>
				 inline constexpr auto reset(U ptr) noexcept -> void
				 requires(noexcept(std::declval<deleter_type>()(std::declval<pointer>()))) // NOLINT
		{
			gsl::owner<pointer> tmp = m_ptr.first(); // NOLINT
			m_ptr.first() = ptr;
			m_ptr.second()(tmp);
		}

		inline constexpr auto swap(UniquePtr& ptr) noexcept -> void
		requires concepts::NoexceptSwappable<CompressedPair<pointer, deleter_type>>
		{
			m_ptr.swap(ptr.m_ptr);
		}

		[[nodiscard]] inline constexpr auto get() noexcept -> pointer {
			return m_ptr.first();
		}

		[[nodiscard]] inline constexpr auto get() const noexcept -> pointer_to_const {
			return m_ptr.first();
		}

		[[nodiscard]] inline constexpr auto get_deleter() const noexcept -> const deleter_type& {
			return m_ptr.second();
		}

		[[nodiscard]] inline constexpr auto get_deleter() noexcept -> deleter_type& {
			return m_ptr.second();
		}

		explicit constexpr operator bool() const noexcept {
			return m_ptr.first() != nullptr;
		}

		inline constexpr auto operator==(const UniquePtr& ptr) const noexcept -> bool {
			return m_ptr.first() == ptr.first();
		}

		inline constexpr auto operator==(std::nullptr_t) const noexcept -> bool {
			return m_ptr.first() == nullptr;
		}

		// constexpr inline auto operator[](concepts::Integral auto i) const
		//	noexcept -> std::add_const_t<std::add_lvalue_reference_t<element_type>> {
		//	return m_ptr.first()[i];
		// }

		inline constexpr auto operator[](concepts::Integral auto index) const noexcept
			-> std::add_lvalue_reference_t<element_type> {
			return m_ptr.first()[index];
		}

		auto operator=(const UniquePtr&) -> UniquePtr& = delete;
		constexpr auto operator=(UniquePtr&& ptr) noexcept -> UniquePtr&
		requires concepts::NoexceptMoveAssignable<deleter_type>
				 && (noexcept(std::declval<deleter_type>()(std::declval<pointer>()))) // NOLINT
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
			requires concepts::NoexceptAssignable<deleter_type, D>
					 && concepts::Convertible<typename UniquePtr<U, D>::element_type(*)[], // NOLINT
										   	  element_type (*)[] > 						   // NOLINT
		{
			reset(ptr.release());
			m_ptr.second() = std::forward<D>(ptr.get_deleter());
			return *this;
		}

		// clang-format on

		constexpr auto operator=(std::nullptr_t) noexcept -> UniquePtr&
		requires(noexcept(std::declval<deleter_type>()(std::declval<pointer>()))) // NOLINT
		{
			reset();
			return *this;
		}

		constexpr auto operator=(pointer ptr) noexcept -> UniquePtr&
		requires(noexcept(std::declval<deleter_type>()(std::declval<pointer>()))) // NOLINT
		{
			reset(ptr);
			return *this;
		}

	  private:
		template<typename T_, typename Deleter_>
		using CompressedPair = CompressedPair<T_, Deleter_>;
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
	inline constexpr auto
	swap(UniquePtr<T, Deleter>& first, UniquePtr<T, Deleter>& second) noexcept -> void {
		first.swap(second);
	}
	// clang-format off

	IGNORE_UNUSED_TEMPLATES_START

    /// @ingroup UniquePtr
    /// @{
    ///	@defgroup make_unique make_unique
    /// @headerfile "Hyperion/memory/UniquePtr.h"
    /// @}

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
	/// @ingroup make_unique
	template<typename T, typename Deleter = DefaultDeleter<T>, typename... Args>
	requires concepts::ConstructibleFrom<T, Args...> && (!std::is_unbounded_array_v<T>)
	inline static constexpr auto make_unique(Args&&... args)
		noexcept(concepts::NoexceptConstructibleFrom<T, Args...>)
		-> UniquePtr<T, Deleter>
	{
		// NOLINTNEXTLINE(modernize-use-auto,hicpp-use-auto,bugprone-unhandled-exception-at-new)
		gsl::owner<T*> ptr = new T(std::forward<Args>(args)...);
		// NOLINTNEXTLINE(clang-analyzer-cplusplus.NewDeleteLeaks)
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
	/// @param num_elements - The initial size of the array managed by the `UniquePtr`
	///
	/// # Requirements
	/// - `std::is_unbounded_array_v<T>`: `T` must be an unbounded array type to make a `UniquePtr`
	/// managing an array.
	///
	/// @return a `UniquePtr<T, Deleter>` managing an array of `N` `ElementType`s
	/// @note The array managed by the returned `UniquePtr` will be uninitialized, and thus each
	/// element must be appropriately initialized before use. Otherwise, the result is undefined
	/// behavior.
	/// @ingroup make_unique
	template<typename T,
			 typename Deleter = DefaultDeleter<T>,
			 typename ElementType = std::remove_extent_t<T>> // NOLINT
	requires std::is_unbounded_array_v<T>
	static inline constexpr auto make_unique(usize num_elements) noexcept -> UniquePtr<T, Deleter> {
		// NOLINTNEXTLINE(modernize-use-auto,hicpp-use-auto,bugprone-unhandled-exception-at-new)
		gsl::owner<ElementType*> ptr = new ElementType[num_elements];
		// NOLINTNEXTLINE(clang-analyzer-cplusplus.NewDeleteLeaks)
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
	///
	/// # Requirements
	/// 	- `concepts::Allocatable<ElementType, Allocator>`
	/// 	(where `using ElementType = std::remove_cv_t<std::remove_all_extents_t<T>>;`)
	/// 	`AllocatorAwareDeleter` can't be
	/// 	instantiated for an `ElementType`-`Allocator` pair where the element type `ElementType`
	/// 	isn't allocatable by the associated allocator type, `Allocator`
	/// @note In the case that `T` is an array type, similarly to how `allocate_unique` will not
	/// handle construction of the array elements, `AllocatorAwareDeleter` will __NOT__ handle
	/// destruction of the array elements. That is up to the calling code to handle.
	/// @note If `T` is an array type and the members of the array have not been properly destroyed
	/// when the call operator of this deleter is used to free the resources associated with it,
	/// the result is undefined behavior.
	/// @ingroup memory
	template<typename T, typename Allocator = std::allocator<T>>
	requires concepts::Allocatable<std::remove_cv_t<std::remove_all_extents_t<T>>, Allocator>
	class AllocatorAwareDeleter {
	  public:
		/// @brief The element type for this deleter, in the case that `T` is an array type
		/// @ingroup memory
		using ElementType = std::remove_cv_t<std::remove_all_extents_t<T>>;
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
		constexpr AllocatorAwareDeleter() noexcept
		requires concepts::NoexceptDefaultConstructible<Alloc>
			: m_allocator() {
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
		explicit constexpr AllocatorAwareDeleter(const Allocator& alloc) noexcept
		requires concepts::NoexceptConstructibleFrom<Alloc, const Allocator&>
			: m_allocator(alloc) {
		}
		/// @brief Copy-Constructs an `AllocatorAwareDeleter` from the given one
		///
		/// # Requirements
		/// - `concepts::NoexceptCopyConstructible<Alloc>`: The associated allocator type must be
		/// noexcept copy constructible in order to copy construct an `AllocatorAwareDeleter`
		/// @ingroup memory
		constexpr AllocatorAwareDeleter(const AllocatorAwareDeleter&) noexcept
		requires concepts::NoexceptCopyConstructible<Alloc>
		= default;
		/// @brief Move-Constructs an `AllocatorAwareDeleter` from the given one
		///
		/// # Requirements
		/// - `concepts::NoexceptMoveConstructible<Alloc>`: The associated allocator type must be
		/// noexcept move constructible in order to copy construct an `AllocatorAwareDeleter`
		/// @ingroup memory
		constexpr AllocatorAwareDeleter(AllocatorAwareDeleter&&) noexcept
		requires concepts::NoexceptMoveConstructible<Alloc>
		= default;

		/// @brief Destructs this `AllocatorAwareDeleter`
		/// @ingroup memory
		constexpr ~AllocatorAwareDeleter() noexcept = default;

		/// @brief Frees the resources associated with the pointed-to object
		///
		/// @param ptr - Pointer to the object to free associated resources of
		/// @ingroup memory
		inline constexpr auto operator()(pointer ptr) const noexcept {
			Alloc allocator = m_allocator;

			Traits::destroy(allocator, std::addressof(*ptr));

			Traits::deallocate(allocator, ptr, 1);
		}

		/// @brief Copy-Assigns this `AllocatorAwareDeleter` from the given one
		///
		/// # Requirements
		/// - `concepts::NoexceptCopyAssignable<Alloc>`: The associated allocator type must be
		/// noexcept copy assignable in order to copy assign this `AllocatorAwareDeleter`
		/// @ingroup memory
		constexpr auto operator=(const AllocatorAwareDeleter&) noexcept -> AllocatorAwareDeleter&
		requires concepts::NoexceptCopyAssignable<Alloc>
		= default;
		/// @brief Move-Assigns this `AllocatorAwareDeleter` from the given one
		///
		/// # Requirements
		/// - `concepts::NoexceptMoveAssignable<Alloc>`: The associated allocator type must be
		/// noexcept move assignable in order to move assign this `AllocatorAwareDeleter`
		/// @ingroup memory
		constexpr auto operator=(AllocatorAwareDeleter&&) noexcept -> AllocatorAwareDeleter&
		requires concepts::NoexceptMoveAssignable<Alloc>
		= default;

	  private:
		[[HYPERION_NO_UNIQUE_ADDRESS]] Alloc m_allocator;
	};

	template<typename T, typename Allocator>
	requires concepts::Allocatable<std::remove_cv_t<std::remove_all_extents_t<T>>, Allocator>
	class AllocatorAwareDeleter<T[], Allocator> { // NOLINT
	  public:
		using ElementType = std::remove_cv_t<std::remove_all_extents_t<T>>;
		using Alloc = typename std::allocator_traits<Allocator>::template rebind_alloc<ElementType>;
		using Traits = std::allocator_traits<Alloc>;
		using pointer = typename Traits::pointer;

		constexpr AllocatorAwareDeleter() noexcept
		requires concepts::NoexceptDefaultConstructible<Alloc>
			: m_allocator() {
		}
		explicit constexpr AllocatorAwareDeleter(const Allocator& alloc,
												 usize num_elements) noexcept
		requires concepts::NoexceptConstructibleFrom<Alloc, const Allocator&>
			: m_allocator(alloc), m_num_elements(num_elements) {
		}
		constexpr AllocatorAwareDeleter(const AllocatorAwareDeleter&) noexcept
		requires concepts::NoexceptCopyConstructible<Alloc>
		= default;
		constexpr AllocatorAwareDeleter(AllocatorAwareDeleter&&) noexcept
		requires concepts::NoexceptMoveConstructible<Alloc>
		= default;
		constexpr ~AllocatorAwareDeleter() noexcept = default;

		inline constexpr auto operator()(pointer ptr) const noexcept {
			Alloc allocator = m_allocator;

			Traits::deallocate(allocator, ptr, m_num_elements);
		}

		constexpr auto operator=(const AllocatorAwareDeleter&) noexcept -> AllocatorAwareDeleter&
		requires concepts::NoexceptCopyAssignable<Alloc>
		= default;
		constexpr auto operator=(AllocatorAwareDeleter&&) noexcept -> AllocatorAwareDeleter&
		requires concepts::NoexceptMoveAssignable<Alloc>
		= default;

	  private:
		[[HYPERION_NO_UNIQUE_ADDRESS]] Alloc m_allocator;
		usize m_num_elements = 1_usize;
	};
	IGNORE_PADDING_STOP
    // clang-format off

	/// @ingroup UniquePtr
	/// @{
	///	@defgroup allocate_unique allocate_unique
	/// @headerfile "Hyperion/memory/UniquePtr.h"
	/// @}

	/// @brief Constructs an allocator-aware `UniquePtr`
	///
	/// Constructs a `UniquePtr<T, AllocatorAwareDeleter<T, Alloc>>`. The managed `T` will be
	/// constructed from the given arguments, `args`, passing them directly to `T`'s constructor.
	/// Allocation of the managed `T` will be performed by the given allocator.
	///
	/// @tparam T - The type to manage in the `UniquePtr`
	/// @tparam Allocator - The type of the allocator to use for allocation and deletion of the `T`
	/// @tparam ElementType - The element type of the array, in the case that `T` is an unbounded
	/// array type. This should not be provided explicitly, it will be deduced from the `T`
	/// template argument.
	/// @tparam Alloc - The allocator type `Allocator` rebound to the element type, `ElementType`.
	/// This should not be provided explicitly, it will be deduced from the `T` and `Allocator`
	/// template arguments.
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
	/// - `concepts::NoexceptConstructibleFrom<Alloc, decltype(alloc)>`: The associated
	/// allocator type, `Alloc`, must be noexcept constructible from the given allocator in
	/// order to create a `UniquePtr` using it
	/// @ingroup allocate_unique
	template<typename T,
			 typename Allocator = std::allocator<T>,
			 typename ElementType = std::remove_cv_t<std::remove_all_extents_t<T>>,
			 typename Alloc
			 = typename std::allocator_traits<Allocator>::template rebind_alloc<ElementType>,
			 typename... Args>
	requires concepts::NoexceptConstructibleFrom<ElementType, Args...>
			 && concepts::Allocatable<ElementType, Alloc>
			 && (!std::is_unbounded_array_v<T>)
             && concepts::NoexceptConstructibleFrom<Alloc, const Allocator&>
	[[nodiscard]] inline constexpr auto allocate_unique(const Allocator& alloc, Args&&... args)
        noexcept -> UniquePtr<T, AllocatorAwareDeleter<T, Alloc>>
	{
		using Traits = std::allocator_traits<Alloc>;
		using Deleter = AllocatorAwareDeleter<T, Alloc>;

		Alloc allocator(alloc);
		auto* ptr = Traits::allocate(allocator, 1);
		Traits::construct(allocator, std::addressof(*ptr), std::forward<Args>(args)...);

		// NOLINTNEXTLINE(clang-analyzer-cplusplus.NewDeleteLeaks)
		return UniquePtr<T, Deleter>(ptr, Deleter(allocator));
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
	/// array type. This should not be provided explicitly, it will be deduced from the `T`
	/// template argument.
	/// @tparam Alloc - The allocator type `Allocator` rebound to the element type, `ElementType`.
	/// This should not be provided explicitly, it will be deduced from the `T` and `Allocator`
	/// template arguments.
	///
	///
	/// @param alloc - The allocator to perform allocation and deallocation with
	/// @param num_elements - The number of elements to allocate in the array
	///
	/// # Requirements
	/// - `concepts::Allocatable<ElementType, Alloc>`: `AllocatorAwareDeleter` can't be
	/// instantiated for an `ElementType`-`Alloc` pair where the element type `ElementType`
	/// isn't allocatable by the associated allocator type, `Alloc`
	/// - `std::is_unbounded_array_v<T>`: `T` must be an unbounded array type to make a
	/// `UniquePtr` managing an array.
	/// - `concepts::NoexceptConstructibleFrom<Alloc, decltype(alloc)>`: The associated allocator
	/// type, `Alloc`, must be noexcept constructible from the given allocator in order to create a
	/// `UniquePtr` using it
	///
	/// @note When calling `allocate_unique` to allocate an unbounded array type, `allocate_unique`
	/// will __NOT__ handle construction of the array members, only allocation. Construction is left
	/// to the calling code.
	/// @ingroup allocate_unique
	template<typename T,
			 typename Allocator = std::allocator<T>,
			 typename ElementType = std::remove_cv_t<std::remove_all_extents_t<T>>,
			 typename Alloc
			 = typename std::allocator_traits<Allocator>::template rebind_alloc<ElementType>>
	requires concepts::Allocatable<ElementType, Alloc>
			 && std::is_unbounded_array_v<T>
			 && concepts::NoexceptConstructibleFrom<Alloc, const Allocator&>
    [[nodiscard]] inline constexpr auto allocate_unique(const Allocator& alloc, usize num_elements)
        noexcept -> UniquePtr<T, AllocatorAwareDeleter<T, Alloc>>
	{
		using Traits = std::allocator_traits<Alloc>;
		using Deleter = AllocatorAwareDeleter<ElementType[], Alloc>; // NOLINT (c arrays)

		Alloc allocator(alloc);

		// NOLINTNEXTLINE(clang-analyzer-cplusplus.NewDeleteLeaks)
		return UniquePtr<ElementType[], Deleter>( // NOLINT (arrays)
			Traits::allocate(allocator, num_elements),
			Deleter(allocator, num_elements));
	}
} // namespace hyperion
