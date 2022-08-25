/// @file NTDomain.h
/// @author Braxton Salyer <braxtonsalyer@gmail.com>
/// @brief `StatusCodeDomain` supporting NT error codes
/// @version 0.1
/// @date 2021-11-10
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

#include <Hyperion/error/GenericDomain.h>
#include <Hyperion/error/StatusCode.h>
#include <Hyperion/error/Win32Domain.h>
#include <concepts>

#if HYPERION_PLATFORM_WINDOWS
namespace hyperion::error {
	namespace detail::win32 {
		using NTSTATUS = long; // NOLINT
		using HMODULE = void*;
		extern auto __stdcall GetModuleHandleW(const wchar_t* lpModuleName) -> HMODULE;
	#pragma comment(lib, "kernel32.lib")
	#if(defined(__x86_64__) || defined(_M_X64)) || (defined(__aarch64__) || defined(_M_ARM64))
		#pragma comment( \
			linker,      \
			"/alternatename:?GetModuleHandleW@win32@system_error2@@YAPEAXPEB_W@Z=GetModuleHandleW")
	#elif defined(__x86__) || defined(_M_IX86) || defined(__i386__)
		#pragma comment( \
			linker,      \
			"/alternatename:?GetModuleHandleW@win32@system_error2@@YGPAXPB_W@Z=__imp__GetModuleHandleW@4")
	#elif defined(__arm__) || defined(_M_ARM)
		#pragma comment( \
			linker,      \
			"/alternatename:?GetModuleHandleW@win32@system_error2@@YAPAXPB_W@Z=GetModuleHandleW")
	#else
		#error Unknown architecture
	#endif
	} // namespace detail::win32

	class NTDomain;
	using NTStatusCode = StatusCode<NTDomain>;
	using NTErrorCode = ErrorCode<NTDomain>;

	/// @brief `NTDomain` is the `StatusCodeDomain` that covers Windows NT error codes
	/// @ingroup error
	/// @headerfile "Hyperion/error/NTDomain.h"
	class [[nodiscard("A StatusCodeDomain should always be used")]] NTDomain {
	  public:
		/// @brief The value type of `NTDomain` status codes is `NTSTATUS`
		/// @ingroup error
		using value_type = detail::win32::NTSTATUS;

		static const constexpr char(&UUID)[num_chars_in_uuid] // NOLINT
			= "2045f27b-499a-4bf8-9b12-3bd13a81bbb0";

		static constexpr u64 ID = parse_uuid_from_string(UUID);

		/// @brief Constructs a `NTDomain` with the default UUID
		/// @ingroup error
		constexpr NTDomain() noexcept = default;
		/// @brief Constructs a `NTDomain` with a user-specific UUID
		///
		/// @note When using a custom UUID __**ALL**__ instances of `NTDomain` in the program
		/// should be constructed with the same custom UUID, otherwise equality comparison between
		/// other domains and `NTDomain` instances could give erroneous results, and equality
		/// comparison between different `NTDomain` instances will give erroneous results.
		/// As a result, this constructor should only be used when you specifically require a custom
		/// UUID and **YOU KNOW WHAT YOU ARE DOING™**
		///
		/// @param uuid - The UUID to use for `NTDomain`
		/// @ingroup error
		explicit constexpr NTDomain(u64 uuid) noexcept : m_uuid(uuid) {
		}
		/// @brief Constructs a `NTDomain` with a user-specific UUID
		///
		/// @note When using a custom UUID __**ALL**__ instances of `NTDomain` in the program
		/// should be constructed with the same custom UUID, otherwise equality comparison between
		/// other domains and `NTDomain` instances could give erroneous results, and equality
		/// comparison between different `NTDomain` instances will give erroneous results.
		/// As a result, this constructor should only be used when you specifically require a custom
		/// UUID and **YOU KNOW WHAT YOU ARE DOING™**
		///
		/// @param uuid - The UUID to use for `NTDomain`
		/// @ingroup error
		template<UUIDString UUID>
		explicit constexpr NTDomain(UUID && uuid) noexcept // NOLINT (forwarding reference)
			: m_uuid(parse_uuid_from_string(std::forward<UUID>(uuid))) {
		}
		/// @brief Copy-Constructor
		/// @ingroup error
		constexpr NTDomain(const NTDomain&) noexcept = default;
		/// @brief Move-Constructor
		/// @ingroup error
		constexpr NTDomain(NTDomain &&) noexcept = default;
		/// @brief Destructor
		/// @ingroup error
		constexpr ~NTDomain() noexcept = default;

		/// @brief Returns the UUID of the domain
		///
		/// @return the domain UUID
		/// @ingroup error
		[[nodiscard]] constexpr auto id() const noexcept->u64 {
			return m_uuid;
		}

		/// @brief Returns the name of the domain
		///
		/// @return the domain name
		/// @ingroup error
		[[nodiscard]] constexpr auto name() const noexcept->std::string_view { // NOLINT
			return "nt domain";
		}

		/// @brief Returns the textual message associated with the given status code
		///
		/// @param code - The status code to get the message for
		///
		/// @return the message associated with the code
		/// @ingroup error
		[[nodiscard]] auto message(value_type code) // NOLINT
			const noexcept->std::string {
			return as_string(code);
		}

		/// @brief Returns the textual message associated with the given status code
		///
		/// @param code - The status code to get the message for
		///
		/// @return the message associated with the code
		/// @ingroup error
		[[nodiscard]] auto message(const NTStatusCode& code) // NOLINT
			const noexcept->std::string {
			return as_string(code.code());
		}

		/// @brief Returns whether the given status code represents an error
		///
		/// @param code - The status code to check
		///
		/// @return `true` if the code represents an error, otherwise `false`
		/// @ingroup error
		[[nodiscard]] constexpr auto is_error(const NTStatusCode& code) // NOLINT (could-b-static)
			const noexcept->bool {
			return code.code() != 0;
		}

		/// @brief Returns whether the given status code represents success
		///
		/// @param code - The status code to check
		///
		/// @return `true` if the code represents success, otherwise `false`
		/// @ingroup error
		[[nodiscard]] constexpr auto is_success(const NTStatusCode& code) // NOLINT (could-b-static)
			const noexcept->bool {
			return code.code() == 0;
		}

		/// @brief Returns whether the given status codes are semantically equivalent
		///
		/// Checks if the given codes are semantically equivalent. For most `StatusCodeDomain`s,
		/// this usually means checking the codes for equality after being converted to
		/// `GenericStatusCode`s.
		///
		/// @tparam Domain - The `StatusCodeDomain` of the second status code
		/// @param lhs - The first status code to compare
		/// @param rhs - The second status code to compare
		/// @return `true` if the codes are semantically equivalent, `false` otherwise
		/// @ingroup error
		template<typename Domain>
		[[nodiscard]] constexpr auto are_equivalent(const NTStatusCode& lhs,
													const StatusCode<Domain>& rhs)
			const noexcept->bool {
			if constexpr(ConvertibleToGenericStatusCode<StatusCode<Domain>>) {
				return as_generic_code(lhs) == rhs.as_generic_code();
			}
			else if constexpr(std::same_as<StatusCode<Domain>, Win32StatusCode>) {
				return as_win32_code(lhs) == rhs;
			}
			else if(rhs.domain() == *this) {
				const auto lhs_code = lhs.code();
				const auto rhs_code = rhs.code();
				return lhs_code == rhs_code && lhs_code != -1 && rhs_code != -1;
			}
			else {
				return false;
			}
		}

		/// @brief Converts the given status code to a `GenericStatusCode`
		///
		/// This will convert the given code to its semantically equivalent counterpart in the
		/// `GenericDomain`.
		///
		/// @param code - The status code to convert to a `GenericStatusCode`
		/// @return The given status code as a `GenericStatusCode`
		/// @note Not all status code values are convertible to the `GenericDomain`, even
		/// from domains fully compatible with `GenericDomain` and that satisfy
		/// `ConvertibleToGenericStatusCode`. In this case, they will map to `Errno::Unknown`.
		/// Codes of value `Errno::Unknown` will never compare as semantically equivalent.
		/// @ingroup error
		[[nodiscard]] constexpr auto as_generic_code(const NTStatusCode& code) // NOLINT
			const noexcept->GenericStatusCode {
			return make_status_code(to_generic_code(code.code()));
		}

		/// @brief Converts the given status code to a `Win32StatusCode`
		///
		/// This will convert the given code to its semantically equivalent counterpart in the
		/// `Win32Domain`.
		///
		/// @param code - The status code to convert to a `Win32StatusCode`
		/// @return The given status code as a `Win32StatusCode`
		/// @note Not all NT status code values are convertible to the `Win32Domain`. In the case
		/// of an incompatible code, it will map to `-1` (unknown error).
		/// Codes of value `-1` (unknown error) will never compare as semantically equivalent.
		/// @ingroup error
		[[nodiscard]] constexpr auto as_win32_code(const NTStatusCode& code) // NOLINT
			const noexcept->Win32StatusCode {
			return Win32StatusCode(to_win32_code(code.code()));
		}

		/// @brief Returns the value indicating success for this domain
		///
		/// @return The domain's success value
		/// @ingroup error
		[[nodiscard]] static inline constexpr auto success_value() noexcept->value_type {
			return 0;
		}

		/// @brief Domain equality comparison operator
		///
		/// @tparam Domain - The type of the second domain to compare
		///
		/// @param lhs - The left-hand domain to compare
		/// @param rhs - The right-hand domain to compare
		///
		/// @return Whether the two domains are equal
		/// @ingroup error
		template<typename Domain>
		friend constexpr auto operator==(const NTDomain& lhs, const Domain& rhs) noexcept->bool {
			return lhs.id() == rhs.id();
		}

		/// @brief Domain inequality comparison operator
		///
		/// @tparam Domain - The type of the second domain to compare
		///
		/// @param lhs - The left-hand domain to compare
		/// @param rhs - The right-hand domain to compare
		///
		/// @return Whether the two domains are __not__ equal
		/// @ingroup error
		template<typename Domain>
		friend constexpr auto operator!=(const NTDomain& lhs, const Domain& rhs) noexcept->bool {
			return lhs.id() != rhs.id();
		}

		/// @brief Copy-assignment operator
		/// @ingroup error
		constexpr auto operator=(const NTDomain&) noexcept->NTDomain& = default;
		/// @brief Move-assignment operator
		/// @ingroup error
		constexpr auto operator=(NTDomain&&) noexcept->NTDomain& = default;

	  private:
		u64 m_uuid = ID;

		/// @brief Gets the message string associated with the given NT error code
		///
		/// This is largely based on Nial Douglas's implementation of `std::status_code` for
		/// WG21 proposal P1028, [status_code](https://github.com/ned14/status-code)
		/// @param code - The NT error code to convert
		///
		/// @return the message string associated with `code`, as a `std::string`
		static inline auto as_string(value_type code) noexcept->std::string {
			// Nial uses 32k in his implementation for proposal P1028, but that seems
			// excessively large
			wchar_t buffer[1024]; // NOLINT
			static const detail::win32::HMODULE ntdll
				= detail::win32::GetModuleHandleW(L"NTDLL.DLL");

			detail::win32::DWORD wide_length = detail::win32::FormatMessageW(
				0x00000800 /*FORMAT_MESSAGE_FROM_HMODULE*/			// NOLINT
					| 0x00001000 /*FORMAT_MESSAGE_FROM_SYSTEM*/		// NOLINT
					| 0x00000200 /*FORMAT_MESSAGE_IGNORE_INSERTS*/, // NOLINT
				ntdll,
				static_cast<detail::win32::DWORD>(code),
				(1U << 10U) /*MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT)*/, // NOLINT
				buffer,													   // NOLINT
				1024,													   // NOLINT
				nullptr);

			auto alloc_size = wide_length * 2_usize;
			detail::win32::DWORD err = 0;
			// try to convert the "wide" error message string (UTF16, or UCS-2) to
			// "narrow/normal" UTF8
			do {
				auto* p = static_cast<char*>(malloc(alloc_size)); // NOLINT
				// if memory allocation fails, bail
				if(p == nullptr) {
					return "failed to get message from system";
				}

				// attempt to convert the "wide" error message string (UTF16, or UCS-2) to
				// "narrow/normal" UTF8
				auto bytes = static_cast<detail::win32::DWORD>(
					detail::win32::WideCharToMultiByte(65001 /**CP_UTF8**/, // NOLINT
													   0,
													   buffer, // NOLINT
													   static_cast<int>(wide_length + 1),
													   p,
													   static_cast<int>(alloc_size),
													   nullptr,
													   nullptr));
				// if conversion succeeded, trim the string and return it
				if(bytes != 0) {
					char* end = strchr(p, 0); // NOLINT
					// We just want the primary error message, so trim all but the first line
					while(end[-1] == '\n' || end[-1] == '\r') { // NOLINT
						--end;									// NOLINT
					}
					*end = '\0';
					auto str = std::string(p, static_cast<usize>(end - p));
					free(p); // NOLINT
					return str;
				}

				free(p); // NOLINT

				// increase the allocation size and try again on the next iteration
				alloc_size += alloc_size >> 2U;
				// or bail if an error other than insufficient buffer size occurred
				err = detail::win32::GetLastError();
			} while(err == 0x7a /**ERROR_INSUFFICIENT_BUFFER**/); // NOLINT

			return "failed to get message from system";
		}

		[[nodiscard]] static inline constexpr auto to_generic_code(
			value_type code) noexcept->Errno {
			if(code >= 0) {
				return Errno::Success;
			}
			switch(static_cast<u64>(code)) {
				case 0x80000002: return Errno::PermissionDenied;			// NOLINT
				case 0x8000000f: return Errno::ResourceUnavailableTryAgain; // NOLINT
				case 0x80000010: return Errno::ResourceUnavailableTryAgain; // NOLINT
				case 0x80000011: return Errno::DeviceOrResourceBusy;		// NOLINT
				case 0xc0000002: return Errno::FunctionNotSupported;		// NOLINT
				case 0xc0000005: return Errno::PermissionDenied;			// NOLINT
				case 0xc0000008: return Errno::InvalidArgument;				// NOLINT
				case 0xc000000e: return Errno::NoSuchFileOrDirectory;		// NOLINT
				case 0xc000000f: return Errno::NoSuchFileOrDirectory;		// NOLINT
				case 0xc0000010: return Errno::FunctionNotSupported;		// NOLINT
				case 0xc0000013: return Errno::ResourceUnavailableTryAgain; // NOLINT
				case 0xc0000017: return Errno::NotEnoughMemory;				// NOLINT
				case 0xc000001c: return Errno::FunctionNotSupported;		// NOLINT
				case 0xc000001e: return Errno::PermissionDenied;			// NOLINT
				case 0xc000001f: return Errno::PermissionDenied;			// NOLINT
				case 0xc0000021: return Errno::PermissionDenied;			// NOLINT
				case 0xc0000022: return Errno::PermissionDenied;			// NOLINT
				case 0xc0000024: return Errno::InvalidArgument;				// NOLINT
				case 0xc0000033: return Errno::InvalidArgument;				// NOLINT
				case 0xc0000034: return Errno::NoSuchFileOrDirectory;		// NOLINT
				case 0xc0000035: return Errno::FileExists;					// NOLINT
				case 0xc0000037: return Errno::InvalidArgument;				// NOLINT
				case 0xc000003a: return Errno::NoSuchFileOrDirectory;		// NOLINT
				case 0xc0000040: return Errno::NotEnoughMemory;				// NOLINT
				case 0xc0000041: return Errno::PermissionDenied;			// NOLINT
				case 0xc0000042: return Errno::InvalidArgument;				// NOLINT
				case 0xc0000043: return Errno::PermissionDenied;			// NOLINT
				case 0xc000004b: return Errno::PermissionDenied;			// NOLINT
				case 0xc0000054: return Errno::NoLockAvailable;				// NOLINT
				case 0xc0000055: return Errno::NoLockAvailable;				// NOLINT
				case 0xc0000056: return Errno::PermissionDenied;			// NOLINT
				case 0xc000007f: return Errno::NoSpaceOnDevice;				// NOLINT
				case 0xc0000087: return Errno::NotEnoughMemory;				// NOLINT
				case 0xc0000097: return Errno::NotEnoughMemory;				// NOLINT
				case 0xc000009b: return Errno::NoSuchFileOrDirectory;		// NOLINT
				case 0xc000009e: return Errno::ResourceUnavailableTryAgain; // NOLINT
				case 0xc00000a2: return Errno::PermissionDenied;			// NOLINT
				case 0xc00000a3: return Errno::ResourceUnavailableTryAgain; // NOLINT
				case 0xc00000af: return Errno::FunctionNotSupported;		// NOLINT
				case 0xc00000ba: return Errno::PermissionDenied;			// NOLINT
				case 0xc00000c0: return Errno::NoSuchDevice;				// NOLINT
				case 0xc00000d4: return Errno::CrossDeviceLink;				// NOLINT
				case 0xc00000d5: return Errno::PermissionDenied;			// NOLINT
				case 0xc00000fb: return Errno::NoSuchFileOrDirectory;		// NOLINT
				case 0xc0000101: return Errno::DirectoryNotEmpty;			// NOLINT
				case 0xc0000103: return Errno::InvalidArgument;				// NOLINT
				case 0xc0000107: return Errno::DeviceOrResourceBusy;		// NOLINT
				case 0xc0000108: return Errno::DeviceOrResourceBusy;		// NOLINT
				case 0xc000010a: return Errno::PermissionDenied;			// NOLINT
				case 0xc000011f: return Errno::TooManyFilesOpen;			// NOLINT
				case 0xc0000120: return Errno::OperationCanceled;			// NOLINT
				case 0xc0000121: return Errno::PermissionDenied;			// NOLINT
				case 0xc0000123: return Errno::PermissionDenied;			// NOLINT
				case 0xc0000128: return Errno::InvalidArgument;				// NOLINT
				case 0xc0000189: return Errno::PermissionDenied;			// NOLINT
				case 0xc00001ad: return Errno::NotEnoughMemory;				// NOLINT
				case 0xc000022d: return Errno::ResourceUnavailableTryAgain; // NOLINT
				case 0xc0000235: return Errno::InvalidArgument;				// NOLINT
				case 0xc000026e: return Errno::ResourceUnavailableTryAgain; // NOLINT
				case 0xc000028a: return Errno::PermissionDenied;			// NOLINT
				case 0xc000028b: return Errno::PermissionDenied;			// NOLINT
				case 0xc000028d: return Errno::PermissionDenied;			// NOLINT
				case 0xc000028e: return Errno::PermissionDenied;			// NOLINT
				case 0xc000028f: return Errno::PermissionDenied;			// NOLINT
				case 0xc0000290: return Errno::PermissionDenied;			// NOLINT
				case 0xc000029c: return Errno::FunctionNotSupported;		// NOLINT
				case 0xc00002c5: return Errno::PermissionDenied;			// NOLINT
				case 0xc00002d3: return Errno::ResourceUnavailableTryAgain; // NOLINT
				case 0xc00002ea: return Errno::PermissionDenied;			// NOLINT
				case 0xc00002f0: return Errno::NoSuchFileOrDirectory;		// NOLINT
				case 0xc0000373: return Errno::NotEnoughMemory;				// NOLINT
				case 0xc0000416: return Errno::NotEnoughMemory;				// NOLINT
				case 0xc0000433: return Errno::DeviceOrResourceBusy;		// NOLINT
				case 0xc0000434: return Errno::DeviceOrResourceBusy;		// NOLINT
				case 0xc0000455: return Errno::InvalidArgument;				// NOLINT
				case 0xc0000467: return Errno::PermissionDenied;			// NOLINT
				case 0xc0000491: return Errno::NoSuchFileOrDirectory;		// NOLINT
				case 0xc0000495: return Errno::ResourceUnavailableTryAgain; // NOLINT
				case 0xc0000503: return Errno::ResourceUnavailableTryAgain; // NOLINT
				case 0xc0000507: return Errno::DeviceOrResourceBusy;		// NOLINT
				case 0xc0000512: return Errno::PermissionDenied;			// NOLINT
				case 0xc000070a: return Errno::InvalidArgument;				// NOLINT
				case 0xc000070b: return Errno::InvalidArgument;				// NOLINT
				case 0xc000070c: return Errno::InvalidArgument;				// NOLINT
				case 0xc000070d: return Errno::InvalidArgument;				// NOLINT
				case 0xc000070e: return Errno::InvalidArgument;				// NOLINT
				case 0xc000070f: return Errno::InvalidArgument;				// NOLINT
				case 0xc0000710: return Errno::FunctionNotSupported;		// NOLINT
				case 0xc0000711: return Errno::FunctionNotSupported;		// NOLINT
				case 0xc0000716: return Errno::InvalidArgument;				// NOLINT
				case 0xc000071b: return Errno::FunctionNotSupported;		// NOLINT
				case 0xc000071d: return Errno::FunctionNotSupported;		// NOLINT
				case 0xc000071e: return Errno::FunctionNotSupported;		// NOLINT
				case 0xc000071f: return Errno::FunctionNotSupported;		// NOLINT
				case 0xc0000720: return Errno::FunctionNotSupported;		// NOLINT
				case 0xc0000721: return Errno::FunctionNotSupported;		// NOLINT
				case 0xc000080f: return Errno::ResourceUnavailableTryAgain; // NOLINT
				case 0xc000a203: return Errno::PermissionDenied;			// NOLINT
				default: return Errno::Unknown;
			}
		}

		// NOLINTNEXTLINE (function-size)
		[[nodiscard]] static inline constexpr auto to_win32_code(
			value_type code) noexcept->Win32Domain::value_type {
			if(code >= 0) {
				return 0;
			}
			switch(static_cast<u64>(code)) {
				case 0x80000002: return 0x3e6;	// NOLINT
				case 0x80000005: return 0xea;	// NOLINT
				case 0x80000006: return 0x12;	// NOLINT
				case 0x80000007: return 0x2a3;	// NOLINT
				case 0x8000000a: return 0x2a4;	// NOLINT
				case 0x8000000b: return 0x56f;	// NOLINT
				case 0x8000000c: return 0x2a8;	// NOLINT
				case 0x8000000d: return 0x12b;	// NOLINT
				case 0x8000000e: return 0x1c;	// NOLINT
				case 0x8000000f: return 0x15;	// NOLINT
				case 0x80000010: return 0x15;	// NOLINT
				case 0x80000011: return 0xaa;	// NOLINT
				case 0x80000012: return 0x103;	// NOLINT
				case 0x80000013: return 0xfe;	// NOLINT
				case 0x80000014: return 0xff;	// NOLINT
				case 0x80000015: return 0xff;	// NOLINT
				case 0x80000016: return 0x456;	// NOLINT
				case 0x80000017: return 0x2a5;	// NOLINT
				case 0x80000018: return 0x2a6;	// NOLINT
				case 0x8000001a: return 0x103;	// NOLINT
				case 0x8000001b: return 0x44d;	// NOLINT
				case 0x8000001c: return 0x456;	// NOLINT
				case 0x8000001d: return 0x457;	// NOLINT
				case 0x8000001e: return 0x44c;	// NOLINT
				case 0x8000001f: return 0x44e;	// NOLINT
				case 0x80000020: return 0x2a7;	// NOLINT
				case 0x80000021: return 0x44f;	// NOLINT
				case 0x80000022: return 0x450;	// NOLINT
				case 0x80000023: return 0x702;	// NOLINT
				case 0x80000024: return 0x713;	// NOLINT
				case 0x80000025: return 0x962;	// NOLINT
				case 0x80000026: return 0x2aa;	// NOLINT
				case 0x80000027: return 0x10f4; // NOLINT
				case 0x80000028: return 0x2ab;	// NOLINT
				case 0x80000029: return 0x2ac;	// NOLINT
				case 0x8000002a: return 0x2ad;	// NOLINT
				case 0x8000002b: return 0x2ae;	// NOLINT
				case 0x8000002c: return 0x2af;	// NOLINT
				case 0x8000002d: return 0x2a9;	// NOLINT
				case 0x8000002e: return 0x321;	// NOLINT
				case 0x8000002f: return 0x324;	// NOLINT
				case 0x80000030: return 0xab;	// NOLINT
				case 0x80000032: return 0xeb;	// NOLINT
				case 0x80000288: return 0x48d;	// NOLINT
				case 0x80000289: return 0x48e;	// NOLINT
				case 0x80000803: return 0x1abb; // NOLINT
				case 0x8000a127: return 0x3bdf; // NOLINT
				case 0x8000cf00: return 0x16e;	// NOLINT
				case 0x8000cf04: return 0x16d;	// NOLINT
				case 0x8000cf05: return 0x176;	// NOLINT
				case 0x80130001: return 0x13c5; // NOLINT
				case 0x80130002: return 0x13c6; // NOLINT
				case 0x80130003: return 0x13c7; // NOLINT
				case 0x80130004: return 0x13c8; // NOLINT
				case 0x80130005: return 0x13c9; // NOLINT
				case 0x80190009: return 0x19e5; // NOLINT
				case 0x80190029: return 0x1aa0; // NOLINT
				case 0x80190031: return 0x1aa2; // NOLINT
				case 0x80190041: return 0x1ab3; // NOLINT
				case 0x80190042: return 0x1ab4; // NOLINT
				case 0x801c0001: return 0x7a;	// NOLINT
				case 0xc0000001: return 0x1f;	// NOLINT
				case 0xc0000002: return 0x1;	// NOLINT
				case 0xc0000003: return 0x57;	// NOLINT
				case 0xc0000004: return 0x18;	// NOLINT
				case 0xc0000005: return 0x3e6;	// NOLINT
				case 0xc0000006: return 0x3e7;	// NOLINT
				case 0xc0000007: return 0x5ae;	// NOLINT
				case 0xc0000008: return 0x6;	// NOLINT
				case 0xc0000009: return 0x3e9;	// NOLINT
				case 0xc000000a: return 0xc1;	// NOLINT
				case 0xc000000b: return 0x57;	// NOLINT
				case 0xc000000c: return 0x21d;	// NOLINT
				case 0xc000000d: return 0x57;	// NOLINT
				case 0xc000000e: return 0x2;	// NOLINT
				case 0xc000000f: return 0x2;	// NOLINT
				case 0xc0000010: return 0x1;	// NOLINT
				case 0xc0000011: return 0x26;	// NOLINT
				case 0xc0000012: return 0x22;	// NOLINT
				case 0xc0000013: return 0x15;	// NOLINT
				case 0xc0000014: return 0x6f9;	// NOLINT
				case 0xc0000015: return 0x1b;	// NOLINT
				case 0xc0000016: return 0xea;	// NOLINT
				case 0xc0000017: return 0x8;	// NOLINT
				case 0xc0000018: return 0x1e7;	// NOLINT
				case 0xc0000019: return 0x1e7;	// NOLINT
				case 0xc000001a: return 0x57;	// NOLINT
				case 0xc000001b: return 0x57;	// NOLINT
				case 0xc000001c: return 0x1;	// NOLINT
				case 0xc000001e: return 0x5;	// NOLINT
				case 0xc000001f: return 0x5;	// NOLINT
				case 0xc0000020: return 0xc1;	// NOLINT
				case 0xc0000021: return 0x5;	// NOLINT
				case 0xc0000022: return 0x5;	// NOLINT
				case 0xc0000023: return 0x7a;	// NOLINT
				case 0xc0000024: return 0x6;	// NOLINT
				case 0xc0000027: return 0x21e;	// NOLINT
				case 0xc0000028: return 0x21f;	// NOLINT
				case 0xc0000029: return 0x220;	// NOLINT
				case 0xc000002a: return 0x9e;	// NOLINT
				case 0xc000002c: return 0x1e7;	// NOLINT
				case 0xc000002d: return 0x1e7;	// NOLINT
				case 0xc000002e: return 0x221;	// NOLINT
				case 0xc000002f: return 0x222;	// NOLINT
				case 0xc0000030: return 0x57;	// NOLINT
				case 0xc0000031: return 0x223;	// NOLINT
				case 0xc0000032: return 0x571;	// NOLINT
				case 0xc0000033: return 0x7b;	// NOLINT
				case 0xc0000034: return 0x2;	// NOLINT
				case 0xc0000035: return 0xb7;	// NOLINT
				case 0xc0000036: return 0x72a;	// NOLINT
				case 0xc0000037: return 0x6;	// NOLINT
				case 0xc0000038: return 0x224;	// NOLINT
				case 0xc0000039: return 0xa1;	// NOLINT
				case 0xc000003a: return 0x3;	// NOLINT
				case 0xc000003b: return 0xa1;	// NOLINT
				case 0xc000003c: return 0x45d;	// NOLINT
				case 0xc000003d: return 0x45d;	// NOLINT
				case 0xc000003e: return 0x17;	// NOLINT
				case 0xc000003f: return 0x17;	// NOLINT
				case 0xc0000040: return 0x8;	// NOLINT
				case 0xc0000041: return 0x5;	// NOLINT
				case 0xc0000042: return 0x6;	// NOLINT
				case 0xc0000043: return 0x20;	// NOLINT
				case 0xc0000044: return 0x718;	// NOLINT
				case 0xc0000045: return 0x57;	// NOLINT
				case 0xc0000046: return 0x120;	// NOLINT
				case 0xc0000047: return 0x12a;	// NOLINT
				case 0xc0000048: return 0x57;	// NOLINT
				case 0xc0000049: return 0x57;	// NOLINT
				case 0xc000004a: return 0x9c;	// NOLINT
				case 0xc000004b: return 0x5;	// NOLINT
				case 0xc000004c: return 0x57;	// NOLINT
				case 0xc000004d: return 0x57;	// NOLINT
				case 0xc000004e: return 0x57;	// NOLINT
				case 0xc000004f: return 0x11a;	// NOLINT
				case 0xc0000050: return 0xff;	// NOLINT
				case 0xc0000051: return 0x570;	// NOLINT
				case 0xc0000052: return 0x570;	// NOLINT
				case 0xc0000053: return 0x570;	// NOLINT
				case 0xc0000054: return 0x21;	// NOLINT
				case 0xc0000055: return 0x21;	// NOLINT
				case 0xc0000056: return 0x5;	// NOLINT
				case 0xc0000057: return 0x32;	// NOLINT
				case 0xc0000058: return 0x519;	// NOLINT
				case 0xc0000059: return 0x51a;	// NOLINT
				case 0xc000005a: return 0x51b;	// NOLINT
				case 0xc000005b: return 0x51c;	// NOLINT
				case 0xc000005c: return 0x51d;	// NOLINT
				case 0xc000005d: return 0x51e;	// NOLINT
				case 0xc000005e: return 0x51f;	// NOLINT
				case 0xc000005f: return 0x520;	// NOLINT
				case 0xc0000060: return 0x521;	// NOLINT
				case 0xc0000061: return 0x522;	// NOLINT
				case 0xc0000062: return 0x523;	// NOLINT
				case 0xc0000063: return 0x524;	// NOLINT
				case 0xc0000064: return 0x525;	// NOLINT
				case 0xc0000065: return 0x526;	// NOLINT
				case 0xc0000066: return 0x527;	// NOLINT
				case 0xc0000067: return 0x528;	// NOLINT
				case 0xc0000068: return 0x529;	// NOLINT
				case 0xc0000069: return 0x52a;	// NOLINT
				case 0xc000006a: return 0x56;	// NOLINT
				case 0xc000006b: return 0x52c;	// NOLINT
				case 0xc000006c: return 0x52d;	// NOLINT
				case 0xc000006d: return 0x52e;	// NOLINT
				case 0xc000006e: return 0x52f;	// NOLINT
				case 0xc000006f: return 0x530;	// NOLINT
				case 0xc0000070: return 0x531;	// NOLINT
				case 0xc0000071: return 0x532;	// NOLINT
				case 0xc0000072: return 0x533;	// NOLINT
				case 0xc0000073: return 0x534;	// NOLINT
				case 0xc0000074: return 0x535;	// NOLINT
				case 0xc0000075: return 0x536;	// NOLINT
				case 0xc0000076: return 0x537;	// NOLINT
				case 0xc0000077: return 0x538;	// NOLINT
				case 0xc0000078: return 0x539;	// NOLINT
				case 0xc0000079: return 0x53a;	// NOLINT
				case 0xc000007a: return 0x7f;	// NOLINT
				case 0xc000007b: return 0xc1;	// NOLINT
				case 0xc000007c: return 0x3f0;	// NOLINT
				case 0xc000007d: return 0x53c;	// NOLINT
				case 0xc000007e: return 0x9e;	// NOLINT
				case 0xc000007f: return 0x70;	// NOLINT
				case 0xc0000080: return 0x53d;	// NOLINT
				case 0xc0000081: return 0x53e;	// NOLINT
				case 0xc0000082: return 0x44;	// NOLINT
				case 0xc0000083: return 0x103;	// NOLINT
				case 0xc0000084: return 0x53f;	// NOLINT
				case 0xc0000085: return 0x103;	// NOLINT
				case 0xc0000086: return 0x9a;	// NOLINT
				case 0xc0000087: return 0xe;	// NOLINT
				case 0xc0000088: return 0x1e7;	// NOLINT
				case 0xc0000089: return 0x714;	// NOLINT
				case 0xc000008a: return 0x715;	// NOLINT
				case 0xc000008b: return 0x716;	// NOLINT
				case 0xc0000095: return 0x216;	// NOLINT
				case 0xc0000097: return 0x8;	// NOLINT
				case 0xc0000098: return 0x3ee;	// NOLINT
				case 0xc0000099: return 0x540;	// NOLINT
				case 0xc000009a: return 0x5aa;	// NOLINT
				case 0xc000009b: return 0x3;	// NOLINT
				case 0xc000009c: return 0x17;	// NOLINT
				case 0xc000009d: return 0x48f;	// NOLINT
				case 0xc000009e: return 0x15;	// NOLINT
				case 0xc000009f: return 0x1e7;	// NOLINT
				case 0xc00000a0: return 0x1e7;	// NOLINT
				case 0xc00000a1: return 0x5ad;	// NOLINT
				case 0xc00000a2: return 0x13;	// NOLINT
				case 0xc00000a3: return 0x15;	// NOLINT
				case 0xc00000a4: return 0x541;	// NOLINT
				case 0xc00000a5: return 0x542;	// NOLINT
				case 0xc00000a6: return 0x543;	// NOLINT
				case 0xc00000a7: return 0x544;	// NOLINT
				case 0xc00000a8: return 0x545;	// NOLINT
				case 0xc00000a9: return 0x57;	// NOLINT
				case 0xc00000aa: return 0x225;	// NOLINT
				case 0xc00000ab: return 0xe7;	// NOLINT
				case 0xc00000ac: return 0xe7;	// NOLINT
				case 0xc00000ad: return 0xe6;	// NOLINT
				case 0xc00000ae: return 0xe7;	// NOLINT
				case 0xc00000af: return 0x1;	// NOLINT
				case 0xc00000b0: return 0xe9;	// NOLINT
				case 0xc00000b1: return 0xe8;	// NOLINT
				case 0xc00000b2: return 0x217;	// NOLINT
				case 0xc00000b3: return 0x218;	// NOLINT
				case 0xc00000b4: return 0xe6;	// NOLINT
				case 0xc00000b5: return 0x79;	// NOLINT
				case 0xc00000b6: return 0x26;	// NOLINT
				case 0xc00000b7: return 0x226;	// NOLINT
				case 0xc00000b8: return 0x227;	// NOLINT
				case 0xc00000b9: return 0x228;	// NOLINT
				case 0xc00000ba: return 0x5;	// NOLINT
				case 0xc00000bb: return 0x32;	// NOLINT
				case 0xc00000bc: return 0x33;	// NOLINT
				case 0xc00000bd: return 0x34;	// NOLINT
				case 0xc00000be: return 0x35;	// NOLINT
				case 0xc00000bf: return 0x36;	// NOLINT
				case 0xc00000c0: return 0x37;	// NOLINT
				case 0xc00000c1: return 0x38;	// NOLINT
				case 0xc00000c2: return 0x39;	// NOLINT
				case 0xc00000c3: return 0x3a;	// NOLINT
				case 0xc00000c4: return 0x3b;	// NOLINT
				case 0xc00000c5: return 0x3c;	// NOLINT
				case 0xc00000c6: return 0x3d;	// NOLINT
				case 0xc00000c7: return 0x3e;	// NOLINT
				case 0xc00000c8: return 0x3f;	// NOLINT
				case 0xc00000c9: return 0x40;	// NOLINT
				case 0xc00000ca: return 0x41;	// NOLINT
				case 0xc00000cb: return 0x42;	// NOLINT
				case 0xc00000cc: return 0x43;	// NOLINT
				case 0xc00000cd: return 0x44;	// NOLINT
				case 0xc00000ce: return 0x45;	// NOLINT
				case 0xc00000cf: return 0x46;	// NOLINT
				case 0xc00000d0: return 0x47;	// NOLINT
				case 0xc00000d1: return 0x48;	// NOLINT
				case 0xc00000d2: return 0x58;	// NOLINT
				case 0xc00000d3: return 0x229;	// NOLINT
				case 0xc00000d4: return 0x11;	// NOLINT
				case 0xc00000d5: return 0x5;	// NOLINT
				case 0xc00000d6: return 0xf0;	// NOLINT
				case 0xc00000d7: return 0x546;	// NOLINT
				case 0xc00000d8: return 0x22a;	// NOLINT
				case 0xc00000d9: return 0xe8;	// NOLINT
				case 0xc00000da: return 0x547;	// NOLINT
				case 0xc00000db: return 0x22b;	// NOLINT
				case 0xc00000dc: return 0x548;	// NOLINT
				case 0xc00000dd: return 0x549;	// NOLINT
				case 0xc00000de: return 0x54a;	// NOLINT
				case 0xc00000df: return 0x54b;	// NOLINT
				case 0xc00000e0: return 0x54c;	// NOLINT
				case 0xc00000e1: return 0x54d;	// NOLINT
				case 0xc00000e2: return 0x12c;	// NOLINT
				case 0xc00000e3: return 0x12d;	// NOLINT
				case 0xc00000e4: return 0x54e;	// NOLINT
				case 0xc00000e5: return 0x54f;	// NOLINT
				case 0xc00000e6: return 0x550;	// NOLINT
				case 0xc00000e7: return 0x551;	// NOLINT
				case 0xc00000e8: return 0x6f8;	// NOLINT
				case 0xc00000e9: return 0x45d;	// NOLINT
				case 0xc00000ea: return 0x22c;	// NOLINT
				case 0xc00000eb: return 0x22d;	// NOLINT
				case 0xc00000ec: return 0x22e;	// NOLINT
				case 0xc00000ed: return 0x552;	// NOLINT
				case 0xc00000ee: return 0x553;	// NOLINT
				case 0xc00000ef: return 0x57;	// NOLINT
				case 0xc00000f0: return 0x57;	// NOLINT
				case 0xc00000f1: return 0x57;	// NOLINT
				case 0xc00000f2: return 0x57;	// NOLINT
				case 0xc00000f3: return 0x57;	// NOLINT
				case 0xc00000f4: return 0x57;	// NOLINT
				case 0xc00000f5: return 0x57;	// NOLINT
				case 0xc00000f6: return 0x57;	// NOLINT
				case 0xc00000f7: return 0x57;	// NOLINT
				case 0xc00000f8: return 0x57;	// NOLINT
				case 0xc00000f9: return 0x57;	// NOLINT
				case 0xc00000fa: return 0x57;	// NOLINT
				case 0xc00000fb: return 0x3;	// NOLINT
				case 0xc00000fc: return 0x420;	// NOLINT
				case 0xc00000fd: return 0x3e9;	// NOLINT
				case 0xc00000fe: return 0x554;	// NOLINT
				case 0xc00000ff: return 0x22f;	// NOLINT
				case 0xc0000100: return 0xcb;	// NOLINT
				case 0xc0000101: return 0x91;	// NOLINT
				case 0xc0000102: return 0x570;	// NOLINT
				case 0xc0000103: return 0x10b;	// NOLINT
				case 0xc0000104: return 0x555;	// NOLINT
				case 0xc0000105: return 0x556;	// NOLINT
				case 0xc0000106: return 0xce;	// NOLINT
				case 0xc0000107: return 0x961;	// NOLINT
				case 0xc0000108: return 0x964;	// NOLINT
				case 0xc000010a: return 0x5;	// NOLINT
				case 0xc000010b: return 0x557;	// NOLINT
				case 0xc000010c: return 0x230;	// NOLINT
				case 0xc000010d: return 0x558;	// NOLINT
				case 0xc000010e: return 0x420;	// NOLINT
				case 0xc000010f: return 0x21a;	// NOLINT
				case 0xc0000110: return 0x21a;	// NOLINT
				case 0xc0000111: return 0x21a;	// NOLINT
				case 0xc0000112: return 0x21a;	// NOLINT
				case 0xc0000113: return 0x21a;	// NOLINT
				case 0xc0000114: return 0x21a;	// NOLINT
				case 0xc0000115: return 0x21a;	// NOLINT
				case 0xc0000116: return 0x21a;	// NOLINT
				case 0xc0000117: return 0x5a4;	// NOLINT
				case 0xc0000118: return 0x231;	// NOLINT
				case 0xc0000119: return 0x233;	// NOLINT
				case 0xc000011a: return 0x234;	// NOLINT
				case 0xc000011b: return 0xc1;	// NOLINT
				case 0xc000011c: return 0x559;	// NOLINT
				case 0xc000011d: return 0x55a;	// NOLINT
				case 0xc000011e: return 0x3ee;	// NOLINT
				case 0xc000011f: return 0x4;	// NOLINT
				case 0xc0000120: return 0x3e3;	// NOLINT
				case 0xc0000121: return 0x5;	// NOLINT
				case 0xc0000122: return 0x4ba;	// NOLINT
				case 0xc0000123: return 0x5;	// NOLINT
				case 0xc0000124: return 0x55b;	// NOLINT
				case 0xc0000125: return 0x55c;	// NOLINT
				case 0xc0000126: return 0x55d;	// NOLINT
				case 0xc0000127: return 0x55e;	// NOLINT
				case 0xc0000128: return 0x6;	// NOLINT
				case 0xc0000129: return 0x235;	// NOLINT
				case 0xc000012a: return 0x236;	// NOLINT
				case 0xc000012b: return 0x55f;	// NOLINT
				case 0xc000012c: return 0x237;	// NOLINT
				case 0xc000012d: return 0x5af;	// NOLINT
				case 0xc000012e: return 0xc1;	// NOLINT
				case 0xc000012f: return 0xc1;	// NOLINT
				case 0xc0000130: return 0xc1;	// NOLINT
				case 0xc0000131: return 0xc1;	// NOLINT
				case 0xc0000132: return 0x238;	// NOLINT
				case 0xc0000133: return 0x576;	// NOLINT
				case 0xc0000134: return 0x239;	// NOLINT
				case 0xc0000135: return 0x7e;	// NOLINT
				case 0xc0000136: return 0x23a;	// NOLINT
				case 0xc0000137: return 0x23b;	// NOLINT
				case 0xc0000138: return 0xb6;	// NOLINT
				case 0xc0000139: return 0x7f;	// NOLINT
				case 0xc000013a: return 0x23c;	// NOLINT
				case 0xc000013b: return 0x40;	// NOLINT
				case 0xc000013c: return 0x40;	// NOLINT
				case 0xc000013d: return 0x33;	// NOLINT
				case 0xc000013e: return 0x3b;	// NOLINT
				case 0xc000013f: return 0x3b;	// NOLINT
				case 0xc0000140: return 0x3b;	// NOLINT
				case 0xc0000141: return 0x3b;	// NOLINT
				case 0xc0000142: return 0x45a;	// NOLINT
				case 0xc0000143: return 0x23d;	// NOLINT
				case 0xc0000144: return 0x23e;	// NOLINT
				case 0xc0000145: return 0x23f;	// NOLINT
				case 0xc0000146: return 0x240;	// NOLINT
				case 0xc0000147: return 0x242;	// NOLINT
				case 0xc0000148: return 0x7c;	// NOLINT
				case 0xc0000149: return 0x56;	// NOLINT
				case 0xc000014a: return 0x243;	// NOLINT
				case 0xc000014b: return 0x6d;	// NOLINT
				case 0xc000014c: return 0x3f1;	// NOLINT
				case 0xc000014d: return 0x3f8;	// NOLINT
				case 0xc000014e: return 0x244;	// NOLINT
				case 0xc000014f: return 0x3ed;	// NOLINT
				case 0xc0000150: return 0x45e;	// NOLINT
				case 0xc0000151: return 0x560;	// NOLINT
				case 0xc0000152: return 0x561;	// NOLINT
				case 0xc0000153: return 0x562;	// NOLINT
				case 0xc0000154: return 0x563;	// NOLINT
				case 0xc0000155: return 0x564;	// NOLINT
				case 0xc0000156: return 0x565;	// NOLINT
				case 0xc0000157: return 0x566;	// NOLINT
				case 0xc0000158: return 0x567;	// NOLINT
				case 0xc0000159: return 0x3ef;	// NOLINT
				case 0xc000015a: return 0x568;	// NOLINT
				case 0xc000015b: return 0x569;	// NOLINT
				case 0xc000015c: return 0x3f9;	// NOLINT
				case 0xc000015d: return 0x56a;	// NOLINT
				case 0xc000015e: return 0x245;	// NOLINT
				case 0xc000015f: return 0x45d;	// NOLINT
				case 0xc0000160: return 0x4db;	// NOLINT
				case 0xc0000161: return 0x246;	// NOLINT
				case 0xc0000162: return 0x459;	// NOLINT
				case 0xc0000163: return 0x247;	// NOLINT
				case 0xc0000164: return 0x248;	// NOLINT
				case 0xc0000165: return 0x462;	// NOLINT
				case 0xc0000166: return 0x463;	// NOLINT
				case 0xc0000167: return 0x464;	// NOLINT
				case 0xc0000168: return 0x465;	// NOLINT
				case 0xc0000169: return 0x466;	// NOLINT
				case 0xc000016a: return 0x467;	// NOLINT
				case 0xc000016b: return 0x468;	// NOLINT
				case 0xc000016c: return 0x45f;	// NOLINT
				case 0xc000016d: return 0x45d;	// NOLINT
				case 0xc000016e: return 0x249;	// NOLINT
				case 0xc0000172: return 0x451;	// NOLINT
				case 0xc0000173: return 0x452;	// NOLINT
				case 0xc0000174: return 0x453;	// NOLINT
				case 0xc0000175: return 0x454;	// NOLINT
				case 0xc0000176: return 0x455;	// NOLINT
				case 0xc0000177: return 0x469;	// NOLINT
				case 0xc0000178: return 0x458;	// NOLINT
				case 0xc000017a: return 0x56b;	// NOLINT
				case 0xc000017b: return 0x56c;	// NOLINT
				case 0xc000017c: return 0x3fa;	// NOLINT
				case 0xc000017d: return 0x3fb;	// NOLINT
				case 0xc000017e: return 0x56d;	// NOLINT
				case 0xc000017f: return 0x56e;	// NOLINT
				case 0xc0000180: return 0x3fc;	// NOLINT
				case 0xc0000181: return 0x3fd;	// NOLINT
				case 0xc0000182: return 0x57;	// NOLINT
				case 0xc0000183: return 0x45d;	// NOLINT
				case 0xc0000184: return 0x16;	// NOLINT
				case 0xc0000185: return 0x45d;	// NOLINT
				case 0xc0000186: return 0x45d;	// NOLINT
				case 0xc0000187: return 0x24a;	// NOLINT
				case 0xc0000188: return 0x5de;	// NOLINT
				case 0xc0000189: return 0x13;	// NOLINT
				case 0xc000018a: return 0x6fa;	// NOLINT
				case 0xc000018b: return 0x6fb;	// NOLINT
				case 0xc000018c: return 0x6fc;	// NOLINT
				case 0xc000018d: return 0x6fd;	// NOLINT
				case 0xc000018e: return 0x5dc;	// NOLINT
				case 0xc000018f: return 0x5dd;	// NOLINT
				case 0xc0000190: return 0x6fe;	// NOLINT
				case 0xc0000191: return 0x24b;	// NOLINT
				case 0xc0000192: return 0x700;	// NOLINT
				case 0xc0000193: return 0x701;	// NOLINT
				case 0xc0000194: return 0x46b;	// NOLINT
				case 0xc0000195: return 0x4c3;	// NOLINT
				case 0xc0000196: return 0x4c4;	// NOLINT
				case 0xc0000197: return 0x5df;	// NOLINT
				case 0xc0000198: return 0x70f;	// NOLINT
				case 0xc0000199: return 0x710;	// NOLINT
				case 0xc000019a: return 0x711;	// NOLINT
				case 0xc000019b: return 0x712;	// NOLINT
				case 0xc000019c: return 0x24c;	// NOLINT
				case 0xc000019d: return 0x420;	// NOLINT
				case 0xc000019e: return 0x130;	// NOLINT
				case 0xc000019f: return 0x131;	// NOLINT
				case 0xc00001a0: return 0x132;	// NOLINT
				case 0xc00001a1: return 0x133;	// NOLINT
				case 0xc00001a2: return 0x325;	// NOLINT
				case 0xc00001a3: return 0x134;	// NOLINT
				case 0xc00001a4: return 0x135;	// NOLINT
				case 0xc00001a5: return 0x136;	// NOLINT
				case 0xc00001a6: return 0x137;	// NOLINT
				case 0xc00001a7: return 0x139;	// NOLINT
				case 0xc00001a8: return 0x1abb; // NOLINT
				case 0xc00001a9: return 0x32;	// NOLINT
				case 0xc00001aa: return 0x3d54; // NOLINT
				case 0xc00001ab: return 0x329;	// NOLINT
				case 0xc00001ac: return 0x678;	// NOLINT
				case 0xc00001ad: return 0x8;	// NOLINT
				case 0xc00001ae: return 0x2f7;	// NOLINT
				case 0xc00001af: return 0x32d;	// NOLINT
				case 0xc0000201: return 0x41;	// NOLINT
				case 0xc0000202: return 0x572;	// NOLINT
				case 0xc0000203: return 0x3b;	// NOLINT
				case 0xc0000204: return 0x717;	// NOLINT
				case 0xc0000205: return 0x46a;	// NOLINT
				case 0xc0000206: return 0x6f8;	// NOLINT
				case 0xc0000207: return 0x4be;	// NOLINT
				case 0xc0000208: return 0x4be;	// NOLINT
				case 0xc0000209: return 0x44;	// NOLINT
				case 0xc000020a: return 0x34;	// NOLINT
				case 0xc000020b: return 0x40;	// NOLINT
				case 0xc000020c: return 0x40;	// NOLINT
				case 0xc000020d: return 0x40;	// NOLINT
				case 0xc000020e: return 0x44;	// NOLINT
				case 0xc000020f: return 0x3b;	// NOLINT
				case 0xc0000210: return 0x3b;	// NOLINT
				case 0xc0000211: return 0x3b;	// NOLINT
				case 0xc0000212: return 0x3b;	// NOLINT
				case 0xc0000213: return 0x3b;	// NOLINT
				case 0xc0000214: return 0x3b;	// NOLINT
				case 0xc0000215: return 0x3b;	// NOLINT
				case 0xc0000216: return 0x32;	// NOLINT
				case 0xc0000217: return 0x32;	// NOLINT
				case 0xc0000218: return 0x24d;	// NOLINT
				case 0xc0000219: return 0x24e;	// NOLINT
				case 0xc000021a: return 0x24f;	// NOLINT
				case 0xc000021b: return 0x250;	// NOLINT
				case 0xc000021c: return 0x17e6; // NOLINT
				case 0xc000021d: return 0x251;	// NOLINT
				case 0xc000021e: return 0x252;	// NOLINT
				case 0xc000021f: return 0x253;	// NOLINT
				case 0xc0000220: return 0x46c;	// NOLINT
				case 0xc0000221: return 0xc1;	// NOLINT
				case 0xc0000222: return 0x254;	// NOLINT
				case 0xc0000223: return 0x255;	// NOLINT
				case 0xc0000224: return 0x773;	// NOLINT
				case 0xc0000225: return 0x490;	// NOLINT
				case 0xc0000226: return 0x256;	// NOLINT
				case 0xc0000227: return 0x4ff;	// NOLINT
				case 0xc0000228: return 0x257;	// NOLINT
				case 0xc0000229: return 0x57;	// NOLINT
				case 0xc000022a: return 0x1392; // NOLINT
				case 0xc000022b: return 0x1392; // NOLINT
				case 0xc000022c: return 0x258;	// NOLINT
				case 0xc000022d: return 0x4d5;	// NOLINT
				case 0xc000022e: return 0x259;	// NOLINT
				case 0xc000022f: return 0x25a;	// NOLINT
				case 0xc0000230: return 0x492;	// NOLINT
				case 0xc0000231: return 0x25b;	// NOLINT
				case 0xc0000232: return 0x25c;	// NOLINT
				case 0xc0000233: return 0x774;	// NOLINT
				case 0xc0000234: return 0x775;	// NOLINT
				case 0xc0000235: return 0x6;	// NOLINT
				case 0xc0000236: return 0x4c9;	// NOLINT
				case 0xc0000237: return 0x4ca;	// NOLINT
				case 0xc0000238: return 0x4cb;	// NOLINT
				case 0xc0000239: return 0x4cc;	// NOLINT
				case 0xc000023a: return 0x4cd;	// NOLINT
				case 0xc000023b: return 0x4ce;	// NOLINT
				case 0xc000023c: return 0x4cf;	// NOLINT
				case 0xc000023d: return 0x4d0;	// NOLINT
				case 0xc000023e: return 0x4d1;	// NOLINT
				case 0xc000023f: return 0x4d2;	// NOLINT
				case 0xc0000240: return 0x4d3;	// NOLINT
				case 0xc0000241: return 0x4d4;	// NOLINT
				case 0xc0000242: return 0x25d;	// NOLINT
				case 0xc0000243: return 0x4c8;	// NOLINT
				case 0xc0000244: return 0x25e;	// NOLINT
				case 0xc0000245: return 0x25f;	// NOLINT
				case 0xc0000246: return 0x4d6;	// NOLINT
				case 0xc0000247: return 0x4d7;	// NOLINT
				case 0xc0000248: return 0x4d8;	// NOLINT
				case 0xc0000249: return 0xc1;	// NOLINT
				case 0xc0000250: return 0x260;	// NOLINT
				case 0xc0000251: return 0x261;	// NOLINT
				case 0xc0000252: return 0x262;	// NOLINT
				case 0xc0000253: return 0x4d4;	// NOLINT
				case 0xc0000254: return 0x263;	// NOLINT
				case 0xc0000255: return 0x264;	// NOLINT
				case 0xc0000256: return 0x265;	// NOLINT
				case 0xc0000257: return 0x4d0;	// NOLINT
				case 0xc0000258: return 0x266;	// NOLINT
				case 0xc0000259: return 0x573;	// NOLINT
				case 0xc000025a: return 0x267;	// NOLINT
				case 0xc000025b: return 0x268;	// NOLINT
				case 0xc000025c: return 0x269;	// NOLINT
				case 0xc000025e: return 0x422;	// NOLINT
				case 0xc000025f: return 0x26a;	// NOLINT
				case 0xc0000260: return 0x26b;	// NOLINT
				case 0xc0000261: return 0x26c;	// NOLINT
				case 0xc0000262: return 0xb6;	// NOLINT
				case 0xc0000263: return 0x7f;	// NOLINT
				case 0xc0000264: return 0x120;	// NOLINT
				case 0xc0000265: return 0x476;	// NOLINT
				case 0xc0000266: return 0x26d;	// NOLINT
				case 0xc0000267: return 0x10fe; // NOLINT
				case 0xc0000268: return 0x26e;	// NOLINT
				case 0xc0000269: return 0x26f;	// NOLINT
				case 0xc000026a: return 0x1b8e; // NOLINT
				case 0xc000026b: return 0x270;	// NOLINT
				case 0xc000026c: return 0x7d1;	// NOLINT
				case 0xc000026d: return 0x4b1;	// NOLINT
				case 0xc000026e: return 0x15;	// NOLINT
				case 0xc000026f: return 0x21c;	// NOLINT
				case 0xc0000270: return 0x21c;	// NOLINT
				case 0xc0000271: return 0x271;	// NOLINT
				case 0xc0000272: return 0x491;	// NOLINT
				case 0xc0000273: return 0x272;	// NOLINT
				case 0xc0000275: return 0x1126; // NOLINT
				case 0xc0000276: return 0x1129; // NOLINT
				case 0xc0000277: return 0x112a; // NOLINT
				case 0xc0000278: return 0x1128; // NOLINT
				case 0xc0000279: return 0x780;	// NOLINT
				case 0xc000027a: return 0x291;	// NOLINT
				case 0xc000027b: return 0x54f;	// NOLINT
				case 0xc000027c: return 0x54f;	// NOLINT
				case 0xc0000280: return 0x781;	// NOLINT
				case 0xc0000281: return 0xa1;	// NOLINT
				case 0xc0000282: return 0x273;	// NOLINT
				case 0xc0000283: return 0x488;	// NOLINT
				case 0xc0000284: return 0x489;	// NOLINT
				case 0xc0000285: return 0x48a;	// NOLINT
				case 0xc0000286: return 0x48b;	// NOLINT
				case 0xc0000287: return 0x48c;	// NOLINT
				case 0xc000028a: return 0x5;	// NOLINT
				case 0xc000028b: return 0x5;	// NOLINT
				case 0xc000028c: return 0x284;	// NOLINT
				case 0xc000028d: return 0x5;	// NOLINT
				case 0xc000028e: return 0x5;	// NOLINT
				case 0xc000028f: return 0x5;	// NOLINT
				case 0xc0000290: return 0x5;	// NOLINT
				case 0xc0000291: return 0x1777; // NOLINT
				case 0xc0000292: return 0x1778; // NOLINT
				case 0xc0000293: return 0x1772; // NOLINT
				case 0xc0000295: return 0x1068; // NOLINT
				case 0xc0000296: return 0x1069; // NOLINT
				case 0xc0000297: return 0x106a; // NOLINT
				case 0xc0000298: return 0x106b; // NOLINT
				case 0xc0000299: return 0x201a; // NOLINT
				case 0xc000029a: return 0x201b; // NOLINT
				case 0xc000029b: return 0x201c; // NOLINT
				case 0xc000029c: return 0x1;	// NOLINT
				case 0xc000029d: return 0x10ff; // NOLINT
				case 0xc000029e: return 0x1100; // NOLINT
				case 0xc000029f: return 0x494;	// NOLINT
				case 0xc00002a0: return 0x274;	// NOLINT
				case 0xc00002a1: return 0x200a; // NOLINT
				case 0xc00002a2: return 0x200b; // NOLINT
				case 0xc00002a3: return 0x200c; // NOLINT
				case 0xc00002a4: return 0x200d; // NOLINT
				case 0xc00002a5: return 0x200e; // NOLINT
				case 0xc00002a6: return 0x200f; // NOLINT
				case 0xc00002a7: return 0x2010; // NOLINT
				case 0xc00002a8: return 0x2011; // NOLINT
				case 0xc00002a9: return 0x2012; // NOLINT
				case 0xc00002aa: return 0x2013; // NOLINT
				case 0xc00002ab: return 0x2014; // NOLINT
				case 0xc00002ac: return 0x2015; // NOLINT
				case 0xc00002ad: return 0x2016; // NOLINT
				case 0xc00002ae: return 0x2017; // NOLINT
				case 0xc00002af: return 0x2018; // NOLINT
				case 0xc00002b0: return 0x2019; // NOLINT
				case 0xc00002b1: return 0x211e; // NOLINT
				case 0xc00002b2: return 0x1127; // NOLINT
				case 0xc00002b3: return 0x275;	// NOLINT
				case 0xc00002b4: return 0x276;	// NOLINT
				case 0xc00002b5: return 0x277;	// NOLINT
				case 0xc00002b6: return 0x651;	// NOLINT
				case 0xc00002b7: return 0x49a;	// NOLINT
				case 0xc00002b8: return 0x49b;	// NOLINT
				case 0xc00002b9: return 0x278;	// NOLINT
				case 0xc00002ba: return 0x2047; // NOLINT
				case 0xc00002c1: return 0x2024; // NOLINT
				case 0xc00002c2: return 0x279;	// NOLINT
				case 0xc00002c3: return 0x575;	// NOLINT
				case 0xc00002c4: return 0x27a;	// NOLINT
				case 0xc00002c5: return 0x3e6;	// NOLINT
				case 0xc00002c6: return 0x1075; // NOLINT
				case 0xc00002c7: return 0x1076; // NOLINT
				case 0xc00002c8: return 0x27b;	// NOLINT
				case 0xc00002c9: return 0x4ed;	// NOLINT
				case 0xc00002ca: return 0x10e8; // NOLINT
				case 0xc00002cb: return 0x2138; // NOLINT
				case 0xc00002cc: return 0x4e3;	// NOLINT
				case 0xc00002cd: return 0x2139; // NOLINT
				case 0xc00002ce: return 0x27c;	// NOLINT
				case 0xc00002cf: return 0x49d;	// NOLINT
				case 0xc00002d0: return 0x213a; // NOLINT
				case 0xc00002d1: return 0x27d;	// NOLINT
				case 0xc00002d2: return 0x27e;	// NOLINT
				case 0xc00002d3: return 0x15;	// NOLINT
				case 0xc00002d4: return 0x2141; // NOLINT
				case 0xc00002d5: return 0x2142; // NOLINT
				case 0xc00002d6: return 0x2143; // NOLINT
				case 0xc00002d7: return 0x2144; // NOLINT
				case 0xc00002d8: return 0x2145; // NOLINT
				case 0xc00002d9: return 0x2146; // NOLINT
				case 0xc00002da: return 0x2147; // NOLINT
				case 0xc00002db: return 0x2148; // NOLINT
				case 0xc00002dc: return 0x2149; // NOLINT
				case 0xc00002dd: return 0x32;	// NOLINT
				case 0xc00002de: return 0x27f;	// NOLINT
				case 0xc00002df: return 0x2151; // NOLINT
				case 0xc00002e0: return 0x2152; // NOLINT
				case 0xc00002e1: return 0x2153; // NOLINT
				case 0xc00002e2: return 0x2154; // NOLINT
				case 0xc00002e3: return 0x215d; // NOLINT
				case 0xc00002e4: return 0x2163; // NOLINT
				case 0xc00002e5: return 0x2164; // NOLINT
				case 0xc00002e6: return 0x2165; // NOLINT
				case 0xc00002e7: return 0x216d; // NOLINT
				case 0xc00002e8: return 0x280;	// NOLINT
				case 0xc00002e9: return 0x577;	// NOLINT
				case 0xc00002ea: return 0x52;	// NOLINT
				case 0xc00002eb: return 0x281;	// NOLINT
				case 0xc00002ec: return 0x2171; // NOLINT
				case 0xc00002ed: return 0x2172; // NOLINT
				case 0xc00002f0: return 0x2;	// NOLINT
				case 0xc00002fe: return 0x45b;	// NOLINT
				case 0xc00002ff: return 0x4e7;	// NOLINT
				case 0xc0000300: return 0x4e6;	// NOLINT
				case 0xc0000301: return 0x106f; // NOLINT
				case 0xc0000302: return 0x1074; // NOLINT
				case 0xc0000303: return 0x106e; // NOLINT
				case 0xc0000304: return 0x12e;	// NOLINT
				case 0xc000030c: return 0x792;	// NOLINT
				case 0xc000030d: return 0x793;	// NOLINT
				case 0xc0000320: return 0x4ef;	// NOLINT
				case 0xc0000321: return 0x4f0;	// NOLINT
				case 0xc0000350: return 0x4e8;	// NOLINT
				case 0xc0000352: return 0x177d; // NOLINT
				case 0xc0000353: return 0x282;	// NOLINT
				case 0xc0000354: return 0x504;	// NOLINT
				case 0xc0000355: return 0x283;	// NOLINT
				case 0xc0000357: return 0x217c; // NOLINT
				case 0xc0000358: return 0x2182; // NOLINT
				case 0xc0000359: return 0xc1;	// NOLINT
				case 0xc000035a: return 0xc1;	// NOLINT
				case 0xc000035c: return 0x572;	// NOLINT
				case 0xc000035d: return 0x4eb;	// NOLINT
				case 0xc000035f: return 0x286;	// NOLINT
				case 0xc0000361: return 0x4ec;	// NOLINT
				case 0xc0000362: return 0x4ec;	// NOLINT
				case 0xc0000363: return 0x4ec;	// NOLINT
				case 0xc0000364: return 0x4ec;	// NOLINT
				case 0xc0000365: return 0x287;	// NOLINT
				case 0xc0000366: return 0x288;	// NOLINT
				case 0xc0000368: return 0x289;	// NOLINT
				case 0xc0000369: return 0x28a;	// NOLINT
				case 0xc000036a: return 0x28b;	// NOLINT
				case 0xc000036b: return 0x4fb;	// NOLINT
				case 0xc000036c: return 0x4fb;	// NOLINT
				case 0xc000036d: return 0x28c;	// NOLINT
				case 0xc000036e: return 0x28d;	// NOLINT
				case 0xc000036f: return 0x4fc;	// NOLINT
				case 0xc0000371: return 0x21ac; // NOLINT
				case 0xc0000372: return 0x312;	// NOLINT
				case 0xc0000373: return 0x8;	// NOLINT
				case 0xc0000374: return 0x54f;	// NOLINT
				case 0xc0000388: return 0x4f1;	// NOLINT
				case 0xc000038e: return 0x28e;	// NOLINT
				case 0xc0000401: return 0x78c;	// NOLINT
				case 0xc0000402: return 0x78d;	// NOLINT
				case 0xc0000403: return 0x78e;	// NOLINT
				case 0xc0000404: return 0x217b; // NOLINT
				case 0xc0000405: return 0x219d; // NOLINT
				case 0xc0000406: return 0x219f; // NOLINT
				case 0xc0000407: return 0x28f;	// NOLINT
				case 0xc0000408: return 0x52e;	// NOLINT
				case 0xc0000409: return 0x502;	// NOLINT
				case 0xc0000410: return 0x503;	// NOLINT
				case 0xc0000411: return 0x290;	// NOLINT
				case 0xc0000412: return 0x505;	// NOLINT
				case 0xc0000413: return 0x78f;	// NOLINT
				case 0xc0000414: return 0x506;	// NOLINT
				case 0xc0000416: return 0x8;	// NOLINT
				case 0xc0000417: return 0x508;	// NOLINT
				case 0xc0000418: return 0x791;	// NOLINT
				case 0xc0000419: return 0x215b; // NOLINT
				case 0xc000041a: return 0x21ba; // NOLINT
				case 0xc000041b: return 0x21bb; // NOLINT
				case 0xc000041c: return 0x21bc; // NOLINT
				case 0xc000041d: return 0x2c9;	// NOLINT
				case 0xc0000420: return 0x29c;	// NOLINT
				case 0xc0000421: return 0x219;	// NOLINT
				case 0xc0000423: return 0x300;	// NOLINT
				case 0xc0000424: return 0x4fb;	// NOLINT
				case 0xc0000425: return 0x3fa;	// NOLINT
				case 0xc0000426: return 0x301;	// NOLINT
				case 0xc0000427: return 0x299;	// NOLINT
				case 0xc0000428: return 0x241;	// NOLINT
				case 0xc0000429: return 0x307;	// NOLINT
				case 0xc000042a: return 0x308;	// NOLINT
				case 0xc000042b: return 0x50c;	// NOLINT
				case 0xc000042c: return 0x2e4;	// NOLINT
				case 0xc0000432: return 0x509;	// NOLINT
				case 0xc0000433: return 0xaa;	// NOLINT
				case 0xc0000434: return 0xaa;	// NOLINT
				case 0xc0000435: return 0x4c8;	// NOLINT
				case 0xc0000441: return 0x1781; // NOLINT
				case 0xc0000442: return 0x1782; // NOLINT
				case 0xc0000443: return 0x1783; // NOLINT
				case 0xc0000444: return 0x1784; // NOLINT
				case 0xc0000445: return 0x1785; // NOLINT
				case 0xc0000446: return 0x513;	// NOLINT
				case 0xc0000450: return 0x50b;	// NOLINT
				case 0xc0000451: return 0x3b92; // NOLINT
				case 0xc0000452: return 0x3bc3; // NOLINT
				case 0xc0000453: return 0x5bb;	// NOLINT
				case 0xc0000454: return 0x5be;	// NOLINT
				case 0xc0000455: return 0x6;	// NOLINT
				case 0xc0000456: return 0x57;	// NOLINT
				case 0xc0000457: return 0x57;	// NOLINT
				case 0xc0000458: return 0x57;	// NOLINT
				case 0xc0000459: return 0xbea;	// NOLINT
				case 0xc0000460: return 0x138;	// NOLINT
				case 0xc0000461: return 0x13a;	// NOLINT
				case 0xc0000462: return 0x3cfc; // NOLINT
				case 0xc0000463: return 0x13c;	// NOLINT
				case 0xc0000464: return 0x141;	// NOLINT
				case 0xc0000465: return 0x13b;	// NOLINT
				case 0xc0000466: return 0x40;	// NOLINT
				case 0xc0000467: return 0x20;	// NOLINT
				case 0xc0000468: return 0x142;	// NOLINT
				case 0xc0000469: return 0x3d00; // NOLINT
				case 0xc000046a: return 0x151;	// NOLINT
				case 0xc000046b: return 0x152;	// NOLINT
				case 0xc000046c: return 0x153;	// NOLINT
				case 0xc000046d: return 0x156;	// NOLINT
				case 0xc000046e: return 0x157;	// NOLINT
				case 0xc000046f: return 0x158;	// NOLINT
				case 0xc0000470: return 0x143;	// NOLINT
				case 0xc0000471: return 0x144;	// NOLINT
				case 0xc0000472: return 0x146;	// NOLINT
				case 0xc0000473: return 0x14b;	// NOLINT
				case 0xc0000474: return 0x147;	// NOLINT
				case 0xc0000475: return 0x148;	// NOLINT
				case 0xc0000476: return 0x149;	// NOLINT
				case 0xc0000477: return 0x14a;	// NOLINT
				case 0xc0000478: return 0x14c;	// NOLINT
				case 0xc0000479: return 0x14d;	// NOLINT
				case 0xc000047a: return 0x14e;	// NOLINT
				case 0xc000047b: return 0x14f;	// NOLINT
				case 0xc000047c: return 0x150;	// NOLINT
				case 0xc000047d: return 0x5b4;	// NOLINT
				case 0xc000047e: return 0x3d07; // NOLINT
				case 0xc000047f: return 0x3d08; // NOLINT
				case 0xc0000480: return 0x40;	// NOLINT
				case 0xc0000481: return 0x7e;	// NOLINT
				case 0xc0000482: return 0x7e;	// NOLINT
				case 0xc0000483: return 0x1e3;	// NOLINT
				case 0xc0000486: return 0x159;	// NOLINT
				case 0xc0000487: return 0x1f;	// NOLINT
				case 0xc0000488: return 0x15a;	// NOLINT
				case 0xc0000489: return 0x3d0f; // NOLINT
				case 0xc000048a: return 0x32a;	// NOLINT
				case 0xc000048b: return 0x32c;	// NOLINT
				case 0xc000048c: return 0x15b;	// NOLINT
				case 0xc000048d: return 0x15c;	// NOLINT
				case 0xc000048e: return 0x162;	// NOLINT
				case 0xc000048f: return 0x15d;	// NOLINT
				case 0xc0000490: return 0x491;	// NOLINT
				case 0xc0000491: return 0x2;	// NOLINT
				case 0xc0000492: return 0x490;	// NOLINT
				case 0xc0000493: return 0x492;	// NOLINT
				case 0xc0000494: return 0x307;	// NOLINT
				case 0xc0000495: return 0x15;	// NOLINT
				case 0xc0000496: return 0x163;	// NOLINT
				case 0xc0000497: return 0x3d5a; // NOLINT
				case 0xc0000499: return 0x167;	// NOLINT
				case 0xc000049a: return 0x168;	// NOLINT
				case 0xc000049b: return 0x12e;	// NOLINT
				case 0xc000049c: return 0x169;	// NOLINT
				case 0xc000049d: return 0x16f;	// NOLINT
				case 0xc000049e: return 0x170;	// NOLINT
				case 0xc000049f: return 0x49f;	// NOLINT
				case 0xc00004a0: return 0x4a0;	// NOLINT
				case 0xc00004a1: return 0x18f;	// NOLINT
				case 0xc0000500: return 0x60e;	// NOLINT
				case 0xc0000501: return 0x60f;	// NOLINT
				case 0xc0000502: return 0x610;	// NOLINT
				case 0xc0000503: return 0x15;	// NOLINT
				case 0xc0000504: return 0x13f;	// NOLINT
				case 0xc0000505: return 0x140;	// NOLINT
				case 0xc0000506: return 0x5bf;	// NOLINT
				case 0xc0000507: return 0xaa;	// NOLINT
				case 0xc0000508: return 0x5e0;	// NOLINT
				case 0xc0000509: return 0x5e1;	// NOLINT
				case 0xc000050b: return 0x112b; // NOLINT
				case 0xc000050e: return 0x115c; // NOLINT
				case 0xc000050f: return 0x10d3; // NOLINT
				case 0xc0000510: return 0x4df;	// NOLINT
				case 0xc0000511: return 0x32e;	// NOLINT
				case 0xc0000512: return 0x5;	// NOLINT
				case 0xc0000513: return 0x180;	// NOLINT
				case 0xc0000514: return 0x115d; // NOLINT
				case 0xc0000602: return 0x675;	// NOLINT
				case 0xc0000604: return 0x677;	// NOLINT
				case 0xc0000606: return 0x679;	// NOLINT
				case 0xc000060a: return 0x67c;	// NOLINT
				case 0xc000060b: return 0x67d;	// NOLINT
				case 0xc0000700: return 0x54f;	// NOLINT
				case 0xc0000701: return 0x54f;	// NOLINT
				case 0xc0000702: return 0x57;	// NOLINT
				case 0xc0000703: return 0x54f;	// NOLINT
				case 0xc0000704: return 0x32;	// NOLINT
				case 0xc0000705: return 0x57;	// NOLINT
				case 0xc0000706: return 0x57;	// NOLINT
				case 0xc0000707: return 0x32;	// NOLINT
				case 0xc0000708: return 0x54f;	// NOLINT
				case 0xc0000709: return 0x30b;	// NOLINT
				case 0xc000070a: return 0x6;	// NOLINT
				case 0xc000070b: return 0x6;	// NOLINT
				case 0xc000070c: return 0x6;	// NOLINT
				case 0xc000070d: return 0x6;	// NOLINT
				case 0xc000070e: return 0x6;	// NOLINT
				case 0xc000070f: return 0x6;	// NOLINT
				case 0xc0000710: return 0x1;	// NOLINT
				case 0xc0000711: return 0x1;	// NOLINT
				case 0xc0000712: return 0x50d;	// NOLINT
				case 0xc0000713: return 0x310;	// NOLINT
				case 0xc0000714: return 0x52e;	// NOLINT
				case 0xc0000715: return 0x5b7;	// NOLINT
				case 0xc0000716: return 0x7b;	// NOLINT
				case 0xc0000717: return 0x459;	// NOLINT
				case 0xc0000718: return 0x54f;	// NOLINT
				case 0xc0000719: return 0x54f;	// NOLINT
				case 0xc000071a: return 0x54f;	// NOLINT
				case 0xc000071b: return 0x1;	// NOLINT
				case 0xc000071c: return 0x57;	// NOLINT
				case 0xc000071d: return 0x1;	// NOLINT
				case 0xc000071e: return 0x1;	// NOLINT
				case 0xc000071f: return 0x1;	// NOLINT
				case 0xc0000720: return 0x1;	// NOLINT
				case 0xc0000721: return 0x1;	// NOLINT
				case 0xc0000722: return 0x72b;	// NOLINT
				case 0xc0000723: return 0x1f;	// NOLINT
				case 0xc0000724: return 0x1f;	// NOLINT
				case 0xc0000725: return 0x1f;	// NOLINT
				case 0xc0000726: return 0x1f;	// NOLINT
				case 0xc0000800: return 0x30c;	// NOLINT
				case 0xc0000801: return 0x21a4; // NOLINT
				case 0xc0000802: return 0x50f;	// NOLINT
				case 0xc0000804: return 0x510;	// NOLINT
				case 0xc0000805: return 0x1ac1; // NOLINT
				case 0xc0000806: return 0x1ac3; // NOLINT
				case 0xc0000808: return 0x319;	// NOLINT
				case 0xc0000809: return 0x31a;	// NOLINT
				case 0xc000080a: return 0x31b;	// NOLINT
				case 0xc000080b: return 0x31c;	// NOLINT
				case 0xc000080c: return 0x31d;	// NOLINT
				case 0xc000080d: return 0x31e;	// NOLINT
				case 0xc000080e: return 0x31f;	// NOLINT
				case 0xc000080f: return 0x4d5;	// NOLINT
				case 0xc0000810: return 0x328;	// NOLINT
				case 0xc0000811: return 0x54f;	// NOLINT
				case 0xc0000901: return 0xdc;	// NOLINT
				case 0xc0000902: return 0xdd;	// NOLINT
				case 0xc0000903: return 0xde;	// NOLINT
				case 0xc0000904: return 0xdf;	// NOLINT
				case 0xc0000905: return 0xe0;	// NOLINT
				case 0xc0000906: return 0xe1;	// NOLINT
				case 0xc0000907: return 0xe2;	// NOLINT
				case 0xc0000908: return 0x317;	// NOLINT
				case 0xc0000909: return 0x322;	// NOLINT
				case 0xc0000910: return 0x326;	// NOLINT
				case 0xc0009898: return 0x29e;	// NOLINT
				case 0xc000a002: return 0x17;	// NOLINT
				case 0xc000a003: return 0x139f; // NOLINT
				case 0xc000a004: return 0x154;	// NOLINT
				case 0xc000a005: return 0x155;	// NOLINT
				case 0xc000a006: return 0x32b;	// NOLINT
				case 0xc000a007: return 0x32;	// NOLINT
				case 0xc000a010: return 0xea;	// NOLINT
				case 0xc000a011: return 0xea;	// NOLINT
				case 0xc000a012: return 0x4d0;	// NOLINT
				case 0xc000a013: return 0x32;	// NOLINT
				case 0xc000a014: return 0x4d1;	// NOLINT
				case 0xc000a080: return 0x314;	// NOLINT
				case 0xc000a081: return 0x315;	// NOLINT
				case 0xc000a082: return 0x316;	// NOLINT
				case 0xc000a083: return 0x5b9;	// NOLINT
				case 0xc000a084: return 0x5ba;	// NOLINT
				case 0xc000a085: return 0x5bc;	// NOLINT
				case 0xc000a086: return 0x5bd;	// NOLINT
				case 0xc000a087: return 0x21bd; // NOLINT
				case 0xc000a088: return 0x21be; // NOLINT
				case 0xc000a089: return 0x21c6; // NOLINT
				case 0xc000a100: return 0x3bc4; // NOLINT
				case 0xc000a101: return 0x3bc5; // NOLINT
				case 0xc000a121: return 0x3bd9; // NOLINT
				case 0xc000a122: return 0x3bda; // NOLINT
				case 0xc000a123: return 0x3bdb; // NOLINT
				case 0xc000a124: return 0x3bdc; // NOLINT
				case 0xc000a125: return 0x3bdd; // NOLINT
				case 0xc000a126: return 0x3bde; // NOLINT
				case 0xc000a141: return 0x3c28; // NOLINT
				case 0xc000a142: return 0x3c29; // NOLINT
				case 0xc000a143: return 0x3c2a; // NOLINT
				case 0xc000a145: return 0x3c2b; // NOLINT
				case 0xc000a146: return 0x3c2c; // NOLINT
				case 0xc000a200: return 0x109a; // NOLINT
				case 0xc000a201: return 0x109c; // NOLINT
				case 0xc000a202: return 0x109d; // NOLINT
				case 0xc000a203: return 0x5;	// NOLINT
				case 0xc000a281: return 0x1130; // NOLINT
				case 0xc000a282: return 0x1131; // NOLINT
				case 0xc000a283: return 0x1132; // NOLINT
				case 0xc000a284: return 0x1133; // NOLINT
				case 0xc000a285: return 0x1134; // NOLINT
				case 0xc000a2a1: return 0x1158; // NOLINT
				case 0xc000a2a2: return 0x1159; // NOLINT
				case 0xc000a2a3: return 0x115a; // NOLINT
				case 0xc000a2a4: return 0x115b; // NOLINT
				case 0xc000ce01: return 0x171;	// NOLINT
				case 0xc000ce02: return 0x172;	// NOLINT
				case 0xc000ce03: return 0x173;	// NOLINT
				case 0xc000ce04: return 0x174;	// NOLINT
				case 0xc000ce05: return 0x181;	// NOLINT
				case 0xc000cf00: return 0x166;	// NOLINT
				case 0xc000cf01: return 0x16a;	// NOLINT
				case 0xc000cf02: return 0x16b;	// NOLINT
				case 0xc000cf03: return 0x16c;	// NOLINT
				case 0xc000cf06: return 0x177;	// NOLINT
				case 0xc000cf07: return 0x178;	// NOLINT
				case 0xc000cf08: return 0x179;	// NOLINT
				case 0xc000cf09: return 0x17a;	// NOLINT
				case 0xc000cf0a: return 0x17b;	// NOLINT
				case 0xc000cf0b: return 0x17c;	// NOLINT
				case 0xc000cf0c: return 0x17d;	// NOLINT
				case 0xc000cf0d: return 0x17e;	// NOLINT
				case 0xc000cf0e: return 0x17f;	// NOLINT
				case 0xc000cf0f: return 0x182;	// NOLINT
				case 0xc000cf10: return 0x183;	// NOLINT
				case 0xc000cf11: return 0x184;	// NOLINT
				case 0xc000cf12: return 0x185;	// NOLINT
				case 0xc000cf13: return 0x186;	// NOLINT
				case 0xc000cf14: return 0x187;	// NOLINT
				case 0xc000cf15: return 0x188;	// NOLINT
				case 0xc000cf16: return 0x189;	// NOLINT
				case 0xc000cf17: return 0x18a;	// NOLINT
				case 0xc000cf18: return 0x18b;	// NOLINT
				case 0xc000cf19: return 0x18c;	// NOLINT
				case 0xc000cf1a: return 0x18d;	// NOLINT
				case 0xc000cf1b: return 0x18e;	// NOLINT
				default: return static_cast<detail::win32::DWORD>(-1);
			}
		}
	};

} // namespace hyperion::error

