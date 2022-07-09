/// @file HyperionDef.h
/// @author Braxton Salyer <braxtonsalyer@gmail.com>
/// @brief Provides various macro definitions for things like compiler-specific attributes,
/// feature enablement, and warning suppression
/// @version 0.1
/// @date 2022-06-07
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

/// @ingroup utils
/// @{
///	@defgroup defines Macro Definitions
/// This module provides a collection of macro definitions for things like compiler-specific
/// attributes, feature inclusion, and warning suppression.
/// @headerfile "Hyperion/HyperionDef.h"
/// @}
#pragma once

#include <Hyperion/Ignore.h>
#include <Hyperion/Platform.h>
#include <version>

#define DOCTEST_CONFIG_NO_SHORT_MACRO_NAMES
#define DOCTEST_CONFIG_NO_COMPARISON_WARNING_SUPPRESSION
#define DOCTEST_CONFIG_USE_STD_HEADERS
#define DOCTEST_CONFIG_SUPER_FAST_ASSERTS
#include <doctest/doctest.h>

/// @def HYPERION_NO_UNIQUE_ADDRESS
/// @brief Platform-dependent, conditional [[no_unique_address]] to account for:
/// 	1. MSVC dragging their feet on providing an actual implementation
/// 	2. Clang not supporting it on Windows because of MSVC's lack of support
/// @ingroup defines
/// @headerfile "Hyperion/HyperionDef.h"
#if HYPERION_PLATFORM_COMPILER_MSVC
	#define HYPERION_NO_UNIQUE_ADDRESS msvc::no_unique_address
#elif HYPERION_PLATFORM_WINDOWS && HYPERION_PLATFORM_COMPILER_CLANG
	#define HYPERION_NO_UNIQUE_ADDRESS
#else
	#define HYPERION_NO_UNIQUE_ADDRESS no_unique_address
#endif

/// @def HYPERION_HAS_EXPERIMENTAL_SOURCE_LOCATION
/// @brief Whether Hyperion will use `std::experimental::source_location` if `<source_location>`
/// is not yet available for the given compiler. This defaults to true when using libstdc++
/// (gcc's std lib). To disable it, define it to false prior to including any Hyperion headers
/// @ingroup defines
/// @headerfile "Hyperion/HyperionDef.h"
#ifndef HYPERION_HAS_EXPERIMENTAL_SOURCE_LOCATION
	#if __has_include(<experimental/source_location>)
		#define HYPERION_HAS_EXPERIMENTAL_SOURCE_LOCATION true
	#else // !__has_include(<experimental/source_location>)
		#define HYPERION_HAS_EXPERIMENTAL_SOURCE_LOCATION false
	#endif // __has_include(<experimental/source_location>)
#endif

/// @def HYPERION_HAS_SOURCE_LOCATION
/// @brief if `<source_location>` is available, this will be defined as true, otherwise it will be
/// false. If this is false, `std::source_location` may be aliased as
/// `std::experimental::source_location` if `HYPERION_HAS_EXPERIMENTAL_SOURCE_LOCATION` is true,
/// otherwise, features relying on `std::source_location` will revert to macros using `__FILE__` and
/// `__LINE__` instead
/// @ingroup defines
/// @headerfile "Hyperion/HyperionDef.h"
#if __has_include(<source_location>)
	#if HYPERION_PLATFORM_COMPILER_GCC \
		|| (HYPERION_PLATFORM_COMPILER_CLANG && !HYPERION_PLATFORM_WINDOWS)
		#if __has_builtin(__builtin_source_location)
			#define HYPERION_HAS_SOURCE_LOCATION true
		#elif HYPERION_PLATFORM_STD_LIB_LIBCPP
			#define HYPERION_HAS_SOURCE_LOCATION true
		#else
			#define HYPERION_HAS_SOURCE_LOCATION false
		#endif
	#else // MSVC
		#define HYPERION_HAS_SOURCE_LOCATION true
	#endif
#else
	#define HYPERION_HAS_SOURCE_LOCATION false
#endif // (defined(__cpp_lib_source_location) && __cpp_lib_source_location >= 201907)

/// @def HYPERION_HAS_JTHREAD
/// @brief if `std::jthread` is available, this will be defined as true, otherwise it will be false.
/// @ingroup defines
/// @headerfile "Hyperion/HyperionDef.h"
#if(defined(__cpp_lib_jthread) && __cpp_lib_jthread >= 201911L)
	#define HYPERION_HAS_JTHREAD true
#else
	#define HYPERION_HAS_JTHREAD false
#endif // (defined(__cpp_lib_jthread) && __cpp_lib_jthread >= 201911L)

/// @def HYPERION_TRIVIAL_ABI
/// @brief Use to apply clang's `trivial_abi` attribute to the following class/struct declaration
/// when compiling with clang. On other compilers this macro is empty
/// @ingroup defines
/// @headerfile "Hyperion/HyperionDef.h"
#if HYPERION_PLATFORM_COMPILER_CLANG
	#define HYPERION_TRIVIAL_ABI [[clang::trivial_abi]]
#else
	#define HYPERION_TRIVIAL_ABI
#endif

/// @def HYPERION_NO_DESTROY
/// @brief Use to apply clang's `no_destroy` attribute to the following variable declaration
/// when compiling with clang. On other compilers this macro is empty
/// @ingroup defines
/// @headerfile "Hyperion/HyperionDef.h"
#if HYPERION_PLATFORM_COMPILER_CLANG
	#define HYPERION_NO_DESTROY [[clang::no_destroy]] // NOLINT
#else
	#define HYPERION_NO_DESTROY
#endif

/// @def HYPERION_CONSTEXPR_STRINGS
/// @brief Conditionally marks a function or variable constexpr if `constexpr std::string` has been
/// implemented for the compiler/standard library implementation being compiled with
/// @ingroup defines
/// @headerfile "Hyperion/HyperionDef.h"
#ifdef __cpp_lib_constexpr_string
	#if(__cpp_lib_constexpr_string >= 201907L)
		#define HYPERION_CONSTEXPR_STRINGS constexpr
	#else
		#define HYPERION_CONSTEXPR_STRINGS
	#endif
#else
	#define HYPERION_CONSTEXPR_STRINGS
#endif

/// @def HYPERION_UNREACHABLE()
/// @brief Marks the containing control flow branch as unreachable. On GCC/Clang, this will use
/// `__builtin_unreachable()`, on MSVC `__assume(false)`
/// @ingroup defines
/// @headerfile "Hyperion/HyperionDef.h"
#if HYPERION_PLATFORM_COMPILER_CLANG || HYPERION_PLATFORM_COMPILER_GCC
	#define HYPERION_UNREACHABLE() __builtin_unreachable()
#elif HYPERION_PLATFORM_COMPILER_MSVC
	#define HYPERION_UNREACHABLE() __assume(false);
#else
	#define HYPERION_UNREACHABLE()
#endif

/// @def HYPERION_HAS_TYPE_PACK_ELEMENT
/// @brief Indicates whether the compiler builtin `__type_pack_element` is available
/// (used for efficient type parameter pack indexing in `mpl`)
/// @ingroup defines
/// @headerfile "Hyperion/HyperionDef.h"
#if defined(__has_builtin)
	#if __has_builtin(__type_pack_element)
		#define HYPERION_HAS_TYPE_PACK_ELEMENT true
	#else
		#define HYPERION_HAS_TYPE_PACK_ELEMENT false
	#endif
#else
	#define HYPERION_HAS_TYPE_PACK_ELEMENT false
#endif
// clang-format off

/// @def IGNORE_UNUSED_MACROS_START
/// @brief Use to temporarily disable warnings for unused macros.
/// Make sure to pair with `IGNORE_UNUSED_MACROS_STOP` to properly scope the area where the warning
/// is ignored
/// @ingroup defines
/// @headerfile "Hyperion/HyperionDef.h"
#if !HYPERION_PLATFORM_COMPILER_MSVC
	// NOLINTNEXTLINE
	#define IGNORE_UNUSED_MACROS_START \
		_Pragma("GCC diagnostic push")\
		_Pragma("GCC diagnostic ignored \"-Wunused-macros\"")
#else
	// NOLINTNEXTLINE
	#define IGNORE_UNUSED_MACROS_START
#endif

/// @def IGNORE_UNUSED_MACROS_STOP
/// @brief Use to re-enable warnings for unused macros after having previously used
/// `IGNORE_UNUSED_MACROS_START`
/// @ingroup defines
/// @headerfile "Hyperion/HyperionDef.h"
#if !HYPERION_PLATFORM_COMPILER_MSVC
	// NOLINTNEXTLINE
	#define IGNORE_UNUSED_MACROS_STOP \
		_Pragma("GCC diagnostic pop")
