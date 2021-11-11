/// @file GenericDomain.h
/// @author Braxton Salyer <braxtonsalyer@gmail.com>
/// @brief `StatusCodeDomain` supporting the values of `errno` __required__ by POSIX
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

#include <Hyperion/Ignore.h>
#include <Hyperion/error/StatusCode.h>
#include <cerrno>

namespace hyperion::error {

	/// @brief POSIX required supported `errno` values
	///
	/// `Errno` represents the __required__ supported values of `errno` on a POSIX compliant system,
	/// and will always agree with the system's POSIX implementation. `Errno` may not represent all
	/// of a system's supported `errno` values if the system implements additional values other than
	/// those strictly required by POSIX.
	/// @ingroup error
	/// @headerfile "Hyperion/error/GenericDomain.h"
	enum class Errno : i32 {
		Success = 0,
		AddressFamilyNotSupported = EAFNOSUPPORT,
		AddressInUse = EADDRINUSE,
		AddressNotAvailable = EADDRNOTAVAIL,
		AlreadyConnected = EISCONN,
		ArgumentListTooLong = E2BIG,
		ArgumentOutOfDomain = EDOM,
		BadAddress = EFAULT,
		BadFileDescriptor = EBADF,
		BadMessage = EBADMSG,
		BrokenPipe = EPIPE,
		ConnectionAborted = ECONNABORTED,
		ConnectionAlreadyInProgress = EALREADY,
		ConnectionRefused = ECONNREFUSED,
		ConnectionReset = ECONNRESET,
		CrossDeviceLink = EXDEV,
		DestinationAddressRequired = EDESTADDRREQ,
		DeviceOrResourceBusy = EBUSY,
		DirectoryNotEmpty = ENOTEMPTY,
		ExecutableFormatError = ENOEXEC,
		FileExists = EEXIST,
		FileTooLarge = EFBIG,
		FilenameTooLong = ENAMETOOLONG,
		FunctionNotSupported = ENOSYS,
		HostUnreachable = EHOSTUNREACH,
		IdentifierRemoved = EIDRM,
		IllegalByteSequence = EILSEQ,
		InappropriateIOControlOperation = ENOTTY,
		Interrupted = EINTR,
		InvalidArgument = EINVAL,
		InvalidSeek = ESPIPE,
		IOError = EIO,
		IsADirectory = EISDIR,
		MessageSize = EMSGSIZE,
		NetworkDown = ENETDOWN,
		NetworkReset = ENETRESET,
		NetworkUnreachable = ENETUNREACH,
		NoBufferSpace = ENOBUFS,
		NoChildProcess = ECHILD,
		NoLink = ENOLINK,
		NoLockAvailable = ENOLCK,
		NoMessageAvailable = ENODATA,
		NoMessage = ENOMSG,
		NoProtocolOption = ENOPROTOOPT,
		NoSpaceOnDevice = ENOSPC,
		NoStreamResources = ENOSR,
		NoSuchDeviceOrAddress = ENXIO,
		NoSuchDevice = ENODEV,
		NoSuchFileOrDirectory = ENOENT,
		NoSuchProcess = ESRCH,
		NotADirectory = ENOTDIR,
		NotASocket = ENOTSOCK,
		NotAStream = ENOSTR,
		NotConnected = ENOTCONN,
		NotEnoughMemory = ENOMEM,
		NotSupported = ENOTSUP,
		OperationCanceled = ECANCELED,
		OperationInProgress = EINPROGRESS,
		OperationNotPermitted = EPERM,
		OperationNotSupported = EOPNOTSUPP,
		OperationWouldBlock = EWOULDBLOCK,
		OwnerDead = EOWNERDEAD,
		PermissionDenied = EACCES,
		ProtocolError = EPROTO,
		ProtocolNotSupported = EPROTONOSUPPORT,
		ReadOnlyFileSystem = EROFS,
		ResourceDeadlockWouldOccur = EDEADLK,
		ResourceUnavailableTryAgain = EAGAIN,
		ResultOutOfRange = ERANGE,
		StateNotRecoverable = ENOTRECOVERABLE,
		StreamTimeout = ETIME,
		TextFileBusy = ETXTBSY,
		TimedOut = ETIMEDOUT,
		TooManyFilesOpenInSystem = ENFILE,
		TooManyFilesOpen = EMFILE,
		TooManyLinks = EMLINK,
		TooManySymbolicLinkLevels = ELOOP,
		ValueTooLarge = EOVERFLOW,
		WrongProtocolType = EPROTOTYPE,
		Unknown = -1
	};

