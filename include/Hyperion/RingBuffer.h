/// @file RingBuffer.h
/// @author Braxton Salyer <braxtonsalyer@gmail.com>
/// @brief Basic ring-buffer (circulary queue) with an API comparable to std::deque
/// @version 0.1
/// @date 2022-12-03
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
#include <Hyperion/Concepts.h>
#include <Hyperion/HyperionDef.h>
#include <Hyperion/Memory.h>
#include <Hyperion/Option.h>
#include <gsl/gsl>
#include <iterator>
#include <limits>

namespace hyperion {

	IGNORE_PADDING_START
	/// @brief A simple Ring Buffer implementation.
	/// Supports resizing, writing, reading, erasing, and provides mutable and immutable
	/// random access iterators.
	///
	/// # Iterator Invalidation
	/// * Iterators are lazily evaluated, so will only ever be invalidated at their current state.
	/// Performing any mutating operation (mutating the iterator, not the underlying data) on them
	/// will re-sync them with their associated `RingBuffer`.
	/// The following operations will invalidate an iterator's current state:
	/// - Read-only operations: never
	/// - clear: always
	/// - reserve: only if the `RingBuffer` changed capacity
	/// - erase: Erased elements and all following elements
	/// - push_back, emplace_back: only `end()` until `capacity()` is reached,
	///   then `begin()` and `end()`
	/// - insert, emplace: only the element at the position inserted/emplaced
	/// - pop_back: the element removed and `end()`
	/// - pop_front: the element removed and `begin()`
	///
	/// @tparam T - The type to store in the `RingBuffer`. Must Be Default Constructible.
	/// Does not currently support `T` of array types (eg, `T` = `U[]` or `T` = `U[N]`)
	template<typename T, template<typename ElementType> typename Allocator = std::allocator>
	class RingBuffer {
	  public:
		/// Default capacity of `RingBuffer`
		static const constexpr usize DEFAULT_CAPACITY = 16;
		using allocator_traits = std::allocator_traits<Allocator<T>>;
		using unique_pointer
			= decltype(allocate_unique<T[]>(std::declval<Allocator<T[]>>(), // NOLINT
											DEFAULT_CAPACITY));

		/// @brief Random-Access Bidirectional iterator for `RingBuffer`
		/// @note All navigation operators are checked such that any movement past `begin()` or
		/// `end()` is ignored.
		class Iterator {
		  public:
			using iterator_category = std::random_access_iterator_tag;
			using difference_type = std::ptrdiff_t;
			using value_type = T;
			using pointer = value_type*;
			using reference = value_type&;

			constexpr explicit Iterator(pointer ptr,
										RingBuffer* containerPtr,
										usize currentIndex) noexcept
				: m_ptr(ptr), m_container_ptr(containerPtr), m_current_index(currentIndex) {
			}
			constexpr Iterator(const Iterator& iter) noexcept = default;
			constexpr Iterator(Iterator&& iter) noexcept = default;
			~Iterator() noexcept = default;

			/// @brief Returns the index in the `RingBuffer` that corresponds
			/// to the element this iterator points to
			///
			/// @return The index corresponding with the element this points to
			[[nodiscard]] inline constexpr auto get_index() const noexcept -> usize {
				return m_current_index;
			}

			constexpr auto operator=(const Iterator& iter) noexcept -> Iterator& = default;
			constexpr auto operator=(Iterator&& iter) noexcept -> Iterator& = default;

			inline constexpr auto operator==(const Iterator& rhs) const noexcept -> bool {
				return m_current_index == rhs.m_current_index
					   && m_container_ptr == rhs.m_container_ptr;
			}

			inline constexpr auto operator!=(const Iterator& rhs) const noexcept -> bool {
				return m_current_index != rhs.m_current_index
					   || m_container_ptr != rhs.m_container_ptr;
			}

			inline constexpr auto operator*() const noexcept -> reference {
				return *m_ptr;
			}

			inline constexpr auto operator->() noexcept -> pointer {
				return m_ptr;
			}

			inline constexpr auto operator++() noexcept -> Iterator& {
				m_current_index++;
				if(m_current_index >= m_container_ptr->capacity()) {
					m_current_index = m_container_ptr->capacity();
					m_ptr = m_container_ptr->end().m_ptr;
				}
				else {
					m_ptr = &(*m_container_ptr)[m_current_index];
				}
				return *this;
			}

			inline constexpr auto operator++(int) noexcept -> Iterator {
				Iterator temp = *this;
				++(*this);
				return temp;
			}

			inline constexpr auto operator--() noexcept -> Iterator& {
				if(m_current_index == 0) {
					return *this;
				}

				m_current_index--;
				m_ptr = &(*m_container_ptr)[m_current_index];
				return *this;
			}

			inline constexpr auto operator--(int) noexcept -> Iterator {
				Iterator temp = *this;
				--(*this);
				return temp;
			}

			inline constexpr auto
			// NOLINTNEXTLINE(misc-no-recursion)
			operator+(concepts::Integral auto rhs) const noexcept -> Iterator {
				const auto diff = static_cast<usize>(rhs);
				if(rhs < 0) {
					return std::move(*this - -rhs);
				}

				auto temp = *this;
				temp.m_current_index += diff;
				if(temp.m_current_index > temp.m_container_ptr->capacity()) {
					temp.m_current_index = temp.m_container_ptr->capacity();
					temp.m_ptr = temp.m_container_ptr->end().m_ptr;
				}
				else {
					temp.m_ptr = &(*temp.m_container_ptr)[temp.m_current_index];
				}
				return temp;
			}

			inline constexpr auto operator+=(concepts::Integral auto rhs) noexcept -> Iterator& {
				*this = std::move(*this + rhs);
				return *this;
			}

			inline constexpr auto
			// NOLINTNEXTLINE(misc-no-recursion)
			operator-(concepts::Integral auto rhs) const noexcept -> Iterator {
				const auto diff = static_cast<usize>(rhs);
				if(rhs < 0) {
					return std::move(*this + -rhs);
				}

				auto temp = *this;
				if(diff > temp.m_current_index) {
					temp.m_ptr = temp.m_container_ptr->begin().m_ptr;
					temp.m_current_index = 0;
				}
				else {
					temp.m_current_index -= diff;
					temp.m_ptr = &(*temp.m_container_ptr)[temp.m_current_index];
				}
				return temp;
			}

			inline constexpr auto operator-=(concepts::Integral auto rhs) noexcept -> Iterator& {
				*this = std::move(*this - rhs);
				return *this;
			}

			inline constexpr auto operator-(const Iterator& rhs) const noexcept -> difference_type {
				return static_cast<std::ptrdiff_t>(m_current_index)
					   - static_cast<std::ptrdiff_t>(rhs.m_current_index);
			}