#else
	// NOLINTNEXTLINE
	#define IGNORE_UNUSED_MACROS_STOP
#endif

IGNORE_UNUSED_MACROS_START

/// @def IGNORE_SUGGEST_DESTRUCTOR_OVERRIDE_START
/// @brief Use to temporarily disable warnings for destructors that override but are not marked
/// `override` (-Wsuggest-destructor-override).
/// Make sure to pair with `IGNORE_SUGGEST_DESTRUCTOR_OVERRIDE_STOP` to properly scope the area
/// where the warning is ignored
/// @ingroup defines
/// @headerfile "Hyperion/HyperionDef.h"
#if HYPERION_PLATFORM_COMPILER_CLANG
// NOLINTNEXTLINE
#define IGNORE_SUGGEST_DESTRUCTOR_OVERRIDE_START \
	_Pragma("GCC diagnostic push") \
	_Pragma("GCC diagnostic ignored \"-Wsuggest-destructor-override\"")
#else
// NOLINTNEXTLINE
#define IGNORE_SUGGEST_DESTRUCTOR_OVERRIDE_START
#endif

/// @def IGNORE_SUGGEST_DESTRUCTOR_OVERRIDE_STOP
/// @brief Use to re-enable warnings for destructors that override but are not marked `override`
/// @ingroup defines
/// @headerfile "Hyperion/HyperionDef.h"
#if HYPERION_PLATFORM_COMPILER_CLANG
// NOLINTNEXTLINE
#define IGNORE_SUGGEST_DESTRUCTOR_OVERRIDE_STOP \
		_Pragma("GCC diagnostic pop")
#else
// NOLINTNEXTLINE
	#define IGNORE_SUGGEST_DESTRUCTOR_OVERRIDE_STOP
#endif

/// @def IGNORE_SWITCH_MISSING_ENUM_VALUES_START
/// @brief Use to temporarily disable warnings for switches that don't cover all enum values of the
/// enum being "switched" on (-Wswitch-enum).
/// Make sure to pair with `IGNORE_SWITCH_MISSING_ENUM_VALUES_STOP` to properly scope the area
/// where the warning is ignored
/// @ingroup defines
/// @headerfile "Hyperion/HyperionDef.h"
#if HYPERION_PLATFORM_COMPILER_CLANG
// NOLINTNEXTLINE
#define IGNORE_SWITCH_MISSING_ENUM_VALUES_START \
	_Pragma("GCC diagnostic push") \
	_Pragma("GCC diagnostic ignored \"-Wswitch-enum\"")
#else
// NOLINTNEXTLINE
#define IGNORE_SWITCH_MISSING_ENUM_VALUES_START
#endif

/// @def IGNORE_SWITCH_MISSING_ENUM_VALUES_STOP
/// @brief Use to re-enable warnings for switches that don't cover all enum values of the enum being
/// "switched" on (-Wswitch-enum)
/// @ingroup defines
/// @headerfile "Hyperion/HyperionDef.h"
#if HYPERION_PLATFORM_COMPILER_CLANG
// NOLINTNEXTLINE
#define IGNORE_SWITCH_MISSING_ENUM_VALUES_STOP \
		_Pragma("GCC diagnostic pop")
#else
// NOLINTNEXTLINE
#define IGNORE_SWITCH_MISSING_ENUM_VALUES_STOP
#endif

/// @def IGNORE_OLD_STYLE_CASTS_START
/// @brief Use to temporarily disable warnings for c-style casts (-Wold-style-cast).
/// Make sure to pair with `IGNORE_OLD_STYLE_CASTS_START` to properly scope the area where
/// the warning is ignored
/// @ingroup defines
/// @headerfile "Hyperion/HyperionDef.h"
#if HYPERION_PLATFORM_COMPILER_CLANG
// NOLINTNEXTLINE
#define IGNORE_OLD_STYLE_CASTS_START \
	_Pragma("GCC diagnostic push") \
	_Pragma("GCC diagnostic ignored \"-Wold-style-cast\"")
#else
// NOLINTNEXTLINE
#define IGNORE_OLD_STYLE_CASTS_START
#endif

/// @def IGNORE_OLD_STYLE_CASTS_STOP
/// @brief Use to re-enable warnings for c-style casts
/// @ingroup defines
/// @headerfile "Hyperion/HyperionDef.h"
#if HYPERION_PLATFORM_COMPILER_CLANG
// NOLINTNEXTLINE
#define IGNORE_OLD_STYLE_CASTS_STOP \
		_Pragma("GCC diagnostic pop")
#else
// NOLINTNEXTLINE
	#define IGNORE_OLD_STYLE_CASTS_STOP
#endif

/// @def IGNORE_INVALID_NORETURN_START
/// @brief Use to temporarily disable warnings for functions marked `[[noreturn]]` that may return
/// (-Winvalid-noreturn).
/// Make sure to pair with `IGNORE_INVALID_NORETURN_STOP` to properly scope the area where
/// the warning is ignored
/// @ingroup defines
/// @headerfile "Hyperion/HyperionDef.h"
#if HYPERION_PLATFORM_COMPILER_CLANG
	// NOLINTNEXTLINE
#define IGNORE_INVALID_NORETURN_START \
	_Pragma("GCC diagnostic push") \
	_Pragma("GCC diagnostic ignored \"-Winvalid-noreturn\"")
#else
// NOLINTNEXTLINE
#define IGNORE_INVALID_NORETURN_START
#endif

/// @def IGNORE_INVALID_NORETURN_STOP
/// @brief Use to re-enable warnings for for functions marked `[[noreturn]]` that may return
/// `IGNORE_INVALID_NORETURN_START`
/// @ingroup defines
/// @headerfile "Hyperion/HyperionDef.h"
#if HYPERION_PLATFORM_COMPILER_CLANG
// NOLINTNEXTLINE
#define IGNORE_INVALID_NORETURN_STOP \
		_Pragma("GCC diagnostic pop")
#else
// NOLINTNEXTLINE
	#define IGNORE_INVALID_NORETURN_STOP
#endif

/// @def IGNORE_DEPRECATED_DECLARATIONS_START
/// @brief Use to temporarily disable warnings for using deprecated declarations
/// (-Wdeprecated-declarations).
/// Make sure to pair with `IGNORE_DEPRECATED_DECLARATIONS_STOP` to properly scope the area where
/// the warning is ignored
/// @ingroup defines
/// @headerfile "Hyperion/HyperionDef.h"
#if HYPERION_PLATFORM_COMPILER_CLANG
	// NOLINTNEXTLINE
	#define IGNORE_DEPRECATED_DECLARATIONS_START \
		_Pragma("GCC diagnostic push") \
		_Pragma("GCC diagnostic ignored \"-Wdeprecated-declarations\"")
#else
	// NOLINTNEXTLINE
	#define IGNORE_DEPRECATED_DECLARATIONS_START
#endif

/// @def IGNORE_DEPRECATED_DECLARATIONS_STOP
/// @brief Use to re-enable warnings for using deprecated declarations after having previously used
/// `IGNORE_DEPRECATED_DECLARATIONS_START`
/// @ingroup defines
/// @headerfile "Hyperion/HyperionDef.h"
#if HYPERION_PLATFORM_COMPILER_CLANG
	// NOLINTNEXTLINE
	#define IGNORE_DEPRECATED_DECLARATIONS_STOP \
		_Pragma("GCC diagnostic pop")
#else
	// NOLINTNEXTLINE
	#define IGNORE_DEPRECATED_DECLARATIONS_STOP
#endif

/// @def IGNORE_UNINITIALIZED_VARIABLES_START
/// @brief Use to temporarily disable warnings for using uninitialized variables
/// (-Wuninitialized).
/// Make sure to pair with `IGNORE_UNINITIALIZED_VARIABLES_STOP` to properly scope the area where
/// the warning is ignored
/// @ingroup defines
/// @headerfile "Hyperion/HyperionDef.h"
#if HYPERION_PLATFORM_COMPILER_CLANG
	// NOLINTNEXTLINE
	#define IGNORE_UNINITIALIZED_VARIABLES_START \
		_Pragma("GCC diagnostic push") \
		_Pragma("GCC diagnostic ignored \"-Wuninitialized\"")
#else
	// NOLINTNEXTLINE
	#define IGNORE_UNINITIALIZED_VARIABLES_START
#endif

/// @def IGNORE_UNINITIALIZED_VARIABLES_STOP
/// @brief Use to re-enable warnings for using uninitialized variables after having previously used
/// `IGNORE_UNINITIALIZED_VARIABLES_START`
/// @ingroup defines
/// @headerfile "Hyperion/HyperionDef.h"
#if HYPERION_PLATFORM_COMPILER_CLANG
	// NOLINTNEXTLINE
	#define IGNORE_UNINITIALIZED_VARIABLES_STOP \
		_Pragma("GCC diagnostic pop")
