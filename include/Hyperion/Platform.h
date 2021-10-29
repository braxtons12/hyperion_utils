/// @file Platform.h
/// @author Braxton Salyer <braxtonsalyer@gmail.com>
/// @brief This module includes macro defines for identifying the platform being compiled for
/// (operating system, compiler, etc)
/// @version 0.1
/// @date 2021-10-19
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

/// @ingroup utils
/// @{
///	@defgroup platform Platform
/// Hyperion provides an assortment of platform detection macros to detect operating system,
/// compiler, little vs big endianness, and architecture.
/// @note while Hyperion provides many different platform detection macros, it has currently only
/// been tested on x86_64 Windows, x86_64 linux, and ARM64 (M1) MacOs
/// @headerfile "Hyperion/Platform.h"
/// @}
#pragma once

#include <bit>

/// @def HYPERION_PLATFORM_WINDOWS
/// @brief Whether the compiled-for platform is WINDOWS
/// @ingroup platform
/// @headerfile "Hyperion/Platform.h"
#if defined(_MSC_VER) || defined(_WIN32) || defined(_WIN64)
	// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
	#define HYPERION_PLATFORM_WINDOWS true
#else
	// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
	#define HYPERION_PLATFORM_WINDOWS false
#endif

/// @def HYPERION_PLATFORM_APPLE
/// @brief Whether the compiled-for platform is APPLE (MacOS, IOS)
/// @ingroup platform
/// @headerfile "Hyperion/Platform.h"
#if defined(__APPLE__)
	// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
	#define HYPERION_PLATFORM_APPLE true
#else
	// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
	#define HYPERION_PLATFORM_APPLE false
#endif

/// @def HYPERION_PLATFORM_UNIX
/// @brief Whether the compiled-for platform is a variant of UNIX
/// @ingroup platform
/// @headerfile "Hyperion/Platform.h"
#if defined(__unix__) || defined(__unix) || defined(unix) \
	|| (HYPERION_PLATFORM_APPLE && defined(__MACH__))
	// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
	#define HYPERION_PLATFORM_UNIX true
#else
	// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
	#define HYPERION_PLATFORM_UNIX false
#endif

/// @def HYPERION_PLATFORM_LINUX
/// @brief Whether the compiled-for platform is LINUX
/// @ingroup platform
/// @headerfile "Hyperion/Platform.h"
#if defined(linux) || defined(__linux__) || defined(__linux) \
	|| (defined(__unix__) && !HYPERION_PLATFORM_APPLE && !defined(BSD))
	// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
	#define HYPERION_PLATFORM_LINUX true
#else
	// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
	#define HYPERION_PLATFORM_LINUX false
#endif

/// @def HYPERION_PLATFORM_BSD
/// @brief Whether the compiled-for platform is a BSD
/// @ingroup platform
/// @headerfile "Hyperion/Platform.h"
#if defined(BSD)
	// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
	#define HYPERION_PLATFORM_BSD true
#else
	// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
	#define HYPERION_PLATFORM_BSD false
#endif

/// @def HYPERION_PLATFORM_ANDROID
/// @brief Whether the compiled-for platform is Android
/// @ingroup platform
/// @headerfile "Hyperion/Platform.h"
#if defined(__ANDROID__)
	// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
	#define HYPERION_PLATFORM_ANDROID true
#else
	// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
	#define HYPERION_PLATFORM_ANDROID false
#endif

/// @def HYPERION_PLATFORM_COMPILER_CLANG
/// @brief Whether the current compiler is CLANG
/// @ingroup platform
/// @headerfile "Hyperion/Platform.h"
#if defined(__clang__)
	// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
	#define HYPERION_PLATFORM_COMPILER_CLANG true
#else
	// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
	#define HYPERION_PLATFORM_COMPILER_CLANG false
#endif

/// @def HYPERION_PLATFORM_COMPILER_GCC
/// @brief Whether the current compiler is GCC
/// @ingroup platform
/// @headerfile "Hyperion/Platform.h"
#if defined(__GNUC__) && !defined(__clang__)
	// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
	#define HYPERION_PLATFORM_COMPILER_GCC true
#else
	// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
	#define HYPERION_PLATFORM_COMPILER_GCC false
#endif

/// @def HYPERION_PLATFORM_COMPILER_MSVC
/// @brief Whether the current compiler is GCC
/// @ingroup platform
/// @headerfile "Hyperion/Platform.h"
#if defined(__MSC_VER) || defined(_MSC_VER) && !HYPERION_PLATFORM_COMPILER_CLANG
	// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
	#define HYPERION_PLATFORM_COMPILER_MSVC true
#else
	// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
	#define HYPERION_PLATFORM_COMPILER_MSVC false
#endif

