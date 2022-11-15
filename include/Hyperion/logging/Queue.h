/// @file Queue.h
/// @author Braxton Salyer <braxtonsalyer@gmail.com>
/// @brief Wraps hyperion::LocFreeQueue in a way that directly maps to asynchronous logging policies
/// @version 0.1
/// @date 2022-06-23
///
/// MIT License
/// @copyright Copyright (c) 2022 Braxton Salyer <braxtonsalyer@gmail.com>
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

#include <Hyperion/BasicTypes.h>
#include <Hyperion/HyperionDef.h>
#include <Hyperion/LockFreeQueue.h>
#include <Hyperion/Result.h>

namespace hyperion::logging {
	/// @brief Types of `Error`s that can occur with `Queue`
	enum class QueueErrorCategory : i8 {
		/// @brief No `Error` occurred
		Success = 0,
		/// @brief The queue was full and policy is `QueuePolicy::ErrWhenFull`
		QueueIsFull = 1,
		/// @brief The queue was empty when a read was attempted
		QueueIsEmpty = 2,
		Unknown = -1
	};
} // namespace hyperion::logging

STATUS_CODE_DOMAIN(
	QueueErrorDomain /**NOLINT**/,
	Queue,
	hyperion::logging,
	hyperion::logging::QueueErrorCategory,
	true,
	"045dd371-9552-4ce1-bd4d-8e95b654fbe0",
	"QueueErrorDomain",
	hyperion::logging::QueueErrorCategory::Success,
	hyperion::logging::QueueErrorCategory::Unknown,
	[](hyperion::logging::QueueErrorCategory code) noexcept -> std::string_view {
		if(code == value_type::Success) {
			return "Success";
		}
		// NOLINTNEXTLINE(readability-else-after-return)
		else if(code == value_type::QueueIsFull) {
			return "Queue is full.";
		}
		else if(code == value_type::QueueIsEmpty) {
			return "Queue is empty.";
		}
		else {
			return "Unknown Queue error.";
		}
	},
	[](const auto& code) noexcept -> hyperion::error::GenericStatusCode {
		if(code == value_type::Success || code == value_type::Unknown) {
			return make_status_code(static_cast<error::Errno>(code));
		}

		return make_status_code(error::Errno::Unknown);
	});

template<>
struct hyperion::error::status_code_enum_info<hyperion::logging::QueueErrorCategory> {
	using domain_type [[maybe_unused]] = hyperion::logging::QueueErrorDomain;
	static constexpr bool value = true;
};

namespace hyperion::logging {

	static_assert(error::StatusCodeDomain<QueueErrorDomain>);

	enum class QueuePolicy : usize {
		ErrWhenFull = 0,
		OverwriteWhenFull = 1,
		BlockWhenFull = 2,
	};

	/// @brief The default capacity for `Queue`
	static constexpr usize DEFAULT_QUEUE_CAPACITY = 64_usize;

	IGNORE_PADDING_START
	template<typename T,
			 QueuePolicy Policy = QueuePolicy::ErrWhenFull,
			 usize Capacity = DEFAULT_QUEUE_CAPACITY>
	class Queue {
	  public:
		Queue() noexcept = default;
		constexpr Queue(const Queue& queue) noexcept = default;
		constexpr Queue(Queue&& queue) noexcept = default;
		constexpr ~Queue() noexcept = default;

		// clang-format off

		template<typename U = T>
		requires concepts::Same<T, std::remove_cvref_t<U>>
		[[nodiscard]] inline auto push(U&& entry) noexcept -> Result<None, QueueError>
				 requires concepts::NoexceptAssignable<T, U> && (Policy == QueuePolicy::ErrWhenFull)
		{
			// clang-format on
			if(!m_data.try_push_back(std::forward<U>(entry))) {
				return Err(QueueError(make_error_code(QueueErrorCategory::QueueIsFull)));
			}

			return Ok();
		}

		// clang-format off

		template<typename... Args>
		requires concepts::ConstructibleFrom<T, Args...>
		 [[nodiscard]] inline auto push(Args&&... args) noexcept -> Result<None, QueueError>
				 requires(Policy == QueuePolicy::ErrWhenFull)
		{
			// clang-format on
			if(!m_data.try_emplace_back(std::forward<Args>(args)...)) {
				return Err(QueueError(make_error_code(QueueErrorCategory::QueueIsFull)));
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

		[[nodiscard]] inline auto read() noexcept -> Result<T, QueueError> {
			if(auto ret = m_data.pop_front(); ret.is_some()) {
				return Ok(ret.unwrap());
			}

			return Err(QueueError(make_error_code(QueueErrorCategory::QueueIsEmpty)));
		}

		[[nodiscard]] inline auto empty() const noexcept -> bool {
			return m_data.empty();
		}

		[[nodiscard]] inline auto full() const noexcept -> bool {
			return m_data.full();
		}

		constexpr auto operator=(const Queue& queue) noexcept -> Queue& = default;
		constexpr auto operator=(Queue&& queue) noexcept -> Queue& = default;

	  private:
		LockFreeQueue<T> m_data = LockFreeQueue<T>(Capacity);
	};
	IGNORE_PADDING_STOP
} // namespace hyperion::logging