#else
	// NOLINTNEXTLINE
	#define IGNORE_UNINITIALIZED_VARIABLES_STOP
#endif

/// @def IGNORE_UNNEEDED_INTERNAL_DECL_START
/// @brief Use to temporarily disable warnings for unneeded internal declarations
/// (-Wunneeded-internal-declaration).
/// Make sure to pair with `IGNORE_UNNEEDED_INTERNAL_DECL_STOP` to properly scope the area where
/// the warning is ignored
/// @ingroup defines
/// @headerfile "Hyperion/HyperionDef.h"
#if HYPERION_PLATFORM_COMPILER_CLANG
	// NOLINTNEXTLINE
	#define IGNORE_UNNEEDED_INTERNAL_DECL_START \
		_Pragma("GCC diagnostic push") \
		_Pragma("GCC diagnostic ignored \"-Wunneeded-internal-declaration\"")
#else
	// NOLINTNEXTLINE
	#define IGNORE_UNNEEDED_INTERNAL_DECL_START
#endif

/// @def IGNORE_UNNEEDED_INTERNAL_DECL_STOP
/// @brief Use to re-enable warnings for unneeded internal declarations after having previously used
/// `IGNORE_UNNEEDED_INTERNAL_DECL_START`
/// @ingroup defines
/// @headerfile "Hyperion/HyperionDef.h"
#if HYPERION_PLATFORM_COMPILER_CLANG
	// NOLINTNEXTLINE
	#define IGNORE_UNNEEDED_INTERNAL_DECL_STOP \
		_Pragma("GCC diagnostic pop")
#else
	// NOLINTNEXTLINE
	#define IGNORE_UNNEEDED_INTERNAL_DECL_STOP
#endif

/// @def IGNORE_CONSTANT_CONDITIONAL_EXPRESSIONS_START
/// @brief Use to temporarily disable warnings for constant conditional expressions in non-constexpr
/// if (MSVC 4127).
/// Make sure to pair with `IGNORE_CONSTANT_CONDITIONAL_EXPRESSIONS_STOP` to properly scope the
/// area where the warning is ignored
/// @ingroup defines
/// @headerfile "Hyperion/HyperionDef.h"
#if !HYPERION_PLATFORM_COMPILER_MSVC
	// NOLINTNEXTLINE
	#define IGNORE_CONSTANT_CONDITIONAL_EXPRESSIONS_START
#else
	// NOLINTNEXTLINE
	#define IGNORE_CONSTANT_CONDITIONAL_EXPRESSIONS_START \
		_Pragma("warning( push )") \
		_Pragma("warning( disable : 4127 )")
#endif

/// @def IGNORE_CONSTANT_CONDITIONAL_EXPRESSIONS_STOP
/// @brief Use to re-enable warnings for constant conditional expressions after having previously
/// used `IGNORE_CONSTANT_CONDITIONAL_EXPRESSIONS_STOP`
/// @ingroup defines
/// @headerfile "Hyperion/HyperionDef.h"
#if !HYPERION_PLATFORM_COMPILER_MSVC
	// NOLINTNEXTLINE
	#define IGNORE_CONSTANT_CONDITIONAL_EXPRESSIONS_STOP
#else
	// NOLINTNEXTLINE
	#define IGNORE_CONSTANT_CONDITIONAL_EXPRESSIONS_STOP \
		_Pragma("warning( pop )")
#endif


/// @def IGNORE_MARKED_NOEXCEPT_BUT_THROWS_START
/// @brief Use to temporarily disable warnings for functions marked noexcept that intentionally
/// throw (MSVC 4297).
/// Make sure to pair with `IGNORE_MARKED_NOEXCEPT_BUT_THROWS_STOP` to properly scope the
/// area where the warning is ignored
/// @note Using this __**ONLY**__ makes sense if it's used to suppress __warnings__ when throwing is
/// intentionally used as a way to force compiler errors in constexpr functions
/// @ingroup defines
/// @headerfile "Hyperion/HyperionDef.h"
#if !HYPERION_PLATFORM_COMPILER_MSVC
	// NOLINTNEXTLINE
	#define IGNORE_MARKED_NOEXCEPT_BUT_THROWS_START
#else
	// NOLINTNEXTLINE
	#define IGNORE_MARKED_NOEXCEPT_BUT_THROWS_START \
		_Pragma("warning( push )") \
		_Pragma("warning( disable : 4297 )")
#endif

/// @def IGNORE_MARKED_NOEXCEPT_BUT_THROWS_STOP
/// @brief Use to re-enable warnings for functions marked noexcept that intentionally throw after
/// having previously used `IGNORE_MARKED_NOEXCEPT_BUT_THROWS_START`
/// @ingroup defines
/// @headerfile "Hyperion/HyperionDef.h"
#if !HYPERION_PLATFORM_COMPILER_MSVC
	// NOLINTNEXTLINE
	#define IGNORE_MARKED_NOEXCEPT_BUT_THROWS_STOP
#else
	// NOLINTNEXTLINE
	#define IGNORE_MARKED_NOEXCEPT_BUT_THROWS_STOP \
		_Pragma("warning( pop )")
#endif

/// @def IGNORE_UNUSED_VARIABLES_START
/// @brief Use to temporarily disable warnings for unused variables
/// Make sure to pair with `IGNORE_UNUSED_VARIABLES_STOP` to properly scope the
/// area where the warning is ignored
/// @ingroup defines
/// @headerfile "Hyperion/HyperionDef.h"
#if !HYPERION_PLATFORM_COMPILER_MSVC
	// NOLINTNEXTLINE
	#define IGNORE_UNUSED_VARIABLES_START \
		_Pragma("GCC diagnostic push") \
		_Pragma("GCC diagnostic ignored \"-Wunused-variable\"")
#else
	// NOLINTNEXTLINE
	#define IGNORE_UNUSED_VARIABLES_START
#endif

/// @def IGNORE_UNUSED_VARIABLES_STOP
/// @brief Use to re-enable warnings for unused variables after having previously used
/// `IGNORE_UNUSED_VARIABLES_START`
/// @ingroup defines
/// @headerfile "Hyperion/HyperionDef.h"
#if !HYPERION_PLATFORM_COMPILER_MSVC
	// NOLINTNEXTLINE
	#define IGNORE_UNUSED_VARIABLES_STOP \
		_Pragma("GCC diagnostic pop")
#else
	// NOLINTNEXTLINE
	#define IGNORE_UNUSED_VARIABLES_STOP
#endif

/// @def IGNORE_MISSING_NORETURN_START
/// @brief Use to temporarily disable warnings for noreturn functions missing a noreturn attribute
/// Make sure to pair with `IGNORE_MISSING_NORETURN_STOP` to properly scope the
/// area where the warning is ignored
/// @ingroup defines
/// @headerfile "Hyperion/HyperionDef.h"
#if !HYPERION_PLATFORM_COMPILER_MSVC
	// NOLINTNEXTLINE
	#define IGNORE_MISSING_NORETURN_START \
		_Pragma("GCC diagnostic push") \
		_Pragma("GCC diagnostic ignored \"-Wmissing-noreturn\"")
#else
	// NOLINTNEXTLINE
	#define IGNORE_MISSING_NORETURN_START
#endif

/// @def IGNORE_MISSING_NORETURN_STOP
/// @brief Use to re-enable warnings for noreturn functions missing a noreturn attribute after
/// having previously used `IGNORE_MISSING_NORETURN_START`
/// @ingroup defines
/// @headerfile "Hyperion/HyperionDef.h"
#if !HYPERION_PLATFORM_COMPILER_MSVC
	// NOLINTNEXTLINE
	#define IGNORE_MISSING_NORETURN_STOP \
		_Pragma("GCC diagnostic pop")
#else
	// NOLINTNEXTLINE
	#define IGNORE_MISSING_NORETURN_STOP
#endif

/// @def IGNORE_RESERVED_IDENTIFIERS_START
/// @brief Use to temporarily disable warnings using reserved identifiers.
/// Make sure to pair with `IGNORE_RESERVED_IDENTIFIERS_STOP` to properly scope the
/// area where the warning is ignored
/// @ingroup defines
/// @headerfile "Hyperion/HyperionDef.h"
#if HYPERION_PLATFORM_COMPILER_CLANG
	// NOLINTNEXTLINE
	#define IGNORE_RESERVED_IDENTIFIERS_START \
		_Pragma("GCC diagnostic push") \
		_Pragma("GCC diagnostic ignored \"-Wreserved-identifier\"")
