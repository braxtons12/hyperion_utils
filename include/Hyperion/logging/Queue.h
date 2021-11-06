/// @brief This is a Lock-Free Single-ended Queue implementation using contiguous memory allocations
#pragma once

#include <Hyperion/BasicTypes.h>
#include <Hyperion/HyperionDef.h>
#include <Hyperion/Result.h>
#include <Hyperion/RingBuffer.h>

namespace hyperion {
	using namespace std::literals::string_literals;

	/// @brief Types of `Error`s that can occur with `LoggingQueue`
	enum class LoggingQueueErrorCategory : i8 {
		/// @brief No `Error` occurred
		Success = 0,
		/// @brief The queue was full and policy is `QueuePolicy::ErrWhenFull`
		QueueIsFull = 1,
		/// @brief The queue was empty when a read was attempted
		QueueIsEmpty = 2,
		Unknown = -1
	};

	class LoggingQueueErrorDomain {
	  public:
		using value_type = LoggingQueueErrorCategory;
		using LoggingQueueStatusCode = error::StatusCode<LoggingQueueErrorDomain>;
		using LoggingQueueErrorCode = error::ErrorCode<LoggingQueueErrorDomain>;

		static const constexpr char (&UUID)[error::num_chars_in_uuid] // NOLINT
			= "045dd371-9552-4ce1-bd4d-8e95b654fbe0";

		static constexpr u64 ID = error::parse_uuid_from_string(UUID);

		constexpr LoggingQueueErrorDomain() noexcept = default;
		explicit constexpr LoggingQueueErrorDomain(u64 uuid) noexcept : m_uuid(uuid) {
		}
		explicit constexpr LoggingQueueErrorDomain(const error::UUIDString auto& uuid) noexcept
			: m_uuid(error::parse_uuid_from_string(uuid)) {
		}
		constexpr LoggingQueueErrorDomain(const LoggingQueueErrorDomain&) noexcept = default;
		constexpr LoggingQueueErrorDomain(LoggingQueueErrorDomain&&) noexcept = default;
		constexpr ~LoggingQueueErrorDomain() noexcept = default;

		[[nodiscard]] inline constexpr auto id() const noexcept -> u64 {
			return m_uuid;
		}

		[[nodiscard]] inline constexpr auto name() const noexcept -> std::string_view { // NOLINT
			return "LoggingQueueErrorDomain";
		}

		[[nodiscard]] inline constexpr auto message(value_type code) // NOLINT
			const noexcept -> std::string_view {
			if(code == value_type::Success) {
				return "Success";
			}
			else if(code == value_type::QueueIsFull) {
				return "LoggingQueue is full.";
			}
			else if(code == value_type::QueueIsEmpty) {
				return "LoggingQueue is empty.";
			}
			else {
				return "Unknown LoggingQueue error.";
			}
		}

		[[nodiscard]] inline constexpr auto message(const LoggingQueueStatusCode& code) // NOLINT
			const noexcept -> std::string_view {
			return message(code.code());
		}

		[[nodiscard]] inline constexpr auto is_error(const LoggingQueueStatusCode& code) // NOLINT
			const noexcept -> bool {
			return code.code() != value_type::Success;
		}

		[[nodiscard]] inline constexpr auto is_success(const LoggingQueueStatusCode& code) // NOLINT
			const noexcept -> bool {
			return code.code() == value_type::Success;
		}

		template<typename Domain2>
		[[nodiscard]] inline constexpr auto
		are_equivalent(const LoggingQueueStatusCode& lhs,
					   const error::StatusCode<Domain2>& rhs) const noexcept -> bool {
			if constexpr(concepts::Same<LoggingQueueStatusCode, error::StatusCode<Domain2>>) {
				return lhs.code() == rhs.code();
			}
			else {
				return false;
			}
		}

		[[nodiscard]] inline constexpr auto
		as_generic_code(const LoggingQueueStatusCode& code) // NOLINT
			const noexcept -> error::GenericStatusCode {
			if(code.code() == value_type::Success || code.code() == value_type::Unknown) {
				return make_status_code(static_cast<error::Errno>(code.code()));
			}
			else {
				return make_status_code(error::Errno::Unknown);
			}
		}

		[[nodiscard]] inline static constexpr auto success_value() noexcept -> value_type {
			return value_type::Success;
		}

		template<typename Domain>
		friend inline constexpr auto
		operator==(const LoggingQueueErrorDomain& lhs, const Domain& rhs) noexcept -> bool {
			return rhs.id() == lhs.id();
		}

		template<typename Domain>
		friend inline constexpr auto
		operator!=(const LoggingQueueErrorDomain& lhs, const Domain& rhs) noexcept -> bool {
			return rhs.id() != lhs.id();
		}

		constexpr auto
		operator=(const LoggingQueueErrorDomain&) noexcept -> LoggingQueueErrorDomain& = default;
		constexpr auto
		operator=(LoggingQueueErrorDomain&&) noexcept -> LoggingQueueErrorDomain& = default;

	  private:
		u64 m_uuid = ID;
	};

	using LoggingQueueStatusCode = LoggingQueueErrorDomain::LoggingQueueStatusCode;
	using LoggingQueueErrorCode = LoggingQueueErrorDomain::LoggingQueueErrorCode;
	using LoggingQueueError = error::Error<LoggingQueueErrorDomain>;

} // namespace hyperion

template<>
inline constexpr auto make_status_code_domain<hyperion::LoggingQueueErrorDomain>() noexcept
	-> hyperion::LoggingQueueErrorDomain {
	return {};
}

