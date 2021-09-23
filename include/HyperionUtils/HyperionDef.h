/// @brief Collection of macros used throughout Hyperion for configuration
#pragma once

#include <version>

#include "Platform.h"

#ifndef HYPERION_USE_EXPERIMENTAL_SOURCE_LOCATION
	#define HYPERION_USE_EXPERIMENTAL_SOURCE_LOCATION true
#endif

#if(defined(__cpp_lib_source_location) && __cpp_lib_source_location == 201907)
	#define HYPERION_HAS_SOURCE_LOCATION true
#else
	#define HYPERION_HAS_SOURCE_LOCATION false
#endif // (defined(__cpp_lib_source_location) && __cpp_lib_source_location == 201907)

#define HYPERION_DECLARE_NON_HEAP_ALLOCATABLE() /** NOLINT(cppcoreguidelines-macro-usage): This \
											   isn't a function-like macro**/                   \
	static auto operator new(std::size_t size) noexcept->void* = delete;                        \
	static auto operator new[](std::size_t size) noexcept->void* = delete;                      \
	static auto operator delete(void* obj) noexcept->void = delete;                             \
	static auto operator delete[](void* obj) noexcept->void = delete;

#ifdef HYPERION_PLATFORM_COMPILER_CLANG
	#define HYPERION_TRIVIAL_ABI [[clang::trivial_abi]]
#else
	#define HYPERION_TRIVIAL_ABI
#endif

/// Use to mark variable as no_destroy on clang
#ifdef HYPERION_PLATFORM_COMPILER_CLANG
	#define HYPERION_NO_DESTROY [[clang::no_destroy]] // NOLINT
#else
	#define HYPERION_NO_DESTROY
#endif

/// Use to conditionally mark something `constexpr` if `std::string` is `constexpr
/// This will be removable once all stdlibs impl `constexpr std::string`
#ifdef __cpp_lib_constexpr_string
	#if(__cpp_lib_constexpr_string == 201907L)
		#define HYPERION_CONSTEXPR_STRINGS constexpr
	#else
		#define HYPERION_CONSTEXPR_STRINGS
	#endif
#else
	#define HYPERION_CONSTEXPR_STRINGS
#endif

#if defined(HYPERION_PLATFORM_COMPILER_GCC) || defined(HYPERION_PLATFORM_COMPILER_CLANG)
	#define HYPERION_UNREACHABLE() __builtin_unreachable();
#elif defined(_MSC_VER)
	#define HYPERION_UNREACHABLE() __assume(false);
#else
	#define HYPERION_UNREACHABLE()
#endif

#if defined(__has_builtin)
	#if __has_builtin(__type_pack_element)
		#define HYPERION_HAS_TYPE_PACK_ELEMENT
	#endif
#endif

/// Use to temporarily disable unused macros warning on GCC/Clang
// clang-format off
#ifndef _MSC_VER
	// NOLINTNEXTLINE
	#define IGNORE_UNUSED_MACROS_START \
		_Pragma("GCC diagnostic push")\
		_Pragma("GCC diagnostic ignored \"-Wunused-macros\"")
#else
	// NOLINTNEXTLINE
	#define IGNORE_UNUSED_MACROS_START
#endif

/// Use to re-enable unused macros warning on GCC/Clang
#ifndef _MSC_VER
	// NOLINTNEXTLINE
	#define IGNORE_UNUSED_MACROS_STOP \
		_Pragma("GCC diagnostic pop")
#else
	// NOLINTNEXTLINE
	#define IGNORE_UNUSED_MACROS_STOP
#endif

IGNORE_UNUSED_MACROS_START

/// Use to temporarily disable missing noreturn warning
#ifndef _MSC_VER
	// NOLINTNEXTLINE
	#define IGNORE_MISSING_NORETURN_START \
		_Pragma("GCC diagnostic push") \
		_Pragma("GCC diagnostic ignored \"-Wmissing-noreturn\"")
#else
	// NOLINTNEXTLINE
	#define IGNORE_PADDING_START
#endif

/// Use to re-enable missing noreturn warning
#ifndef _MSC_VER
	// NOLINTNEXTLINE
	#define IGNORE_MISSING_NORETURN_STOP \
		_Pragma("GCC diagnostic pop")
#else
	// NOLINTNEXTLINE
	#define IGNORE_PADDING_STOP
#endif