#elif HYPERION_PLATFORM_COMPILER_MSVC
	// NOLINTNEXTLINE
	#define IGNORE_RESERVED_IDENTIFIERS_START \
		_Pragma("warning( push )") \
		_Pragma("warning( disable : 4405 )")
#else
	// NOLINTNEXTLINE
	#define IGNORE_RESERVED_IDENTIFIERS_START
#endif

/// @def IGNORE_RESERVED_IDENTIFIERS_STOP
/// @brief Use to re-enable warnings for using reserved identifiers after having previously used
/// `IGNORE_RESERVED_IDENTIFIERS_START`
/// @ingroup defines
/// @headerfile "Hyperion/HyperionDef.h"
#if HYPERION_PLATFORM_COMPILER_CLANG
	// NOLINTNEXTLINE
	#define IGNORE_RESERVED_IDENTIFIERS_STOP \
		_Pragma("GCC diagnostic pop")
#elif HYPERION_PLATFORM_COMPILER_MSVC
	// NOLINTNEXTLINE
	#define IGNORE_RESERVED_IDENTIFIERS_STOP \
		_Pragma("warning( pop )")
#else
	// NOLINTNEXTLINE
	#define IGNORE_RESERVED_IDENTIFIERS_STOP
#endif

/// @def IGNORE_RESERVED_MACRO_IDENTIFIERS_START
/// @brief Use to temporarily disable warnings using reserved macro identifiers.
/// Make sure to pair with `IGNORE_RESERVED_MACRO_IDENTIFIERS_STOP` to properly scope the
/// area where the warning is ignored
/// @ingroup defines
/// @headerfile "Hyperion/HyperionDef.h"
#if HYPERION_PLATFORM_COMPILER_CLANG
	// NOLINTNEXTLINE
	#define IGNORE_RESERVED_MACRO_IDENTIFIERS_START \
		_Pragma("GCC diagnostic push") \
		_Pragma("GCC diagnostic ignored \"-Wreserved-macro-identifier\"")
#else
	// NOLINTNEXTLINE
	#define IGNORE_RESERVED_MACRO_IDENTIFIERS_START
#endif

/// @def IGNORE_RESERVED_MACRO_IDENTIFIERS_STOP
/// @brief Use to re-enable warnings for using reserved macro identifiers after having previously
/// used `IGNORE_RESERVED_MACRO_IDENTIFIERS_START`
/// @ingroup defines
/// @headerfile "Hyperion/HyperionDef.h"
#if HYPERION_PLATFORM_COMPILER_CLANG
	// NOLINTNEXTLINE
	#define IGNORE_RESERVED_MACRO_IDENTIFIERS_STOP \
		_Pragma("GCC diagnostic pop")
#else
	// NOLINTNEXTLINE
	#define IGNORE_RESERVED_MACRO_IDENTIFIERS_STOP
#endif

/// @def IGNORE_PADDING_START
/// @brief Use to temporarily disable warnings for class/struct definitions requiring padding.
/// Make sure to pair with `IGNORE_PADDING_STOP` to properly scope the
/// area where the warning is ignored
/// @ingroup defines
/// @headerfile "Hyperion/HyperionDef.h"
#if !HYPERION_PLATFORM_COMPILER_MSVC
	// NOLINTNEXTLINE
	#define IGNORE_PADDING_START \
		_Pragma("GCC diagnostic push") \
		_Pragma("GCC diagnostic ignored \"-Wpadded\"")
#else
	// NOLINTNEXTLINE
	#define IGNORE_PADDING_START \
		_Pragma("warning( push )") \
		_Pragma("warning( disable : 4820 )")
#endif

/// @def IGNORE_PADDING_STOP
/// @brief Use to re-enable warnings for class/struct definitions requiring padding after having
/// previously used `IGNORE_PADDING_START`
/// @ingroup defines
/// @headerfile "Hyperion/HyperionDef.h"
#if !HYPERION_PLATFORM_COMPILER_MSVC
	// NOLINTNEXTLINE
	#define IGNORE_PADDING_STOP \
		_Pragma("GCC diagnostic pop")
#else
	// NOLINTNEXTLINE
	#define IGNORE_PADDING_STOP \
		_Pragma("warning( pop )")
#endif

/// @def IGNORE_WEAK_VTABLES_START
/// @brief Use to temporarily disable warnings for virtual classes with weak vtables.
/// Make sure to pair with `IGNORE_WEAK_VTABLES_STOP` to properly scope the
/// area where the warning is ignored
/// @ingroup defines
/// @headerfile "Hyperion/HyperionDef.h"
#if HYPERION_PLATFORM_COMPILER_CLANG
		// NOLINTNEXTLINE
		#define IGNORE_WEAK_VTABLES_START \
			_Pragma("GCC diagnostic push") \
			_Pragma("GCC diagnostic ignored \"-Wweak-vtables\"")
#else
	// NOLINTNEXTLINE
	#define IGNORE_WEAK_VTABLES_START
#endif

/// @def IGNORE_WEAK_VTABLES_STOP
/// @brief Use to re-enable warnings for virtual classes with weak vtables after having
/// previously used `IGNORE_WEAK_VTABLES_START`
/// @ingroup defines
/// @headerfile "Hyperion/HyperionDef.h"
#if HYPERION_PLATFORM_COMPILER_CLANG
		// NOLINTNEXTLINE
		#define IGNORE_WEAK_VTABLES_STOP \
			_Pragma("GCC diagnostic pop")
#else
	// NOLINTNEXTLINE
	#define IGNORE_WEAK_VTABLES_STOP
#endif

/// @def IGNORE_UNUSED_TEMPLATES_START
/// @brief Use to temporarily disable warnings for unused function templates
/// Make sure to pair with `IGNORE_UNUSED_TEMPLATES_STOP` to properly scope the
/// area where the warning is ignored
/// @ingroup defines
/// @headerfile "Hyperion/HyperionDef.h"
#if HYPERION_PLATFORM_COMPILER_CLANG
		// NOLINTNEXTLINE
		#define IGNORE_UNUSED_TEMPLATES_START \
			_Pragma("GCC diagnostic push")\
			_Pragma("GCC diagnostic ignored \"-Wunused-template\"")
#else
	// NOLINTNEXTLINE
	#define IGNORE_UNUSED_TEMPLATES_START
#endif

/// @def IGNORE_UNUSED_TEMPLATES_STOP
/// @brief Use to re-enable warnings for unused function templates after having
/// previously used `IGNORE_UNUSED_TEMPLATES_START`
/// @ingroup defines
/// @headerfile "Hyperion/HyperionDef.h"
#if HYPERION_PLATFORM_COMPILER_CLANG
		// NOLINTNEXTLINE
		#define IGNORE_UNUSED_TEMPLATES_STOP \
			_Pragma("GCC diagnostic pop")
#else
	// NOLINTNEXTLINE
	#define IGNORE_UNUSED_TEMPLATES_STOP
#endif

/// @def IGNORE_UNUSED_FUNCTIONS_START
/// @brief Use to temporarily disable warnings for unused functions
/// Make sure to pair with `IGNORE_UNUSED_FUNCTIONS_STOP` to properly scope the
/// area where the warning is ignored
/// @ingroup defines
/// @headerfile "Hyperion/HyperionDef.h"
#if HYPERION_PLATFORM_COMPILER_CLANG
		// NOLINTNEXTLINE
		#define IGNORE_UNUSED_FUNCTIONS_START \
			_Pragma("GCC diagnostic push")\
			_Pragma("GCC diagnostic ignored \"-Wunused-function\"")
#else
	// NOLINTNEXTLINE
	#define IGNORE_UNUSED_FUNCTIONS_START
#endif

/// @def IGNORE_UNUSED_FUNCTIONS_STOP
/// @brief Use to re-enable warnings for unused functions after having
/// previously used `IGNORE_UNUSED_FUNCTIONS_START`
/// @ingroup defines
/// @headerfile "Hyperion/HyperionDef.h"
#if HYPERION_PLATFORM_COMPILER_CLANG
		// NOLINTNEXTLINE
		#define IGNORE_UNUSED_FUNCTIONS_STOP \
			_Pragma("GCC diagnostic pop")
#else
	// NOLINTNEXTLINE
	#define IGNORE_UNUSED_FUNCTIONS_STOP
#endif

/// @def IGNORE_UNUSED_MEMBER_FUNCTIONS_START
/// @brief Use to temporarily disable warnings for unused member functions
/// Make sure to pair with `IGNORE_UNUSED_MEMBER_FUNCTIONS_STOP` to properly scope the
/// area where the warning is ignored
/// @ingroup defines
/// @headerfile "Hyperion/HyperionDef.h"
#if HYPERION_PLATFORM_COMPILER_CLANG
		// NOLINTNEXTLINE
		#define IGNORE_UNUSED_MEMBER_FUNCTIONS_START \
			_Pragma("GCC diagnostic push")\
			_Pragma("GCC diagnostic ignored \"-Wunused-member-function\"")