/// @def HYPERION_PLATFORM_STD_LIB_LIBCPP
/// @brief Whether the used standard library is libc++ (llvm)
/// @ingroup platform
/// @headerfile "Hyperion/Platform.h"

/// @def HYPERION_PLATFORM_STD_LIB_LIBSTDCPP
/// @brief Whether the used standard library is libstdc++ (gcc)
/// @ingroup platform
/// @headerfile "Hyperion/Platform.h"

/// @def HYPERION_PLATFORM_STD_LIB_MSVC
/// @brief Whether the used standard library is MSVC
/// @ingroup platform
/// @headerfile "Hyperion/Platform.h"

#ifdef _LIBCPP_VERSION
	// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
	#define HYPERION_PLATFORM_STD_LIB_LIBCPP true
	// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
	#define HYPERION_PLATFORM_STD_LIB_LIBSTDCPP false
	// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
	#define HYPERION_PLATFORM_STD_LIB_MSVC false
#elif defined(__GLIBCXX__)
	// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
	#define HYPERION_PLATFORM_STD_LIB_LIBCPP	false
	// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
	#define HYPERION_PLATFORM_STD_LIB_LIBSTDCPP true
	// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
	#define HYPERION_PLATFORM_STD_LIB_MSVC		false
#elif defined(_MSC_VER)
	// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
	#define HYPERION_PLATFORM_STD_LIB_LIBCPP	false
	// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
	#define HYPERION_PLATFORM_STD_LIB_LIBSTDCPP false
	// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
	#define HYPERION_PLATFORM_STD_LIB_MSVC		true
#else
	// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
	#define HYPERION_PLATFORM_STD_LIB_LIBCPP	false
	// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
	#define HYPERION_PLATFORM_STD_LIB_LIBSTDCPP false
	// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
	#define HYPERION_PLATFORM_STD_LIB_MSVC		false
#endif // _LIBCPP_VERSION

/// @def HYPERION_PLATFORM_DEBUG
/// @brief Whether the current build mode is DEBUG
/// @ingroup platform
/// @headerfile "Hyperion/Platform.h"

/// @def HYPERION_PLATFORM_RELEASE
/// @brief Whether the current build mode is RELEASE
/// @ingroup platform
/// @headerfile "Hyperion/Platform.h"

#if defined(DEBUG) || !defined(NDEBUG)
	// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
	#define HYPERION_PLATFORM_DEBUG true
	// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
	#define HYPERION_PLATFORM_RELEASE false
#else
	// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
	#define HYPERION_PLATFORM_DEBUG	  false
	// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
	#define HYPERION_PLATFORM_RELEASE true
#endif // defined(DEBUG) || !defined(NDEBUG)

/// @brief The list of supported architectures to compile for
/// @ingroup platform
/// @headerfile "Hyperion/Platform.h"
#define HYPERION_PLATFORM_x86_64 (1U << 1U)							  // NOLINT
#define HYPERION_PLATFORM_x86	 (1U << 2U)							  // NOLINT
#define HYPERION_PLATFORM_ARMv6	 (1U << 7U)							  // NOLINT
#define HYPERION_PLATFORM_ARMv7	 (1U << 8U)							  // NOLINT
#define HYPERION_PLATFORM_ARMv7a (HYPERION_PLATFORM_ARMv7 | 1U << 9U) // NOLINT
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define HYPERION_PLATFORM_ARMv7r \
	(HYPERION_PLATFORM_ARMv7 | HYPERION_PLATFORM_ARMv7a | 1U << 10U) // NOLINT
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define HYPERION_PLATFORM_ARMv7m                                                   \
	(HYPERION_PLATFORM_ARMv7 | HYPERION_PLATFORM_ARMv7a | HYPERION_PLATFORM_ARMv7r \
	 | 1U << 11U) // NOLINT
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define HYPERION_PLATFORM_ARMv7s                                                   \
	(HYPERION_PLATFORM_ARMv7 | HYPERION_PLATFORM_ARMv7a | HYPERION_PLATFORM_ARMv7r \
	 | 1U << 12U)							  // NOLINT
#define HYPERION_PLATFORM_ARMv8	  (1U << 13U) // NOLINT
#define HYPERION_PLATFORM_ARM64	  (1U << 13U) // NOLINT
#define HYPERION_PLATFORM_Unknown (1U << 14U) // NOLINT

/// @def HYPERION_PLATFORM_ARCHITECTURE
/// @brief The CPU architecture family being compiled for
///
/// Can be any of `HyperionPlatformArchitecture`
/// @ingroup platform
/// @headerfile "Hyperion/Platform.h"

#if defined(__x86_64__) || defined(_M_X64) || defined(__X86_64__)
	#define HYPERION_PLATFORM_ARCHITECTURE HYPERION_PLATFORM_x86_64 // NOLINT
