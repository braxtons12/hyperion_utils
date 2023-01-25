/// @file EmptyBaseClass.h
/// @author Braxton Salyer <braxtonsalyer@gmail.com>
/// @brief This includes a utility class for performing
/// [Empty Base Class Optimization](https://en.cppreference.com/w/cpp/language/ebo)
/// @version 0.1
/// @date 2023-01-25
///
/// MIT License
/// @copyright Copyright (c) 2023 Braxton Salyer <braxtonsalyer@gmail.com>
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

/// @ingroup memory
/// @{
///	@defgroup EmptyBaseClass EmptyBaseClass
/// This modules provides a class template for wrapping a type for use in an
/// [Empty Base Class Optimized](https://en.cppreference.com/w/cpp/language/ebo) manner, along with
/// relevant utilities for the associated implementation. This is useful, even in C++20 and up, when
/// you __require__ that a type be EBOd portably, as things like `[[no_unique_address]]` are not
/// guarateed to actually apply the optimization. For example, on Windows, `[[no_unique_address]]`
/// does nothing on MSVC (until they are able to take an ABI break) and Clang, and MSVC's stand-in,
/// `[[msvc::no_unique_address]]`, is not supported by Clang on Windows. This is just one example
/// where the only way to ensure EBO is using this method instead of relying on the new attribute.
/// @headerfile "Hyperion/memory/EmptyBaseClass.h"
/// @}
namespace hyperion {

	/// @brief Tag type indicating the type should be left initialized
	/// @ingroup EmptyBaseClass
	template<typename T>
	struct UnInitTag { };

	/// @brief Tag type indicating the type should be default-constructed
	/// @ingroup EmptyBaseClass
	template<concepts::DefaultConstructible T>
	struct DefaultInitTag { };

	/// @brief Concept Requiring that a member of type `T` can be Empty Base Class Optimized
	/// @ingroup EmptyBaseClass
	template<typename T>
	concept EmptyBaseClassOptimizable = std::is_empty_v<T>;

	/// @brief Concept Requiring that a member of type `T` can NOT be Empty Base Class Optimized
	/// @ingroup EmptyBaseClass
	template<typename T>
	concept NotEmptyBaseClassOptimizable = !
	EmptyBaseClassOptimizable<T>;

	/// @brief `EmptyBaseClass` is a utility class template for wrapping a type for use in an
	/// [Empty Base Class Optimized](https://en.cppreference.com/w/cpp/language/ebo) manner, along
	/// with relevant utilities for the associated implementation. This is useful, even in C++20
	/// and up, when you __require__ that a type be EBOd portably, as things like
	/// `[[no_unique_address]]` are not guarateed to actually apply the optimization. For example,
	/// on Windows, `[[no_unique_address]]` does nothing on MSVC (until they are able to take an ABI
	/// break) and Clang, and MSVC's stand-in,
	/// `[[msvc::no_unique_address]]`, is not supported by Clang on Windows. This is just one
	/// example where the only way to ensure EBO is using this method instead of relying on the new
	/// attribute.
    ///
	/// @tparam T - The type to wrap
	/// @ingroup EmptyBaseClass
	template<typename T>
	class EmptyBaseClass;

	template<EmptyBaseClassOptimizable T>
	class EmptyBaseClass<T> : private T {
		// clang-format on
	  public:
		/// @brief The type wrapped by this `EmptyBaseClass`
		/// @ingroup EmptyBaseClass
		using type = T;
		/// @brief Reference to the type wrapped by this `EmptyBaseClass`
		/// @ingroup EmptyBaseClass
		using reference = std::add_lvalue_reference_t<type>;
		/// @brief Reference to const the type wrapped by this `EmptyBaseClass`
		/// @ingroup EmptyBaseClass
		using const_reference = std::add_const_t<std::add_lvalue_reference_t<type>>;

		/// @brief Constructs this `EmptyBaseClass` in an uninitialized state
		/// @ingroup EmptyBaseClass
		constexpr EmptyBaseClass(UnInitTag<T>) noexcept { // NOLINT
		}
		/// @brief Constructs this `EmptyBaseClass` in a default-constructed state
		/// @ingroup EmptyBaseClass
		constexpr EmptyBaseClass(DefaultInitTag<T>) // NOLINT
			noexcept(concepts::NoexceptDefaultConstructible<T>)
			: EmptyBaseClass() {
		}
		// clang-format off
		constexpr EmptyBaseClass(const EmptyBaseClass&)
			noexcept(concepts::NoexceptCopyConstructible<T>)
			requires concepts::CopyConstructible<T>
			= default;
		constexpr EmptyBaseClass(EmptyBaseClass&&)
			noexcept(concepts::NoexceptMoveConstructible<T>)
			requires concepts::MoveConstructible<T>
			= default;

