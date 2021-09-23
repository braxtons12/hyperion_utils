/// @brief This is a Lock-Free Single-ended Queue implementation using contiguous memory allocations
#pragma once

#include <system_error>

#include "BasicTypes.h"
#include "HyperionDef.h"
#include "Result.h"
#include "RingBuffer.h"

namespace hyperion {
	using namespace std::literals::string_literals;

	/// @brief Types of `Error`s that can occur with `LockFreeQueue`
	enum class LockFreeQueueErrorCategory : i8
	{
		/// @brief No `Error` occurred
		Success = 0,
		/// @brief The queue was full and policy is `QueuePolicy::ErrWhenFull`
		QueueIsFull = 1,
		/// @brief The queue was empty when a read was attempted
		QueueIsEmpty = 2,
		Unknown = -1
	};

	class LockFreeQueueErrorDomain {
	  public:
		using value_type = LockFreeQueueErrorCategory;
		using LockFreeQueueStatusCode = error::StatusCode<LockFreeQueueErrorDomain>;
		using LockFreeQueueErrorCode = error::ErrorCode<LockFreeQueueErrorDomain>;

		static constexpr const char (&UUID)[error::num_chars_in_uuid] // NOLINT
			= "045dd371-9552-4ce1-bd4d-8e95b654fbe0";

		static constexpr u64 ID = error::parse_uuid_from_string(UUID);

		constexpr LockFreeQueueErrorDomain() noexcept = default;
		explicit constexpr LockFreeQueueErrorDomain(u64 uuid) noexcept : m_uuid(uuid) {
		}
		explicit constexpr LockFreeQueueErrorDomain(const error::UUIDString auto& uuid) noexcept
			: m_uuid(error::parse_uuid_from_string(uuid)) {
		}
		constexpr LockFreeQueueErrorDomain(const LockFreeQueueErrorDomain&) noexcept = default;
		constexpr LockFreeQueueErrorDomain(LockFreeQueueErrorDomain&&) noexcept = default;
		constexpr ~LockFreeQueueErrorDomain() noexcept = default;

		[[nodiscard]] constexpr inline auto id() const noexcept -> u64 {
			return m_uuid;
		}

		[[nodiscard]] constexpr inline auto name() const noexcept -> std::string_view { // NOLINT
			return "LockFreeQueueErrorDomain";
		}

		[[nodiscard]] constexpr inline auto message(value_type code) // NOLINT
			const noexcept -> std::string_view {
			if(code == value_type::Success) {
				return "Success";
			}
			else if(code == value_type::QueueIsFull) {
				return "LockFreeQueue is full.";
			}
			else if(code == value_type::QueueIsEmpty) {
				return "LockFreeQueue is empty.";
			}
			else {
				return "Unknown LockFreeQueue error.";
			}
		}

		[[nodiscard]] constexpr inline auto message(const LockFreeQueueStatusCode& code) // NOLINT
			const noexcept -> std::string_view {
			return message(code.code());
		}

		[[nodiscard]] constexpr inline auto is_error(const LockFreeQueueStatusCode& code) // NOLINT
			const noexcept -> bool {
			return code.code() != value_type::Success;
		}

		[[nodiscard]] constexpr inline auto
		is_success(const LockFreeQueueStatusCode& code) // NOLINT
			const noexcept -> bool {
			return code.code() == value_type::Success;
		}

		template<typename Domain2>
		[[nodiscard]] constexpr inline auto
		are_equivalent(const LockFreeQueueStatusCode& lhs,
					   const error::StatusCode<Domain2>& rhs) const noexcept -> bool {
			if constexpr(concepts::Same<LockFreeQueueStatusCode, error::StatusCode<Domain2>>) {
				return lhs.code() == rhs.code();
			}
			else {
				return false;
			}
		}

		[[nodiscard]] constexpr inline auto
		as_generic_code(const LockFreeQueueStatusCode& code) // NOLINT
			const noexcept -> error::GenericStatusCode {
			if(code.code() == value_type::Success || code.code() == value_type::Unknown) {
				return make_status_code(static_cast<error::Errno>(code.code()));
			}
			else {
				return make_status_code(error::Errno::Unknown);
			}
		}

		[[nodiscard]] constexpr inline auto success_value() const noexcept -> value_type { // NOLINT
			return value_type::Success;
		}

		template<typename Domain>
		friend constexpr inline auto
		operator==(const LockFreeQueueErrorDomain& lhs, const Domain& rhs) noexcept -> bool {
			return rhs.id() == lhs.id();
		}

		template<typename Domain>
		friend constexpr inline auto
		operator!=(const LockFreeQueueErrorDomain& lhs, const Domain& rhs) noexcept -> bool {
			return rhs.id() != lhs.id();
		}

		constexpr auto
		operator=(const LockFreeQueueErrorDomain&) noexcept -> LockFreeQueueErrorDomain& = default;
		constexpr auto
		operator=(LockFreeQueueErrorDomain&&) noexcept -> LockFreeQueueErrorDomain& = default;

