#pragma once

#include <cstdint>

#include "Macros.h"
#include "OptionAndResult.h"
#include "RingBuffer.h"

namespace hyperion::utils {
#ifndef _MSVC_VER
	using std::size_t;
#endif

	enum class LockFreeQueueErrorCategory
	{
		QueueIsFull,
		QueueIsEmpty
	};

	enum class QueuePolicy : size_t
	{
		ErrWhenFull = 0,
		OverwriteWhenFull = 1
	};

	IGNORE_PADDING_START
	template<LockFreeQueueErrorCategory Type>
	class LockFreeQueueError final : public Error {
	  public:
		LockFreeQueueError() noexcept {
			if constexpr(Type == LockFreeQueueErrorCategory::QueueIsFull) {
				this->m_message = "Failed to push entry into LockFreeQueue: LockFreeQueue Is Full";
			}
			else {
				this->m_message = "Failed to read entry from LockFreeQueue: LockFreeQueue Is Empty";
			}
		}
		LockFreeQueueError(const LockFreeQueueError& error) noexcept = default;
		LockFreeQueueError(LockFreeQueueError&& error) noexcept = default;
		~LockFreeQueueError() noexcept final = default;

		auto operator=(const LockFreeQueueError& error) noexcept -> LockFreeQueueError& = default;
		auto operator=(LockFreeQueueError&& error) noexcept -> LockFreeQueueError& = default;
	};

	/// @brief The default capacityfor `LockFreeQueue`
	static constexpr size_t DEFAULT_QUEUE_CAPACITY = 512ULL;

	template<typename T,
			 QueuePolicy Policy = QueuePolicy::ErrWhenFull,
			 size_t Capacity = DEFAULT_QUEUE_CAPACITY>
	class LockFreeQueue {
	  public:
		using PushError = LockFreeQueueError<LockFreeQueueErrorCategory::QueueIsFull>;
		using ReadError = LockFreeQueueError<LockFreeQueueErrorCategory::QueueIsEmpty>;

		constexpr LockFreeQueue() noexcept = default;
		constexpr LockFreeQueue(const LockFreeQueue& queue) noexcept = default;
		constexpr LockFreeQueue(LockFreeQueue&& queue) noexcept = default;
		constexpr ~LockFreeQueue() noexcept = default;

		[[nodiscard]] inline auto push(const T& entry) noexcept -> Result<bool, PushError>
		requires Copyable<T> &&(Policy == QueuePolicy::ErrWhenFull) {
			const auto pusher = [&]() {
				m_data.push_back(entry);
				return Ok(true);
			};

			if(full()) {
				return Err(PushError());
			}
			else {
				return pusher();
			}
		}

		[[nodiscard]] inline auto push(T&& entry) noexcept -> Result<bool, PushError>
		requires(Policy == QueuePolicy::ErrWhenFull) {
			const auto pusher = [&]() {
				m_data.push_back(std::forward<T>(entry));
				return Ok(true);
			};

			if(full()) {
				return Err(PushError());
			}
			else {
				return pusher();
			}
		}

		template<typename... Args>
		requires ConstructibleFrom<T, Args...>
		[[nodiscard]] inline auto push(Args&&... args) noexcept -> Result<bool, PushError>
		requires(Policy == QueuePolicy::ErrWhenFull) {
			const auto pusher = [&]() {
				m_data.emplace_back(std::forward<Args>(args)...);
				return Ok(true);
			};

			if(full()) {
				return Err(PushError());
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

		[[nodiscard]] inline auto read() noexcept -> Result<T, ReadError> {
			if(empty()) {
				return Err(ReadError());
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
} // namespace hyperion::utils