/// @brief Specialize `make_status_code_domain` for `NTDomain` and `u64`.
/// Creates a `NTDomain` with a custom UUID.
///
/// @note When using a custom UUID __**ALL**__ instances of `NTDomain` in the program
/// should be constructed with the same custom UUID, otherwise equality comparison between
/// other domains and `NTDomain` instances could give erroneous results, and equality
/// comparison between different `NTDomain` instances will give erroneous results.
/// As a result, this constructor should only be used when you specifically require a custom
/// UUID and **YOU KNOW WHAT YOU ARE DOING™**
///
/// @param uuid - The UUID to use for `NTDomain`
///
/// @return a `NTDomain`
/// @ingroup error
template<>
[[nodiscard]] inline constexpr auto
// NOLINTNEXTLINE(readability-inconsistent-declaration-parameter-name)
make_status_code_domain<hyperion::error::NTDomain, hyperion::u64>(hyperion::u64&& uuid) noexcept
	-> hyperion::error::NTDomain {
	return hyperion::error::NTDomain(uuid);
}

/// @brief Specialize `make_status_code_domain` for `NTDomain` and a `UUIDString`.
/// Creates a `NTDomain` with a custom UUID.
///
/// @note When using a custom UUID __**ALL**__ instances of `NTDomain` in the program
/// should be constructed with the same custom UUID, otherwise equality comparison between
/// other domains and `NTDomain` instances could give erroneous results, and equality
/// comparison between different `NTDomain` instances will give erroneous results.
/// As a result, this constructor should only be used when you specifically require a custom
/// UUID and **YOU KNOW WHAT YOU ARE DOING™**
///
/// @param uuid - The UUID to use for `NTDomain`
///
/// @return a `NTDomain`
/// @ingroup error
template<>
[[nodiscard]] inline constexpr auto
	// NOLINTNEXTLINE(readability-inconsistent-declaration-parameter-name)
	make_status_code_domain<hyperion::error::NTDomain,
							const char (&)[hyperion::error::num_chars_in_uuid]> // NOLINT
	(const char (&uuid)[hyperion::error::num_chars_in_uuid]) noexcept			// NOLINT
	-> hyperion::error::NTDomain {
	return hyperion::error::NTDomain(uuid);
}