#else
	// NOLINTNEXTLINE
	#define IGNORE_UNUSED_MEMBER_FUNCTIONS_START
#endif

/// @def IGNORE_UNUSED_MEMBER_FUNCTIONS_STOP
/// @brief Use to re-enable warnings for unused member functions after having
/// previously used `IGNORE_UNUSED_MEMBER_FUNCTIONS_START`
/// @ingroup defines
/// @headerfile "Hyperion/HyperionDef.h"
#if HYPERION_PLATFORM_COMPILER_CLANG
		// NOLINTNEXTLINE
		#define IGNORE_UNUSED_MEMBER_FUNCTIONS_STOP \
			_Pragma("GCC diagnostic pop")
#else
	// NOLINTNEXTLINE
	#define IGNORE_UNUSED_MEMBER_FUNCTIONS_STOP
#endif

/// @def IGNORE_SIGNED_BITFIELD_START
/// @brief Use to temporarily disable warnings for enums with signed underlying type used as
/// bitfields
/// Make sure to pair with `IGNORE_SIGNED_BITFIELD_STOP` to properly scope the
/// area where the warning is ignored
/// @ingroup defines
/// @headerfile "Hyperion/HyperionDef.h"
#if HYPERION_PLATFORM_COMPILER_CLANG
		// NOLINTNEXTLINE
		#define IGNORE_SIGNED_BITFIELD_START \
			_Pragma("GCC diagnostic push") \
			_Pragma("GCC diagnostic ignored \"-Wsigned-enum-bitfield\"")
#else
	// NOLINTNEXTLINE
	#define IGNORE_SIGNED_BITFIELD_START
#endif

/// @def IGNORE_SIGNED_BITFIELD_STOP
/// @brief Use to re-enable warnings for enums with signed underlying type used as bitfields after
/// having previously used `IGNORE_SIGNED_BITFIELD_START`
/// @ingroup defines
/// @headerfile "Hyperion/HyperionDef.h"
#if HYPERION_PLATFORM_COMPILER_CLANG
		// NOLINTNEXTLINE
		#define IGNORE_SIGNED_BITFIELD_STOP \
			_Pragma("GCC diagnostic pop")
#else
	// NOLINTNEXTLINE
	#define IGNORE_SIGNED_BITFIELD_STOP
#endif

/// @def IGNORE_UNKNOWN_DOC_COMMAND_START
/// @brief Use to temporarily disable warnings unknown documentation commands
/// Make sure to pair with `IGNORE_UNKNOWN_DOC_COMMAND_STOP` to properly scope the
/// area where the warning is ignored
/// @ingroup defines
/// @headerfile "Hyperion/HyperionDef.h"
#if HYPERION_PLATFORM_COMPILER_CLANG
		// NOLINTNEXTLINE
		#define IGNORE_UNKNOWN_DOC_COMMAND_START \
			_Pragma("GCC diagnostic push") \
			_Pragma("GCC diagnostic ignored \"-Wdocumentation-unknown-command\"")
#else
	// NOLINTNEXTLINE
	#define IGNORE_UNKNOWN_DOC_COMMAND_START
#endif

/// @def IGNORE_UNKNOWN_DOC_COMMAND_STOP
/// @brief Use to re-enable warnings for unknown documentation commands after having previously used
/// `IGNORE_UNKNOWN_DOC_COMMAND_START`
/// @ingroup defines
/// @headerfile "Hyperion/HyperionDef.h"
#if HYPERION_PLATFORM_COMPILER_CLANG
		// NOLINTNEXTLINE
		#define IGNORE_UNKNOWN_DOC_COMMAND_STOP \
			_Pragma("GCC diagnostic pop")
#else
	// NOLINTNEXTLINE
	#define IGNORE_UNKNOWN_DOC_COMMAND_STOP
#endif

/// @def IGNORE_CONSTRUCTOR_SHADOW_FIELDS_START
/// @brief Use to temporarily disable warnings for constructor parameters that shadow class/struct
/// members
/// Make sure to pair with `IGNORE_CONSTRUCTOR_SHADOW_FIELDS_STOP` to properly scope the
/// area where the warning is ignored
/// @ingroup defines
/// @headerfile "Hyperion/HyperionDef.h"
#if HYPERION_PLATFORM_COMPILER_CLANG
		// NOLINTNEXTLINE
		#define IGNORE_CONSTRUCTOR_SHADOW_FIELDS_START \
			_Pragma("GCC diagnostic push") \
			_Pragma("GCC diagnostic ignored \"-Wshadow-field-in-constructor\"")
#else
	// NOLINTNEXTLINE
	#define IGNORE_CONSTRUCTOR_SHADOW_FIELDS_START
#endif

/// @def IGNORE_CONSTRUCTOR_SHADOW_FIELDS_STOP
/// @brief Use to re-enable warnings for constructor parameters that shadow class/struct members
/// after having previously used `IGNORE_CONSTRUCTOR_SHADOW_FIELDS_STOP`
/// @ingroup defines
/// @headerfile "Hyperion/HyperionDef.h"
#if HYPERION_PLATFORM_COMPILER_CLANG
		// NOLINTNEXTLINE
		#define IGNORE_CONSTRUCTOR_SHADOW_FIELDS_STOP \
			_Pragma("GCC diagnostic pop")
#else
	// NOLINTNEXTLINE
	#define IGNORE_CONSTRUCTOR_SHADOW_FIELDS_STOP
#endif

/// @def IGNORE_FLOAT_EQUALITY_START
/// @brief Use to temporarily disable warnings for checking for floating point equality
/// Make sure to pair with `IGNORE_FLOAT_EQUALITY_STOP` to properly scope the
/// area where the warning is ignored
/// @ingroup defines
/// @headerfile "Hyperion/HyperionDef.h"
#if HYPERION_PLATFORM_COMPILER_CLANG
		// NOLINTNEXTLINE
		#define IGNORE_FLOAT_EQUALITY_START \
			_Pragma("GCC diagnostic push") \
			_Pragma("GCC diagnostic ignored \"-Wfloat-equal\"")
#else
	// NOLINTNEXTLINE
	#define IGNORE_FLOAT_EQUALITY_START
#endif

/// @def IGNORE_FLOAT_EQUALITY_STOP
/// @brief Use to re-enable warnings for checking for floating point equality
/// after having previously used `IGNORE_FLOAT_EQUALITY_STOP`
/// @ingroup defines
/// @headerfile "Hyperion/HyperionDef.h"
#if HYPERION_PLATFORM_COMPILER_CLANG
		// NOLINTNEXTLINE
		#define IGNORE_FLOAT_EQUALITY_STOP \
			_Pragma("GCC diagnostic pop")
#else
	// NOLINTNEXTLINE
	#define IGNORE_FLOAT_EQUALITY_STOP
#endif

/// @def IGNORE_COMMA_MISUSE_START
/// @brief Use to temporarily disable warnings for comma operator misuse
/// Make sure to pair with `IGNORE_COMMA_MISUSE_STOP` to properly scope the
/// area where the warning is ignored
/// @ingroup defines
/// @headerfile "Hyperion/HyperionDef.h"
#if HYPERION_PLATFORM_COMPILER_CLANG
		// NOLINTNEXTLINE
		#define IGNORE_COMMA_MISUSE_START \
			_Pragma("GCC diagnostic push") \
			_Pragma("GCC diagnostic ignored \"-Wcomma\"")
#else
	// NOLINTNEXTLINE
	#define IGNORE_COMMA_MISUSE_START
#endif

/// @def IGNORE_COMMA_MISUSE_STOP
/// @brief Use to re-enable warnings for comma operator misuse after having previously used
/// `IGNORE_COMMA_MISUSE_START`
/// @ingroup defines
/// @headerfile "Hyperion/HyperionDef.h"
#if HYPERION_PLATFORM_COMPILER_CLANG
		// NOLINTNEXTLINE
		#define IGNORE_COMMA_MISUSE_STOP \
			_Pragma("GCC diagnostic pop")
#else
	// NOLINTNEXTLINE
	#define IGNORE_COMMA_MISUSE_STOP
#endif

// clang-format on

/// @brief Use to define a doctest test suite.
/// Usage is identical to `DOCTEST_TEST_SUITE()`, this just ignores particular warnings coming from
/// doctest.
/// @ingroup defines
/// @headerfile "Hyperion/HyperionDef.h"
#define TEST_SUITE(...) /** NOLINT(cppcoreguidelines-macro-usage) **/ \
	IGNORE_RESERVED_IDENTIFIERS_START                                 \
	IGNORE_UNUSED_TEMPLATES_START                                     \
	DOCTEST_TEST_SUITE(__VA_ARGS__)                                   \
	/** NOLINT(modernize-use-trailing-return-type) **/                \
	IGNORE_RESERVED_IDENTIFIERS_STOP IGNORE_UNUSED_TEMPLATES_STOP