			inline constexpr auto operator[](concepts::Integral auto index) noexcept -> Iterator {
				return std::move(*this + index);
			}

			inline constexpr auto operator<=>(const Iterator& rhs) const noexcept -> std::partial_ordering {
				if(m_container_ptr != rhs.m_container_ptr) {
					return std::partial_ordering::unordered;
				}

				if(m_current_index == rhs.m_current_index) {
					return std::partial_ordering::equivalent;
				}

				if(m_current_index < rhs.m_current_index) {
					return std::partial_ordering::less;
				}

				return std::partial_ordering::greater;
			};

		  private:
			pointer m_ptr;
			RingBuffer* m_container_ptr = nullptr;
			usize m_current_index = 0;
		};

		/// @brief Read-only Random-Access Bidirectional iterator for `RingBuffer`
		/// @note All navigation operators are checked such that any movement past `begin()` or
		/// `end()` is ignored.
		class ConstIterator {
		  public:
			using iterator_category = std::random_access_iterator_tag;
			using difference_type = std::ptrdiff_t;
			using value_type = T;
			using pointer = const value_type*;
			using reference = const value_type&;

			constexpr explicit ConstIterator(pointer ptr,
											 RingBuffer* containerPtr,
											 usize currentIndex) noexcept
				: m_ptr(ptr), m_container_ptr(containerPtr), m_current_index(currentIndex) {
			}
			constexpr ConstIterator(const ConstIterator& iter) noexcept = default;
			constexpr ConstIterator(ConstIterator&& iter) noexcept = default;
			~ConstIterator() noexcept = default;

			/// @brief Returns the index in the `RingBuffer` that corresponds
			/// to the element this iterator points to
			///
			/// @return The index corresponding with the element this points to
			[[nodiscard]] inline constexpr auto get_index() const noexcept -> usize {
				return m_current_index;
			}

			constexpr auto
			operator=(const ConstIterator& iter) noexcept -> ConstIterator& = default;
			constexpr auto operator=(ConstIterator&& iter) noexcept -> ConstIterator& = default;

			inline constexpr auto operator==(const ConstIterator& rhs) const noexcept -> bool {
				return m_current_index == rhs.m_current_index
					   && m_container_ptr == rhs.m_container_ptr;
			}

			inline constexpr auto operator!=(const ConstIterator& rhs) const noexcept -> bool {
				return m_current_index != rhs.m_current_index
				       || m_container_ptr != rhs.m_container_ptr;
			}

			inline constexpr auto operator*() const noexcept -> reference {
				return *m_ptr;
			}

			inline constexpr auto operator->() const noexcept -> pointer {
				return m_ptr;
			}

			inline constexpr auto operator++() noexcept -> ConstIterator& {
				m_current_index++;
				if(m_current_index >= m_container_ptr->capacity()) {
					m_current_index = m_container_ptr->capacity();
					m_ptr = m_container_ptr->end().m_ptr;
				}
				else {
					m_ptr = &(*m_container_ptr)[m_current_index];
				}
				return *this;
			}

			inline constexpr auto operator++(int) noexcept -> ConstIterator {
				ConstIterator temp = *this;
				++(*this);
				return temp;
			}

			inline constexpr auto operator--() noexcept -> ConstIterator& {
				if(m_current_index == 0) {
					return *this;
				}

				m_current_index--;
				m_ptr = &(*m_container_ptr)[m_current_index];
				return *this;
			}

			inline constexpr auto operator--(int) noexcept -> ConstIterator {
				ConstIterator temp = *this;
				--(*this);
				return temp;
			}

			inline constexpr auto
			operator+(concepts::Integral auto rhs) const noexcept -> ConstIterator {
				const auto diff = static_cast<usize>(rhs);
				if(rhs < 0) {
					return std::move(*this - -rhs);
				}

				auto temp = *this;
				temp.m_current_index += diff;
				if(temp.m_current_index > temp.m_container_ptr->capacity()) {
					temp.m_current_index = temp.m_container_ptr->capacity();
					temp.m_ptr = temp.m_container_ptr->end().m_ptr;
				}
				else {
					temp.m_ptr = &(*temp.m_container_ptr)[temp.m_current_index];
				}
				return temp;
			}

			inline constexpr auto
			operator+=(concepts::Integral auto rhs) noexcept -> ConstIterator& {
				*this = std::move(*this + rhs);
				return *this;
			}

			inline constexpr auto
			operator-(concepts::Integral auto rhs) const noexcept -> ConstIterator {
				const auto diff = static_cast<usize>(rhs);
				if(rhs < 0) {
					return std::move(*this + -rhs);
				}

				auto temp = *this;
				if(diff > temp.m_current_index) {
					temp.m_ptr = temp.m_container_ptr->begin().m_ptr;
					temp.m_current_index = 0;
				}
				else {
					temp.m_current_index -= diff;
					temp.m_ptr = &(*temp.m_container_ptr)[temp.m_current_index];
				}
				return temp;
			}

			inline constexpr auto
			operator-=(concepts::Integral auto rhs) noexcept -> ConstIterator& {
				*this = std::move(*this - rhs);
				return *this;
			}

			inline constexpr auto
			operator-(const ConstIterator& rhs) const noexcept -> difference_type {
				return static_cast<std::ptrdiff_t>(m_current_index)
					   - static_cast<std::ptrdiff_t>(rhs.m_current_index);
			}

			inline constexpr auto
			operator[](concepts::Integral auto index) const noexcept -> ConstIterator {
				return std::move(*this + index);
			}

			inline constexpr auto operator<=>(const Iterator& rhs) const noexcept -> std::partial_ordering {
				if(m_container_ptr != rhs.m_container_ptr) {
					return std::partial_ordering::unordered;
				}

				if(m_current_index == rhs.m_current_index) {
					return std::partial_ordering::equivalent;
				}

				if(m_current_index < rhs.m_current_index) {
					return std::partial_ordering::less;
				}

				return std::partial_ordering::greater;
			};

		  private:
			pointer m_ptr;
			RingBuffer* m_container_ptr = nullptr;
			usize m_current_index = 0;
		};

		/// @brief Creates a `RingBuffer` with default capacity
		constexpr RingBuffer() noexcept = default;

		/// @brief Creates a `RingBuffer` with (at least) the given initial capacity
		///
		/// @param initial_capacity - The initial capacity of the `RingBuffer`
		constexpr explicit RingBuffer(usize initial_capacity) noexcept
			: m_buffer(allocate_unique<T[]>(m_allocator, initial_capacity + 1)), // NOLINT
			  m_capacity(initial_capacity + 1) {
			if(!std::is_trivially_destructible_v<T>) {
				m_constructed = ConstructedChecker<T, Allocator>(m_capacity, m_allocator);
			}
		}