/// Use to temporarily disable reserved identifier warning
#ifndef _MSC_VER
	// NOLINTNEXTLINE
	#define IGNORE_RESERVED_IDENTIFIERS_START \
		_Pragma("GCC diagnostic push") \
		_Pragma("GCC diagnostic ignored \"-Wreserved-identifier\"")
#else
	// NOLINTNEXTLINE
	#define IGNORE_PADDING_START \
		_Pragma("warning( push )") \
		_Pragma("warning( disable : 4405 )")
#endif

/// Use to re-enable reserved identifier warning
#ifndef _MSC_VER
	// NOLINTNEXTLINE
	#define IGNORE_RESERVED_IDENTIFIERS_STOP \
		_Pragma("GCC diagnostic pop")
#else
	// NOLINTNEXTLINE
	#define IGNORE_PADDING_STOP \
		_Pragma("warning( pop )")
#endif

/// Use to temporarily disable padding warning
#ifndef _MSC_VER
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

/// Use to re-enable padding warning
#ifndef _MSC_VER
	// NOLINTNEXTLINE
	#define IGNORE_PADDING_STOP \
		_Pragma("GCC diagnostic pop")
#else
	// NOLINTNEXTLINE
	#define IGNORE_PADDING_STOP \
		_Pragma("warning( pop )")
#endif

/// Use to temporarily disable weak vtable warning on clang
#ifndef _MSC_VER
	#ifdef HYPERION_PLATFORM_COMPILER_CLANG
		// NOLINTNEXTLINE
		#define IGNORE_WEAK_VTABLES_START \
			_Pragma("GCC diagnostic push") \
			_Pragma("GCC diagnostic ignored \"-Wweak-vtables\"")
	#else
		// NOLINTNEXTLINE
		#define IGNORE_WEAK_VTABLES_START
	#endif
#else
	// NOLINTNEXTLINE
	#define IGNORE_WEAK_VTABLES_START
#endif

/// Use to re-enable weak vtable warning on clang
#ifndef _MSC_VER
	#ifdef HYPERION_PLATFORM_COMPILER_CLANG
		// NOLINTNEXTLINE
		#define IGNORE_WEAK_VTABLES_STOP \
			_Pragma("GCC diagnostic pop")
	#else
		// NOLINTNEXTLINE
		#define IGNORE_WEAK_VTABLES_STOP
	#endif
#else
	// NOLINTNEXTLINE
	#define IGNORE_WEAK_VTABLES_STOP
#endif

/// Use to temporarily disable unused templates warning on clang
#ifndef _MSC_VER
	#ifdef HYPERION_PLATFORM_COMPILER_CLANG
		// NOLINTNEXTLINE
		#define IGNORE_UNUSED_TEMPLATES_START \
			_Pragma("GCC diagnostic push")\
			_Pragma("GCC diagnostic ignored \"-Wunused-template\"")
	#else
		// NOLINTNEXTLINE
		#define IGNORE_UNUSED_TEMPLATES_START
	#endif
#else
	// NOLINTNEXTLINE
	#define IGNORE_UNUSED_TEMPLATES_START
#endif

/// Use to re-enable unused templates warning on clang
#ifndef _MSC_VER
	#ifdef HYPERION_PLATFORM_COMPILER_CLANG
		// NOLINTNEXTLINE
		#define IGNORE_UNUSED_TEMPLATES_STOP \
			_Pragma("GCC diagnostic pop")
	#else
		// NOLINTNEXTLINE
		#define IGNORE_UNUSED_TEMPLATES_STOP
	#endif
#else
	// NOLINTNEXTLINE
	#define IGNORE_UNUSED_TEMPLATES_STOP
#endif

/// Use to temporarily disable signed-enum-bitfield warning on clang
#ifndef _MSC_VER
	#ifdef HYPERION_PLATFORM_COMPILER_CLANG
		// NOLINTNEXTLINE
		#define IGNORE_SIGNED_BITFIELD_START \
			_Pragma("GCC diagnostic push") \
			_Pragma("GCC diagnostic ignored \"-Wsigned-enum-bitfield\"")
	#else
		// NOLINTNEXTLINE
		#define IGNORE_SIGNED_BITFIELD_START
	#endif
#else
	// NOLINTNEXTLINE
	#define IGNORE_SIGNED_BITFIELD_START
#endif