/// @brief Use to define a doctest test case.
/// Usage is identical to `DOCTEST_TEST_CASE()`, this just ignores particular warnings coming from
/// doctest.
/// @ingroup defines
/// @headerfile "Hyperion/HyperionDef.h"
#define TEST_CASE(...) /** NOLINT(cppcoreguidelines-macro-usage) **/ \
	IGNORE_RESERVED_IDENTIFIERS_START                                \
	IGNORE_UNUSED_TEMPLATES_START                                    \
	/** NOLINTNEXTLINE **/                                           \
	DOCTEST_TEST_CASE(__VA_ARGS__)                                   \
	/** NOLINT **/                                                   \
	IGNORE_RESERVED_IDENTIFIERS_STOP IGNORE_UNUSED_TEMPLATES_STOP
/// @brief Use to define a doctest test subcase.
/// Usage is identical to `DOCTEST_SUBCASE()`, this just ignores particular warnings coming from
/// doctest.
/// @ingroup defines
/// @headerfile "Hyperion/HyperionDef.h"
#define SUBCASE(...) /** NOLINT(cppcoreguidelines-macro-usage) **/ \
	IGNORE_RESERVED_IDENTIFIERS_START                              \
	IGNORE_UNUSED_TEMPLATES_START                                  \
	/** NOLINTNEXTLINE **/                                         \
	DOCTEST_SUBCASE(__VA_ARGS__)                                   \
	/** NOLINT **/                                                 \
	IGNORE_RESERVED_IDENTIFIERS_STOP IGNORE_UNUSED_TEMPLATES_STOP

#if !defined(DOCTEST_CONFIG_DISABLE)
	/// @brief Use to do a doctest test assertion.
	/// Usage is identical to `DOCTEST_CHECK()`, this just ignores particular warnings coming from
	/// doctest.
	/// @ingroup defines
	/// @headerfile "Hyperion/HyperionDef.h"
	#define CHECK(...) /** NOLINT(cppcoreguidelines-macro-usage) **/                               \
		IGNORE_RESERVED_IDENTIFIERS_START IGNORE_UNUSED_TEMPLATES_START DOCTEST_CHECK(__VA_ARGS__) \
		IGNORE_RESERVED_IDENTIFIERS_STOP IGNORE_UNUSED_TEMPLATES_STOP
	/// @brief Use to do a doctest test assertion.
	/// Usage is identical to `DOCTEST_CHECK_FALSE()`, this just ignores particular warnings coming
	/// from doctest.
	/// @ingroup defines
	/// @headerfile "Hyperion/HyperionDef.h"
	#define CHECK_FALSE(...) /** NOLINT(cppcoreguidelines-macro-usage) **/                   \
		IGNORE_RESERVED_IDENTIFIERS_START IGNORE_UNUSED_TEMPLATES_START DOCTEST_CHECK_FALSE( \
			__VA_ARGS__)                                                                     \
		IGNORE_RESERVED_IDENTIFIERS_STOP IGNORE_UNUSED_TEMPLATES_STOP
	/// @brief Use to do a doctest test assertion.
	/// Usage is identical to `DOCTEST_CHECK_EQ()`, this just ignores particular warnings coming
	/// from doctest.
	/// @ingroup defines
	/// @headerfile "Hyperion/HyperionDef.h"
	#define CHECK_EQ(...) /** NOLINTNEXTLINE(cppcoreguidelines-macro-usage) **/           \
		IGNORE_RESERVED_IDENTIFIERS_START IGNORE_UNUSED_TEMPLATES_START DOCTEST_CHECK_EQ( \
			__VA_ARGS__)                                                                  \
		IGNORE_RESERVED_IDENTIFIERS_STOP IGNORE_UNUSED_TEMPLATES_STOP
	/// @brief Use to do a doctest test assertion.
	/// Usage is identical to `DOCTEST_CHECK_NE()`, this just ignores particular warnings coming
	/// from doctest.
	/// @ingroup defines
	/// @headerfile "Hyperion/HyperionDef.h"
	#define CHECK_NE(...) /** NOLINT(cppcoreguidelines-macro-usage) **/                   \
		IGNORE_RESERVED_IDENTIFIERS_START IGNORE_UNUSED_TEMPLATES_START DOCTEST_CHECK_NE( \
			__VA_ARGS__)                                                                  \
		IGNORE_RESERVED_IDENTIFIERS_STOP IGNORE_UNUSED_TEMPLATES_STOP
	/// @brief Use to do a doctest test assertion.
	/// Usage is identical to `DOCTEST_CHECK_GT()`, this just ignores particular warnings coming
	/// from doctest.
	/// @ingroup defines
	/// @headerfile "Hyperion/HyperionDef.h"
	#define CHECK_GT(...) /** NOLINT(cppcoreguidelines-macro-usage) **/                   \
		IGNORE_RESERVED_IDENTIFIERS_START IGNORE_UNUSED_TEMPLATES_START DOCTEST_CHECK_GT( \
			__VA_ARGS__)                                                                  \
		IGNORE_RESERVED_IDENTIFIERS_STOP IGNORE_UNUSED_TEMPLATES_STOP
	/// @brief Use to do a doctest test assertion.
	/// Usage is identical to `DOCTEST_CHECK_LT()`, this just ignores particular warnings coming
	/// from doctest.
	/// @ingroup defines
	/// @headerfile "Hyperion/HyperionDef.h"
	#define CHECK_LT(...) /** NOLINT(cppcoreguidelines-macro-usage) **/                   \
		IGNORE_RESERVED_IDENTIFIERS_START IGNORE_UNUSED_TEMPLATES_START DOCTEST_CHECK_LT( \
			__VA_ARGS__)                                                                  \
		IGNORE_RESERVED_IDENTIFIERS_STOP IGNORE_UNUSED_TEMPLATES_STOP
	/// @brief Use to do a doctest test assertion.
	/// Usage is identical to `DOCTEST_CHECK_GE()`, this just ignores particular warnings coming
	/// from doctest.
	/// @ingroup defines
	/// @headerfile "Hyperion/HyperionDef.h"
	#define CHECK_GE(...) /** NOLINT(cppcoreguidelines-macro-usage) **/                   \
		IGNORE_RESERVED_IDENTIFIERS_START IGNORE_UNUSED_TEMPLATES_START DOCTEST_CHECK_GE( \
			__VA_ARGS__)                                                                  \
		IGNORE_RESERVED_IDENTIFIERS_STOP IGNORE_UNUSED_TEMPLATES_STOP
	/// @brief Use to do a doctest test assertion.
	/// Usage is identical to `DOCTEST_CHECK_LE()`, this just ignores particular warnings coming
	/// from doctest.
	/// @ingroup defines
	/// @headerfile "Hyperion/HyperionDef.h"
	#define CHECK_LE(...) /** NOLINT(cppcoreguidelines-macro-usage) **/                   \
		IGNORE_RESERVED_IDENTIFIERS_START IGNORE_UNUSED_TEMPLATES_START DOCTEST_CHECK_LE( \
			__VA_ARGS__)                                                                  \
		IGNORE_RESERVED_IDENTIFIERS_STOP IGNORE_UNUSED_TEMPLATES_STOP

	/// @brief Use to do a doctest test assertion.
	/// Usage is identical to `DOCTEST_REQUIRE()`, this just ignores particular warnings coming from
	/// doctest.
	/// @ingroup defines
	/// @headerfile "Hyperion/HyperionDef.h"
	#define REQUIRE(...) /** NOLINT(cppcoreguidelines-macro-usage) **/                   \
		IGNORE_RESERVED_IDENTIFIERS_START IGNORE_UNUSED_TEMPLATES_START DOCTEST_REQUIRE( \
			__VA_ARGS__)                                                                 \
		IGNORE_RESERVED_IDENTIFIERS_STOP IGNORE_UNUSED_TEMPLATES_STOP
	/// @brief Use to do a doctest test assertion.
	/// Usage is identical to `DOCTEST_REQUIRE_FALSE()`, this just ignores particular warnings
	/// coming from doctest.
	/// @ingroup defines
	/// @headerfile "Hyperion/HyperionDef.h"
	#define REQUIRE_FALSE(...) /** NOLINT(cppcoreguidelines-macro-usage) **/                   \
		IGNORE_RESERVED_IDENTIFIERS_START IGNORE_UNUSED_TEMPLATES_START DOCTEST_REQUIRE_FALSE( \
			__VA_ARGS__)                                                                       \
		IGNORE_RESERVED_IDENTIFIERS_STOP IGNORE_UNUSED_TEMPLATES_STOP
	/// @brief Use to do a doctest test assertion.
	/// Usage is identical to `DOCTEST_REQUIRE_EQ()`, this just ignores particular warnings coming
	/// from doctest.
	/// @ingroup defines
	/// @headerfile "Hyperion/HyperionDef.h"
	#define REQUIRE_EQ(...) /** NOLINTNEXTLINE(cppcoreguidelines-macro-usage) **/           \
		IGNORE_RESERVED_IDENTIFIERS_START IGNORE_UNUSED_TEMPLATES_START DOCTEST_REQUIRE_EQ( \
			__VA_ARGS__)                                                                    \
		IGNORE_RESERVED_IDENTIFIERS_STOP IGNORE_UNUSED_TEMPLATES_STOP
	/// @brief Use to do a doctest test assertion.
	/// Usage is identical to `DOCTEST_REQUIRE_NE()`, this just ignores particular warnings coming
	/// from doctest.
	/// @ingroup defines
	/// @headerfile "Hyperion/HyperionDef.h"
	#define REQUIRE_NE(...) /** NOLINT(cppcoreguidelines-macro-usage) **/                   \
		IGNORE_RESERVED_IDENTIFIERS_START IGNORE_UNUSED_TEMPLATES_START DOCTEST_REQUIRE_NE( \
			__VA_ARGS__)                                                                    \
		IGNORE_RESERVED_IDENTIFIERS_STOP IGNORE_UNUSED_TEMPLATES_STOP
	/// @brief Use to do a doctest test assertion.
	/// Usage is identical to `DOCTEST_REQUIRE_GT()`, this just ignores particular warnings coming
	/// from doctest.
	/// @ingroup defines
	/// @headerfile "Hyperion/HyperionDef.h"
	#define REQUIRE_GT(...) /** NOLINT(cppcoreguidelines-macro-usage) **/                   \
		IGNORE_RESERVED_IDENTIFIERS_START IGNORE_UNUSED_TEMPLATES_START DOCTEST_REQUIRE_GT( \
			__VA_ARGS__)                                                                    \
		IGNORE_RESERVED_IDENTIFIERS_STOP IGNORE_UNUSED_TEMPLATES_STOP
	/// @brief Use to do a doctest test assertion.
	/// Usage is identical to `DOCTEST_REQUIRE_LT()`, this just ignores particular warnings coming
	/// from doctest.
	/// @ingroup defines
	/// @headerfile "Hyperion/HyperionDef.h"
	#define REQUIRE_LT(...) /** NOLINT(cppcoreguidelines-macro-usage) **/                   \
		IGNORE_RESERVED_IDENTIFIERS_START IGNORE_UNUSED_TEMPLATES_START DOCTEST_REQUIRE_LT( \
			__VA_ARGS__)                                                                    \
		IGNORE_RESERVED_IDENTIFIERS_STOP IGNORE_UNUSED_TEMPLATES_STOP
	/// @brief Use to do a doctest test assertion.
	/// Usage is identical to `DOCTEST_REQUIRE_GE()`, this just ignores particular warnings coming
	/// from doctest.
	/// @ingroup defines
	/// @headerfile "Hyperion/HyperionDef.h"
	#define REQUIRE_GE(...) /** NOLINT(cppcoreguidelines-macro-usage) **/                   \
		IGNORE_RESERVED_IDENTIFIERS_START IGNORE_UNUSED_TEMPLATES_START DOCTEST_REQUIRE_GE( \
			__VA_ARGS__)                                                                    \
		IGNORE_RESERVED_IDENTIFIERS_STOP IGNORE_UNUSED_TEMPLATES_STOP
	/// @brief Use to do a doctest test assertion.
	/// Usage is identical to `DOCTEST_REQUIRE_LE()`, this just ignores particular warnings coming
	/// from doctest.
	/// @ingroup defines
	/// @headerfile "Hyperion/HyperionDef.h"
	#define REQUIRE_LE(...) /** NOLINT(cppcoreguidelines-macro-usage) **/                   \
		IGNORE_RESERVED_IDENTIFIERS_START IGNORE_UNUSED_TEMPLATES_START DOCTEST_REQUIRE_LE( \
			__VA_ARGS__)                                                                    \
		IGNORE_RESERVED_IDENTIFIERS_STOP IGNORE_UNUSED_TEMPLATES_STOP

