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

	template<typename T, QueuePolicy Policy = QueuePolicy::ErrWhenFull, size_t Capacity = 512>
	class LockFreeQueue {
	  public:
		using PushError = LockFreeQueueError<LockFreeQueueErrorCategory::QueueIsFull>;
		using ReadError = LockFreeQueueError<LockFreeQueueErrorCategory::QueueIsEmpty>;

		constexpr LockFreeQueue() noexcept = default;
		constexpr LockFreeQueue(const LockFreeQueue& queue) noexcept = default;
		constexpr LockFreeQueue(LockFreeQueue&& queue) noexcept = default;

		[[nodiscard]] inline auto push(const T& entry) noexcept -> Result<bool, PushError>
		requires Copyable<T> &&(Policy == QueuePolicy::ErrWhenFull) {
			if(m_data.size() == Capacity) {
				return Err(PushError());
			}

			m_data.push_back(entry);
			return Ok(true);
		}

		[[nodiscard]] inline auto push(T&& entry) noexcept -> Result<bool, PushError>
		requires(Policy == QueuePolicy::ErrWhenFull) {
			if(m_data.size() == Capacity) {
				return Err(PushError());
			}

			m_data.push_back(std::move(entry));
			return Ok(true);
		}

		template<typename... Args>
		requires ConstructibleFrom<T, Args...>
		[[nodiscard]] inline auto push(Args&&... args) noexcept -> Result<bool, PushError>
		requires(Policy == QueuePolicy::ErrWhenFull) {
			if(m_data.size() == Capacity) {
				return Err(PushError());
			}

			m_data.emplace_back(args...);
			return Ok(true);
		}

		inline auto push(const T& entry) noexcept
			-> void requires Copyable<T> &&(Policy == QueuePolicy::OverwriteWhenFull) {
			m_data.push_back(std::forward<T>(entry));
		}

		inline auto
		push(T&& entry) noexcept -> void requires(Policy == QueuePolicy::OverwriteWhenFull) {
			m_data.push_back(std::move(entry));
		}

		template<typename... Args>
		requires ConstructibleFrom<T, Args...>
		inline auto
		push(Args&&... args) noexcept -> void requires(Policy == QueuePolicy::OverwriteWhenFull) {
			m_data.emplace_back(args...);
		}

		[[nodiscard]] inline auto read() noexcept -> Result<T, ReadError> {
			if(empty()) {
				return Err(ReadError());
			}
			else {
				return Ok(m_data.pop_front());
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
		RingBuffer<T> m_data = RingBuffer<T>(Capacity);
	};
	IGNORE_PADDING_STOP
} // namespace hyperion::utils