		/// @brief Constructs a new `RingBuffer` with the given initial capacity and
		/// fills it with `default_value`
		///
		/// @param initial_capacity - The initial capacity of the `RingBuffer`
		/// @param default_value - The value to fill the `RingBuffer` with
		constexpr RingBuffer(usize initial_capacity,
							 const T& default_value) noexcept
		requires concepts::NoexceptCopyConstructible<T>
		: m_buffer(allocate_unique<T[]>(m_allocator, // NOLINT
										initial_capacity + 1,
										default_value)),
		  m_write_index(initial_capacity),
		  m_start_index(0_usize), // NOLINT
		  m_capacity(initial_capacity + 1) {

			if(!std::is_trivially_destructible_v<T>) {
				m_constructed = ConstructedChecker<T, Allocator>(m_capacity, m_allocator);
			}

			for(auto index = m_start_index; index < m_write_index; ++index) {
				allocator_traits::construct(m_allocator,
											std::addressof(m_buffer[index]),
											default_value);
				if constexpr(!std::is_trivially_destructible_v<T>) {
					m_constructed.set_constructed(index, true);
				}
			}
		}

		constexpr RingBuffer(std::initializer_list<T> values) noexcept
		requires concepts::NoexceptMoveAssignable<T>
		: m_buffer(allocate_unique<T[]>(m_allocator, // NOLINT
										values.size() + 1)),
		  m_write_index(values.size()),
		  m_capacity(values.size() + 1) {

			if(!std::is_trivially_destructible_v<T>) {
				m_constructed = ConstructedChecker<T, Allocator>(m_capacity, m_allocator);
			}

			auto index = m_start_index;
			for(auto&& val : values) {
				allocator_traits::construct(m_allocator,
											std::addressof(m_buffer[index]),
											std::move(val));
				if constexpr(!std::is_trivially_destructible_v<T>) {
					m_constructed.set_constructed(index, true);
				}
				index++;
			}
		}

		constexpr RingBuffer(const RingBuffer& buffer) noexcept
		requires concepts::NoexceptCopyConstructible<T>
		: m_buffer(allocate_unique<T[]>(m_allocator, buffer.m_capacity)), // NOLINT
		  m_write_index(buffer.size()),									  // NOLINT
		  m_start_index(0_usize),										  // NOLINT
		  m_capacity(buffer.m_capacity) {

			if(!std::is_trivially_destructible_v<T>) {
				m_constructed = ConstructedChecker<T, Allocator>(m_capacity, m_allocator);
			}

			const auto size = m_buffer.size();
			for(auto index = 0_usize; index < size; ++index) {
				allocator_traits::construct(m_allocator,
											std::addressof(m_buffer[index]),
											buffer[index]);
				if constexpr(!std::is_trivially_destructible_v<T>) {
					m_constructed.set_constructed(index, true);
				}
			}
		}

		constexpr RingBuffer(RingBuffer&& buffer) noexcept
			: m_allocator(buffer.m_allocator),
			  m_buffer(std::move(buffer.m_buffer)),
			  m_write_index(buffer.m_write_index),
			  m_start_index(buffer.m_start_index),
			  m_capacity(buffer.m_capacity),
			  m_constructed(std::move(buffer.m_constructed)) {
			buffer.m_capacity = 0_usize;
			buffer.m_write_index = 0_usize;
			buffer.m_start_index = 0_usize;
			buffer.m_buffer = nullptr;
		}

		~RingBuffer() noexcept
		requires concepts::NoexceptDestructible<T>
		{
			if constexpr(!std::is_trivially_destructible_v<T>) {
				for(auto index = 0_usize; index < size(); ++index) {
					const auto internal = get_adjusted_internal_index(index);
					if(m_constructed.is_constructed(internal)) {
						allocator_traits::destroy(m_allocator, std::addressof(m_buffer[internal]));
					}
				}
			}
		}

		/// @brief Returns the element at the given index.
		/// @note This is not checked in the same manner as STL containers:
		/// if index >= capacity, the element at capacity - 1 is returned.
		///
		/// @param index - The index of the desired element
		///
		/// @return The element at the given index, or at capacity - 1 if index >= capacity
		[[nodiscard]] inline constexpr auto at(concepts::Integral auto index) noexcept -> T& {
			auto _index = get_adjusted_internal_index(index);

			// NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
			return m_buffer[_index];
		}

		/// @brief Returns the first element in the `RingBuffer`
		///
		/// @return The first element
		[[nodiscard]] inline constexpr auto front() noexcept -> T& {
			// NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
			return m_buffer[m_start_index];
		}

		/// @brief Returns the last element in the `RingBuffer`
		/// @note If <= 1 elements are in the `RingBuffer`, this will be the same as `front`
		///
		/// @return The last element
		[[nodiscard]] inline constexpr auto back() noexcept -> T& {
			const auto index = get_adjusted_internal_index(size() - 1);

			// NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
			return m_buffer[index];
		}

		/// @brief Returns a pointer to the underlying data in the `RingBuffer`.
		/// @note This is not sorted in any way to match the representation used by the `RingBuffer`
		///
		/// @return A pointer to the underlying data
		[[nodiscard]] inline constexpr auto data() noexcept -> T* {
			return m_buffer;
		}

		/// @brief Returns whether the `RingBuffer` is empty
		///
		/// @return `true` if the `RingBuffer` is empty, `false` otherwise
		[[nodiscard]] inline constexpr auto empty() const noexcept -> bool {
			return m_write_index == m_start_index;
		}

		/// @brief Returns whether the `RingBuffer` is full
		///
		/// @return `true` if the `RingBuffer` is full, `false` otherwise
		[[nodiscard]] inline constexpr auto full() const noexcept -> bool {
			return size() == m_capacity - 1;
		}

		/// @brief Returns the current number of elements in the `RingBuffer`
		///
		/// @return The current number of elements
		[[nodiscard]] inline constexpr auto size() const noexcept -> usize {
			return m_write_index >= m_start_index ? (m_write_index - m_start_index) :
													(m_capacity - (m_start_index - m_write_index));
		}

		/// @brief Returns the maximum possible number of elements this `RingBuffer` could store
		/// if grown to maximum possible capacity
		///
		/// @return The maximum possible number of storable elements
		[[nodiscard]] inline constexpr auto max_size() const noexcept -> usize {
			return allocator_traits::max_size(m_allocator) - 1;
		}