#else
	/// @brief Use to do a doctest test assertion.
	/// Usage is identical to `DOCTEST_CHECK()`, this just ignores particular warnings coming from
	/// doctest.
	/// @ingroup defines
	/// @headerfile "Hyperion/HyperionDef.h"
	#define CHECK(...)		 /** NOLINT(cppcoreguidelines-macro-usage) **/ hyperion::ignore(__VA_ARGS__)
	/// @brief Use to do a doctest test assertion.
	/// Usage is identical to `DOCTEST_CHECK_FALSE()`, this just ignores particular warnings coming
	/// from doctest.
	/// @ingroup defines
	/// @headerfile "Hyperion/HyperionDef.h"
	#define CHECK_FALSE(...) /** NOLINT(cppcoreguidelines-macro-usage) **/ \
		hyperion::ignore(__VA_ARGS__)
	/// @brief Use to do a doctest test assertion.
	/// Usage is identical to `DOCTEST_CHECK_EQ()`, this just ignores particular warnings coming
	/// from doctest.
	/// @ingroup defines
	/// @headerfile "Hyperion/HyperionDef.h"
	#define CHECK_EQ(...) /** NOLINT(cppcoreguidelines-macro-usage) **/ \
		hyperion::ignore(__VA_ARGS__)
	/// @brief Use to do a doctest test assertion.
	/// Usage is identical to `DOCTEST_CHECK_NE()`, this just ignores particular warnings coming
	/// from doctest.
	/// @ingroup defines
	/// @headerfile "Hyperion/HyperionDef.h"
	#define CHECK_NE(...) /** NOLINT(cppcoreguidelines-macro-usage) **/ \
		hyperion::ignore(__VA_ARGS__)
	/// @brief Use to do a doctest test assertion.
	/// Usage is identical to `DOCTEST_CHECK_GT()`, this just ignores particular warnings coming
	/// from doctest.
	/// @ingroup defines
	/// @headerfile "Hyperion/HyperionDef.h"
	#define CHECK_GT(...) /** NOLINT(cppcoreguidelines-macro-usage) **/ \
		hyperion::ignore(__VA_ARGS__)
	/// @brief Use to do a doctest test assertion.
	/// Usage is identical to `DOCTEST_CHECK_LT()`, this just ignores particular warnings coming
	/// from doctest.
	/// @ingroup defines
	/// @headerfile "Hyperion/HyperionDef.h"
	#define CHECK_LT(...) /** NOLINT(cppcoreguidelines-macro-usage) **/ \
		hyperion::ignore(__VA_ARGS__)
	/// @brief Use to do a doctest test assertion.
	/// Usage is identical to `DOCTEST_CHECK_GE()`, this just ignores particular warnings coming
	/// from doctest.
	/// @ingroup defines
	/// @headerfile "Hyperion/HyperionDef.h"
	#define CHECK_GE(...) /** NOLINT(cppcoreguidelines-macro-usage) **/ \
		hyperion::ignore(__VA_ARGS__)
	/// @brief Use to do a doctest test assertion.
	/// Usage is identical to `DOCTEST_CHECK_LE()`, this just ignores particular warnings coming
	/// from doctest.
	/// @ingroup defines
	/// @headerfile "Hyperion/HyperionDef.h"
	#define CHECK_LE(...) /** NOLINT(cppcoreguidelines-macro-usage) **/ \
		hyperion::ignore(__VA_ARGS__)

	/// @brief Use to do a doctest test assertion.
	/// Usage is identical to `DOCTEST_REQUIRE()`, this just ignores particular warnings coming from
	/// doctest.
	/// @ingroup defines
	/// @headerfile "Hyperion/HyperionDef.h"
	#define REQUIRE(...)	   /** NOLINT(cppcoreguidelines-macro-usage) **/ hyperion::ignore(__VA_ARGS__)
	/// @brief Use to do a doctest test assertion.
	/// Usage is identical to `DOCTEST_REQUIRE_FALSE()`, this just ignores particular warnings
	/// coming from doctest.
	/// @ingroup defines
	/// @headerfile "Hyperion/HyperionDef.h"
	#define REQUIRE_FALSE(...) /** NOLINT(cppcoreguidelines-macro-usage) **/ \
		hyperion::ignore(__VA_ARGS__)
	/// @brief Use to do a doctest test assertion.
	/// Usage is identical to `DOCTEST_REQUIRE_EQ()`, this just ignores particular warnings coming
	/// from doctest.
	/// @ingroup defines
	/// @headerfile "Hyperion/HyperionDef.h"
	#define REQUIRE_EQ(...) /** NOLINT(cppcoreguidelines-macro-usage) **/ \
		hyperion::ignore(__VA_ARGS__)
	/// @brief Use to do a doctest test assertion.
	/// Usage is identical to `DOCTEST_REQUIRE_NE()`, this just ignores particular warnings coming
	/// from doctest.
	/// @ingroup defines
	/// @headerfile "Hyperion/HyperionDef.h"
	#define REQUIRE_NE(...) /** NOLINT(cppcoreguidelines-macro-usage) **/ \
		hyperion::ignore(__VA_ARGS__)
	/// @brief Use to do a doctest test assertion.
	/// Usage is identical to `DOCTEST_REQUIRE_GT()`, this just ignores particular warnings coming
	/// from doctest.
	/// @ingroup defines
	/// @headerfile "Hyperion/HyperionDef.h"
	#define REQUIRE_GT(...) /** NOLINT(cppcoreguidelines-macro-usage) **/ \
		hyperion::ignore(__VA_ARGS__)
	/// @brief Use to do a doctest test assertion.
	/// Usage is identical to `DOCTEST_REQUIRE_LT()`, this just ignores particular warnings coming
	/// from doctest.
	/// @ingroup defines
	/// @headerfile "Hyperion/HyperionDef.h"
	#define REQUIRE_LT(...) /** NOLINT(cppcoreguidelines-macro-usage) **/ \
		hyperion::ignore(__VA_ARGS__)
	/// @brief Use to do a doctest test assertion.
	/// Usage is identical to `DOCTEST_REQUIRE_GE()`, this just ignores particular warnings coming
	/// from doctest.
	/// @ingroup defines
	/// @headerfile "Hyperion/HyperionDef.h"
	#define REQUIRE_GE(...) /** NOLINT(cppcoreguidelines-macro-usage) **/ \
		hyperion::ignore(__VA_ARGS__)
	/// @brief Use to do a doctest test assertion.
	/// Usage is identical to `DOCTEST_REQUIRE_LE()`, this just ignores particular warnings coming
	/// from doctest.
	/// @ingroup defines
	/// @headerfile "Hyperion/HyperionDef.h"
	#define REQUIRE_LE(...) /** NOLINT(cppcoreguidelines-macro-usage) **/ \
		hyperion::ignore(__VA_ARGS__)
