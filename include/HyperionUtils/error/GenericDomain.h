#pragma once

#include <cerrno>

#include "../Ignore.h"
#include "StatusCode.h"

namespace hyperion::error {

	enum class Errno : i32
	{
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

	class [[nodiscard]] GenericDomain {
	  public:
		using value_type = Errno;

		static constexpr const char (&UUID)[num_chars_in_uuid] // NOLINT
			= "bb14ea47-2e32-4296-8ff8-1f9e2660ccc6";

		static constexpr u64 ID = parse_uuid_from_string(UUID);

		constexpr GenericDomain() noexcept = default;
		explicit constexpr GenericDomain(u64 uuid) noexcept : m_uuid(uuid) {
		}
		explicit constexpr GenericDomain(const UUIDString auto& uuid) noexcept
			: m_uuid(parse_uuid_from_string(uuid)) {
		}
		constexpr ~GenericDomain() noexcept = default;
		constexpr GenericDomain(const GenericDomain&) = default;
		constexpr GenericDomain(GenericDomain&&) = default;

		[[nodiscard]] constexpr inline auto id() const noexcept -> u64 {
			return m_uuid;
		}

		[[nodiscard]] constexpr inline auto name() const noexcept -> std::string_view { // NOLINT
			return "generic domain";
		}

		[[nodiscard]] constexpr inline auto message(value_type code) // NOLINT
			const noexcept -> std::string_view {

			return to_posix_message(code);
		}

		[[nodiscard]] constexpr inline auto message(const GenericStatusCode& code) // NOLINT
			const noexcept -> std::string_view {

			return to_posix_message(code.code());
		}

		[[nodiscard]] constexpr inline auto
		is_error(const GenericStatusCode& code) const noexcept -> bool { // NOLINT
			return code.code() != Errno::Success;
		}

		[[nodiscard]] constexpr inline auto
		is_success(const GenericStatusCode& code) const noexcept -> bool { // NOLINT
			return code.code() == Errno::Success;
		}

		template<typename Domain2>
		[[nodiscard]] constexpr inline auto
		are_equivalent(const GenericStatusCode& lhs, const StatusCode<Domain2>& rhs) const noexcept
			-> bool {
			if constexpr(ConvertibleToGenericStatusCode<StatusCode<Domain2>>) {
				const auto as_generic = rhs.as_generic_code();
				return lhs.code() == as_generic.code();
			}
			else {
				return false;
			}
		}

		[[nodiscard]] constexpr inline auto
		as_generic_code(const ConvertibleToGenericStatusCode auto& code) const noexcept
			-> GenericStatusCode {
			return code.domain().as_generic_code(code);
		}

		[[nodiscard]] constexpr inline auto success_value() const noexcept -> value_type { // NOLINT
			return Errno::Success;
		}

		template<typename Domain>
		friend constexpr inline auto
		operator==(const GenericDomain& lhs, const Domain& rhs) noexcept -> bool {
			return rhs.id() == lhs.id();
		}

		template<typename Domain>
		friend constexpr inline auto
		operator!=(const GenericDomain& lhs, const Domain& rhs) noexcept -> bool {
			return rhs.id() != lhs.id();
		}

		constexpr auto operator=(const GenericDomain&) noexcept -> GenericDomain& = default;
		constexpr auto operator=(GenericDomain&&) noexcept -> GenericDomain& = default;

	  private:
		u64 m_uuid = ID;

		static constexpr inline auto to_posix_message(Errno code) noexcept -> std::string_view {
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
				case Errno::DestinationAddressRequired: return "Desination address required.";
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
				case Errno::Unknown: return "Unkown error occurred.";
			}
		}
	};

} // namespace hyperion::error
template<>
constexpr inline auto make_status_code_domain<hyperion::error::GenericDomain>() noexcept
	-> hyperion::error::GenericDomain {
	return {};
}

namespace hyperion::error {

	static_assert(StatusCodeDomain<GenericDomain>);

	template<>
	struct status_code_enum_info<Errno> {
		using domain_type = GenericDomain;
		static constexpr bool value = true;
	};
} // namespace hyperion::error
