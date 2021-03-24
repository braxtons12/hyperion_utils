#pragma once

#include <array>
#include <cstdint>

#include "Macros.h"
#include "OptionAndResult.h"

namespace hyperion::utils {
#ifndef _MSVC_VER
	using std::size_t;
#endif

	enum class LockFreeQueueErrorTypes
	{
		QueueIsFull,
		QueueIsEmpty
	};

	IGNORE_PADDING_START
	template<LockFreeQueueErrorTypes Type>
	class LockFreeQueueError final : public Error {
	  public:
		constexpr LockFreeQueueError() noexcept {
			if constexpr(Type == LockFreeQueueErrorTypes::QueueIsFull) {
				this->m_message = "Failed to push entry into LockFreeQueue: LockFreeQueue Is Full";
			}
			else {
				this->m_message = "Failed to read entry from LockFreeQueue: LockFreeQueue Is Empty";
			}
		}
		constexpr LockFreeQueueError(const LockFreeQueueError<Type>& error) noexcept = default;
		constexpr LockFreeQueueError(LockFreeQueueError<Type>&& error) noexcept = default;
		~LockFreeQueueError() noexcept final = default;

		constexpr auto operator=(const LockFreeQueueError<Type>& error) noexcept
			-> LockFreeQueueError<Type>& = default;
		constexpr auto
		operator=(LockFreeQueueError<Type>&& error) noexcept -> LockFreeQueueError<Type>& = default;
	};

	template<typename T, size_t Capacity = 512>
	class LockFreeQueue {
	  public:
		using PushError = LockFreeQueueError<LockFreeQueueErrorTypes::QueueIsFull>;
		using ReadError = LockFreeQueueError<LockFreeQueueErrorTypes::QueueIsEmpty>;

		constexpr LockFreeQueue() noexcept = default;
		constexpr LockFreeQueue(const LockFreeQueue<T, Capacity>& queue) noexcept = default;
		constexpr LockFreeQueue(LockFreeQueue<T, Capacity>&& queue) noexcept = default;

		[[nodiscard]] inline auto push(T entry) noexcept -> Result<bool, PushError> {
			if(m_write_index == m_read_index && m_write_generation > m_read_generation) {
				return Err(PushError());
			}

			m_data.at(m_write_index) = entry;
			m_write_index++;
			if(m_write_index == m_capacity - 1) {
				m_write_index = 0;
				m_write_generation++;
			}
			return Ok(true);
		}

		[[nodiscard]] inline auto read() noexcept -> Result<T, ReadError> {
			if(isEmpty()) {
				return Err(ReadError());
			}
			else {
				m_read_index++;
				if(m_read_index == m_capacity - 1) {
					m_read_index = 0;
					m_read_generation++;
				}
				return Ok(m_data.at(m_read_index));
			}
		}

		[[nodiscard]] inline auto isEmpty() const noexcept -> bool {
			if(m_write_generation == m_read_generation) {
				return m_write_index == m_read_index;
			}
			else {
				return !(m_write_generation > m_read_generation);
			}
		}

	  private:
		static const constexpr size_t m_capacity = Capacity;
		size_t m_read_index = 0;
		size_t m_read_generation = 0;
		size_t m_write_index = 0;
		size_t m_write_generation = 0;
		std::array<T, m_capacity> m_data = std::array<T, m_capacity>();
	};
	IGNORE_PADDING_STOP
} // namespace hyperion::utils