	/// @brief `GenericDomain` is the `StatusCodeDomain` that covers status codes matching those
	/// strictly required by POSIX (those represented by `Errno`).
	///
	/// `GenericDomain` does not cover additional values of `errno` used or provided by a system's
	/// specific implementation. For an extended support POSIX-compliant `StatusCodeDomain`,
	/// @see `PosixDomain`.
	/// @ingroup error
	/// @headerfile "Hyperion/error/GenericDomain.h"
	class [[nodiscard("A StatusCodeDomain should always be used")]] GenericDomain {
	  public:
		/// @brief The value type of `GenericDomain` status codes is `Errno`
		/// @ingroup error
		using value_type = Errno;

		static const constexpr char(&UUID)[num_chars_in_uuid] // NOLINT
			= "bb14ea47-2e32-4296-8ff8-1f9e2660ccc6";

		static constexpr u64 ID = parse_uuid_from_string(UUID);

		/// @brief Constructs a `GenericDomain` with the default UUID
		/// @ingroup error
		constexpr GenericDomain() noexcept = default;
		/// @brief Constructs a `GenericDomain` with a user-specific UUID
		///
		/// @note When using a custom UUID __**ALL**__ instances of `GenericDomain` in the program
		/// should be constructed with the same custom UUID, otherwise equality comparison between
		/// other domains and `GenericDomain` instances could give erroneous results, and equality
		/// comparison between different `GenericDomain` instances will give erroneous results.
		/// As a result, this constructor should only be used when you specifically require a custom
		/// UUID and **YOU KNOW WHAT YOU ARE DOING™**
		///
		/// @param uuid - The UUID to use for `GenericDomain`
		/// @ingroup error
		explicit constexpr GenericDomain(u64 uuid) noexcept : m_uuid(uuid) {
		}
		/// @brief Constructs a `GenericDomain` with a user-specific UUID
		///
		/// @note When using a custom UUID __**ALL**__ instances of `GenericDomain` in the program
		/// should be constructed with the same custom UUID, otherwise equality comparison between
		/// other domains and `GenericDomain` instances could give erroneous results, and equality
		/// comparison between different `GenericDomain` instances will give erroneous results.
		/// As a result, this constructor should only be used when you specifically require a custom
		/// UUID and **YOU KNOW WHAT YOU ARE DOING™**
		///
		/// @param uuid - The UUID to use for `GenericDomain`
		/// @ingroup error
		template<UUIDString UUID>
		explicit constexpr GenericDomain(UUID && uuid) noexcept // NOLINT (forwarding reference)
			: m_uuid(parse_uuid_from_string(std::forward<UUID>(uuid))) {
		}
		/// @brief Copy-Constructor
		/// @ingroup error
		constexpr GenericDomain(const GenericDomain&) = default;
		/// @brief Move-Constructor
		/// @ingroup error
		constexpr GenericDomain(GenericDomain &&) = default;
		/// @brief Destructor
		/// @ingroup error
		constexpr ~GenericDomain() noexcept = default;

		/// @brief Returns the UUID of the domain
		///
		/// @return the domain UUID
		/// @ingroup error
		[[nodiscard]] inline constexpr auto id() const noexcept->u64 {
			return m_uuid;
		}

		/// @brief Returns the name of the domain
		///
		/// @return the domain name
		/// @ingroup error
		[[nodiscard]] inline constexpr auto name() const noexcept->std::string_view { // NOLINT
			return "generic domain";
		}

		/// @brief Returns the textual message associated with the given status code
		///
		/// @param code - The status code to get the message for
		///
		/// @return the message associated with the code
		/// @ingroup error
		[[nodiscard]] inline constexpr auto message(value_type code) // NOLINT
			const noexcept->std::string_view {

			return to_posix_message(code);
		}

		/// @brief Returns the textual message associated with the given status code
		///
		/// @param code - The status code to get the message for
		///
		/// @return the message associated with the code
		/// @ingroup error
		[[nodiscard]] inline constexpr auto message(const GenericStatusCode& code) // NOLINT
			const noexcept->std::string_view {

			return to_posix_message(code.code());
		}