		/// @brief Returns the current capacity of the `RingBuffer`;
		/// the number of elements it can currently store
		///
		/// @return The current capacity
		[[nodiscard]] inline constexpr auto capacity() const noexcept -> usize {
			return m_capacity - 1;
		}

		/// @brief Reserves more storage for the `RingBuffer`. If `new_capacity` is > capacity,
		/// then the capacity of the `RingBuffer` will be extended until at least `new_capacity`
		/// elements can be stored.
		/// @note Memory contiguity is maintained, so no **elements** will be lost or invalidated.
		/// However, all iterators and references to elements will be invalidated.
		///
		/// @param new_capacity - The new capacity of the `RingBuffer`
		inline constexpr auto reserve(usize new_capacity) noexcept -> void {
			// we only need to do anything if `new_capacity` is actually larger than `m_capacity`
			if(new_capacity > m_capacity) {
				auto temp
					= allocate_unique<T[], Allocator<T[]>>(m_allocator, new_capacity + 1); // NOLINT
				auto span = gsl::make_span(std::addressof(temp[0]), new_capacity + 1);

				const auto size = this->size();
				for(auto index = 0_usize; index < size; ++index) {
					allocator_traits::construct(
						m_allocator,
						// NOLINTNEXTLINE
						std::addressof(span[index]),
						// NOLINTNEXTLINE
						std::move(m_buffer[get_adjusted_internal_index(index)]));
				}

				if constexpr(!std::is_trivially_destructible_v<T>) {
					ConstructedChecker<T, Allocator> checker
						= ConstructedChecker<T, Allocator>(new_capacity, m_allocator);

					for(auto index = 0_usize; index < size; ++index) {
						const auto internal = get_adjusted_internal_index(index);
						checker.set_constructed(index, m_constructed.is_constructed(internal));
					}

					m_constructed = std::move(checker);
				}
				m_buffer = std::move(temp);
				m_start_index = 0;
				m_write_index = m_capacity - 1;
				m_capacity = new_capacity + 1;
			}
		}

		/// @brief Erases all elements from the `RingBuffer`
		inline constexpr auto clear() noexcept -> void {
			if constexpr(!std::is_trivially_destructible_v<T>) {
				for(auto index = 0_usize; index < size(); ++index) {
					const auto internal = get_adjusted_internal_index(index);
					// NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
					allocator_traits::destroy(m_allocator, std::addressof(m_buffer[internal]));
					m_constructed.set_constructed(index, false);
				}
			}
			m_start_index = 0;
			m_write_index = 0;
		}

		/// @brief Inserts the given element at the end of the `RingBuffer`
		/// @note if `size() == capacity()` then this loops and overwrites `front()`
		///
		/// @param value - the element to insert
		template<typename U>
		requires concepts::Same<T, std::remove_cvref_t<U>>
				 inline constexpr auto push_back(U&& value) noexcept -> void
				 requires concepts::NoexceptConstructibleFrom<T, U>
		{
			if constexpr(!std::is_trivially_destructible_v<T>) {
				if(m_constructed.is_constructed(m_write_index)) {
					allocator_traits::destroy(
						m_allocator,
						// NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
						std::addressof(m_buffer[m_write_index]));
				}
			}

			allocator_traits::construct(
				m_allocator,
				// NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
				std::addressof(m_buffer[m_write_index]),
				std::forward<U>(value));
			if constexpr(!std::is_trivially_destructible_v<T>) {
				m_constructed.set_constructed(m_write_index, true);
			}

			increment_indices();
		}

		/// @brief Constructs the given element in place at the end of the `RingBuffer`
		/// @note if `size() == capacity()` then this loops and overwrites `front()`
		///
		/// @tparam Args - The types of the element's constructor arguments
		/// @param args - The constructor arguments for the element
		///
		/// @return A reference to the element constructed at the end of the `RingBuffer`
		template<typename... Args>
		requires concepts::NoexceptConstructibleFrom<T, Args...>
		inline constexpr auto emplace_back(Args&&... args) noexcept -> T& {
			if constexpr(!std::is_trivially_destructible_v<T>) {
				if(m_constructed.is_constructed(m_write_index)) {
					allocator_traits::destroy(
						m_allocator,
						// NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
						std::addressof(m_buffer[m_write_index]));
				}
			}

			allocator_traits::construct(m_allocator,
										std::addressof(m_buffer[m_write_index]), // NOLINT
										std::forward<Args>(args)...);
			if constexpr(!std::is_trivially_destructible_v<T>) {
				m_constructed.set_constructed(m_write_index, true);
			}

			auto index = m_write_index;

			increment_indices();

			return m_buffer[index]; // NOLINT
		}

		/// @brief Constructs the given element in place at the location
		/// indicated by the `Iterator` `position`
		///
		/// @tparam Args - The types of the element's constructor arguments
		/// @param position - `Iterator` indicating where in the `RingBuffer` to construct the
		/// element
		/// @param args - The constructor arguments for the element
		///
		/// @return A reference to the element constructed at the location indicated by `position`
		template<typename... Args>
		requires concepts::NoexceptConstructibleFrom<T, Args...>
		inline constexpr auto emplace(const Iterator& position, Args&&... args) noexcept -> T& {
			auto index = get_adjusted_internal_index(position.get_index());

			if constexpr(!std::is_trivially_destructible_v<T>) {
				if(m_constructed.is_constructed(index)) {
					allocator_traits::destroy(
						m_allocator,
						// NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
						std::addressof(m_buffer[m_write_index]));
				}
			}

			allocator_traits::construct(
				m_allocator,
				// NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
				std::addressof(m_buffer[index]),
				std::forward<Args>(args)...);
			if constexpr(!std::is_trivially_destructible_v<T>) {
				m_constructed.set_constructed(index, true);
			}

			return m_buffer[index]; // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
		}

		/// @brief Constructs the given element in place at the location
		/// indicated by the `ConstIterator` `position`
		///
		/// @tparam Args - The types of the element's constructor arguments
		/// @param position - `ConstIterator` indicating where in the `RingBuffer` to construct the
		/// element
		/// @param args - The constructor arguments for the element
		///
		/// @return A reference to the element constructed at the location indicated by `position`
		template<typename... Args>
		inline constexpr auto
		emplace(const ConstIterator& position, Args&&... args) noexcept -> T& {
			auto index = get_adjusted_internal_index(position.get_index());

			if constexpr(!std::is_trivially_destructible_v<T>) {
				if(m_constructed.is_constructed(index)) {
					allocator_traits::destroy(
						m_allocator,
						// NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
						std::addressof(m_buffer[m_write_index]));
				}
			}

			allocator_traits::construct(
				m_allocator,
				// NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
				std::addressof(m_buffer[index]),
				std::forward<Args>(args)...);
			if constexpr(!std::is_trivially_destructible_v<T>) {
				m_constructed.set_constructed(index, true);
			}

			return m_buffer[index]; // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
		}

