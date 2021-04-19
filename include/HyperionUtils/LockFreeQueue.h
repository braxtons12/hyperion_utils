/// @brief This is a Lock-Free Single-ended Queue implementation using contiguous memory allocations
#pragma once

#include <system_error>

#include "BasicTypes.h"
#include "Macros.h"
#include "Monads.h"
#include "RingBuffer.h"

namespace hyperion {

	/// @brief Types of `Error`s that can occur with `LockFreeQueue`
	enum class LockFreeQueueErrorType : u8
	{
		/// @brief No `Error` occurred
		Success = 0,
		/// @brief The queue was full and policy is `QueuePolicy::ErrWhenFull`
		QueueIsFull = 1,
		/// @brief The queue was empty when a read was attempted
		QueueIsEmpty = 2
	};

	IGNORE_WEAK_VTABLES_START
	/// @brief `std::error_category` type for `LockFreeQueueErrorType`s. Provides error catgory
	/// functionality for `LockFreeQueueErrorType`s
	struct LockFreeQueueErrorCategory final : public std::error_category {
	  public:
		/// @brief Default Constructor
		LockFreeQueueErrorCategory() noexcept = default;
		/// @brief Copy Constructor. Deleted. Error Category types are singletons
		LockFreeQueueErrorCategory(const LockFreeQueueErrorCategory&) = delete;
		/// @brief Move Constructor. Deleted. Error Category types are singletons
		LockFreeQueueErrorCategory(LockFreeQueueErrorCategory&&) = delete;
		/// @brief Destructor
		~LockFreeQueueErrorCategory() noexcept final = default;

		/// @brief Returns the name of this error category
		///
		/// @return const char * - The name
		[[nodiscard]] inline constexpr auto name() const noexcept -> const char* final {
			return "LockFreeQueue Error";
		}

		/// @brief Returns the message associated with the given error condition
		///
		/// @param condition - The condition to get the message for
		///
		/// @return The associated message
		[[nodiscard]] inline HYPERION_CONSTEXPR_STRINGS auto
		message(int condition) const noexcept -> std::string final {
			const auto category = static_cast<LockFreeQueueErrorType>(condition);
			if(category == LockFreeQueueErrorType::Success) {
				return "Success"s;
			}
			else if(category == LockFreeQueueErrorType::QueueIsFull) {
				return "Queueing entry failed, Queue is full"s;
			}
			else if(category == LockFreeQueueErrorType::QueueIsEmpty) {
				return "Reading from Queue failed, Queue is empty"s;
			}
			else {
				return "Unknown Error"s;
			}
		}

		/// @brief Copy assignment operator. Deleted. Error Category types are singletons
		auto operator=(const LockFreeQueueErrorCategory&) -> LockFreeQueueErrorCategory& = delete;
		/// @brief Move assignment operator. Deleted. Error Category types are singletons
		auto operator=(LockFreeQueueErrorCategory&&) -> LockFreeQueueErrorCategory& = delete;
	};
	IGNORE_WEAK_VTABLES_STOP

	static inline auto lock_free_queue_category() noexcept -> const LockFreeQueueErrorCategory& {
		HYPERION_NO_DESTROY static const LockFreeQueueErrorCategory category{};
		return category;
	}

} // namespace hyperion

namespace std {
	template<>
	struct is_error_code_enum<hyperion::LockFreeQueueErrorType> : std::true_type { };

} // namespace std

inline auto make_error_code(hyperion::LockFreeQueueErrorType code) noexcept -> std::error_code {
	return {static_cast<int>(code), hyperion::lock_free_queue_category()};
}

