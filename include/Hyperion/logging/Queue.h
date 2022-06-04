/// @brief This is a Lock-Free Single-ended Queue implementation using contiguous memory allocations
#pragma once

#include <Hyperion/BasicTypes.h>
#include <Hyperion/HyperionDef.h>
#include <Hyperion/LockFreeQueue.h>
#include <Hyperion/Result.h>

namespace hyperion {
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
} // namespace hyperion

STATUS_CODE_DOMAIN(
	LoggingQueueErrorDomain /**NOLINT**/,
	LoggingQueue,
	hyperion,
	hyperion::LoggingQueueErrorCategory,
	true,
	"045dd371-9552-4ce1-bd4d-8e95b654fbe0",
	"LoggingQueueErrorDomain",
	hyperion::LoggingQueueErrorCategory::Success,
	hyperion::LoggingQueueErrorCategory::Unknown,
	[](hyperion::LoggingQueueErrorCategory code) noexcept -> std::string_view {
		if(code == value_type::Success) {
			return "Success";
		}
		// NOLINTNEXTLINE(readability-else-after-return)
		else if(code == value_type::QueueIsFull) {
			return "LoggingQueue is full.";
		}
		else if(code == value_type::QueueIsEmpty) {
			return "LoggingQueue is empty.";
		}
		else {
			return "Unknown LoggingQueue error.";
		}
	},
	[](const auto& code) noexcept -> hyperion::error::GenericStatusCode {
		if(code == value_type::Success || code == value_type::Unknown) {
			return make_status_code(static_cast<error::Errno>(code));
		}
		else {
			return make_status_code(error::Errno::Unknown);
		}
	});

namespace hyperion {

	static_assert(error::StatusCodeDomain<LoggingQueueErrorDomain>);

	template<>
	struct error::status_code_enum_info<LoggingQueueErrorCategory> {
		using domain_type [[maybe_unused]] = LoggingQueueErrorDomain;
		static constexpr bool value = true;
	};

	enum class QueuePolicy : usize {
		ErrWhenFull = 0,
		OverwriteWhenFull = 1,
		BlockWhenFull = 2,
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

		// clang-format off

		template<typename U = T>
		requires concepts::Same<T, std::remove_cvref_t<U>>
		[[nodiscard]] inline auto push(U&& entry) noexcept -> Result<None, LoggingQueueError>
				 requires concepts::NoexceptAssignable<T, U> && (Policy == QueuePolicy::ErrWhenFull)
		{
			// clang-format on
			if(!m_data.try_push_back(std::forward<U>(entry))) {
				return Err(
					LoggingQueueError(make_error_code(LoggingQueueErrorCategory::QueueIsFull)));
			}

			return Ok();
		}

		// clang-format off

		template<typename... Args>
		requires concepts::ConstructibleFrom<T, Args...>
		 [[nodiscard]] inline auto push(Args&&... args) noexcept -> Result<None, LoggingQueueError>
				 requires(Policy == QueuePolicy::ErrWhenFull)
		{
			// clang-format on
			if(!m_data.try_emplace_back(std::forward<Args>(args)...)) {
				return Err(
					LoggingQueueError(make_error_code(LoggingQueueErrorCategory::QueueIsFull)));
			}

			return Ok();
		}

		// clang-format off

		template<typename U = T>
		requires concepts::Same<T, std::remove_cvref_t<U>>
		inline auto push(U&& entry) noexcept -> void
			requires concepts::NoexceptAssignable<T, U>
			    && (Policy == QueuePolicy::OverwriteWhenFull)
		{
			// clang-format on
			m_data.force_push_back(std::forward<U>(entry));
		}

		// clang-format off

		template<typename... Args>
		requires concepts::ConstructibleFrom<T, Args...>
		inline auto push(Args&&... args) noexcept -> void
			requires(Policy == QueuePolicy::OverwriteWhenFull)
		{
			// clang-format on
			ignore(m_data.force_emplace_back(std::forward<Args>(args)...));
		}

		// clang-format off

		template<typename U = T>
		requires concepts::Same<T, std::remove_cvref_t<U>>
		inline auto push(U&& entry) noexcept -> void
			requires concepts::NoexceptAssignable<T, U>
				&& (Policy == QueuePolicy::BlockWhenFull)
		{
			// clang-format on
			m_data.push_back(std::forward<U>(entry));
		}

		// clang-format off

		template<typename... Args>
		requires concepts::ConstructibleFrom<T, Args...>
		inline auto push(Args&&... args) noexcept -> void
			requires(Policy == QueuePolicy::BlockWhenFull)
		{
			// clang-format on
			ignore(m_data.emplace_back(std::forward<Args>(args)...));
		}

		[[nodiscard]] inline auto read() noexcept -> Result<T, LoggingQueueError> {
			if(auto ret = m_data.pop_front(); ret.is_some()) {
				return Ok(ret.unwrap());
			}

			return Err(
				LoggingQueueError(make_error_code(LoggingQueueErrorCategory::QueueIsEmpty)));
		}

		[[nodiscard]] inline auto empty() const noexcept -> bool {
			return m_data.empty();
		}

		[[nodiscard]] inline auto full() const noexcept -> bool {
			return m_data.full();
		}

		constexpr auto operator=(const LoggingQueue& queue) noexcept -> LoggingQueue& = default;
		constexpr auto operator=(LoggingQueue&& queue) noexcept -> LoggingQueue& = default;

	  private:
		LockFreeQueue<T> m_data = LockFreeQueue<T>(Capacity);
	};
	IGNORE_PADDING_STOP
} // namespace hyperion
