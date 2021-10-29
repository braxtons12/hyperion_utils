/// @file PosixDomain.h
/// @author Braxton Salyer <braxtonsalyer@gmail.com>
/// @brief `StatusCodeDomain` supporting platform-implementation-specific values of `errno` in
/// addition to those __required__ by POSIX
/// @version 0.1
/// @date 2021-10-16
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
#include <cstring>
#include <gsl/gsl>

namespace hyperion::error {

	class PosixDomain;

	using PosixStatusCode = StatusCode<PosixDomain>;
	using PosixErrorCode = ErrorCode<PosixDomain>;

	/// @brief `PosixDomain` is the `StatusCodeDomain` that covers status codes covering a
	/// platform's specific implementation of `errno` values in addition to those __strictly__
	/// required by POSIX (those represented by `Errno`).
	/// @ingroup error
	/// @headerfile "Hyperion/error/PosixDomain.h"
	class [[nodiscard]] PosixDomain {
	  public:
		/// @brief The value type of `PosixDomain` status codes is `i64`
		/// @ingroup error
		using value_type = i64;

		static const constexpr char (&UUID)[num_chars_in_uuid] // NOLINT
			= "4a6a9b0f-c335-473e-bc42-d23974a25bb0";

		static constexpr u64 ID = parse_uuid_from_string(UUID);

		/// @brief Constructs a `PosixDomain` with the default UUID
		/// @ingroup error
		constexpr PosixDomain() noexcept = default;
		/// @brief Constructs a `PosixDomain` with a user-specific UUID
		///
		/// @note When using a custom UUID __**ALL**__ instances of `PosixDomain` in the program
		/// should be constructed with the same custom UUID, otherwise equality comparison between
		/// other domains and `PosixDomain` instances could give erroneous results, and equality
		/// comparison between different `PosixDomain` instances will give erroneous results.
		/// As a result, this constructor should only be used when you specifically require a custom
		/// UUID and **YOU KNOW WHAT YOU ARE DOING™**
		///
		/// @param uuid - The UUID to use for `PosixDomain`
		/// @ingroup error
		explicit constexpr PosixDomain(u64 uuid) noexcept : m_uuid(uuid) {
		}
		/// @brief Constructs a `PosixDomain` with a user-specific UUID
		///
		/// @note When using a custom UUID __**ALL**__ instances of `PosixDomain` in the program
		/// should be constructed with the same custom UUID, otherwise equality comparison between
		/// other domains and `PosixDomain` instances could give erroneous results, and equality
		/// comparison between different `PosixDomain` instances will give erroneous results.
		/// As a result, this constructor should only be used when you specifically require a custom
		/// UUID and **YOU KNOW WHAT YOU ARE DOING™**
		///
		/// @param uuid - The UUID to use for `PosixDomain`
		/// @ingroup error
		template<UUIDString UUID>
		explicit constexpr PosixDomain(UUID&& uuid) noexcept // NOLINT (forwarding reference)
			: m_uuid(parse_uuid_from_string(std::forward<UUID>(uuid))) {
		}
		/// @brief Copy-Constructor
		/// @ingroup error
		constexpr PosixDomain(const PosixDomain&) noexcept = default;
		/// @brief Move-Constructor
		/// @ingroup error
		constexpr PosixDomain(PosixDomain&&) noexcept = default;
		/// @brief Destructor
		/// @ingroup error
		constexpr ~PosixDomain() noexcept = default;

		/// @brief Returns the UUID of the domain
		///
		/// @return the domain UUID
		/// @ingroup error
		[[nodiscard]] constexpr auto id() const noexcept -> u64 {
			return m_uuid;
		}

		/// @brief Returns the name of the domain
		///
		/// @return the domain name
		/// @ingroup error
		[[nodiscard]] constexpr auto name() const noexcept -> std::string_view { // NOLINT
			return "POSIX domain";
		}

		/// @brief Returns the textual message associated with the given status code
		///
		/// @param code - The status code to get the message for
		///
		/// @return the message associated with the code
		/// @ingroup error
		[[nodiscard]] auto message(value_type code) // NOLINT
			const noexcept -> std::string {

			return as_string(code);
		}