/// @brief Specialize `make_status_code_domain` for `NTDomain` and an MS-style `UUIDString`.
/// Creates a `NTDomain` with a custom UUID.
///
/// @note When using a custom UUID __**ALL**__ instances of `NTDomain` in the program
/// should be constructed with the same custom UUID, otherwise equality comparison between
/// other domains and `NTDomain` instances could give erroneous results, and equality
/// comparison between different `NTDomain` instances will give erroneous results.
/// As a result, this constructor should only be used when you specifically require a custom
/// UUID and **YOU KNOW WHAT YOU ARE DOING™**
///
/// @param uuid - The UUID to use for `NTDomain`
///
/// @return a `NTDomain`
/// @ingroup error
template<>
[[nodiscard]] inline constexpr auto
	// NOLINTNEXTLINE(readability-inconsistent-declaration-parameter-name)
	make_status_code_domain<hyperion::error::NTDomain,
							const char (&)[hyperion::error::num_chars_in_ms_uuid]> // NOLINT
	(const char (&uuid)[hyperion::error::num_chars_in_ms_uuid]) noexcept		   // NOLINT
	-> hyperion::error::NTDomain {
	return hyperion::error::NTDomain(uuid);
}

/// @brief Specialize `make_status_code_domain` for `NTDomain` with no arguments. Creates
/// a `NTDomain` with the default UUID.
///
/// @return a `NTDomain`
/// @ingroup error
template<>
inline constexpr auto
make_status_code_domain<hyperion::error::NTDomain>() noexcept -> hyperion::error::NTDomain {
	return {};
}

namespace hyperion::error {
	static_assert(StatusCodeDomain<NTDomain>);
} // namespace hyperion::error
#endif // HYPERION_PLATFORM_WINDOWS
