#pragma once

#include "GenericDomain.h"
#include "StatusCode.h"

#if HYPERION_PLATFORM_WINDOWS
namespace hyperion::error {

	namespace detail::win32 {
		using DWORD = u32;
		using UINT = DWORD;
		using LPCVOID = const void*;
		using LPSTR = char*;
		using LPCSTR = const char*;
		using LPCCH = const char*;
		using LPWSTR = wchar_t*;
		using LPCWCH = const wchar_t*;
		using LPBOOL = int*; // BOOL far*
		extern auto __stdcall GetLastError() -> DWORD;
		extern auto __stdcall FormatMessageW(DWORD dwflags,
											 LPCVOID lpSource,
											 DWORD dwMessageId,
											 DWORD dwLanguageId,
											 LPWSTR lpBuffer,
											 DWORD nSize,
											 std::va_list* Arguments) -> DWORD;
		extern auto __stdcall WideCharToMultiByte(UINT CodePage,
												  DWORD dwFlags,
												  LPCWCH lpWideCharStr,
												  int cchWideChar,
												  LPSTR lpMultiByteStr,
												  int cbMultiByte,
												  LPCCH lpDefaultChar,
												  LPBOOL lpUsedDefaultChar) -> int;
		// get the linker to link in the symbols for the above functions without requiring the user
		// to manually add them to their linker flags
	#pragma comment(lib, "kernel32.lib")
	#if(defined(__x86_64__) || defined(_M_X64)) || (defined(__aarch64__) || defined(_M_ARM64))
		#pragma comment(linker, \
						"/alternatename:?GetLastError@win32@system_error2@@YAKXZ=GetLastError")
		#pragma comment( \
			linker,      \
			"/alternatename:?FormatMessageW@win32@system_error2@@YAKKPEBXKKPEA_WKPEAX@Z=FormatMessageW")
		#pragma comment( \
			linker,      \
			"/alternatename:?WideCharToMultiByte@win32@system_error2@@YAHIKPEB_WHPEADHPEBDPEAH@Z=WideCharToMultiByte")
	#elif defined(__x86__) || defined(_M_IX86) || defined(__i386__)
		#pragma comment( \
			linker,      \
			"/alternatename:?GetLastError@win32@system_error2@@YGKXZ=__imp__GetLastError@0")
		#pragma comment( \
			linker,      \
			"/alternatename:?FormatMessageW@win32@system_error2@@YGKKPBXKKPA_WKPAX@Z=__imp__FormatMessageW@28")
		#pragma comment( \
			linker,      \
			"/alternatename:?WideCharToMultiByte@win32@system_error2@@YGHIKPB_WHPADHPBDPAH@Z=__imp__WideCharToMultiByte@32")
	#elif defined(__arm__) || defined(_M_ARM)
		#pragma comment(linker, \
						"/alternatename:?GetLastError@win32@system_error2@@YAKXZ=GetLastError")
		#pragma comment( \
			linker,      \
			"/alternatename:?FormatMessageW@win32@system_error2@@YAKKPBXKKPA_WKPAX@Z=FormatMessageW")
		#pragma comment( \
			linker,      \
			"/alternatename:?WideCharToMultiByte@win32@system_error2@@YAHIKPB_WHPADHPBDPAH@Z=WideCharToMultiByte")
	#else
		#error Unknown architecture
	#endif
	} // namespace detail::win32

	class Win32Domain;

	using Win32StatusCode = StatusCode<Win32Domain>;
	using Win32ErrorCode = ErrorCode<Win32Domain>;

	class [[nodiscard]] Win32Domain {
	  public:
		using value_type = detail::win32::DWORD;

		static constexpr const char (&UUID)[num_chars_in_uuid] // NOLINT
			= "53b43298-f1f6-4a7b-a998-49dfa96c7159";

		static constexpr u64 ID = parse_uuid_from_string(UUID);

		constexpr Win32Domain() noexcept = default;
		explicit constexpr Win32Domain(u64 uuid) noexcept : m_uuid(uuid) {
		}
		explicit constexpr Win32Domain(const UUIDString auto& uuid) noexcept
			: m_uuid(parse_uuid_from_string(uuid)) {
		}
		constexpr ~Win32Domain() noexcept = default;
		constexpr Win32Domain(const Win32Domain&) noexcept = default;
		constexpr Win32Domain(Win32Domain&&) noexcept = default;

		[[nodiscard]] constexpr auto id() const noexcept -> u64 {
			return m_uuid;
		}

		[[nodiscard]] constexpr auto name() const noexcept -> std::string_view { // NOLINT
			return "win32 domain";
		}

		[[nodiscard]] auto message(value_type code) // NOLINT
			const noexcept -> std::string {
			return as_string(code);
		}

		[[nodiscard]] auto message(const Win32StatusCode& code) // NOLINT
			const noexcept -> std::string {
			return as_string(code.code());
		}

		[[nodiscard]] constexpr auto
		is_error(const Win32StatusCode& code) const noexcept -> bool { // NOLINT
			return code.code() != 0;
		}

		[[nodiscard]] constexpr auto
		is_success(const Win32StatusCode& code) const noexcept -> bool { // NOLINT
			return code.code() == 0;
		}

		template<typename Domain2>
		[[nodiscard]] constexpr auto
		are_equivalent(const Win32StatusCode& lhs, const StatusCode<Domain2>& rhs) const noexcept
			-> bool {
			if constexpr(ConvertibleToGenericStatusCode<StatusCode<Domain2>>) {
				const auto as_generic_lhs = as_generic_code(lhs);
				const auto as_generic_rhs = rhs.as_generic_code();
				return as_generic_lhs.code() == as_generic_rhs.code();
			}
			else if(rhs.domain() == *this) {
				return lhs.code() == rhs.code();
			}
			else {
				return false;
			}
		}

		[[nodiscard]] constexpr auto
		as_generic_code(const Win32StatusCode& code) const noexcept -> GenericStatusCode { // NOLINT
			return make_status_code(to_generic_code(code.code()));
		}

		[[nodiscard]] constexpr inline auto success_value() const noexcept -> value_type { // NOLINT
			return 0;
		}

		template<typename Domain>
		friend constexpr auto
		operator==(const Win32Domain& lhs, const Domain& rhs) noexcept -> bool {
			return lhs.id() == rhs.id();
		}

		template<typename Domain>
		friend constexpr auto
		operator!=(const Win32Domain& lhs, const Domain& rhs) noexcept -> bool {
			return lhs.id() != rhs.id();
		}

		constexpr auto operator=(const Win32Domain&) noexcept -> Win32Domain& = default;
		constexpr auto operator=(Win32Domain&&) noexcept -> Win32Domain& = default;

	  private:
		u64 m_uuid = ID;

		/// @brief Gets the message string associated with the given win32 error code
		///
		/// This is largely based on Nial Douglas's implementation of `std::status_code` for
		/// WG21 proposal P1028, [status_code](https://github.com/ned14/status-code)
		/// @param code - The win32 error code to convert
		///
		/// @return the message string associated with `code`, as a `std::string`
		static inline auto as_string(value_type code) -> std::string {
			// Nial uses 32k in his implementation for proposal P1028, but that seems excessively
			// large
			wchar_t buffer[1024]; // NOLINT
			// get the win32 system error message for the error code
			detail::win32::DWORD wide_length = detail::win32::FormatMessageW(
				0x00001000 /**FORMAT_MESSAGE_FROM_SYSTEM**/			  // NOLINT
					| 0x00000200 /**FORMAT_MESSAGE_IGNORE_INSERTS**/, // NOLINT
				nullptr,
				code,
				0,
				buffer, // NOLINT
				1024,	// NOLINT
				nullptr);

			// bail if getting the message failed
			if(wide_length == 0) {
				return "failed to get message from system";
			}

			usize alloc_size = static_cast<usize>(wide_length) * 2;
			detail::win32::DWORD error = 0;
			// try to convert the "wide" error message string (UTF16, or UCS-2) to "narrow/normal"
			// UTF8
			do {
				auto* p = static_cast<char*>(malloc(alloc_size)); // NOLINT
				// if memory allocation fails, bail
				if(p == nullptr) {
					return "failed to get message from system";
				}

				// attempt to convert the system error message from "wide" string (UTF16 or UCS-2)
				// to "narrow/normal" UTF8
				detail::win32::DWORD bytes
					= detail::win32::WideCharToMultiByte(65001 /**CP_UTF8**/, // NOLINT
														 0,
														 buffer, // NOLINT
														 static_cast<int>(wide_length + 1),
														 p,
														 static_cast<int>(alloc_size),
														 nullptr,
														 nullptr);
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
				error = detail::win32::GetLastError();
			} while(error == 0x7a /**ERROR_INSUFFICIENT_BUFFER**/); // NOLINT

			return "failed to get message from system";
		}

		/// @brief converts relevant win32 error codes to POSIX error codes
		///
		/// This is extracted from Nial Douglas's implementation of `std::status_code` for
		/// WG21 proposal P1028, [status_code](https://github.com/ned14/status-code)
		///
		/// @param code - The win32 error code to convert to its corresponding POSIX error code
		///
		/// @return `code` as its corresponding POSIX error code
		static constexpr inline auto to_generic_code(value_type code) noexcept -> Errno {
			switch(code) {
				case 0: return Errno::Success;
				case 0x1: return Errno::FunctionNotSupported;			// NOLINT
				case 0x2: return Errno::NoSuchFileOrDirectory;			// NOLINT
				case 0x3: return Errno::NoSuchFileOrDirectory;			// NOLINT
				case 0x4: return Errno::TooManyFilesOpen;				// NOLINT
				case 0x5: return Errno::PermissionDenied;				// NOLINT
				case 0x6: return Errno::InvalidArgument;				// NOLINT
				case 0x8: return Errno::NotEnoughMemory;				// NOLINT
				case 0xc: return Errno::PermissionDenied;				// NOLINT
				case 0xe: return Errno::NotEnoughMemory;				// NOLINT
				case 0xf: return Errno::NoSuchDevice;					// NOLINT
				case 0x10: return Errno::PermissionDenied;				// NOLINT
				case 0x11: return Errno::CrossDeviceLink;				// NOLINT
				case 0x13: return Errno::PermissionDenied;				// NOLINT
				case 0x14: return Errno::NoSuchDevice;					// NOLINT
				case 0x15: return Errno::ResourceUnavailableTryAgain;	// NOLINT
				case 0x19: return Errno::IOError;						// NOLINT
				case 0x1d: return Errno::IOError;						// NOLINT
				case 0x1e: return Errno::IOError;						// NOLINT
				case 0x20: return Errno::PermissionDenied;				// NOLINT
				case 0x21: return Errno::NoLockAvailable;				// NOLINT
				case 0x27: return Errno::NoSpaceOnDevice;				// NOLINT
				case 0x37: return Errno::NoSuchDevice;					// NOLINT
				case 0x50: return Errno::FileExists;					// NOLINT
				case 0x52: return Errno::PermissionDenied;				// NOLINT
				case 0x57: return Errno::InvalidArgument;				// NOLINT
				case 0x6e: return Errno::IOError;						// NOLINT
				case 0x6f: return Errno::FilenameTooLong;				// NOLINT
				case 0x70: return Errno::NoSpaceOnDevice;				// NOLINT
				case 0x7b: return Errno::InvalidArgument;				// NOLINT
				case 0x83: return Errno::InvalidArgument;				// NOLINT
				case 0x8e: return Errno::DeviceOrResourceBusy;			// NOLINT
				case 0x91: return Errno::DirectoryNotEmpty;				// NOLINT
				case 0xaa: return Errno::DeviceOrResourceBusy;			// NOLINT
				case 0xb7: return Errno::FileExists;					// NOLINT
				case 0xd4: return Errno::NoLockAvailable;				// NOLINT
				case 0x10b: return Errno::InvalidArgument;				// NOLINT
				case 0x3e3: return Errno::OperationCanceled;			// NOLINT
				case 0x3e6: return Errno::PermissionDenied;				// NOLINT
				case 0x3f3: return Errno::IOError;						// NOLINT
				case 0x3f4: return Errno::IOError;						// NOLINT
				case 0x3f5: return Errno::IOError;						// NOLINT
				case 0x4d5: return Errno::ResourceUnavailableTryAgain;	// NOLINT
				case 0x961: return Errno::DeviceOrResourceBusy;			// NOLINT
				case 0x964: return Errno::DeviceOrResourceBusy;			// NOLINT
				case 0x2714: return Errno::Interrupted;					// NOLINT
				case 0x2719: return Errno::BadFileDescriptor;			// NOLINT
				case 0x271d: return Errno::PermissionDenied;			// NOLINT
				case 0x271e: return Errno::BadAddress;					// NOLINT
				case 0x2726: return Errno::InvalidArgument;				// NOLINT
				case 0x2728: return Errno::TooManyFilesOpen;			// NOLINT
				case 0x2733: return Errno::OperationWouldBlock;			// NOLINT
				case 0x2734: return Errno::OperationInProgress;			// NOLINT
				case 0x2735: return Errno::ConnectionAlreadyInProgress; // NOLINT
				case 0x2736: return Errno::NotASocket;					// NOLINT
				case 0x2737: return Errno::DestinationAddressRequired;	// NOLINT
				case 0x2738: return Errno::MessageSize;					// NOLINT
				case 0x2739: return Errno::WrongProtocolType;			// NOLINT
				case 0x273a: return Errno::NoProtocolOption;			// NOLINT
				case 0x273b: return Errno::ProtocolNotSupported;		// NOLINT
				case 0x273d: return Errno::NotSupported;				// NOLINT
				case 0x273f: return Errno::AddressFamilyNotSupported;	// NOLINT
				case 0x2740: return Errno::AddressInUse;				// NOLINT
				case 0x2741: return Errno::AddressNotAvailable;			// NOLINT
				case 0x2742: return Errno::NetworkDown;					// NOLINT
				case 0x2743: return Errno::NetworkUnreachable;			// NOLINT
				case 0x2744: return Errno::NetworkReset;				// NOLINT
				case 0x2745: return Errno::ConnectionAborted;			// NOLINT
				case 0x2746: return Errno::ConnectionReset;				// NOLINT
				case 0x2747: return Errno::NoBufferSpace;				// NOLINT
				case 0x2748: return Errno::AlreadyConnected;			// NOLINT
				case 0x2749: return Errno::NotConnected;				// NOLINT
				case 0x274c: return Errno::TimedOut;					// NOLINT
				case 0x274d: return Errno::ConnectionRefused;			// NOLINT
				case 0x274f: return Errno::FilenameTooLong;				// NOLINT
				case 0x2751: return Errno::HostUnreachable;				// NOLINT
				default: return Errno::Unknown;
			}
		}
	};
} // namespace hyperion::error

template<>
constexpr inline auto
make_status_code_domain<hyperion::error::Win32Domain>() noexcept -> hyperion::error::Win32Domain {
	return {};
}

namespace hyperion::error {
	static_assert(StatusCodeDomain<Win32Domain>);
} // namespace hyperion::error
#endif // HYPERION_PLATFORM_WINDOWS