		/// @brief Returns whether the given status code represents an error
		///
		/// @param code - The status code to check
		///
		/// @return `true` if the code represents an error, otherwise `false`
		/// @ingroup error
		[[nodiscard]] inline constexpr auto is_error(const GenericStatusCode& code) // NOLINT
			const noexcept->bool {
			return code.code() != Errno::Success;
		}

		/// @brief Returns whether the given status code represents success
		///
		/// @param code - The status code to check
		///
		/// @return `true` if the code represents success, otherwise `false`
		/// @ingroup error
		[[nodiscard]] inline constexpr auto is_success(const GenericStatusCode& code) // NOLINT
			const noexcept->bool {
			return code.code() == Errno::Success;
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
		[[nodiscard]] inline constexpr auto are_equivalent(const GenericStatusCode& lhs,
														   const StatusCode<Domain>& rhs)
			const noexcept->bool {
			if constexpr(ConvertibleToGenericStatusCode<StatusCode<Domain>>) {
				const auto as_generic = rhs.as_generic_code();
				const auto lhs_code = lhs.code();
				const auto as_generic_code = as_generic.code();
				return lhs_code == as_generic_code && lhs_code != Errno::Unknown
					   && as_generic_code != Errno::Unknown;
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
		[[nodiscard]] inline constexpr auto as_generic_code(
			const ConvertibleToGenericStatusCode auto& code) const noexcept->GenericStatusCode {
			return code.domain().as_generic_code(code);
		}

		/// @brief Returns the value indicating success for this domain
		///
		/// @return The domain's success value
		/// @ingroup error
		[[nodiscard]] static inline constexpr auto success_value() noexcept->value_type {
			return Errno::Success;
		}

		/// @brief Returns the most recent value of `errno`
		///
		/// @return the value of `errno` at the time of the call
		/// @ingroup error
		[[nodiscard]] static inline auto get_last_error() noexcept->value_type {
			return static_cast<Errno>(errno);
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
		friend inline constexpr auto operator==(const GenericDomain& lhs,
												const Domain& rhs) noexcept->bool {
			return rhs.id() == lhs.id();
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
		friend inline constexpr auto operator!=(const GenericDomain& lhs,
												const Domain& rhs) noexcept->bool {
			return rhs.id() != lhs.id();
		}

		/// @brief Copy-assignment operator
		/// @ingroup error
		constexpr auto operator=(const GenericDomain&) noexcept->GenericDomain& = default;
		/// @brief Move-assignment operator
		/// @ingroup error
		constexpr auto operator=(GenericDomain&&) noexcept->GenericDomain& = default;

	  private:
		u64 m_uuid = ID;

		[[nodiscard]] static inline constexpr auto to_posix_message(
			Errno code) noexcept->std::string_view {
			switch(code) {
				case Errno::Success: return "Success.";
				case Errno::AddressFamilyNotSupported: return "Address family not supported.";
				case Errno::AddressInUse: return "Address in use.";
				case Errno::AddressNotAvailable: return "Address not available.";
				case Errno::AlreadyConnected: return "Socket is already connected.";
				case Errno::ArgumentListTooLong: return "Argument list is too long.";
				case Errno::ArgumentOutOfDomain:
					return "Mathematics argument out of domain of function.";
				case Errno::BadAddress: return "Bad address.";
				case Errno::BadFileDescriptor: return "Bad file descriptor.";
				case Errno::BadMessage: return "Bad message.";
				case Errno::BrokenPipe: return "Broken pipe.";
				case Errno::ConnectionAborted: return "Connection aborted.";
				case Errno::ConnectionAlreadyInProgress: return "Connection already in progress.";
				case Errno::ConnectionRefused: return "Connection refused.";
				case Errno::ConnectionReset: return "Connection reset.";
				case Errno::CrossDeviceLink: return "Cross-device link.";
				case Errno::DestinationAddressRequired: return "Destination address required.";
				case Errno::DeviceOrResourceBusy: return "Device or resource busy.";
				case Errno::DirectoryNotEmpty: return "Directory not empty.";
				case Errno::ExecutableFormatError: return "Executable file format error.";
				case Errno::FileExists: return "File exists.";
				case Errno::FileTooLarge: return "File too large.";
				case Errno::FilenameTooLong: return "Filename too long.";
				case Errno::FunctionNotSupported: return "Functionality not supported.";
				case Errno::HostUnreachable: return "Host is unreachable.";
				case Errno::IdentifierRemoved: return "Identifier removed.";
				case Errno::IllegalByteSequence: return "Illegal byte sequence.";
				case Errno::InappropriateIOControlOperation:
					return "Inappropriate I/O control operation.";
				case Errno::Interrupted: return "Interrupted function.";
				case Errno::InvalidArgument: return "Invalid argument.";
				case Errno::InvalidSeek: return "Invalid seek.";
				case Errno::IOError: return "I/O error.";
				case Errno::IsADirectory: return "Is a directory.";
				case Errno::MessageSize: return "Message too large.";
				case Errno::NetworkDown: return "Network is down.";
				case Errno::NetworkReset: return "Connection aborted by network.";
				case Errno::NetworkUnreachable: return "Network unreachable.";
				case Errno::NoBufferSpace: return "No buffer space available.";
				case Errno::NoChildProcess: return "No child process.";
				case Errno::NoLink: return "Link has been severed.";
				case Errno::NoLockAvailable: return "No locks available.";
				case Errno::NoMessageAvailable:
					return "No message is available on the STREAM head read queue.";
				case Errno::NoMessage: return "No message of the desired type.";
				case Errno::NoProtocolOption: return "Protocol not available.";
				case Errno::NoSpaceOnDevice: return "No space left on device.";
				case Errno::NoStreamResources: return "No STREAM resources.";
				case Errno::NoSuchDeviceOrAddress: return "No such device or address.";
				case Errno::NoSuchDevice: return "No such device.";
				case Errno::NoSuchFileOrDirectory: return "No such file or directory.";
				case Errno::NoSuchProcess: return "No such process.";
				case Errno::NotADirectory:
					return "Not a directory or symbolic link to a directory.";
				case Errno::NotASocket: return "Not a socket.";
				case Errno::NotAStream: return "Not a STREAM.";
				case Errno::NotConnected: return "The socket is not connected.";
				case Errno::NotEnoughMemory: return "Not enough space.";
#if ENOTSUP != EOPNOTSUPP
				case Errno::NotSupported: return "Operation not supported.";
#endif // ENOTSUP != EOPNOTSUPP
				case Errno::OperationCanceled: return "Operation canceled.";
				case Errno::OperationInProgress: return "Operation in progress.";
				case Errno::OperationNotPermitted: return "Operation not permitted.";
				case Errno::OperationNotSupported: return "Operation not supported on socket.";
#if EWOULDBLOCK != EAGAIN
				case Errno::OperationWouldBlock: return "Operation would block.";
#endif // EWOULDBLOCK != EAGAIN
				case Errno::OwnerDead: return "Previous owner died.";
				case Errno::PermissionDenied: return "Permission denied.";
				case Errno::ProtocolError: return "Protocol error.";
				case Errno::ProtocolNotSupported: return "Protocol not supported.";
				case Errno::ReadOnlyFileSystem: return "Read-only file system.";
				case Errno::ResourceDeadlockWouldOccur: return "Resource deadlock would occur.";
				case Errno::ResourceUnavailableTryAgain: return "Resource unavailable, try again.";
				case Errno::ResultOutOfRange: return "Result out of range (Result too large).";
				case Errno::StateNotRecoverable: return "State not recoverable.";
				case Errno::StreamTimeout: return "STREAM timeout.";
				case Errno::TextFileBusy: return "Text file busy.";
				case Errno::TimedOut: return "Connection timed out.";
				case Errno::TooManyFilesOpenInSystem: return "Too many files open in system.";
				case Errno::TooManyFilesOpen:
					return "Too many files open (file descriptor value too large).";
				case Errno::TooManyLinks: return "Too many links.";
				case Errno::TooManySymbolicLinkLevels: return "Too many levels of symbolic links.";
				case Errno::ValueTooLarge: return "Value too large to be stored in data type.";
				case Errno::WrongProtocolType: return "Wrong protocol type for socket.";
				case Errno::Unknown: return "Unknown error occurred.";
			}
		}
	};
} // namespace hyperion::error

/// @brief Specialize `make_status_code_domain` for `GenericDomain` and `u64`.
/// Creates a `GenericDomain` with a custom UUID.
///
/// @note When using a custom UUID __**ALL**__ instances of `GenericDomain` in the program
/// should be constructed with the same custom UUID, otherwise equality comparison between
/// other domains and `GenericDomain` instances could give erroneous results, and equality
/// comparison between different `GenericDomain` instances will give erroneous results.
/// As a result, this constructor should only be used when you specifically require a custom
/// UUID and **YOU KNOW WHAT YOU ARE DOING™**
///
/// @param uuid - The UUID to use for `GenericDomain`
///
/// @return a `GenericDomain`
/// @ingroup error
template<>
[[nodiscard]] inline constexpr auto
// NOLINTNEXTLINE(readability-inconsistent-declaration-parameter-name)
make_status_code_domain<hyperion::error::GenericDomain, hyperion::u64>(
	hyperion::u64&& uuid) noexcept -> hyperion::error::GenericDomain {
	return hyperion::error::GenericDomain(uuid);
}

/// @brief Specialize `make_status_code_domain` for `GenericDomain` and a `UUIDString`.
/// Creates a `GenericDomain` with a custom UUID.
///
/// @note When using a custom UUID __**ALL**__ instances of `GenericDomain` in the program
/// should be constructed with the same custom UUID, otherwise equality comparison between
/// other domains and `GenericDomain` instances could give erroneous results, and equality
/// comparison between different `GenericDomain` instances will give erroneous results.
/// As a result, this constructor should only be used when you specifically require a custom
/// UUID and **YOU KNOW WHAT YOU ARE DOING™**
///
/// @param uuid - The UUID to use for `GenericDomain`
///
/// @return a `GenericDomain`
/// @ingroup error
template<>
[[nodiscard]] inline constexpr auto
	// NOLINTNEXTLINE(readability-inconsistent-declaration-parameter-name)
	make_status_code_domain<hyperion::error::GenericDomain,
							const char (&)[hyperion::error::num_chars_in_uuid]> // NOLINT
	(const char (&uuid)[hyperion::error::num_chars_in_uuid]) noexcept			// NOLINT
	-> hyperion::error::GenericDomain {
	return hyperion::error::GenericDomain(uuid);
}

/// @brief Specialize `make_status_code_domain` for `GenericDomain` and an MS-style `UUIDString`.
/// Creates a `GenericDomain` with a custom UUID.
///
/// @note When using a custom UUID __**ALL**__ instances of `GenericDomain` in the program
/// should be constructed with the same custom UUID, otherwise equality comparison between
/// other domains and `GenericDomain` instances could give erroneous results, and equality
/// comparison between different `GenericDomain` instances will give erroneous results.
/// As a result, this constructor should only be used when you specifically require a custom
/// UUID and **YOU KNOW WHAT YOU ARE DOING™**
///
/// @param uuid - The UUID to use for `GenericDomain`
///
/// @return a `GenericDomain`
/// @ingroup error
template<>
[[nodiscard]] inline constexpr auto
	// NOLINTNEXTLINE(readability-inconsistent-declaration-parameter-name)
	make_status_code_domain<hyperion::error::GenericDomain,
							const char (&)[hyperion::error::num_chars_in_ms_uuid]> // NOLINT
	(const char (&uuid)[hyperion::error::num_chars_in_ms_uuid]) noexcept		   // NOLINT
	-> hyperion::error::GenericDomain {
	return hyperion::error::GenericDomain(uuid);
}

/// @brief Specialize `make_status_code_domain` for `GenericDomain` with no arguments. Creates
/// a `GenericDomain` with the default UUID.
///
/// @return a `GenericDomain`
/// @ingroup error
template<>
inline constexpr auto make_status_code_domain<hyperion::error::GenericDomain>() noexcept
	-> hyperion::error::GenericDomain {
	return {};
}

namespace hyperion::error {

	static_assert(StatusCodeDomain<GenericDomain>);

	/// @brief Specialize `status_code_enum_info` for `Errno`, registering that `Errno` is a
	/// `StatusCodeEnum` associated with the `GenericDomain`
	/// @ingroup error
	template<>
	struct status_code_enum_info<Errno> {
		using domain_type = GenericDomain;
		static constexpr bool value = true;
	};
} // namespace hyperion::error