/// Use to re-enable signed-enum-bitfield warning on clang
#ifndef _MSC_VER
	#ifdef HYPERION_PLATFORM_COMPILER_CLANG
		// NOLINTNEXTLINE
		#define IGNORE_SIGNED_BITFIELD_STOP \
			_Pragma("GCC diagnostic pop")
	#else
		// NOLINTNEXTLINE
		#define IGNORE_SIGNED_BITFIELD_STOP
	#endif
#else
	// NOLINTNEXTLINE
	#define IGNORE_SIGNED_BITFIELD_STOP
#endif

/// Use to temporarily disable documentation-unknown-command warning on clang
#ifndef _MSC_VER
	#ifdef HYPERION_PLATFORM_COMPILER_CLANG
		// NOLINTNEXTLINE
		#define IGNORE_UNKNOWN_DOC_COMMAND_START \
			_Pragma("GCC diagnostic push") \
			_Pragma("GCC diagnostic ignored \"-Wdocumentation-unknown-command\"")
	#else
		// NOLINTNEXTLINE
		#define IGNORE_UNKNOWN_DOC_COMMAND_START
	#endif
#else
	// NOLINTNEXTLINE
	#define IGNORE_UNKNOWN_DOC_COMMAND_START
#endif

/// Use to re-enable documentation-unknown-command warning on clang
#ifndef _MSC_VER
	#ifdef HYPERION_PLATFORM_COMPILER_CLANG
		// NOLINTNEXTLINE
		#define IGNORE_UNKNOWN_DOC_COMMAND_STOP \
			_Pragma("GCC diagnostic pop")
	#else
		// NOLINTNEXTLINE
		#define IGNORE_UNKNOWN_DOC_COMMAND_STOP
	#endif
#else
	// NOLINTNEXTLINE
	#define IGNORE_UNKNOWN_DOC_COMMAND_STOP
#endif

/// Use to temporarily disable shadow-field-in-constructor warning on clang
#ifndef _MSC_VER
	#ifdef HYPERION_PLATFORM_COMPILER_CLANG
		// NOLINTNEXTLINE
		#define IGNORE_CONSTRUCTOR_SHADOW_FIELDS_START \
			_Pragma("GCC diagnostic push") \
			_Pragma("GCC diagnostic ignored \"-Wshadow-field-in-constructor\"")
	#else
		// NOLINTNEXTLINE
		#define IGNORE_CONSTRUCTOR_SHADOW_FIELDS_START
	#endif
#else
	// NOLINTNEXTLINE
	#define IGNORE_CONSTRUCTOR_SHADOW_FIELDS_START
#endif

/// Use to re-enable shadow-field-in-constructor warning on clang
#ifndef _MSC_VER
	#ifdef HYPERION_PLATFORM_COMPILER_CLANG
		// NOLINTNEXTLINE
		#define IGNORE_CONSTRUCTOR_SHADOW_FIELDS_STOP \
			_Pragma("GCC diagnostic pop")
	#else
		// NOLINTNEXTLINE
		#define IGNORE_CONSTRUCTOR_SHADOW_FIELDS_STOP
	#endif
#else
	// NOLINTNEXTLINE
	#define IGNORE_CONSTRUCTOR_SHADOW_FIELDS_STOP
#endif

/// Use to temporarily disable comma misuse warning on clang
#ifndef _MSC_VER
	#ifdef HYPERION_PLATFORM_COMPILER_CLANG
		// NOLINTNEXTLINE
		#define IGNORE_COMMA_MISUSE_START \
			_Pragma("GCC diagnostic push") \
			_Pragma("GCC diagnostic ignored \"-Wcomma\"")
	#else
		// NOLINTNEXTLINE
		#define IGNORE_COMMA_MISUSE_START
	#endif
#else
	// NOLINTNEXTLINE
	#define IGNORE_COMMA_MISUSE_START
#endif

/// Use to re-enable comma misuse warning on clang
#ifndef _MSC_VER
	#ifdef HYPERION_PLATFORM_COMPILER_CLANG
		// NOLINTNEXTLINE
		#define IGNORE_COMMA_MISUSE_STOP \
			_Pragma("GCC diagnostic pop")
	#else
		// NOLINTNEXTLINE
		#define IGNORE_COMMA_MISUSE_STOP
	#endif
#else
	// NOLINTNEXTLINE
	#define IGNORE_COMMA_MISUSE_STOP
#endif

// clang-format on
IGNORE_UNUSED_MACROS_STOP