namespace hyperion {

	static_assert(error::StatusCodeDomain<LoggingQueueErrorDomain>);

	template<>
	struct error::status_code_enum_info<LoggingQueueErrorCategory> {
		using domain_type = LoggingQueueErrorDomain;
		static constexpr bool value = true;
	};

	enum class QueuePolicy : usize {
		ErrWhenFull = 0,
		OverwriteWhenFull = 1
	};

	/// @brief The default capacity for `LoggingQueue`
	static constexpr usize DEFAULT_QUEUE_CAPACITY = 64_usize;

	IGNORE_PADDING_START
	template<typename T,
			 QueuePolicy Policy = QueuePolicy::ErrWhenFull,
			 usize Capacity = DEFAULT_QUEUE_CAPACITY>
	class LoggingQueue {
	  public:
		LoggingQueue() noexcept = default;
		constexpr LoggingQueue(const LoggingQueue& queue) noexcept = default;
		constexpr LoggingQueue(LoggingQueue&& queue) noexcept = default;
		constexpr ~LoggingQueue() noexcept = default;

		[[nodiscard]] inline auto push(const T& entry) noexcept -> Result<bool, LoggingQueueError>
		requires concepts::CopyAssignable<T> &&(Policy == QueuePolicy::ErrWhenFull) {
			std::atomic_thread_fence(std::memory_order_acquire);
			if(m_data.size() == Capacity) {
				std::atomic_thread_fence(std::memory_order_release);
				return Err(
					LoggingQueueError(make_error_code(LoggingQueueErrorCategory::QueueIsFull)));
			}
			else {
				m_data.push_back(entry);
				std::atomic_thread_fence(std::memory_order_release);
				return Ok();
			}
		}

		[[nodiscard]] inline auto push(T&& entry) noexcept -> Result<bool, LoggingQueueError>
		requires concepts::MoveAssignable<T> &&(Policy == QueuePolicy::ErrWhenFull) {
			std::atomic_thread_fence(std::memory_order_acquire);
			if(m_data.size() == Capacity) {
				std::atomic_thread_fence(std::memory_order_release);
				return Err(
					LoggingQueueError(make_error_code(LoggingQueueErrorCategory::QueueIsFull)));
			}
			else {
				m_data.push_back(std::move(entry));
				std::atomic_thread_fence(std::memory_order_release);
				return Ok();
			}
		}

		template<typename... Args>
		requires concepts::ConstructibleFrom<T, Args...>
		[[nodiscard]] inline auto push(Args&&... args) noexcept -> Result<bool, LoggingQueueError>
		requires(Policy == QueuePolicy::ErrWhenFull) {
			std::atomic_thread_fence(std::memory_order_acquire);
			if(m_data.size() == Capacity) {
				std::atomic_thread_fence(std::memory_order_release);
				return Err(
					LoggingQueueError(make_error_code(LoggingQueueErrorCategory::QueueIsFull)));
			}
			else {
				std::atomic_thread_fence(std::memory_order_release);
				m_data.emplace_back(std::forward<Args>(args)...);
				return Ok();
			}
		}

		inline auto push(const T& entry) noexcept
			-> void requires concepts::CopyAssignable<T> &&(Policy
															== QueuePolicy::OverwriteWhenFull) {
			std::atomic_thread_fence(std::memory_order_acquire);
			m_data.push_back(entry);
			std::atomic_thread_fence(std::memory_order_release);
		}

		inline auto
		push(T&& entry) noexcept -> void requires(Policy == QueuePolicy::OverwriteWhenFull) {
			std::atomic_thread_fence(std::memory_order_acquire);
			m_data.push_back(std::forward<T>(entry));
			std::atomic_thread_fence(std::memory_order_release);
		}

		template<typename... Args>
		requires concepts::ConstructibleFrom<T, Args...>
		inline auto
		push(Args&&... args) noexcept -> void requires(Policy == QueuePolicy::OverwriteWhenFull) {
			std::atomic_thread_fence(std::memory_order_acquire);
			m_data.emplace_back(std::forward<Args>(args)...);
			std::atomic_thread_fence(std::memory_order_release);
		}

		[[nodiscard]] inline auto read() noexcept -> Result<T, LoggingQueueError> {
			std::atomic_thread_fence(std::memory_order_acquire);
			if(m_data.empty()) {
				std::atomic_thread_fence(std::memory_order_release);
				return Err(
					LoggingQueueError(make_error_code(LoggingQueueErrorCategory::QueueIsEmpty)));
			}
			else {
				// return Ok(*(m_data.pop_back()));
				auto ret = Ok(m_data.pop_front());
				std::atomic_thread_fence(std::memory_order_release);
				return ret;
			}
		}

		[[nodiscard]] inline auto empty() const noexcept -> bool {
			std::atomic_thread_fence(std::memory_order_acquire);
			return m_data.empty();
		}

		[[nodiscard]] inline auto full() const noexcept -> bool {
			std::atomic_thread_fence(std::memory_order_acquire);
			return m_data.size() == Capacity;
		}

		constexpr auto operator=(const LoggingQueue& queue) noexcept -> LoggingQueue& = default;
		constexpr auto operator=(LoggingQueue&& queue) noexcept -> LoggingQueue& = default;

	  private:
		// RingBuffer<T, RingBufferType::ThreadSafe> m_data
		//	= RingBuffer<T, RingBufferType::ThreadSafe>(Capacity);
		RingBuffer<T> m_data = RingBuffer<T>(Capacity);
	};
	IGNORE_PADDING_STOP
} // namespace hyperion