namespace hyperion {
	IGNORE_PADDING_START
	IGNORE_WEAK_VTABLES_START
	/// @brief `Error` type for communicating queueing errors
	/// `Error`s can occur if an entry fails to queue due to queueing policy
	/// or an entry can't be read because the queue is empty
	class LockFreeQueueError final : public Error {
	  public:
		/// @brief Default Constructor
		LockFreeQueueError() noexcept {
			this->m_message = "Unknown LockFreeQueueError occurred"s;
		}
		/// @brief Constructs ` LockFreeQueueError` as a `std::error_code` from the given
		/// `LockFreeQueueErrorType`
		///
		/// @param type - The error type
		LockFreeQueueError(LockFreeQueueErrorType type) noexcept // NOLINT
			: Error(make_error_code(type)) {
			if(type == LockFreeQueueErrorType::QueueIsFull) {
				this->m_message = "Failed to push entry into LockFreeQueue: LockFreeQueue Is Full";
			}
			else {
				this->m_message = "Failed to read entry from LockFreeQueue: LockFreeQueue Is Empty";
			}
		}
		/// @brief Copy Constructor
		LockFreeQueueError(const LockFreeQueueError& error) noexcept = default;
		/// @brief Move Constructor
		LockFreeQueueError(LockFreeQueueError&& error) noexcept = default;
		/// @brief Destructor
		~LockFreeQueueError() noexcept final = default;

		/// @brief Copy assignment operator
		auto operator=(const LockFreeQueueError& error) noexcept -> LockFreeQueueError& = default;
		/// @brief Move assignment operator
		auto operator=(LockFreeQueueError&& error) noexcept -> LockFreeQueueError& = default;
	};
	IGNORE_WEAK_VTABLES_STOP

	enum class QueuePolicy : usize
	{
		ErrWhenFull = 0,
		OverwriteWhenFull = 1
	};

	/// @brief The default capacityfor `LockFreeQueue`
	static constexpr usize DEFAULT_QUEUE_CAPACITY = 512_usize;

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
		requires Copyable<T> &&(Policy == QueuePolicy::ErrWhenFull) {
			const auto pusher = [&]() {
				m_data.push_back(entry);
				return Ok(true);
			};

			if(full()) {
				return Err(LockFreeQueueError(LockFreeQueueErrorType::QueueIsFull));
			}
			else {
				return pusher();
			}
		}

		[[nodiscard]] inline auto push(T&& entry) noexcept -> Result<bool, LockFreeQueueError>
		requires(Policy == QueuePolicy::ErrWhenFull) {
			const auto pusher = [&]() {
				m_data.push_back(std::forward<T>(entry));
				return Ok(true);
			};

			if(full()) {
				return Err(LockFreeQueueError(LockFreeQueueErrorType::QueueIsFull));
			}
			else {
				return pusher();
			}
		}

		template<typename... Args>
		requires ConstructibleFrom<T, Args...>
		[[nodiscard]] inline auto push(Args&&... args) noexcept -> Result<bool, LockFreeQueueError>
		requires(Policy == QueuePolicy::ErrWhenFull) {
			const auto pusher = [&]() {
				m_data.emplace_back(std::forward<Args>(args)...);
				return Ok(true);
			};

			if(full()) {
				return Err(LockFreeQueueError(LockFreeQueueErrorType::QueueIsFull));
			}
			else {
				return pusher();
			}
		}

		inline auto push(const T& entry) noexcept
			-> void requires Copyable<T> &&(Policy == QueuePolicy::OverwriteWhenFull) {
			m_data.push_back(entry);
		}

		inline auto
		push(T&& entry) noexcept -> void requires(Policy == QueuePolicy::OverwriteWhenFull) {
			m_data.push_back(std::forward<T>(entry));
		}

		template<typename... Args>
		requires ConstructibleFrom<T, Args...>
		inline auto
		push(Args&&... args) noexcept -> void requires(Policy == QueuePolicy::OverwriteWhenFull) {
			m_data.emplace_back(std::forward<Args>(args)...);
		}

		[[nodiscard]] inline auto read() noexcept -> Result<T, LockFreeQueueError> {
			if(empty()) {
				return Err(LockFreeQueueError(LockFreeQueueErrorType::QueueIsEmpty));
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