		/// @brief Returns the textual message associated with the given status code
		///
		/// @param code - The status code to get the message for
		///
		/// @return the message associated with the code
		/// @ingroup error
		[[nodiscard]] auto message(const PosixStatusCode& code) // NOLINT
			const noexcept -> std::string {

			return as_string(code.code());
		}

		/// @brief Returns whether the given status code represents an error
		///
		/// @param code - The status code to check
		///
		/// @return `true` if the code represents an error, otherwise `false`
		/// @ingroup error
		[[nodiscard]] constexpr auto
		is_error(const PosixStatusCode& code) const noexcept -> bool { // NOLINT
			return code.code() != 0;
		}

		/// @brief Returns whether the given status code represents success
		///
		/// @param code - The status code to check
		///
		/// @return `true` if the code represents success, otherwise `false`
		/// @ingroup error
		[[nodiscard]] constexpr auto
		is_success(const PosixStatusCode& code) const noexcept -> bool { // NOLINT
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
		[[nodiscard]] constexpr auto
		are_equivalent(const PosixStatusCode& lhs, const StatusCode<Domain>& rhs) const noexcept
			-> bool {
			if constexpr(ConvertibleToGenericStatusCode<StatusCode<Domain>>) {
				return as_generic_code(lhs) == rhs.as_generic_code();
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
		[[nodiscard]] constexpr auto
		as_generic_code(const PosixStatusCode& code) const noexcept -> GenericStatusCode { // NOLINT
			return make_status_code(to_generic_code(code.code()));
		}

		/// @brief Returns the value indicating success for this domain
		///
		/// @return The domain's success value
		/// @ingroup error
		[[nodiscard]] inline static constexpr auto success_value() noexcept -> value_type {
			return 0;
		}

		/// @brief Returns the most recent value of `errno`
		///
		/// @return the value of `errno` at the time of the call
		/// @ingroup error
		[[nodiscard]] inline static auto get_last_error() noexcept -> value_type {
			return errno;
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
		friend constexpr auto
		operator==(const PosixDomain& lhs, const Domain& rhs) noexcept -> bool {
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
		friend constexpr auto
		operator!=(const PosixDomain& lhs, const Domain& rhs) noexcept -> bool {
			return lhs.id() != rhs.id();
		}

		/// @brief Copy-assignment operator
		/// @ingroup error
		constexpr auto operator=(const PosixDomain&) noexcept -> PosixDomain& = default;
		/// @brief Move-assignment operator
		/// @ingroup error
		constexpr auto operator=(PosixDomain&&) noexcept -> PosixDomain& = default;

	  private:
		u64 m_uuid = ID;

		inline static auto as_string(value_type code) noexcept -> std::string {
			char buffer[1024]; // NOLINT
#if HYPERION_PLATFORM_WINDOWS
			strerror_s(buffer, 1024, gsl::narrow_cast<i32>(code)); // NOLINT
#elif defined(__gnu_linux__) && !defined(__ANDROID__)
			char* message = strerror_r(gsl::narrow_cast<i32>(code), buffer, 1024); // NOLINT
			if(message != nullptr) {
				strncpy(buffer, message, 1024); // NOLINT
				buffer[1023] = 0;				// NOLINT
			}
#else
			str_error_r(code, buffer, 1024); // NOLINT
#endif
			usize length = strlen(buffer);		// NOLINT
			return std::string(buffer, length); // NOLINT
		}

		inline static constexpr auto to_generic_code(value_type code) noexcept -> Errno {
			switch(code) {
				case 0: return Errno::Success;
				case EAFNOSUPPORT: return Errno::AddressFamilyNotSupported;
				case EADDRINUSE: return Errno::AddressInUse;
				case EADDRNOTAVAIL: return Errno::AddressNotAvailable;
				case EISCONN: return Errno::AlreadyConnected;
				case E2BIG: return Errno::ArgumentListTooLong;
				case EDOM: return Errno::ArgumentOutOfDomain;
				case EFAULT: return Errno::BadAddress;
				case EBADF: return Errno::BadFileDescriptor;
				case EBADMSG: return Errno::BadMessage;
				case EPIPE: return Errno::BrokenPipe;
				case ECONNABORTED: return Errno::ConnectionAborted;
				case EALREADY: return Errno::ConnectionAlreadyInProgress;
				case ECONNREFUSED: return Errno::ConnectionRefused;
				case ECONNRESET: return Errno::ConnectionReset;
				case EXDEV: return Errno::CrossDeviceLink;
				case EDESTADDRREQ: return Errno::DestinationAddressRequired;
				case EBUSY: return Errno::DeviceOrResourceBusy;
				case ENOTEMPTY: return Errno::DirectoryNotEmpty;
				case ENOEXEC: return Errno::ExecutableFormatError;
				case EEXIST: return Errno::FileExists;
				case EFBIG: return Errno::FileTooLarge;
				case ENAMETOOLONG: return Errno::FilenameTooLong;
				case ENOSYS: return Errno::FunctionNotSupported;
				case EHOSTUNREACH: return Errno::HostUnreachable;
				case EIDRM: return Errno::IdentifierRemoved;
				case EILSEQ: return Errno::IllegalByteSequence;
				case ENOTTY: return Errno::InappropriateIOControlOperation;
				case EINTR: return Errno::Interrupted;
				case EINVAL: return Errno::InvalidArgument;
				case ESPIPE: return Errno::InvalidSeek;
				case EIO: return Errno::IOError;
				case EISDIR: return Errno::IsADirectory;
				case EMSGSIZE: return Errno::MessageSize;
				case ENETDOWN: return Errno::NetworkDown;
				case ENETRESET: return Errno::NetworkReset;
				case ENETUNREACH: return Errno::NetworkUnreachable;
				case ENOBUFS: return Errno::NoBufferSpace;
				case ECHILD: return Errno::NoChildProcess;
				case ENOLINK: return Errno::NoLink;
				case ENOLCK: return Errno::NoLockAvailable;
				case ENODATA: return Errno::NoMessageAvailable;
				case ENOMSG: return Errno::NoMessage;
				case ENOPROTOOPT: return Errno::NoProtocolOption;
				case ENOSPC: return Errno::NoSpaceOnDevice;
				case ENOSR: return Errno::NoStreamResources;
				case ENXIO: return Errno::NoSuchDeviceOrAddress;
				case ENODEV: return Errno::NoSuchDevice;
				case ENOENT: return Errno::NoSuchFileOrDirectory;
				case ESRCH: return Errno::NoSuchProcess;
				case ENOTDIR: return Errno::NotADirectory;
				case ENOTSOCK: return Errno::NotASocket;
				case ENOSTR: return Errno::NotAStream;
				case ENOTCONN: return Errno::NotConnected;
				case ENOMEM: return Errno::NotEnoughMemory;
#if ENOTSUP != EOPNOTSUPP
				case ENOTSUP: return Errno::NotSupported;
#endif
				case ECANCELED: return Errno::OperationCanceled;
				case EINPROGRESS: return Errno::OperationInProgress;
				case EPERM: return Errno::OperationNotPermitted;
				case EOPNOTSUPP: return Errno::OperationNotSupported;
#if EWOULDBLOCK != EAGAIN
				case EWOULDBLOCK: return Errno::OperationWouldBlock;
#endif
				case EOWNERDEAD: return Errno::OwnerDead;
				case EACCES: return Errno::PermissionDenied;
				case EPROTO: return Errno::ProtocolError;
				case EPROTONOSUPPORT: return Errno::ProtocolNotSupported;
				case EROFS: return Errno::ReadOnlyFileSystem;
				case EDEADLK: return Errno::ResourceDeadlockWouldOccur;
				case EAGAIN: return Errno::ResourceUnavailableTryAgain;
				case ERANGE: return Errno::ResultOutOfRange;
				case ENOTRECOVERABLE: return Errno::StateNotRecoverable;
				case ETIME: return Errno::StreamTimeout;
				case ETXTBSY: return Errno::TextFileBusy;
				case ETIMEDOUT: return Errno::TimedOut;
				case ENFILE: return Errno::TooManyFilesOpenInSystem;
				case EMFILE: return Errno::TooManyFilesOpen;
				case EMLINK: return Errno::TooManyLinks;
				case ELOOP: return Errno::TooManySymbolicLinkLevels;
				case EOVERFLOW: return Errno::ValueTooLarge;
				case EPROTOTYPE: return Errno::WrongProtocolType;
				default: return Errno::Unknown;
			}
		}
	};
} // namespace hyperion::error

/// @brief Specialize `make_status_code_domain` for `PosixDomain` and `u64`.
/// Creates a `PosixDomain` with a custom UUID.
///
/// @note When using a custom UUID __**ALL**__ instances of `PosixDomain` in the program
/// should be constructed with the same custom UUID, otherwise equality comparison between
/// other domains and `PosixDomain` instances could give erroneous results, and equality
/// comparison between different `PosixDomain` instances will give erroneous results.
/// As a result, this constructor should only be used when you specifically require a custom
/// UUID and **YOU KNOW WHAT YOU ARE DOING™**
///
/// @param uuid - The UUID to use for `PosixDomain`
///
/// @return a `PosixDomain`
/// @ingroup error
template<>
[[nodiscard]] inline constexpr auto
// NOLINTNEXTLINE(readability-inconsistent-declaration-parameter-name)
make_status_code_domain<hyperion::error::PosixDomain, hyperion::u64>(hyperion::u64&& uuid) noexcept
	-> hyperion::error::PosixDomain {
	return hyperion::error::PosixDomain(uuid);
}

/// @brief Specialize `make_status_code_domain` for `PosixDomain` and a `UUIDString`.
/// Creates a `PosixDomain` with a custom UUID.
///
/// @note When using a custom UUID __**ALL**__ instances of `PosixDomain` in the program
/// should be constructed with the same custom UUID, otherwise equality comparison between
/// other domains and `PosixDomain` instances could give erroneous results, and equality
/// comparison between different `PosixDomain` instances will give erroneous results.
/// As a result, this constructor should only be used when you specifically require a custom
/// UUID and **YOU KNOW WHAT YOU ARE DOING™**
///
/// @param uuid - The UUID to use for `PosixDomain`
///
/// @return a `PosixDomain`
/// @ingroup error
template<>
[[nodiscard]] inline constexpr auto
	// NOLINTNEXTLINE(readability-inconsistent-declaration-parameter-name)
	make_status_code_domain<hyperion::error::PosixDomain,
							const char (&)[hyperion::error::num_chars_in_uuid]> // NOLINT
	(const char (&uuid)[hyperion::error::num_chars_in_uuid]) noexcept			// NOLINT
	-> hyperion::error::PosixDomain {
	return hyperion::error::PosixDomain(uuid);
}

/// @brief Specialize `make_status_code_domain` for `PosixDomain` and an MS-style `UUIDString`.
/// Creates a `PosixDomain` with a custom UUID.
///
/// @note When using a custom UUID __**ALL**__ instances of `PosixDomain` in the program
/// should be constructed with the same custom UUID, otherwise equality comparison between
/// other domains and `PosixDomain` instances could give erroneous results, and equality
/// comparison between different `PosixDomain` instances will give erroneous results.
/// As a result, this constructor should only be used when you specifically require a custom
/// UUID and **YOU KNOW WHAT YOU ARE DOING™**
///
/// @param uuid - The UUID to use for `PosixDomain`
///
/// @return a `PosixDomain`
/// @ingroup error
template<>
[[nodiscard]] inline constexpr auto
	// NOLINTNEXTLINE(readability-inconsistent-declaration-parameter-name)
	make_status_code_domain<hyperion::error::PosixDomain,
							const char (&)[hyperion::error::num_chars_in_ms_uuid]> // NOLINT
	(const char (&uuid)[hyperion::error::num_chars_in_ms_uuid]) noexcept		   // NOLINT
	-> hyperion::error::PosixDomain {
	return hyperion::error::PosixDomain(uuid);
}

/// @brief Specialize `make_status_code_domain` for `PosixDomain` with no arguments. Creates
/// a `PosixDomain` with the default UUID.
///
/// @return a `PosixDomain`
/// @ingroup error
template<>
inline constexpr auto
make_status_code_domain<hyperion::error::PosixDomain>() noexcept -> hyperion::error::PosixDomain {
	return {};
}

namespace hyperion::error {
	static_assert(StatusCodeDomain<PosixDomain>);
} // namespace hyperion::error
