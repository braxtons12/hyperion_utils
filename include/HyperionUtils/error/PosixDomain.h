#pragma once

#include <cstring>
#include <gsl/gsl>

#include "GenericDomain.h"
#include "StatusCode.h"

namespace hyperion::error {

	class PosixDomain;

	using PosixStatusCode = StatusCode<PosixDomain>;
	using PosixErrorCode = ErrorCode<PosixDomain>;

	class [[nodiscard]] PosixDomain {
	  public:
		using value_type = i64;

		static constexpr const char (&UUID)[num_chars_in_uuid] // NOLINT
			= "4a6a9b0f-c335-473e-bc42-d23974a25bb0";

		static constexpr u64 ID = parse_uuid_from_string(UUID);

		constexpr PosixDomain() noexcept = default;
		explicit constexpr PosixDomain(u64 uuid) noexcept : m_uuid(uuid) {
		}
		explicit constexpr PosixDomain(const UUIDString auto& uuid) noexcept
			: m_uuid(parse_uuid_from_string(uuid)) {
		}
		constexpr ~PosixDomain() noexcept = default;
		constexpr PosixDomain(const PosixDomain&) noexcept = default;
		constexpr PosixDomain(PosixDomain&&) noexcept = default;

		[[nodiscard]] constexpr auto id() const noexcept -> u64 {
			return m_uuid;
		}

		[[nodiscard]] constexpr auto name() const noexcept -> std::string_view { // NOLINT
			return "POSIX domain";
		}

		[[nodiscard]] auto message(value_type code) // NOLINT
			const noexcept -> std::string {

			return as_string(code);
		}

		[[nodiscard]] auto message(const PosixStatusCode& code) // NOLINT
			const noexcept -> std::string {

			return as_string(code.code());
		}

		[[nodiscard]] constexpr auto
		is_error(const PosixStatusCode& code) const noexcept -> bool { // NOLINT
			return code.code() != 0;
		}

		[[nodiscard]] constexpr auto
		is_success(const PosixStatusCode& code) const noexcept -> bool { // NOLINT
			return code.code() == 0;
		}

		template<typename Domain2>
		[[nodiscard]] constexpr auto
		are_equivalent(const PosixStatusCode& lhs, const StatusCode<Domain2>& rhs) const noexcept
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
		as_generic_code(const PosixStatusCode& code) const noexcept -> GenericStatusCode { // NOLINT
			return make_status_code(to_generic_code(code.code()));
		}

		[[nodiscard]] constexpr inline auto success_value() const noexcept -> value_type { // NOLINT
			return 0;
		}

		template<typename Domain>
		friend constexpr auto
		operator==(const PosixDomain& lhs, const Domain& rhs) noexcept -> bool {
			return lhs.id() == rhs.id();
		}

		template<typename Domain>
		friend constexpr auto
		operator!=(const PosixDomain& lhs, const Domain& rhs) noexcept -> bool {
			return lhs.id() != rhs.id();
		}

		constexpr auto operator=(const PosixDomain&) noexcept -> PosixDomain& = default;
		constexpr auto operator=(PosixDomain&&) noexcept -> PosixDomain& = default;

	  private:
		u64 m_uuid = ID;

		inline static auto as_string(value_type code) noexcept -> std::string {
			char buffer[1024]; // NOLINT
#if HYPERION_PLATFORM_WINDOWS
			strerror_s(buffer, 1024, code); // NOLINT
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

template<>
constexpr inline auto
make_status_code_domain<hyperion::error::PosixDomain>() noexcept -> hyperion::error::PosixDomain {
	return {};
}

namespace hyperion::error {
	static_assert(StatusCodeDomain<PosixDomain>);
} // namespace hyperion::error