		/// @brief Assigns the given element to the position indicated
		/// by the `Iterator` `position`
		/// @note if `size() == capacity()` this drops the last element out of the `RingBuffer`
		///
		/// @param position - `Iterator` indicating where in the `RingBuffer` to place the element
		/// @param element - The element to store in the `RingBuffer`
		inline constexpr auto insert(const Iterator& position, const T& element) noexcept -> void
		requires concepts::CopyAssignable<T>
		{
			insert_internal(position.get_index(), element);
		}

		/// @brief Assigns the given element to the position indicated
		/// by the `Iterator` `position`
		/// @note if `size() == capacity()` this drops the last element out of the `RingBuffer`
		///
		/// @param position - `Iterator` indicating where in the `RingBuffer` to place the element
		/// @param element - The element to store in the `RingBuffer`
		inline constexpr auto insert(const Iterator& position, T&& element) noexcept -> void {
			insert_internal(position.get_index(), std::forward<T>(element));
		}

		/// @brief Assigns the given element to the position indicated
		/// by the `ConstIterator` `position`
		/// @note if `size() == capacity()` this drops the last element out of the `RingBuffer`
		///
		/// @param position - `ConstIterator` indicating where in the `RingBuffer` to place the
		/// element
		/// @param element - The element to store in the `RingBuffer`
		inline constexpr auto
		insert(const ConstIterator& position, const T& element) noexcept -> void
		requires concepts::CopyAssignable<T>
		{
			insert_internal(position.get_index(), element);
		}

		/// @brief Assigns the given element to the position indicated
		/// by the `ConstIterator` `position`
		/// @note if `size() == capacity()` this drops the last element out of the `RingBuffer`
		///
		/// @param position - `ConstIterator` indicating where in the `RingBuffer` to place the
		/// element
		/// @param element - The element to store in the `RingBuffer`
		inline constexpr auto insert(const ConstIterator& position, T&& element) noexcept -> void {
			insert_internal(position.get_index(), std::forward<T>(element));
		}

		/// @brief Constructs the given element at the insertion position indicated
		/// by the `ConstIterator` `position`
		/// @note if `size() == capacity()` this drops the last element out of the `RingBuffer`
		///
		/// @param position - `ConstIterator` indicating where in the `RingBuffer` to place the
		/// element
		/// @param args - The arguments to the constructor for
		/// the element to store in the `RingBuffer`
		template<typename... Args>
		requires concepts::ConstructibleFrom<T, Args...>
		inline constexpr auto
		insert_emplace(const Iterator& position, Args&&... args) noexcept -> T& {
			return insert_emplace_internal(position.get_index(), std::forward<Args>(args)...);
		}

		/// @brief Constructs the given element at the insertion position indicated
		/// by the `ConstIterator` `position`
		/// @note if `size() == capacity()` this drops the last element out of the `RingBuffer`
		///
		/// @param position - `ConstIterator` indicating where in the `RingBuffer` to place the
		/// element
		/// @param args - The arguments to the constructor for
		/// the element to store in the `RingBuffer`
		template<typename... Args>
		requires concepts::ConstructibleFrom<T, Args...>
		inline constexpr auto
		insert_emplace(const ConstIterator& position, Args&&... args) noexcept -> T& {
			return insert_emplace_internal(position.get_index(), std::forward<Args>(args)...);
		}

		/// @brief Erases the element at the given `position`, moving other elements backward
		/// in the buffer to maintain contiguity
		///
		/// @param position - The element to erase
		///
		/// @return `Iterator` pointing to the element after the one erased
		inline constexpr auto erase(const Iterator& position) noexcept -> Iterator {
			return erase_internal(position.get_index());
		}

		/// @brief Erases the element at the given `position`, moving other elements backward
		/// in the buffer to maintain contiguity
		///
		/// @param position - The element to erase
		///
		/// @return `Iterator` pointing to the element after the one erased
		inline constexpr auto erase(const ConstIterator& position) noexcept -> Iterator {
			return erase_internal(position.get_index());
		}

		/// @brief Erases the range of elements in [`first`, `last`)
		/// Returns an `Iterator` to the element after the last one erased
		/// @note In the case `first` >= `last`, no elements are erased and `last` is returned;
		///
		/// @param first - The first element in the range to erase
		/// @param last - The last element in the range
		///
		/// @return `Iterator` pointing to the element after the last one erased
		inline constexpr auto
		erase(const Iterator& first, const Iterator& last) noexcept -> Iterator {
			if(first >= last) {
				return last;
			}

			return erase_internal(first.get_index(), last.get_index());
		}

		/// @brief Erases the range of elements in [`first`, `last`)
		/// Returns an `Iterator` to the element after the last one erased
		/// @note In the case `first` >= `last`, no elements are erased and `last` is returned;
		///
		/// @param first - The first element in the range to erase
		/// @param last - The last element in the range
		///
		/// @return `Iterator` pointing to the element after the last one erased
		inline constexpr auto
		erase(const ConstIterator& first, const ConstIterator& last) noexcept -> Iterator {
			if(first >= last) {
				return last;
			}

			return erase_internal(first.get_index(), last.get_index());
		}

		/// @brief Removes the last element in the `RingBuffer` and returns it
		///
		/// @return The last element in the `RingBuffer`
		[[nodiscard]] inline constexpr auto pop_back() noexcept -> hyperion::Option<T>
		requires concepts::NoexceptMoveConstructible<T>
		{
			if(size() > 0) {
				const auto index = get_adjusted_internal_index(size() - 1);
				auto ret = Some(std::move(m_buffer[index]));

				decrement_write();
				if constexpr(!std::is_trivially_destructible_v<T>) {
					allocator_traits::destroy(m_allocator, std::addressof(m_buffer[m_write_index]));
					m_constructed.set_constructed(m_write_index, false);
				}

				return ret;
			}

			return None();
		}

		/// @brief Removes the first element in the `RingBuffer` and returns it
		///
		/// @return The first element in the `RingBuffer`
		[[nodiscard]] inline constexpr auto pop_front() noexcept -> hyperion::Option<T>
		requires concepts::NoexceptMoveConstructible<T>
		{
			if(size() > 0) {
				auto ret = Some(std::move(m_buffer[m_start_index]));

				if constexpr(!std::is_trivially_destructible_v<T>) {
					allocator_traits::destroy(m_allocator, std::addressof(m_buffer[m_start_index]));
					m_constructed.set_constructed(m_start_index, false);
				}
				increment_start();

				return ret;
			}

			return None();
		}

