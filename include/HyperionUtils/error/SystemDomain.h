#pragma once

#include "../Platform.h"
#include "GenericDomain.h"
#include "PosixDomain.h"
#include "StatusCode.h"
#include "Win32Domain.h"

namespace hyperion::error {

#if HYPERION_PLATFORM_WINDOWS
	#ifndef HYPERION_SYSTEM_STATUS_CODE_DOMAIN
		#define HYPERION_SYSTEM_STATUS_CODE_DOMAIN Win32Domain // NOLINT
	#endif													   // HYPERION_SYSTEM_STATUS_CODE_DOMAIN
#elif !defined(HYPERION_SYSTEM_STATUS_CODE_NOT_POSIX)
	#ifndef HYPERION_SYSTEM_STATUS_CODE_DOMAIN
		#define HYPERION_SYSTEM_STATUS_CODE_DOMAIN PosixDomain // NOLINT
	#endif													   // HYPERION_SYSTEM_STATUS_CODE_DOMAIN
#else
	#ifndef HYPERION_SYSTEM_STATUS_CODE_DOMAIN
		#error HYPERION_SYSTEM_STATUS_CODE_DOMAIN MUST be defined to your system's status code domain prior to including "HyperionUtils/error/SystemDomain" (directly or transitively) when defined(HYPERION_SYSTEM_STATUS_CODE_NOT_POSIX)
	#endif // HYPERION_SYSTEM_STATUS_CODE_DOMAIN
#endif	   // !HYPERION_PLATFORM_WINDOWS

	using SystemDomain = HYPERION_SYSTEM_STATUS_CODE_DOMAIN;
	using SystemStatusCode = StatusCode<SystemDomain>;
	using SystemErrorCode = ErrorCode<SystemDomain>;

	static constexpr const char (&SYSTEM_DOMAIN_UUID)[num_chars_in_uuid] // NOLINT
		= HYPERION_SYSTEM_STATUS_CODE_DOMAIN::UUID;
	static constexpr u64 SYSTEM_DOMAIN_ID = HYPERION_SYSTEM_STATUS_CODE_DOMAIN::ID;

	template<template<typename Domain> typename Code, typename Domain>
	concept IsStatusCode = StatusCodeDomain<Domain> &&(
		std::same_as<StatusCode<Domain>,
					 Code<Domain>> || std::derived_from<Code<Domain>, StatusCode<Domain>>);
} // namespace hyperion::error