#elif defined(i386) || defined(__i386__) || defined(__i386) || defined(_M_IX86)
	#define HYPERION_PLATFORM_ARCHITECTURE HYPERION_PLATFORM_x86 // NOLINT
#elif defined(__ARM_ARCH_6__) || defined(__ARM_ARCH_6J__) || defined(__ARM_ARCH_6K__) \
	|| defined(__ARM_ARCH_6Z__) || defined(__ARM_ARCH_6ZK__)
	// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
	#define HYPERION_PLATFORM_ARCHITECTURE HYPERION_PLATFORM_ARMv6
#elif defined(__ARM_ARCH_7__) || defined(__ARM_ARCH_7A__) || defined(__ARM_ARCH_7R__) \
	|| defined(__ARM_ARCH_7M__) || defined(__ARM_ARCH_7S__)

	#if defined(__ARM_ARCH_7S__)
		// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
		#define HYPERION_PLATFORM_ARCHITECTURE HYPERION_PLATFORM_ARMv7s
	#elif defined(__ARM_ARCH_7M__) || defined(__ARM_ARCH_7S__)
		// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
		#define HYPERION_PLATFORM_ARCHITECTURE HYPERION_PLATFORM_ARMv7m
	#elif defined(__ARM_ARCH_7R__) || defined(__ARM_ARCH_7M__) || defined(__ARM_ARCH_7S__)
		// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
		#define HYPERION_PLATFORM_ARCHITECTURE HYPERION_PLATFORM_ARMv7r
	#elif defined(__ARM_ARCH_7A__) || defined(__ARM_ARCH_7R__) || defined(__ARM_ARCH_7M__) \
		|| defined(__ARM_ARCH_7S__)
		// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
		#define HYPERION_PLATFORM_ARCHITECTURE HYPERION_PLATFORM_ARMv7a
	#else
		// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
		#define HYPERION_PLATFORM_ARCHITECTURE HYPERION_PLATFORM_ARMv7
	#endif
#elif defined(__aarch64__) || defined(aarch64) || defined(__arm64__) || defined(arm64) \
	|| defined(_M_ARM64)
	// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
	#define HYPERION_PLATFORM_ARCHITECTURE HYPERION_PLATFORM_ARMv8
#else
	// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
	#define HYPERION_PLATFORM_ARCHITECTURE HYPERION_PLATFORM_Unknown
#endif

/// @brief Determines if the compiled-for architecture is the given one
///
/// This will determine if the architecture being compiled for matches or is a super set of
/// the given architecture to check for.
///
/// @param arch - The platform architecture to check against. Must be one of the enum
/// `HyperionPlatformArchitecture`
///
/// @return whether the compiled-for architecture is the given one
/// @ingroup platform
/// @headerfile "Hyperion/Platform.h"
#define IS_HYPERION_PLATFORM_ARCHITECTURE(arch) /** NOLINT **/ \
	((HYPERION_PLATFORM_ARCHITECTURE & (arch)) > 0)

// BIG OR LITTLE ENDIAN ?

/// @brief Whether the compiled-for architecture is a little endian architecture
/// @ingroup platform
/// @headerfile "Hyperion/Platform.h"
#define HYPERION_PLATFORM_LITTLE_ENDIAN /** NOLINT(cppcoreguidelines-macro-usage) **/ \
	(!IS_HYPERION_PLATFORM_ARCHITECTURE(HYPERION_PLATFORM_Unknown))
/// @brief Whether the compiled-for architecture is a big endian architecture
/// @ingroup platform
/// @headerfile "Hyperion/Platform.h"
#define HYPERION_PLATFORM_BIG_ENDIAN /** NOLINT(cppcoreguidelines-macro-usage) **/ \
	IS_HYPERION_PLATFORM_ARCHITECTURE(HYPERION_PLATFORM_Unknown)

/// @def HYPERION_PLATFORM_CACHE_LINE_SIZE
/// @brief The architecture cache-line size
/// @ingroup platform
/// @headerfile "Hyperion/Platform.h"
#if IS_HYPERION_PLATFORM_ARCHITECTURE(HYPERION_PLATFORM_x86_64) \
	|| IS_HYPERION_PLATFORM_ARCHITECTURE(HYPERION_PLATFORM_ARMv8)
	/// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
	#define HYPERION_PLATFORM_CACHE_LINE_SIZE 64
#elif IS_HYPERION_PLATFORM_ARCHITECTURE(HYPERION_PLATFORM_x86)    \
	|| IS_HYPERION_PLATFORM_ARCHITECTURE(HYPERION_PLATFORM_ARMv7) \
	|| IS_HYPERION_PLATFORM_ARCHITECTURE(HYPERION_PLATFORM_ARMv6)
	/// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
	#define HYPERION_PLATFORM_CACHE_LINE_SIZE 32
#else
	/// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
	#define HYPERION_PLATFORM_CACHE_LINE_SIZE 128
#endif