#endif // !defined(DOCTEST_CONFIG_DISABLE)

#ifdef TRACY_ENABLE

IGNORE_RESERVED_IDENTIFIERS_START
IGNORE_OLD_STYLE_CASTS_START
IGNORE_PADDING_START
IGNORE_SUGGEST_DESTRUCTOR_OVERRIDE_START
	#include <Tracy.hpp>
IGNORE_SUGGEST_DESTRUCTOR_OVERRIDE_STOP
IGNORE_PADDING_STOP
IGNORE_OLD_STYLE_CASTS_STOP
IGNORE_RESERVED_IDENTIFIERS_STOP

	/// @brief Indicates whether Tracy profiling is enabled for this build
	/// @ingroup defines
	/// @headerfile "Hyperion/HyperionDef.h"
	#define HYPERION_PLATFORM_PROFILING_ENABLED /** NOLINT(cppcoreguidelines-macro-usage) **/ true
	/// @brief Profiles the containing scope with Tracy in builds where Tracy profiling is enabled
	/// @ingroup defines
	/// @headerfile "Hyperion/HyperionDef.h"
	#define HYPERION_PROFILE_FUNCTION() /** NOLINT(cppcoreguidelines-macro-usage) **/             \
		IGNORE_RESERVED_IDENTIFIERS_START                                                         \
		IGNORE_OLD_STYLE_CASTS_START                                                              \
		/** NOLINT(cppcoreguidelines-pro-bounds-array-to-pointer-decay, hicpp-no-array-decay) **/ \
		ZoneScoped /** NOLINT(cppcoreguidelines-pro-bounds-array-to-pointer-decay,                \
					   hicpp-no-array-decay) **/                                                  \
			IGNORE_OLD_STYLE_CASTS_STOP IGNORE_RESERVED_IDENTIFIERS_STOP
	/// @brief Starts a profiling frame with the given name with Tracy in builds where Tracy
	/// profiling is enabled
	/// @ingroup defines
	/// @headerfile "Hyperion/HyperionDef.h"
	#define HYPERION_PROFILE_START_FRAME(name) /** NOLINT(cppcoreguidelines-macro-usage) **/      \
		IGNORE_RESERVED_IDENTIFIERS_START                                                         \
		IGNORE_OLD_STYLE_CASTS_START                                                              \
		/** NOLINT(cppcoreguidelines-pro-bounds-array-to-pointer-decay, hicpp-no-array-decay) **/ \
		FrameMarkStart(name) /** NOLINT(cppcoreguidelines-pro-bounds-array-to-pointer-decay,      \
								 hicpp-no-array-decay) **/                                        \
			IGNORE_OLD_STYLE_CASTS_STOP IGNORE_RESERVED_IDENTIFIERS_STOP
	/// @brief Ends the profiling frame with the given name with Tracy in builds where Tracy
	/// profiling is enabled
	/// @ingroup defines
	/// @headerfile "Hyperion/HyperionDef.h"
	#define HYPERION_PROFILE_END_FRAME(name) /** NOLINT(cppcoreguidelines-macro-usage) **/        \
		IGNORE_RESERVED_IDENTIFIERS_START                                                         \
		IGNORE_OLD_STYLE_CASTS_START                                                              \
		/** NOLINT(cppcoreguidelines-pro-bounds-array-to-pointer-decay, hicpp-no-array-decay) **/ \
		FrameMarkEnd(name) /** NOLINT(cppcoreguidelines-pro-bounds-array-to-pointer-decay,        \
							   hicpp-no-array-decay) **/                                          \
			IGNORE_OLD_STYLE_CASTS_STOP IGNORE_RESERVED_IDENTIFIERS_STOP
	/// @brief Marks the end of a profiling frame with Tracy in builds where Tracy profiling is enabled
	/// @ingroup defines
	/// @headerfile "Hyperion/HyperionDef.h"
	#define HYPERION_PROFILE_MARK_FRAME() /** NOLINT(cppcoreguidelines-macro-usage) **/           \
		IGNORE_RESERVED_IDENTIFIERS_START                                                         \
		IGNORE_OLD_STYLE_CASTS_START                                                              \
		/** NOLINT(cppcoreguidelines-pro-bounds-array-to-pointer-decay, hicpp-no-array-decay) **/ \
		FrameMark /** NOLINT(cppcoreguidelines-pro-bounds-array-to-pointer-decay,                 \
					  hicpp-no-array-decay) **/                                                   \
			IGNORE_OLD_STYLE_CASTS_STOP IGNORE_RESERVED_IDENTIFIERS_STOP
#else
	/// @brief Indicates whether Tracy profiling is enabled for this build
	/// @ingroup defines
	/// @headerfile "Hyperion/HyperionDef.h"
	#define HYPERION_PLATFORM_PROFILING_ENABLED /** NOLINT(cppcoreguidelines-macro-usage) **/ false
	/// @brief Profiles the containing scope with Tracy in builds where Tracy profiling is enabled
	/// @ingroup defines
	/// @headerfile "Hyperion/HyperionDef.h"
	#define HYPERION_PROFILE_FUNCTION()			/** NOLINT(cppcoreguidelines-macro-usage) **/
	/// @brief Starts a profiling frame with the given name with Tracy in builds where Tracy
	/// profiling is enabled
	/// @ingroup defines
	/// @headerfile "Hyperion/HyperionDef.h"
	#define HYPERION_PROFILE_START_FRAME(name)	/** NOLINT(cppcoreguidelines-macro-usage) **/
	/// @brief Ends the profiling frame with the given name with Tracy in builds where Tracy
	/// profiling is enabled
	/// @ingroup defines
	/// @headerfile "Hyperion/HyperionDef.h"
	#define HYPERION_PROFILE_END_FRAME(name)	/** NOLINT(cppcoreguidelines-macro-usage) **/
	/// @brief Marks the end of a profiling frame with Tracy in builds where Tracy profiling is enabled
	/// @ingroup defines
	/// @headerfile "Hyperion/HyperionDef.h"
	#define HYPERION_PROFILE_MARK_FRAME()		/** NOLINT(cppcoreguidelines-macro-usage) **/
#endif

IGNORE_UNUSED_MACROS_STOP