		/// @brief Returns a Random Access Bidirectional iterator over the `RingBuffer`,
		/// at the beginning
		///
		/// @return The iterator, at the beginning
		[[nodiscard]] inline constexpr auto begin() -> Iterator {
			// NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
			T* ptr = std::addressof(m_buffer[m_start_index]);

			return Iterator(ptr, this, 0_usize);
		}

		/// @brief Returns a Random Access Bidirectional iterator over the `RingBuffer`,
		/// at the end
		///
		/// @return The iterator, at the end
		[[nodiscard]] inline constexpr auto end() -> Iterator {
			// NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
			T* ptr = std::addressof(m_buffer[m_write_index]);

			return Iterator(ptr, this, size());
		}

		/// @brief Returns a Random Access Bidirectional read-only iterator over the `RingBuffer`,
		/// at the beginning
		///
		/// @return The iterator, at the beginning
		[[nodiscard]] inline constexpr auto cbegin() -> ConstIterator {
			// NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
			T* ptr = std::addressof(m_buffer[m_start_index]);

			return ConstIterator(ptr, this, 0_usize);
		}

		/// @brief Returns a Random Access Bidirectional read-only iterator over the `RingBuffer`,
		/// at the end
		///
		/// @return The iterator, at the end
		[[nodiscard]] inline constexpr auto cend() -> ConstIterator {
			// NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
			T* ptr = std::addressof(m_buffer[m_write_index]);

			return ConstIterator(ptr, this, size());
		}

		/// @brief Unchecked access-by-index operator
		///
		/// @param index - The index to get the corresponding element for
		///
		/// @return - The element at index
		[[nodiscard]] inline constexpr auto
		operator[](concepts::Integral auto index) noexcept -> T& {
			auto _index = get_adjusted_internal_index(index);

			return m_buffer[_index]; // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
		}

		constexpr auto operator=(const RingBuffer& buffer) noexcept -> RingBuffer&
		requires concepts::NoexceptCopyConstructible<T>
		{
			if(this == &buffer) {
				return *this;
			}
			auto temp = allocate_unique<T[]>(m_allocator, buffer.m_capacity); // NOLINT
			const auto size = buffer.size();
			auto constructed = ConstructedChecker<T, Allocator>(buffer.m_capacity, m_allocator);

			for(auto index = 0_usize; index < size; ++index) {
				if constexpr(!std::is_trivially_destructible_v<T>) {
					const auto internal = buffer.get_adjusted_internal_index(index);
					if(buffer.m_constructed.is_constructed(internal)) {
						allocator_traits::construct(m_allocator,
													std::addressof(temp[index]),
													buffer[index]);
						constructed.set_constructed(index, true);
					}
				}
				else {
					const auto internal = buffer.get_adjusted_internal_index(index);
					allocator_traits::construct(m_allocator,
												std::addressof(temp[index]),
												buffer[index]);
				}
			}
			if constexpr(!std::is_trivially_destructible_v<T>) {
				for(auto index = 0_usize; index < m_capacity; ++index) {
					if(m_constructed.is_constructed(index)) {
						allocator_traits::destroy(m_allocator, std::addressof(m_buffer[index]));
					}
				}
			}
			m_buffer = std::move(temp);
			m_constructed = std::move(constructed);
			m_capacity = buffer.m_capacity;
			m_start_index = 0_usize;
			m_write_index = size;
			return *this;
		}
		constexpr auto operator=(RingBuffer&& buffer) noexcept -> RingBuffer& {
			if constexpr(!std::is_trivially_destructible_v<T>) {
				for(auto index = 0_usize; index < m_capacity; ++index) {
					if(m_constructed.is_constructed(index)) {
						allocator_traits::destroy(m_allocator, std::addressof(m_buffer[index]));
					}
				}
			}
			m_allocator = buffer.m_allocator;
			m_buffer = std::move(buffer.m_buffer);
			m_write_index = buffer.m_write_index;
			m_start_index = buffer.m_start_index;
			m_capacity = buffer.m_capacity;
			buffer.m_buffer = nullptr;
			buffer.m_write_index = 0_usize;
			buffer.m_start_index = 0_usize;
			buffer.m_capacity = 0_usize;
			return *this;
		}

	  private:
		template<typename U, template<typename ElementType> typename UAlloc = std::allocator>
		class ConstructedChecker;

		template<typename U, template<typename ElementType> typename UAlloc>
		requires(std::is_trivially_destructible_v<T>)
		class ConstructedChecker<U, UAlloc> {
		  public:
			using Alloc = UAlloc<bool>;
			explicit ConstructedChecker([[maybe_unused]] usize size,
										[[maybe_unused]] Alloc alloc = Alloc()) noexcept {
			}

			ConstructedChecker(const ConstructedChecker&) noexcept = default;
			ConstructedChecker(ConstructedChecker&&) noexcept = default;
			~ConstructedChecker() noexcept = default;

			inline auto set_constructed([[maybe_unused]] usize index,
										[[maybe_unused]] bool constructed) noexcept -> void {
				// if std::is_trivially_destructible_v<T>, then we do nothing
			}
			inline auto is_constructed([[maybe_unused]] usize index) noexcept -> bool {
				return false;
			}

			auto operator=(const ConstructedChecker&) noexcept -> ConstructedChecker& = default;
			auto operator=(ConstructedChecker&&) noexcept -> ConstructedChecker& = default;
		};

		template<typename U, template<typename ElementType> typename UAlloc>
		requires(!std::is_trivially_destructible_v<T>)
		class ConstructedChecker<U, UAlloc> {
		  public:
			using Alloc = UAlloc<bool>;

			explicit ConstructedChecker(usize size, Alloc alloc = Alloc()) noexcept
				: m_flags(size, false, alloc) {
			}

			ConstructedChecker(const ConstructedChecker&) noexcept = default;
			ConstructedChecker(ConstructedChecker&&) noexcept = default;
			~ConstructedChecker() noexcept = default;

			inline auto set_constructed(usize index, bool constructed) noexcept -> void {
				m_flags[index] = constructed;
			}
			inline auto is_constructed(usize index) noexcept -> bool {
				return m_flags[index];
			}

			auto operator=(const ConstructedChecker&) noexcept -> ConstructedChecker& = default;
			auto operator=(ConstructedChecker&&) noexcept -> ConstructedChecker& = default;

		  private:
			std::vector<bool, Alloc> m_flags;
		};