	  private:
		u64 m_uuid = ID;
	};

	using LockFreeQueueStatusCode = LockFreeQueueErrorDomain::LockFreeQueueStatusCode;
	using LockFreeQueueErrorCode = LockFreeQueueErrorDomain::LockFreeQueueErrorCode;
	using LockFreeQueueError = error::Error<LockFreeQueueErrorDomain>;

} // namespace hyperion

template<>
constexpr inline auto make_status_code_domain<hyperion::LockFreeQueueErrorDomain>() noexcept
	-> hyperion::LockFreeQueueErrorDomain {
	return {};
}

namespace hyperion {

	static_assert(error::StatusCodeDomain<LockFreeQueueErrorDomain>);

	template<>
	struct error::status_code_enum_info<LockFreeQueueErrorCategory> {
		using domain_type = LockFreeQueueErrorDomain;
		static constexpr bool value = true;
	};

	enum class QueuePolicy : usize
	{
		ErrWhenFull = 0,
		OverwriteWhenFull = 1
	};

	/// @brief The default capacityfor `LockFreeQueue`
	static constexpr usize DEFAULT_QUEUE_CAPACITY = 512_usize;

	IGNORE_PADDING_START
	template<typename T,
			 QueuePolicy Policy = QueuePolicy::ErrWhenFull,
			 usize Capacity = DEFAULT_QUEUE_CAPACITY>
	class LockFreeQueue {
	  public:
		constexpr LockFreeQueue() noexcept = default;
		constexpr LockFreeQueue(const LockFreeQueue& queue) noexcept = default;
		constexpr LockFreeQueue(LockFreeQueue&& queue) noexcept = default;
		constexpr ~LockFreeQueue() noexcept = default;

		[[nodiscard]] inline auto push(const T& entry) noexcept -> Result<bool, LockFreeQueueError>
		requires concepts::CopyAssignable<T> &&(Policy == QueuePolicy::ErrWhenFull) {
			const auto pusher = [&]() {
				m_data.push_back(entry);
				return Ok(true);
			};

			if(full()) {
				return Err(
					LockFreeQueueError(make_error_code(LockFreeQueueErrorCategory::QueueIsFull)));
			}
			else {
				return pusher();
			}
		}

		[[nodiscard]] inline auto push(T&& entry) noexcept -> Result<bool, LockFreeQueueError>
		requires concepts::MoveAssignable<T> &&(Policy == QueuePolicy::ErrWhenFull) {
			const auto pusher = [&]() {
				m_data.push_back(std::move(entry));
				return Ok(true);
			};

			if(full()) {
				return Err(
					LockFreeQueueError(make_error_code(LockFreeQueueErrorCategory::QueueIsFull)));
			}
			else {
				return pusher();
			}
		}

		template<typename... Args>
		requires concepts::ConstructibleFrom<T, Args...>
		[[nodiscard]] inline auto push(Args&&... args) noexcept -> Result<bool, LockFreeQueueError>
		requires(Policy == QueuePolicy::ErrWhenFull) {
			const auto pusher = [&]() {
				m_data.emplace_back(std::forward<Args>(args)...);
				return Ok(true);
			};

			if(full()) {
				return Err(
					LockFreeQueueError(make_error_code(LockFreeQueueErrorCategory::QueueIsFull)));
			}
			else {
				return pusher();
			}
		}

		inline auto push(const T& entry) noexcept
			-> void requires concepts::CopyAssignable<T> &&(Policy
															== QueuePolicy::OverwriteWhenFull) {
			m_data.push_back(entry);
		}

		inline auto
		push(T&& entry) noexcept -> void requires(Policy == QueuePolicy::OverwriteWhenFull) {
			m_data.push_back(std::forward<T>(entry));
		}

		template<typename... Args>
		requires concepts::ConstructibleFrom<T, Args...>
		inline auto
		push(Args&&... args) noexcept -> void requires(Policy == QueuePolicy::OverwriteWhenFull) {
			m_data.emplace_back(std::forward<Args>(args)...);
		}

		[[nodiscard]] inline auto read() noexcept -> Result<T, LockFreeQueueError> {
			if(empty()) {
				return Err(
					LockFreeQueueError(make_error_code(LockFreeQueueErrorCategory::QueueIsEmpty)));
			}
			else {
				return Ok(*(m_data.pop_back()));
			}
		}

		[[nodiscard]] inline auto empty() const noexcept -> bool {
			return m_data.empty();
		}

		[[nodiscard]] inline auto full() const noexcept -> bool {
			return m_data.size() == Capacity;
		}

		constexpr auto operator=(const LockFreeQueue& queue) noexcept -> LockFreeQueue& = default;
		constexpr auto operator=(LockFreeQueue&& queue) noexcept -> LockFreeQueue& = default;

	  private:
		RingBuffer<T, RingBufferType::ThreadSafe> m_data
			= RingBuffer<T, RingBufferType::ThreadSafe>(Capacity);
	};
	IGNORE_PADDING_STOP
} // namespace hyperion
