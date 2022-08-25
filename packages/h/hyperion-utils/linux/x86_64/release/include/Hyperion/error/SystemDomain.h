/// @file SystemDomain.h
/// @author Braxton Salyer <braxtonsalyer@gmail.com>
/// @brief `StatusCodeDomain` supporting the system's default error reporting mechanism
/// (e.g. `errno`, `GetLastError()`, etc)
/// @version 0.1
/// @date 2021-10-15
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

#include <Hyperion/HyperionDef.h>
#include <Hyperion/Platform.h>
#include <Hyperion/error/GenericDomain.h>
#include <Hyperion/error/NTDomain.h>
#include <Hyperion/error/PosixDomain.h>
#include <Hyperion/error/StatusCode.h>
#include <Hyperion/error/Win32Domain.h>

namespace hyperion::error {

	/// @def HYPERION_WINDOWS_USES_POSIX_CODES
	/// @brief Configures Hyperion to use `hyperion::error::PosixDomain` as the system
	/// `StatusCodeDomain` on Windows instead of `hyperion::error::Win32Domain`.
	///
	/// By default this is `true`. To use the Win32 status code domain instead, define this to
	/// `false` prior to including any Hyperion headers
	/// @ingroup error

	/// @def HYPERION_SYSTEM_STATUS_CODE_NOT_POSIX
	/// @brief Configures Hyperion to __NOT__ use `hyperion::error::PosixDomain` as the system
	/// `StatusCodeDomain`.
	///
	/// To enable this configuration, define `HYPERION_SYSTEM_STATUS_CODE_NOT_POSIX` to any value
	/// Normally, this would only be used on systems that do not support POSIX, but can also be used
	/// to disable it on systems that would normally use POSIX compliant error codes (`errno`),
	/// but shouldn't for your use case. In either situation, you __MUST__ define
	/// `HYPERION_SYSTEM_STATUS_CODE_DOMAIN` to the type of your desired `StatusCodeDomain`
	/// prior to including __any__ Hyperion headers other than "Hyperion/error/StatusCodeDomain.h"
	/// @ingroup error

	/// @def HYPERION_SYSTEM_STATUS_CODE_DOMAIN
	/// @brief Configures what `StatusCodeDomain` Hyperion should use as the system's default
	/// `StatusCodeDomain`.
	///
	/// This defaults to `hyperion::error::PosixDomain`. To change this, define
	/// `HYPERION_SYSTEM_STATUS_CODE_DOMAIN` to your own `StatusCodeDomain` type prior to
	/// including __any__ Hyperion headers other than "Hyperion/error/StatusCodeDomain.h"
	/// @ingroup error

#if HYPERION_PLATFORM_WINDOWS
	#ifndef HYPERION_WINDOWS_USES_POSIX_CODES
		// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
		#define HYPERION_WINDOWS_USES_POSIX_CODES true
	#endif
	#ifndef HYPERION_SYSTEM_STATUS_CODE_DOMAIN
		#if HYPERION_WINDOWS_USES_POSIX_CODES
			// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
			#define HYPERION_SYSTEM_STATUS_CODE_DOMAIN PosixDomain
		#else // HYPERION_WINDOWS_USES_POSIX_CODES
			// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
			#define HYPERION_SYSTEM_STATUS_CODE_DOMAIN Win32Domain
		#endif // HYPERION_WINDOWS_USES_POSIX_CODES
	#endif	   // HYPERION_SYSTEM_STATUS_CODE_DOMAIN
#elif !defined(HYPERION_SYSTEM_STATUS_CODE_NOT_POSIX)
	IGNORE_UNUSED_MACROS_START
	#ifndef HYPERION_WINDOWS_USES_POSIX_CODES
		// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
		#define HYPERION_WINDOWS_USES_POSIX_CODES true
	#endif
	IGNORE_UNUSED_MACROS_STOP
	#ifndef HYPERION_SYSTEM_STATUS_CODE_DOMAIN
		// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
		#define HYPERION_SYSTEM_STATUS_CODE_DOMAIN PosixDomain
	#endif // HYPERION_SYSTEM_STATUS_CODE_DOMAIN
#else
	#ifndef HYPERION_SYSTEM_STATUS_CODE_DOMAIN
		#error "HYPERION_SYSTEM_STATUS_CODE_DOMAIN MUST be defined to your system's status code \
domain prior to including \"Hyperion/error/SystemDomain\" \
(directly or transitively) when defined(HYPERION_SYSTEM_STATUS_CODE_NOT_POSIX)"
	#endif // HYPERION_SYSTEM_STATUS_CODE_DOMAIN
#endif	   // !HYPERION_PLATFORM_WINDOWS

	/// @brief The `StatusCodeDomain` to report errors from the system's default error reporting
	/// mechanisms (e.g., from `errno`)
	/// @ingroup error
	using SystemDomain = HYPERION_SYSTEM_STATUS_CODE_DOMAIN;
	/// @brief The `StatusCode<Domain>` that represents the result of an operation that uses the
	/// system's default error reporting mechanisms (e.g., from `errno`)
	/// @ingroup error
	using SystemStatusCode = StatusCode<SystemDomain>;
	/// @brief The `ErrorCode<Domain>` that represents an error resulting from an operation that
	/// uses the system's default error reporting mechanisms (e.g., from `errno`)
	/// @ingroup error
	using SystemErrorCode = ErrorCode<SystemDomain>;

	IGNORE_UNUSED_VARIABLES_START
	static const constexpr char (&SYSTEM_DOMAIN_UUID)[num_chars_in_uuid] // NOLINT
		= HYPERION_SYSTEM_STATUS_CODE_DOMAIN::UUID;
	static constexpr u64 SYSTEM_DOMAIN_ID = HYPERION_SYSTEM_STATUS_CODE_DOMAIN::ID;
	IGNORE_UNUSED_VARIABLES_STOP

	/// @brief Concept that requires that `Code` is a valid `StatusCode<Domain>` type
	/// @ingroup error
	template<template<typename Domain> typename Code, typename Domain>
	concept IsStatusCode = StatusCodeDomain<Domain> &&(
		std::same_as<StatusCode<Domain>,
					 Code<Domain>> || std::derived_from<Code<Domain>, StatusCode<Domain>>);
} // namespace hyperion::error