		static const constexpr usize DEFAULT_CAPACITY_INTERNAL = DEFAULT_CAPACITY + 1;
		[[HYPERION_NO_UNIQUE_ADDRESS]] Allocator<T> m_allocator = Allocator<T>();
		unique_pointer m_buffer
			= allocate_unique<T[]>(m_allocator, DEFAULT_CAPACITY_INTERNAL); // NOLINT
		usize m_write_index = 0_usize;
		usize m_start_index = 0_usize;
		usize m_capacity = DEFAULT_CAPACITY_INTERNAL;
		[[HYPERION_NO_UNIQUE_ADDRESS]] ConstructedChecker<T, Allocator> m_constructed
			= ConstructedChecker<T, Allocator>(DEFAULT_CAPACITY_INTERNAL, m_allocator);

		/// @brief Converts the given `RingBuffer` index into the corresponding index into then
		/// underlying `T` array
		///
		/// @param index - The `RingBuffer` index to convert
		///
		/// @return The corresponding index into the underlying `T` array
		[[nodiscard]] inline constexpr auto
		get_adjusted_internal_index(concepts::Integral auto index) const noexcept -> usize {
			auto _index = static_cast<usize>(index);
			return (m_start_index + _index) % m_capacity;
		}

		/// @brief Used to increment the start and write indices into the underlying `T` array,
		/// and the size property, after pushing an element at the back,
		/// maintaining the logical `RingBuffer` structure
		inline constexpr auto increment_indices() noexcept -> void {
			m_write_index = (m_write_index + 1) % m_capacity;

			// if write index is at start, we need to push start forward to maintain
			// the "invalid" spacer element for this.end()
			if(m_write_index == m_start_index) {
				m_start_index = (m_start_index + 1) % m_capacity;
			}
		}

		/// @brief Used to increment the start index into the underlying `T` array
		/// and the size property after popping an element from the front,
		/// maintaining the logical `RingBuffer` structure
		inline constexpr auto increment_start() noexcept -> void {
			if(m_start_index != m_write_index) {
				m_start_index = (m_start_index + 1) % m_capacity;
			}
		}

		/// @brief Used to decrement the write index into the underlying `T` array
		/// when popping an element from the back
		inline constexpr auto decrement_write() noexcept -> void {
			if(m_write_index == 0_usize) {
				m_write_index = m_capacity - 1;
			}
			else {
				m_write_index--;
			}
		}

		inline constexpr auto
		decrement_write_n(concepts::UnsignedIntegral auto n) noexcept -> void {
			auto amount_to_decrement = static_cast<usize>(n);
			if(amount_to_decrement > m_write_index) {
				amount_to_decrement -= m_write_index;
				m_write_index = (m_capacity - 1) - amount_to_decrement;
			}
			else {
				m_write_index -= amount_to_decrement;
			}
		}

		/// @brief Inserts the given element at the position indicated
		/// by the `external_index`
		/// @note if `size() == capacity()` this drops the last element out of the `RingBuffer`
		///
		/// @param external_index - The user-facing index into the `RingBuffer` to insert the
		/// element at
		/// @param elem - The element to store in the `RingBuffer`
		template<typename U>
		requires concepts::NoexceptConstructibleFrom<T, U>
		inline constexpr auto
		insert_internal(usize external_index, U&& elem) noexcept -> void { // NOLINT
			auto index = get_adjusted_internal_index(external_index);

			if(index == m_write_index) {
				emplace_back(elem);
			}
			else {
				const auto size_ = size();
				auto num_to_move = size_ - external_index;
				auto source_index = num_to_move - 1;

				// if we're full, drop the last element in the buffer
				if(size_ == m_capacity - 1) [[likely]] { // NOLINT
					--num_to_move;
					--source_index;
					index++;
				}

				for(auto i = 0_usize; i < num_to_move; ++i, --source_index) {
					const auto destination = get_adjusted_internal_index(size_ - i);
					const auto source = get_adjusted_internal_index(external_index + source_index);
					if(!m_constructed.is_constructed(destination)) {
						if(m_constructed.is_constructed(source)) {
							if constexpr(concepts::NoexceptMoveAssignable<T>) {
								allocator_traits::construct(m_allocator,
															std::addressof(m_buffer[destination]),
															std::move(m_buffer[source]));
							}
							else {
								allocator_traits::construct(m_allocator,
															std::addressof(m_buffer[destination]),
															m_buffer[source]);
							}

							m_constructed.set_constructed(destination, true);

							if(!std::is_trivially_destructible_v<T>) {
								allocator_traits::destroy(m_allocator,
														  std::addressof(m_buffer[source]));
								m_constructed.set_constructed(source, false);
							}
						}
					}
					else {
						if(m_constructed.is_constructed(source)) {
							if constexpr(concepts::NoexceptMoveAssignable<T>) {
								m_buffer[destination] = std::move(m_buffer[source]);
							}
							else {
								m_buffer[destination] = m_buffer[source];
							}
							if(!std::is_trivially_destructible_v<T>) {
								allocator_traits::destroy(m_allocator,
														  std::addressof(m_buffer[source]));
								m_constructed.set_constructed(source, false);
							}
						}
					}
				}

				if(m_constructed.is_constructed(index)) [[unlikely]] {
					m_buffer[index] = std::forward<U>(elem);
				}
				else {
					allocator_traits::construct(m_allocator,
												std::addressof(m_buffer[index]),
												std::forward<U>(elem));
					m_constructed.set_constructed(index, true);
				}
				increment_indices();
			}
		}