		/// @brief Constructs this `EmptyBaseClass` from the given arguments
		///
		/// @tparam Args - The types of arguments to pass to `T`'s constructor
		///
		/// @param args - The arguments to pass to `T`'s constructor
		///
		/// # Requirements
		/// - `concepts::ConstructibleFrom<T, Args...>`: `T` must be constructible from the
		/// `Args...` to construct this `EmptyBaseClass` from them
		/// @ingroup EmptyBaseClass
		template<typename... Args>
		requires concepts::ConstructibleFrom<T, Args...>
		explicit constexpr EmptyBaseClass(Args&&... args)
			noexcept(concepts::NoexceptConstructibleFrom<T, Args...>)
			: type(std::forward<Args>(args)...) {
		}
		// clang-format on

		constexpr ~EmptyBaseClass() noexcept(concepts::NoexceptDestructible<T>) = default;

		/// @brief Returns a reference to const to the `T` wrapped by this
		///
		/// @return reference to const `T`
		/// @ingroup EmptyBaseClass
		inline constexpr auto get() const noexcept -> const_reference {
			return *static_cast<const type*>(this);
		}

		/// @brief Returns a reference to the `T` wrapped by this
		///
		/// @return reference to `T`
		/// @ingroup EmptyBaseClass
		inline constexpr auto get() noexcept -> reference {
			return *static_cast<type*>(this);
		}

		// clang-format off
		constexpr auto operator=(const EmptyBaseClass&)
			noexcept(concepts::NoexceptCopyAssignable<T>)
			-> EmptyBaseClass&
			requires concepts::CopyAssignable<T>
			= default;
		constexpr auto operator=(EmptyBaseClass&&)
			noexcept(concepts::NoexceptMoveAssignable<T>)
			-> EmptyBaseClass&
			requires concepts::MoveAssignable<T>
			= default;
		// clang-format on

	  private:
		constexpr EmptyBaseClass() noexcept(concepts::NoexceptDefaultConstructible<T>) : type() {
		}
	};
	// clang-format off

	template<NotEmptyBaseClassOptimizable T>
	class EmptyBaseClass<T> {
		// clang-format on
	  public:
		/// @brief The type wrapped by this `EmptyBaseClass`
		using type = T;
		/// @brief Reference to the type wrapped by this `EmptyBaseClass`
		using reference = std::add_lvalue_reference_t<type>;
		/// @brief Reference to const the type wrapped by this `EmptyBaseClass`
		using const_reference = std::add_lvalue_reference_t<std::add_const_t<type>>;

		/// @brief Constructs this `EmptyBaseClass` in an uninitialized state
		constexpr EmptyBaseClass(UnInitTag<T>) noexcept { // NOLINT
		}
		/// @brief Constructs this `EmptyBaseClass` in a default-constructed state
		constexpr EmptyBaseClass(DefaultInitTag<T>) // NOLINT
			noexcept(concepts::NoexceptDefaultConstructible<T>)
			: EmptyBaseClass() {
		}
		// clang-format off
		constexpr EmptyBaseClass(const EmptyBaseClass&)
			noexcept(concepts::NoexceptCopyConstructible<T>)
			requires concepts::CopyConstructible<T>
			= default;
		constexpr EmptyBaseClass(EmptyBaseClass&&)
			noexcept(concepts::NoexceptMoveConstructible<T>)
			requires concepts::MoveConstructible<T>
			= default;

		/// @brief Constructs this `EmptyBaseClass` from the given arguments
		///
		/// @tparam Args - The types of arguments to pass to `T`'s constructor
		///
		/// @param args - The arguments to pass to `T`'s constructor
		///
		/// # Requirements
		/// - `concepts::ConstructibleFrom<T, Args...>`: `T` must be constructible from the
		/// `Args...` to construct this `EmptyBaseClass` from them
		template<typename... Args>
		requires concepts::ConstructibleFrom<T, Args...>
		explicit constexpr EmptyBaseClass(Args&&... args)
			noexcept(concepts::NoexceptConstructibleFrom<T, Args...>)
			: m_value(std::forward<Args>(args)...) {
		}
		// clang-format on

		constexpr ~EmptyBaseClass() noexcept(concepts::NoexceptDestructible<T>) = default;

		/// @brief Returns a reference to const to the `T` wrapped by this
		///
		/// @return reference to const `T`
		inline constexpr auto get() const noexcept -> const_reference {
			return m_value;
		}

		/// @brief Returns a reference to the `T` wrapped by this
		///
		/// @return reference to `T`
		inline constexpr auto get() noexcept -> reference {
			return m_value;
		}

		// clang-format off
		constexpr auto operator=(const EmptyBaseClass&)
			noexcept(concepts::NoexceptCopyAssignable<T>)
			-> EmptyBaseClass&
			requires concepts::CopyAssignable<T>
			= default;
		constexpr auto operator=(EmptyBaseClass&&)
			noexcept(concepts::NoexceptMoveAssignable<T>)
			-> EmptyBaseClass&
			requires concepts::MoveAssignable<T>
			= default;
		// clang-format on

	  private:
		constexpr EmptyBaseClass() noexcept(concepts::NoexceptDefaultConstructible<T>)
			: m_value(type()) {
		}
		type m_value;
	};
} // namespace hyperion