		/// @brief Constructs the given element at the insertion position indicated
		/// by the `external_index`
		/// @note if `size() == capacity()` this drops the last element out of the `RingBuffer`
		///
		/// @param external_index - The user-facing index into the `RingBuffer` to insert the
		/// element at
		/// @param args - The arguments to the constructor for
		/// the element to store in the `RingBuffer`
		template<typename... Args>
		requires concepts::ConstructibleFrom<T, Args...>
		inline constexpr auto
		insert_emplace_internal(usize external_index, Args&&... args) noexcept -> T& { // NOLINT
			auto index = get_adjusted_internal_index(external_index);

			if(index == m_write_index) {
				return emplace_back(std::forward<Args>(args)...);
			}

			const auto size_ = size();
			auto num_to_move = size_ - external_index;
			auto source_index = num_to_move - 1;

			// if we're full, drop the last element in the buffer
			if(size_ == m_capacity - 1) [[likely]] { // NOLINT
				--num_to_move;
				--source_index;
				index++;
			}

			for(auto i = 0_usize; i < num_to_move; ++i, --source_index) {
				const auto destination = get_adjusted_internal_index(size_ - i);
				const auto source = get_adjusted_internal_index(external_index + source_index);
				if(!m_constructed.is_constructed(destination)) {
					if(m_constructed.is_constructed(source)) {
						if constexpr(concepts::NoexceptMoveAssignable<T>) {
							allocator_traits::construct(m_allocator,
														std::addressof(m_buffer[destination]),
														std::move(m_buffer[source]));
						}
						else {
							allocator_traits::construct(m_allocator,
														std::addressof(m_buffer[destination]),
														m_buffer[source]);
						}

						m_constructed.set_constructed(destination, true);

						if(!std::is_trivially_destructible_v<T>) {
							allocator_traits::destroy(m_allocator,
													  std::addressof(m_buffer[source]));
							m_constructed.set_constructed(source, false);
						}
					}
				}
				else {
					if(m_constructed.is_constructed(source)) {
						if constexpr(concepts::NoexceptMoveAssignable<T>) {
							m_buffer[destination] = std::move(m_buffer[source]);
						}
						else {
							m_buffer[destination] = m_buffer[source];
						}
						if(!std::is_trivially_destructible_v<T>) {
							allocator_traits::destroy(m_allocator,
													  std::addressof(m_buffer[source]));
							m_constructed.set_constructed(source, false);
						}
					}
				}
			}

			if(m_constructed.is_constructed(index)) [[unlikely]] {
				allocator_traits::destroy(m_allocator, std::addressof(m_buffer[index]));
			}
			else {
				m_constructed.set_constructed(index, true);
			}

			allocator_traits::construct(m_allocator,
										std::addressof(m_buffer[index]),
										std::forward<Args>(args)...);

			increment_indices();
			return m_buffer[index];
		}

		/// @brief Erases the element at the given index, returning an `Iterator` to the element
		/// after the removed one
		///
		/// @param external_index - The index to the element to remove. This should be a
		/// `RingBuffer` index: IE, not an internal one into the `T` array
		///
		/// @return `Iterator` pointing to the element after the one removed
		[[nodiscard]] inline constexpr auto
		erase_internal(usize external_index) noexcept -> Iterator { // NOLINT
			const auto index = get_adjusted_internal_index(external_index);

			if(index == m_write_index) [[unlikely]] { // NOLINT
				return end();
			}
			else {
				const auto size_ = size();
				auto num_to_move = (size_ - 1) - external_index;
				const auto pos_to_move = external_index + 1;
				const auto pos_to_replace = external_index;
				for(auto i = 0_usize; i < num_to_move; ++i) {
					const auto destination = get_adjusted_internal_index(pos_to_replace + i);
					const auto source = get_adjusted_internal_index(pos_to_move + i);
					if(!m_constructed.is_constructed(destination)) {
						if(m_constructed.is_constructed(source)) {
							if constexpr(concepts::NoexceptMoveAssignable<T>) {
								allocator_traits::construct(m_allocator,
															std::addressof(m_buffer[destination]),
															std::move(m_buffer[source]));
							}
							else {
								allocator_traits::construct(m_allocator,
															std::addressof(m_buffer[destination]),
															m_buffer[source]);
							}

							m_constructed.set_constructed(destination, true);

							if(!std::is_trivially_destructible_v<T>) {
								allocator_traits::destroy(m_allocator,
														  std::addressof(m_buffer[source]));
								m_constructed.set_constructed(source, false);
							}
						}
					}
					else {
						if(m_constructed.is_constructed(source)) {
							if constexpr(concepts::NoexceptMoveAssignable<T>) {
								m_buffer[destination] = std::move(m_buffer[source]);
							}
							else {
								m_buffer[destination] = m_buffer[source];
							}
							if(!std::is_trivially_destructible_v<T>) {
								allocator_traits::destroy(m_allocator,
														  std::addressof(m_buffer[source]));
								m_constructed.set_constructed(source, false);
							}
						}
					}
				}

				decrement_write();

				return begin() + external_index;
			}
		}

		/// @brief Erases the range of elements in [`first`, `last`)
		/// Returns an `Iterator` to the element after the last one erased
		///
		/// @param first - The first index in the range to erase. This should be a `RingBuffer`
		/// index: IE, not an internal one into the `T` array
		/// @param last - The last index` in the range to erase. This should be a `RingBuffer`
		/// index: IE, not an internal one into the `T` array
		///
		/// @return `Iterator` pointing to the element after the last one erased
		[[nodiscard]] inline constexpr auto
		erase_internal(usize first, usize last) noexcept -> Iterator { // NOLINT
			const auto size_ = size();
			const auto last_internal = get_adjusted_internal_index(last);
			const auto num_to_remove = (last - first);

			if(last_internal > m_write_index) {
				if(m_write_index > m_start_index) {
					decrement_write_n(num_to_remove);
				}
				else if(m_write_index < m_start_index) {
					auto num_after_start_index
						= (m_write_index > num_to_remove ? m_write_index - num_to_remove : 0_usize);
					auto num_before_start_index = num_to_remove - num_after_start_index;
					if(num_after_start_index > 0) {
						num_after_start_index--;
						m_write_index = (m_capacity - 1) - num_after_start_index;
					}
					else {
						decrement_write_n(num_before_start_index);
					}
				}
				return end();
			}

			const auto num_to_move = size_ - last;
			const auto pos_to_move = last;
			const auto pos_to_replace = first;
			for(auto i = 0_usize; i < num_to_move; ++i) {
				const auto destination = get_adjusted_internal_index(pos_to_replace + i);
				const auto source = get_adjusted_internal_index(pos_to_move + i);
				if(!m_constructed.is_constructed(destination)) {
					if(m_constructed.is_constructed(source)) {
						if constexpr(concepts::NoexceptMoveAssignable<T>) {
							allocator_traits::construct(m_allocator,
														std::addressof(m_buffer[destination]),
														std::move(m_buffer[source]));
						}
						else {
							allocator_traits::construct(m_allocator,
														std::addressof(m_buffer[destination]),
														m_buffer[source]);
						}

						m_constructed.set_constructed(destination, true);

						if(!std::is_trivially_destructible_v<T>) {
							allocator_traits::destroy(m_allocator,
													  std::addressof(m_buffer[source]));
							m_constructed.set_constructed(source, false);
						}
					}
				}
				else {
					if(m_constructed.is_constructed(source)) {
						if constexpr(concepts::NoexceptMoveAssignable<T>) {
							m_buffer[destination] = std::move(m_buffer[source]);
						}
						else {
							m_buffer[destination] = m_buffer[source];
						}
						if(!std::is_trivially_destructible_v<T>) {
							allocator_traits::destroy(m_allocator,
													  std::addressof(m_buffer[source]));
							m_constructed.set_constructed(source, false);
						}
					}
				}
			}
			decrement_write_n(num_to_remove);

			return begin() + first;
		}
	};
	IGNORE_PADDING_STOP
} // namespace hyperion
