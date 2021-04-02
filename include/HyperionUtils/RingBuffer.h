#pragma once

#include <atomic>
#include <compare>
#include <cstddef>
#include <gsl/gsl>
#include <iostream>
#include <iterator>
#include <limits>
#include <memory>
#include <memory_resource>

#include "Concepts.h"
#include "Macros.h"
#include "detail/AllocateUnique.h"

namespace hyperion::utils {
	using concepts::DefaultConstructible, concepts::Integral, concepts::UnsignedIntegral,
		concepts::Copyable, concepts::Movable, concepts::NotMovable, concepts::ConstructibleFrom;

	using detail::allocate_unique;

	/// @brief The thread-safety type of the `RingBuffer`
	enum class RingBufferType : size_t
	{
		NotThreadSafe = 0,
		ThreadSafe = 1
	};

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
	template<DefaultConstructible T,
			 RingBufferType ThreadSafety = RingBufferType::NotThreadSafe,
			 template<typename ElementType> typename Allocator = std::allocator>
	class RingBuffer {
	  public:
		/// Default capacity of `RingBuffer`
		static const constexpr size_t DEFAULT_CAPACITY = 16;
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
										size_t currentIndex) noexcept
				: m_ptr(ptr), m_container_ptr(containerPtr), m_current_index(currentIndex) {
			}
			constexpr Iterator(const Iterator& iter) noexcept = default;
			constexpr Iterator(Iterator&& iter) noexcept = default;
			~Iterator() noexcept = default;

			/// @brief Returns the index in the `RingBuffer` that corresponds
			/// to the element this iterator points to
			///
			/// @return The index corresponding with the element this points to
			[[nodiscard]] constexpr inline auto get_index() const noexcept -> size_t {
				return m_current_index;
			}

			constexpr auto operator=(const Iterator& iter) noexcept -> Iterator& = default;
			constexpr auto operator=(Iterator&& iter) noexcept -> Iterator& = default;

			constexpr inline auto operator==(const Iterator& rhs) const noexcept -> bool {
				return m_ptr == rhs.m_ptr;
			}

			constexpr inline auto operator!=(const Iterator& rhs) const noexcept -> bool {
				return m_ptr != rhs.m_ptr;
			}

			constexpr inline auto operator*() const noexcept -> reference {
				return *m_ptr;
			}

			constexpr inline auto operator->() noexcept -> pointer {
				return m_ptr;
			}

			constexpr inline auto operator++() noexcept -> Iterator& {
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

			constexpr inline auto operator++(int) noexcept -> Iterator {
				Iterator temp = *this;
				++(*this);
				return temp;
			}

			constexpr inline auto operator--() noexcept -> Iterator& {
				if(m_current_index == 0) {
					return *this;
				}
				else {
					m_current_index--;
					m_ptr = &(*m_container_ptr)[m_current_index];
				}
				return *this;
			}

			constexpr inline auto operator--(int) noexcept -> Iterator {
				Iterator temp = *this;
				--(*this);
				return temp;
			}

			constexpr inline auto operator+(Integral auto rhs) const noexcept -> Iterator {
				const auto diff = static_cast<size_t>(rhs);
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

			constexpr inline auto operator+=(Integral auto rhs) noexcept -> Iterator& {
				*this = std::move(*this + rhs);
				return *this;
			}

			constexpr inline auto operator-(Integral auto rhs) const noexcept -> Iterator {
				const auto diff = static_cast<size_t>(rhs);
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

			constexpr inline auto operator-=(Integral auto rhs) noexcept -> Iterator& {
				*this = std::move(*this - rhs);
				return *this;
			}

			constexpr inline auto operator-(const Iterator& rhs) const noexcept -> difference_type {
				return static_cast<std::ptrdiff_t>(m_current_index)
					   - static_cast<std::ptrdiff_t>(rhs.m_current_index);
			}

			constexpr inline auto operator[](Integral auto index) noexcept -> Iterator {
				return std::move(*this + index);
			}

			constexpr inline auto operator>(const Iterator& rhs) const noexcept -> bool {
				return m_current_index > rhs.m_current_index;
			}

			constexpr inline auto operator<(const Iterator& rhs) const noexcept -> bool {
				return m_current_index < rhs.m_current_index;
			}

			constexpr inline auto operator>=(const Iterator& rhs) const noexcept -> bool {
				return m_current_index >= rhs.m_current_index;
			}

			constexpr inline auto operator<=(const Iterator& rhs) const noexcept -> bool {
				return m_current_index <= rhs.m_current_index;
			}

		  private:
			pointer m_ptr;
			RingBuffer* m_container_ptr = nullptr;
			size_t m_current_index = 0;
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
											 size_t currentIndex) noexcept
				: m_ptr(ptr), m_container_ptr(containerPtr), m_current_index(currentIndex) {
			}
			constexpr ConstIterator(const ConstIterator& iter) noexcept = default;
			constexpr ConstIterator(ConstIterator&& iter) noexcept = default;
			~ConstIterator() noexcept = default;

			/// @brief Returns the index in the `RingBuffer` that corresponds
			/// to the element this iterator points to
			///
			/// @return The index corresponding with the element this points to
			[[nodiscard]] constexpr inline auto get_index() const noexcept -> size_t {
				return m_current_index;
			}

			constexpr auto
			operator=(const ConstIterator& iter) noexcept -> ConstIterator& = default;
			constexpr auto operator=(ConstIterator&& iter) noexcept -> ConstIterator& = default;

			constexpr inline auto operator==(const ConstIterator& rhs) const noexcept -> bool {
				return m_ptr == rhs.m_ptr;
			}

			constexpr inline auto operator!=(const ConstIterator& rhs) const noexcept -> bool {
				return m_ptr != rhs.m_ptr;
			}

			constexpr inline auto operator*() const noexcept -> reference {
				return *m_ptr;
			}

			constexpr inline auto operator->() const noexcept -> pointer {
				return m_ptr;
			}

			constexpr inline auto operator++() noexcept -> ConstIterator& {
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

			constexpr inline auto operator++(int) noexcept -> ConstIterator {
				ConstIterator temp = *this;
				++(*this);
				return temp;
			}

			constexpr inline auto operator--() noexcept -> ConstIterator& {
				if(m_current_index == 0) {
					return *this;
				}
				else {
					m_current_index--;
					m_ptr = &(*m_container_ptr)[m_current_index];
				}
				return *this;
			}

			constexpr inline auto operator--(int) noexcept -> ConstIterator {
				ConstIterator temp = *this;
				--(*this);
				return temp;
			}

			constexpr inline auto operator+(Integral auto rhs) const noexcept -> ConstIterator {
				const auto diff = static_cast<size_t>(rhs);
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

			constexpr inline auto operator+=(Integral auto rhs) noexcept -> ConstIterator& {
				*this = std::move(*this + rhs);
				return *this;
			}

			constexpr inline auto operator-(Integral auto rhs) const noexcept -> ConstIterator {
				const auto diff = static_cast<size_t>(rhs);
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

			constexpr inline auto operator-=(Integral auto rhs) noexcept -> ConstIterator& {
				*this = std::move(*this - rhs);
				return *this;
			}

			constexpr inline auto
			operator-(const ConstIterator& rhs) const noexcept -> difference_type {
				return static_cast<std::ptrdiff_t>(m_current_index)
					   - static_cast<std::ptrdiff_t>(rhs.m_current_index);
			}

			constexpr inline auto operator[](Integral auto index) const noexcept -> ConstIterator {
				return std::move(*this + index);
			}

			constexpr inline auto operator>(const ConstIterator& rhs) const noexcept -> bool {
				return m_current_index > rhs.m_current_index;
			}

			constexpr inline auto operator<(const ConstIterator& rhs) const noexcept -> bool {
				return m_current_index < rhs.m_current_index;
			}

			constexpr inline auto operator>=(const ConstIterator& rhs) const noexcept -> bool {
				return m_current_index >= rhs.m_current_index;
			}

			constexpr inline auto operator<=(const ConstIterator& rhs) const noexcept -> bool {
				return m_current_index <= rhs.m_current_index;
			}

		  private:
			pointer m_ptr;
			RingBuffer* m_container_ptr = nullptr;
			size_t m_current_index = 0;
		};

		/// @brief Creates a `RingBuffer` with default capacity
		constexpr RingBuffer() noexcept = default;

		/// @brief Creates a `RingBuffer` with (at least) the given initial capacity
		///
		/// @param intitial_capacity - The initial capacity of the `RingBuffer`
		constexpr explicit RingBuffer(size_t intitial_capacity) noexcept
			: m_buffer(allocate_unique<T[]>(m_allocator, intitial_capacity + 1)), // NOLINT
			  m_loop_index(intitial_capacity), m_capacity(intitial_capacity + 1) {
		}

		/// @brief Constructs a new `RingBuffer` with the given initial capacity and
		/// fills it with `default_value`
		///
		/// @param intitial_capacity - The initial capacity of the `RingBuffer`
		/// @param default_value - The value to fill the `RingBuffer` with
		constexpr RingBuffer(size_t intitial_capacity,
							 const T& default_value) noexcept requires Copyable<T>
			: m_buffer(allocate_unique<T[]>(m_allocator, // NOLINT
											intitial_capacity + 1,
											default_value)),
			  m_write_index(intitial_capacity),
			  m_start_index(0ULL), // NOLINT
			  m_loop_index(intitial_capacity),
			  m_capacity(intitial_capacity + 1) {
		}

		constexpr RingBuffer(const RingBuffer& buffer) noexcept requires Copyable<T>
			: m_buffer(allocate_unique<T[]>(m_allocator, buffer.m_capacity)), // NOLINT
			  m_write_index(0ULL),											  // NOLINT
			  m_start_index(0ULL),											  // NOLINT
			  m_loop_index(buffer.m_loop_index),
			  m_capacity(buffer.m_capacity) {
			const auto size = m_buffer.size();
			for(auto i = 0ULL; i < size; ++i) {
				push_back(buffer.m_buffer[i]);
			}
			// clang-format off
			m_start_index = buffer.m_start_index; // NOLINT(cppcoreguidelines-prefer-member-initializer)
			m_write_index = buffer.m_write_index; // NOLINT(cppcoreguidelines-prefer-member-initializer)
			// clang-format on
		}

		constexpr RingBuffer(RingBuffer&& buffer) noexcept
			: m_allocator(buffer.m_allocator), m_buffer(std::move(buffer.m_buffer)),
			  m_write_index(buffer.m_write_index), m_start_index(buffer.m_start_index),
			  m_loop_index(buffer.m_loop_index), m_capacity(buffer.m_capacity) {
			buffer.m_capacity = 0ULL;
			buffer.m_loop_index = 0ULL;
			buffer.m_write_index = 0ULL;
			buffer.m_start_index = 0ULL;
			buffer.m_buffer = nullptr;
		}

		~RingBuffer() noexcept = default;

		/// @brief Returns the element at the given index.
		/// @note This is not checked in the same manner as STL containers:
		/// if index >= capacity, the element at capacity - 1 is returned.
		///
		/// @param index - The index of the desired element
		///
		/// @return The element at the given index, or at capacity - 1 if index >= capacity
		[[nodiscard]] constexpr inline auto at(Integral auto index) noexcept -> T& {
			auto i = get_adjusted_internal_index(index);

			if(i > m_loop_index) {
				i = m_loop_index;
			}

			return m_buffer[i]; // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
		}

		/// @brief Returns the first element in the `RingBuffer`
		///
		/// @return The first element
		[[nodiscard]] constexpr inline auto front() noexcept -> T& {
			return m_buffer
				[m_start_index]; // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
		}

		/// @brief Returns the last element in the `RingBuffer`
		/// @note If <= 1 elements are in the `RingBuffer`, this will be the same as `front`
		///
		/// @return The last element
		[[nodiscard]] constexpr inline auto back() noexcept -> T& {
			const auto index = (m_start_index + size() - 1) % (m_capacity);

			return m_buffer[index]; // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
		}

		/// @brief Returns a pointer to the underlying data in the `RingBuffer`.
		/// @note This is not sorted in any way to match the representation used by the `RingBuffer`
		///
		/// @return A pointer to the underlying data
		[[nodiscard]] constexpr inline auto data() noexcept -> T* {
			return m_buffer;
		}

		/// @brief Returns whether the `RingBuffer` is empty
		///
		/// @return `true` if the `RingBuffer` is empty, `false` otherwise
		[[nodiscard]] constexpr inline auto empty() const noexcept -> bool {
			return m_write_index == m_start_index;
		}

		/// @brief Returns whether the `RingBuffer` is full
		///
		/// @return `true` if the `RingBuffer` is full, `false` otherwise
		[[nodiscard]] constexpr inline auto full() const noexcept -> bool {
			return size() == m_capacity - 1;
		}

		/// @brief Returns the current number of elements in the `RingBuffer`
		///
		/// @return The current number of elements
		[[nodiscard]] constexpr inline auto size() const noexcept -> size_t {
			return m_write_index >= m_start_index ? m_write_index - m_start_index :
													  (m_capacity) - (m_start_index - m_write_index);
		}

		/// @brief Returns the maximum possible number of elements this `RingBuffer` could store
		/// if grown to maximum possible capacity
		///
		/// @return The maximum possible number of storable elements
		[[nodiscard]] constexpr inline auto max_size() const noexcept -> size_t {
			return allocator_traits::max_size(m_allocator);
		}

		/// @brief Returns the current capacity of the `RingBuffer`;
		/// the number of elements it can currently store
		///
		/// @return The current capacity
		[[nodiscard]] constexpr inline auto capacity() const noexcept -> size_t {
			return m_capacity - 1;
		}

		/// @brief Reserves more storage for the `RingBuffer`. If `new_capacity` is > capacity,
		/// then the capacity of the `RingBuffer` will be extended until at least `new_capacity`
		/// elements can be stored.
		/// @note Memory contiguity is maintained, so no **elements** will be lost or invalidated.
		/// However, all iterators and references to elements will be invalidated.
		///
		/// @param new_capacity - The new capacity of the `RingBuffer`
		constexpr inline auto reserve(size_t new_capacity) noexcept -> void {
			// we only need to do anything if `new_capacity` is actually larger than `m_capacity`
			if(new_capacity > m_capacity) {
				auto temp
					= allocate_unique<T[], Allocator<T[]>>(m_allocator, new_capacity + 1); // NOLINT
				auto span = gsl::make_span(&temp[0], new_capacity + 1);
				std::copy(begin(), end(), span.begin());
				m_buffer = std::move(temp);
				m_start_index = 0;
				m_write_index = m_loop_index + 1;
				m_loop_index = new_capacity;
				m_capacity = new_capacity + 1;
			}
		}

		/// @brief Erases all elements from the `RingBuffer`
		constexpr inline auto clear() noexcept -> void {
			m_start_index = 0;
			m_write_index = 0;
		}

		/// @brief Inserts the given element at the end of the `RingBuffer`
		/// @note if `size() == capacity()` then this loops and overwrites `front()`
		///
		/// @param value - the element to insert
		constexpr inline auto push_back(const T& value) noexcept -> void requires Copyable<T> {
			// clang-format off
			m_buffer[m_write_index] = value; // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
			// clang-format on

			increment_indices();
		}

		/// @brief Inserts the given element at the end of the `RingBuffer`
		/// @note if `size() == capacity()` then this loops and overwrites `front()`
		///
		/// @param value - the element to insert
		constexpr inline auto push_back(T&& value) noexcept -> void {
			// clang-format off
			m_buffer[m_write_index] = std::forward<T>(value); // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
			// clang-format on

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
		requires ConstructibleFrom<T, Args...>
		constexpr inline auto emplace_back(Args&&... args) noexcept -> T& {
			allocator_traits::template construct<T>(m_allocator,
													&m_buffer[m_write_index],
													std::forward<Args>(args)...); // NOLINT

			increment_indices();

			auto index = m_write_index == 0ULL ? m_loop_index : m_write_index - 1;
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
		requires ConstructibleFrom<T, Args...>
		constexpr inline auto emplace(const Iterator& position, Args&&... args) noexcept -> T& {
			auto index = get_adjusted_internal_index(position.get_index());

			allocator_traits::template construct<T>(m_allocator,
													&m_buffer[index],
													std::forward<Args>(args)...); // NOLINT

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
		constexpr inline auto
		emplace(const ConstIterator& position, Args&&... args) noexcept -> T& {
			auto index = get_adjusted_internal_index(position.get_index());

			allocator_traits::template construct<T>(m_allocator,
													&m_buffer[index],
													std::forward<Args>(args)...); // NOLINT

			return m_buffer[index]; // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
		}

		/// @brief Assigns the given element to the position indicated
		/// by the `Iterator` `position`
		/// @note if `size() == capacity()` this drops the last element out of the `RingBuffer`
		///
		/// @param position - `Iterator` indicating where in the `RingBuffer` to place the element
		/// @param element - The element to store in the `RingBuffer`
		constexpr inline auto
		insert(const Iterator& position, const T& element) noexcept -> void requires Copyable<T> {
			insert_internal(position.get_index(), element);
		}

		/// @brief Assigns the given element to the position indicated
		/// by the `Iterator` `position`
		/// @note if `size() == capacity()` this drops the last element out of the `RingBuffer`
		///
		/// @param position - `Iterator` indicating where in the `RingBuffer` to place the element
		/// @param element - The element to store in the `RingBuffer`
		constexpr inline auto insert(const Iterator& position, T&& element) noexcept -> void {
			insert_internal(position.get_index(), std::forward<T>(element));
		}

		/// @brief Assigns the given element to the position indicated
		/// by the `ConstIterator` `position`
		/// @note if `size() == capacity()` this drops the last element out of the `RingBuffer`
		///
		/// @param position - `ConstIterator` indicating where in the `RingBuffer` to place the
		/// element
		/// @param element - The element to store in the `RingBuffer`
		constexpr inline auto insert(const ConstIterator& position, const T& element) noexcept
			-> void requires Copyable<T> {
			insert_internal(position.get_index(), element);
		}

		/// @brief Assigns the given element to the position indicated
		/// by the `ConstIterator` `position`
		/// @note if `size() == capacity()` this drops the last element out of the `RingBuffer`
		///
		/// @param position - `ConstIterator` indicating where in the `RingBuffer` to place the
		/// element
		/// @param element - The element to store in the `RingBuffer`
		constexpr inline auto insert(const ConstIterator& position, T&& element) noexcept -> void {
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
		requires ConstructibleFrom<T, Args...>
		constexpr inline auto
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
		requires ConstructibleFrom<T, Args...>
		constexpr inline auto
		insert_emplace(const ConstIterator& position, Args&&... args) noexcept -> T& {
			return insert_emplace_internal(position.get_index(), std::forward<Args>(args)...);
		}

		/// @brief Erases the element at the given `position`, moving other elements backward
		/// in the buffer to maintain contiguity
		///
		/// @param position - The element to erase
		///
		/// @return `Iterator` pointing to the element after the one erased
		constexpr inline auto erase(const Iterator& position) noexcept -> Iterator {
			return erase_internal(position.get_index());
		}

		/// @brief Erases the element at the given `position`, moving other elements backward
		/// in the buffer to maintain contiguity
		///
		/// @param position - The element to erase
		///
		/// @return `Iterator` pointing to the element after the one erased
		constexpr inline auto erase(const ConstIterator& position) noexcept -> Iterator {
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
		constexpr inline auto
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
		constexpr inline auto
		erase(const ConstIterator& first, const ConstIterator& last) noexcept -> Iterator {
			if(first >= last) {
				return last;
			}

			return erase_internal(first.get_index(), last.get_index());
		}

		/// @brief Removes the last element in the `RingBuffer` and returns it
		///
		/// @return The last element in the `RingBuffer`
		[[nodiscard]] constexpr inline auto pop_back() noexcept -> T requires Copyable<T> {
			T back_ = back();
			erase(--end());
			return back_;
		}

		/// @brief Removes the first element in the `RingBuffer` and returns it
		///
		/// @return The first element in the `RingBuffer`
		[[nodiscard]] constexpr inline auto pop_front() noexcept -> T requires Copyable<T> {
			T front_ = front();
			increment_indices_from_start();
			return front_;
		}

		/// @brief Returns a Random Access Bidirectional iterator over the `RingBuffer`,
		/// at the beginning
		///
		/// @return The iterator, at the beginning
		[[nodiscard]] constexpr inline auto begin() -> Iterator {
			T* p = &m_buffer
					   [m_start_index]; // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)

			return Iterator(p, this, 0ULL);
		}

		/// @brief Returns a Random Access Bidirectional iterator over the `RingBuffer`,
		/// at the end
		///
		/// @return The iterator, at the end
		[[nodiscard]] constexpr inline auto end() -> Iterator {
			T* p = &m_buffer
					   [m_write_index]; // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)

			return Iterator(p, this, size());
		}

		/// @brief Returns a Random Access Bidirectional read-only iterator over the `RingBuffer`,
		/// at the beginning
		///
		/// @return The iterator, at the beginning
		[[nodiscard]] constexpr inline auto cbegin() -> ConstIterator {
			T* p = &m_buffer
					   [m_start_index]; // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)

			return ConstIterator(p, this, 0ULL);
		}

		/// @brief Returns a Random Access Bidirectional read-only iterator over the `RingBuffer`,
		/// at the end
		///
		/// @return The iterator, at the end
		[[nodiscard]] constexpr inline auto cend() -> ConstIterator {
			T* p = &m_buffer
					   [m_write_index]; // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
			return ConstIterator(p, this, size());
		}

		/// @brief Unchecked access-by-index operator
		///
		/// @param index - The index to get the corresponding element for
		///
		/// @return - The element at index
		[[nodiscard]] constexpr inline auto operator[](Integral auto index) noexcept -> T& {
			auto i = get_adjusted_internal_index(index);

			return m_buffer[i]; // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
		}

		constexpr auto
		operator=(const RingBuffer& buffer) noexcept -> RingBuffer& requires Copyable<T> {
			if(this == &buffer) {
				return *this;
			}
			auto temp = allocate_unique<T[]>(m_allocator, buffer.m_capacity); // NOLINT
			const auto size = buffer.size();
			for(auto i = 0ULL; i < size; ++i) {
				temp[i] = buffer.m_buffer[i];
			}
			m_buffer = std::move(temp);
			m_capacity = buffer.m_capacity;
			m_start_index = buffer.m_start_index;
			m_write_index = buffer.m_write_index;
			m_loop_index = buffer.m_loop_index;
			return *this;
		}
		constexpr auto operator=(RingBuffer&& buffer) noexcept -> RingBuffer& {
			m_allocator = buffer.m_allocator;
			m_buffer = std::move(buffer.m_buffer);
			m_write_index = buffer.m_write_index;
			m_start_index = buffer.m_start_index;
			m_loop_index = buffer.m_loop_index;
			m_capacity = buffer.m_capacity;
			buffer.m_buffer = nullptr;
			buffer.m_write_index = 0ULL;
			buffer.m_start_index = 0ULL;
			buffer.m_capacity = 0ULL;
			return *this;
		}

	  private:
		static const constexpr size_t DEFAULT_CAPACITY_INTERNAL = DEFAULT_CAPACITY + 1;
		Allocator<T> m_allocator = Allocator<T>();
		unique_pointer m_buffer
			= allocate_unique<T[]>(m_allocator, DEFAULT_CAPACITY_INTERNAL); // NOLINT
		size_t m_write_index = 0ULL;
		size_t m_start_index = 0ULL;
		size_t m_loop_index = DEFAULT_CAPACITY;
		size_t m_capacity = DEFAULT_CAPACITY_INTERNAL;

		/// @brief Converts the given `RingBuffer` index into the corresponding index into then
		/// underlying `T` array
		///
		/// @param index - The `RingBuffer` index to convert
		///
		/// @return The corresponding index into the underlying `T` array
		[[nodiscard]] constexpr inline auto
		get_adjusted_internal_index(Integral auto index) const noexcept -> size_t {
			auto i = static_cast<size_t>(index);
			return (m_start_index + i) % (m_capacity);
		}

		/// @brief Converts the given index into the underlying `T` array into
		/// a using facing index into the `RingBuffer`
		///
		/// @param index - The internal index
		///
		/// @return The corresponding user-facing index
		[[nodiscard]] constexpr inline auto
		get_external_index_from_internal(Integral auto index) const noexcept -> size_t {
			auto i = static_cast<size_t>(index);
			if(i >= m_start_index && i <= m_loop_index) {
				return i - m_start_index;
			}
			else if(i < m_start_index) {
				return m_capacity - (m_start_index - i);
			}
			else {
				return m_capacity - 1;
			}
		}

		/// @brief Used to increment the start and write indices into the underlying `T` array,
		/// and the size property, after pushing an element at the back,
		/// maintaining the logical `RingBuffer` structure
		constexpr inline auto increment_indices() noexcept -> void {
			m_write_index = (m_write_index + 1) % (m_capacity);

			// if write index is at start, we need to push start forward to maintain
			// the "invalid" spacer element for this.end()
			if(m_write_index == m_start_index) {
				m_start_index = (m_start_index + 1) % (m_capacity);
			}
		}

		/// @brief Used to increment the start index into the underlying `T` array
		/// and the size property after popping an element from the front,
		/// maintaining the logical `RingBuffer` structure
		constexpr inline auto increment_indices_from_start() noexcept -> void {
			if(m_start_index != m_write_index) {
				m_start_index = (m_start_index + 1) % (m_capacity);
			}
		}

		/// @brief Inserts the given element at the position indicated
		/// by the `external_index`
		/// @note if `size() == capacity()` this drops the last element out of the `RingBuffer`
		///
		/// @param external_index - The user-facing index into the `RingBuffer` to insert the
		/// element at
		/// @param elem - The element to store in the `RingBuffer`
		constexpr inline auto
		insert_internal(size_t external_index, const T& elem) noexcept -> void requires Movable<T> {
			const auto index = get_adjusted_internal_index(external_index);

			if(index == m_write_index) {
				emplace_back(elem);
			}
			else {
				auto num_to_move = size() - external_index;
				auto iter = begin() + external_index;
				if(full()) [[likely]] { // NOLINT
					for(size_t i = 1ULL, j = num_to_move - 2; i < num_to_move; ++i, --j) {
						*((end() - i) + 1) = std::move(*(iter + j));
					}
				}
				else {
					for(size_t i = 0ULL, j = num_to_move - 1; i < num_to_move; ++i, --j) {
						*(end() - i) = std::move(*(iter + j));
					}
				}
				increment_indices();
				if(external_index == 0) {
					// clang-format off
					m_buffer[m_start_index] = elem; // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
					// clang-format on
				}
				else {
					// clang-format off
					m_buffer[index] = elem; // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
					// clang-format on
				}
			}
		}

		/// @brief Inserts the given element at the position indicated
		/// by the `external_index`
		/// @note if `size() == capacity()` this drops the last element out of the `RingBuffer`
		///
		/// @param external_index - The user-facing index into the `RingBuffer` to insert the
		/// element at
		/// @param elem - The element to store in the `RingBuffer`
		constexpr inline auto insert_internal(size_t external_index, const T& elem) noexcept
			-> void requires NotMovable<T> {
			const auto index = get_adjusted_internal_index(external_index);

			if(index == m_write_index) {
				emplace_back(elem);
			}
			else {
				auto num_to_move = size() - external_index;
				auto iter = begin() + external_index;
				if(full()) [[likely]] { // NOLINT
					for(size_t i = 1ULL, j = num_to_move - 2; i < num_to_move; ++i, --j) {
						*((end() - i) + 1) = *(iter + j);
					}
				}
				else {
					for(size_t i = 0ULL, j = num_to_move - 1; i < num_to_move; ++i, --j) {
						*(end() - i) = *(iter + j);
					}
				}
				increment_indices();
				if(external_index == 0) {
					// clang-format off
					m_buffer[m_start_index] = elem; // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
					// clang-format on
				}
				else {
					// clang-format off
					m_buffer[index] = elem; // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
					// clang-format on
				}
			}
		}

		/// @brief Inserts the given element at the position indicated
		/// by the `external_index`
		/// @note if `size() == capacity()` this drops the last element out of the `RingBuffer`
		///
		/// @param external_index - The user-facing index into the `RingBuffer` to insert the
		/// element at
		/// @param elem - The element to store in the `RingBuffer`
		constexpr inline auto
		insert_internal(size_t external_index, T&& elem) noexcept -> void requires Movable<T> {
			const auto index = get_adjusted_internal_index(external_index);

			if(index == m_write_index) {
				emplace_back(elem);
			}
			else {
				auto num_to_move = size() - external_index;
				auto iter = begin() + external_index;
				if(full()) [[likely]] { // NOLINT
					for(size_t i = 1ULL, j = num_to_move - 2; i < num_to_move; ++i, --j) {
						*((end() - i) + 1) = std::move(*(iter + j));
					}
				}
				else {
					for(size_t i = 0ULL, j = num_to_move - 1; i < num_to_move; ++i, --j) {
						*(end() - i) = std::move(*(iter + j));
					}
				}
				increment_indices();
				if(external_index == 0) {
					// clang-format off
					m_buffer[m_start_index] = std::move(elem); // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
					// clang-format on
				}
				else {
					// clang-format off
					m_buffer[index] = std::move(elem); // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
					// clang-format on
				}
			}
		}

		/// @brief Inserts the given element at the position indicated
		/// by the `external_index`
		/// @note if `size() == capacity()` this drops the last element out of the `RingBuffer`
		///
		/// @param external_index - The user-facing index into the `RingBuffer` to insert the
		/// element at
		/// @param elem - The element to store in the `RingBuffer`
		constexpr inline auto
		insert_internal(size_t external_index, T&& elem) noexcept -> void requires NotMovable<T> {
			const auto index = get_adjusted_internal_index(external_index);

			if(index == m_write_index) {
				emplace_back(elem);
			}
			else {
				auto num_to_move = size() - external_index;
				auto iter = begin() + external_index;
				if(full()) [[likely]] { // NOLINT
					for(size_t i = 1ULL, j = num_to_move - 2; i < num_to_move; ++i, --j) {
						*((end() - i) + 1) = *(iter + j);
					}
				}
				else {
					for(size_t i = 0ULL, j = num_to_move - 1; i < num_to_move; ++i, --j) {
						*(end() - i) = *(iter + j);
					}
				}
				increment_indices();
				if(external_index == 0) {
					// clang-format off
					m_buffer[m_start_index] = elem; // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
					// clang-format on
				}
				else {
					// clang-format off
					m_buffer[index] = elem; // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
					// clang-format on
				}
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
		requires ConstructibleFrom<T, Args...>
		constexpr inline auto
		insert_emplace_internal(size_t external_index, Args&&... args) noexcept
			-> T& requires Movable<T> {
			const auto index = get_adjusted_internal_index(external_index);

			if(index == m_write_index) {
				return emplace_back(std::forward<Args>(args)...);
			}
			else {
				auto num_to_move = size() - external_index;
				auto iter = begin() + external_index;
				if(full()) [[likely]] { // NOLINT
					for(size_t i = 1ULL, j = num_to_move - 2; i < num_to_move; ++i, --j) {
						*((end() - i) + 1) = std::move(*(iter + j));
					}
				}
				else {
					for(size_t i = 0ULL, j = num_to_move - 1; i < num_to_move; ++i, --j) {
						*(end() - i) = std::move(*(iter + j));
					}
				}
				increment_indices();
				if(external_index == 0) {
					// clang-format off
					allocator_traits::template construct<T>(m_allocator, &m_buffer[m_start_index], std::forward<Args>(args)...); // NOLINT
					return m_buffer[m_start_index];// NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
					// clang-format on
				}
				else {
					// clang-format off
					allocator_traits::template construct<T>(m_allocator, &m_buffer[index], std::forward<Args>(args)...); // NOLINT
					return m_buffer[index];// NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
					// clang-format on
				}
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
		requires ConstructibleFrom<T, Args...>
		constexpr inline auto
		insert_emplace_internal(size_t external_index, Args&&... args) noexcept
			-> T& requires NotMovable<T> {
			const auto index = get_adjusted_internal_index(external_index);

			if(index == m_write_index) {
				return emplace_back(std::forward<Args>(args)...);
			}
			else {
				auto num_to_move = size() - external_index;
				auto iter = begin() + external_index;
				if(full()) [[likely]] { // NOLINT
					for(size_t i = 1ULL, j = num_to_move - 2; i < num_to_move; ++i, --j) {
						*((end() - i) + 1) = *(iter + j);
					}
				}
				else {
					for(size_t i = 0ULL, j = num_to_move - 1; i < num_to_move; ++i, --j) {
						*(end() - i) = *(iter + j);
					}
				}
				increment_indices();
				if(external_index == 0) {
					// clang-format off
					allocator_traits::template construct<T>(m_allocator, &m_buffer[m_start_index], std::forward<Args>(args)...); // NOLINT
					return m_buffer[m_start_index];// NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
					// clang-format on
				}
				else {
					// clang-format off
					allocator_traits::template construct<T>(m_allocator, &m_buffer[index], std::forward<Args>(args)...); // NOLINT
					return m_buffer[index];// NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
					// clang-format on
				}
			}
		}

		/// @brief Erases the element at the given index, returning an `Iterator` to the element
		/// after the removed one
		///
		/// @param index - The index to the element to remove. This should be a `RingBuffer` index:
		/// IE, not an interal one into the `T` array
		///
		/// @return `Iterator` pointing to the element after the one removed
		[[nodiscard]] constexpr inline auto erase_internal(size_t index) noexcept -> Iterator {
			auto index_internal = get_adjusted_internal_index(index);
			if(index_internal == m_write_index) [[unlikely]] { // NOLINT
				return end();
			}
			else {
				auto num_to_move = (size() - 1) - index;
				m_write_index = index_internal;
				auto pos_to_move = index + 1;
				for(auto i = 0ULL; i < num_to_move; ++i) {
					// clang-format off
					emplace_back(m_buffer[get_adjusted_internal_index(pos_to_move + i)]); // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
					// clang-format on
				}

				auto iter = begin() + get_external_index_from_internal(m_write_index);
				return iter;
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
		[[nodiscard]] constexpr inline auto
		erase_internal(size_t first, size_t last) noexcept -> Iterator {
			auto first_internal = get_adjusted_internal_index(first);
			auto last_internal = get_adjusted_internal_index(last);
			if(last_internal == m_write_index) {
				if(m_write_index == m_loop_index) {
					// m_start_index = m_loop_index;
					m_write_index -= (last - first);
				}
				else if(m_write_index < m_start_index) {
					auto num_to_remove = (last - first);
					auto num_after_start_index
						= (m_write_index > num_to_remove) ? m_write_index - num_to_remove : 0ULL;
					auto num_before_start_index = num_to_remove - num_after_start_index;
					if(num_after_start_index > 0) {
						m_write_index = m_loop_index;
						num_after_start_index--;
						m_write_index -= num_after_start_index;
					}
					else {
						m_write_index -= num_before_start_index;
					}
				}
				else if(m_write_index > m_start_index) {
					auto num_to_remove = (last - first);
					m_write_index -= num_to_remove;
				}
				return end();
			}
			else {
				auto num_to_move = size() - last;
				m_write_index = first_internal;
				auto pos_to_move = last;
				for(auto i = 0ULL; i < num_to_move; ++i) {
					// clang-format off
					emplace_back(m_buffer[get_adjusted_internal_index(pos_to_move + i)]); // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
					// clang-format on
				}

				auto iter = begin() + first;
				return iter;
			}
		}
	};

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
	template<DefaultConstructible T, template<typename ElementType> typename Allocator>
	class RingBuffer<T, RingBufferType::ThreadSafe, Allocator> {
	  public:
		/// Default capacity of `RingBuffer`
		static const constexpr size_t DEFAULT_CAPACITY = 16;

		struct Element {
			std::shared_ptr<T> m_element = nullptr;

			constexpr Element() noexcept = default;
			explicit constexpr Element(const std::shared_ptr<T>& element) noexcept
				: m_element(element) {
			}
			explicit constexpr Element(std::shared_ptr<T>&& element) noexcept
				: m_element(std::move(element)) {
			}
			explicit constexpr Element(T* element) noexcept : m_element(element) {
			}
			constexpr Element(Allocator<Element>& alloc, const T& element) noexcept
				: m_element(std::allocate_shared<T, Allocator<Element>>(alloc, element)) {
			}
			constexpr Element(Allocator<Element>& alloc, T&& element) noexcept
				: m_element(std::allocate_shared<T, Allocator<Element>>(alloc, element)) {
			}
			template<typename... Args>
			requires ConstructibleFrom<T, Args...>
			explicit constexpr Element(Allocator<Element>& alloc, Args&&... args) noexcept
				: m_element(
					std::allocate_shared<T, Allocator<Element>>(alloc,
																std::forward<Args>(args)...)) {
			}
			constexpr Element(const Element& element) noexcept = default;
			constexpr Element(Element&& element) noexcept = default;

			inline constexpr auto operator=(const Element& element) noexcept -> Element& = default;
			inline constexpr auto operator=(Element&& element) noexcept -> Element& = default;
			inline constexpr auto
			operator=(const std::shared_ptr<T>& element) noexcept -> Element& {
				m_element = element;
				return *this;
			}
			inline constexpr auto operator=(std::shared_ptr<T>&& element) noexcept -> Element& {
				m_element = std::move(element);
				return *this;
			}
			// inline constexpr auto operator=(const T& element) noexcept -> Element& {
			//	m_element = std::make_shared<T>(element);
			//	return *this;
			// }
			// inline constexpr auto operator=(T&& element) noexcept -> Element& {
			//	m_element = std::make_shared<T>(element);
			//	return *this;
			// }

			inline constexpr auto operator==(const Element& element) const noexcept -> bool {
				return *m_element == *(element.m_element);
			}

			inline constexpr auto operator!=(const Element& element) const noexcept -> bool {
				return *(m_element) != *(element.m_element);
			}

			// friend inline constexpr auto
			// operator==(const Element& lhs, const T& rhs) noexcept -> bool {
			//	return (*lhs.m_element) == rhs;
			// }

			// friend inline constexpr auto
			// operator!=(const Element& lhs, const T& rhs) noexcept -> bool {
			//	return (*lhs.m_element) != rhs;
			// }

			inline constexpr operator T&() noexcept { // NOLINT
				return *m_element;
			}
			inline constexpr operator const T&() const noexcept { // NOLINT
				return *m_element;
			}
			inline constexpr auto operator*() noexcept -> T& {
				return *m_element;
			}
			inline constexpr auto operator*() const noexcept -> const T& {
				return *m_element;
			}
			inline constexpr auto operator->() noexcept -> T* {
				return m_element.get();
			}
			inline constexpr auto operator->() const noexcept -> const T* {
				return m_element.get();
			}
		};
		using allocator_traits = std::allocator_traits<Allocator<Element>>;
		using unique_pointer
			= decltype(allocate_unique<Element[]>(std::declval<Allocator<Element[]>>(), // NOLINT
												  DEFAULT_CAPACITY));

		/// @brief Random-Access Bidirectional iterator for `RingBuffer`
		/// @note All navigation operators are checked such that any movement past `begin()` or
		/// `end()` is ignored.
		class Iterator {
		  public:
			using iterator_category = std::random_access_iterator_tag;
			using difference_type = std::ptrdiff_t;
			using value_type = Element;
			using pointer = Element;
			using reference = Element&;

			constexpr explicit Iterator(pointer ptr,
										RingBuffer* containerPtr,
										size_t currentIndex) noexcept
				: m_ptr(ptr), m_container_ptr(containerPtr), m_current_index(currentIndex) {
			}
			constexpr Iterator(const Iterator& iter) noexcept = default;
			constexpr Iterator(Iterator&& iter) noexcept = default;
			~Iterator() noexcept = default;

			/// @brief Returns the index in the `RingBuffer` that corresponds
			/// to the element this iterator points to
			///
			/// @return The index corresponding with the element this points to
			[[nodiscard]] constexpr inline auto get_index() const noexcept -> size_t {
				return m_current_index;
			}

			constexpr auto operator=(const Iterator& iter) noexcept -> Iterator& = default;
			constexpr auto operator=(Iterator&& iter) noexcept -> Iterator& = default;

			constexpr inline auto operator==(const Iterator& rhs) const noexcept -> bool {
				return m_ptr == rhs.m_ptr;
			}

			constexpr inline auto operator!=(const Iterator& rhs) const noexcept -> bool {
				return m_ptr != rhs.m_ptr;
			}

			constexpr inline auto operator*() noexcept -> reference {
				return m_ptr;
			}

			constexpr inline auto operator->() noexcept -> pointer {
				return m_ptr;
			}

			constexpr inline auto operator++() noexcept -> Iterator& {
				m_current_index++;
				if(m_current_index >= m_container_ptr->capacity()) {
					m_current_index = m_container_ptr->capacity();
					m_ptr = m_container_ptr->end().m_ptr;
				}
				else {
					m_ptr = (*m_container_ptr)[m_current_index];
				}
				return *this;
			}

			constexpr inline auto operator++(int) noexcept -> Iterator {
				Iterator temp = *this;
				++(*this);
				return temp;
			}

			constexpr inline auto operator--() noexcept -> Iterator& {
				if(m_current_index == 0) {
					return *this;
				}
				else {
					m_current_index--;
					m_ptr = (*m_container_ptr)[m_current_index];
				}
				return *this;
			}

			constexpr inline auto operator--(int) noexcept -> Iterator {
				Iterator temp = *this;
				--(*this);
				return temp;
			}

			constexpr inline auto operator+(Integral auto rhs) const noexcept -> Iterator {
				const auto diff = static_cast<size_t>(rhs);
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
					temp.m_ptr = (*temp.m_container_ptr)[temp.m_current_index];
				}
				return temp;
			}

			constexpr inline auto operator+=(Integral auto rhs) noexcept -> Iterator& {
				*this = std::move(*this + rhs);
				return *this;
			}

			constexpr inline auto operator-(Integral auto rhs) const noexcept -> Iterator {
				const auto diff = static_cast<size_t>(rhs);
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
					temp.m_ptr = (*temp.m_container_ptr)[temp.m_current_index];
				}
				return temp;
			}

			constexpr inline auto operator-=(Integral auto rhs) noexcept -> Iterator& {
				*this = std::move(*this - rhs);
				return *this;
			}

			constexpr inline auto operator-(const Iterator& rhs) const noexcept -> difference_type {
				return static_cast<std::ptrdiff_t>(m_current_index)
					   - static_cast<std::ptrdiff_t>(rhs.m_current_index);
			}

			constexpr inline auto operator[](Integral auto index) noexcept -> Iterator {
				return std::move(*this + index);
			}

			constexpr inline auto operator>(const Iterator& rhs) const noexcept -> bool {
				return m_current_index > rhs.m_current_index;
			}

			constexpr inline auto operator<(const Iterator& rhs) const noexcept -> bool {
				return m_current_index < rhs.m_current_index;
			}

			constexpr inline auto operator>=(const Iterator& rhs) const noexcept -> bool {
				return m_current_index >= rhs.m_current_index;
			}

			constexpr inline auto operator<=(const Iterator& rhs) const noexcept -> bool {
				return m_current_index <= rhs.m_current_index;
			}

		  private:
			pointer m_ptr;
			RingBuffer* m_container_ptr = nullptr;
			size_t m_current_index = 0;
		};

		/// @brief Read-only Random-Access Bidirectional iterator for `RingBuffer`
		/// @note All navigation operators are checked such that any movement past `begin()` or
		/// `end()` is ignored.
		class ConstIterator {
		  public:
			using iterator_category = std::random_access_iterator_tag;
			using difference_type = std::ptrdiff_t;
			using value_type = Element;
			using pointer = const Element;
			using reference = const Element&;

			constexpr explicit ConstIterator(pointer ptr,
											 RingBuffer* containerPtr,
											 size_t currentIndex) noexcept
				: m_ptr(ptr), m_container_ptr(containerPtr), m_current_index(currentIndex) {
			}
			constexpr ConstIterator(const ConstIterator& iter) noexcept = default;
			constexpr ConstIterator(ConstIterator&& iter) noexcept = default;
			~ConstIterator() noexcept = default;

			/// @brief Returns the index in the `RingBuffer` that corresponds
			/// to the element this iterator points to
			///
			/// @return The index corresponding with the element this points to
			[[nodiscard]] constexpr inline auto get_index() const noexcept -> size_t {
				return m_current_index;
			}

			constexpr auto
			operator=(const ConstIterator& iter) noexcept -> ConstIterator& = default;
			constexpr auto operator=(ConstIterator&& iter) noexcept -> ConstIterator& = default;

			constexpr inline auto operator==(const ConstIterator& rhs) const noexcept -> bool {
				return m_ptr == rhs.m_ptr;
			}

			constexpr inline auto operator!=(const ConstIterator& rhs) const noexcept -> bool {
				return m_ptr != rhs.m_ptr;
			}

			constexpr inline auto operator*() const noexcept -> reference {
				return m_ptr;
			}

			constexpr inline auto operator->() const noexcept -> pointer {
				return m_ptr;
			}

			constexpr inline auto operator++() noexcept -> ConstIterator& {
				m_current_index++;
				if(m_current_index >= m_container_ptr->capacity()) {
					m_current_index = m_container_ptr->capacity();
					m_ptr = m_container_ptr->end().m_ptr;
				}
				else {
					m_ptr = (*m_container_ptr)[m_current_index];
				}
				return *this;
			}

			constexpr inline auto operator++(int) noexcept -> ConstIterator {
				ConstIterator temp = *this;
				++(*this);
				return temp;
			}

			constexpr inline auto operator--() noexcept -> ConstIterator& {
				if(m_current_index == 0) {
					return *this;
				}
				else {
					m_current_index--;
					m_ptr = (*m_container_ptr)[m_current_index];
				}
				return *this;
			}

			constexpr inline auto operator--(int) noexcept -> ConstIterator {
				ConstIterator temp = *this;
				--(*this);
				return temp;
			}

			constexpr inline auto operator+(Integral auto rhs) const noexcept -> ConstIterator {
				const auto diff = static_cast<size_t>(rhs);
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
					temp.m_ptr = (*temp.m_container_ptr)[temp.m_current_index];
				}
				return temp;
			}

			constexpr inline auto operator+=(Integral auto rhs) noexcept -> ConstIterator& {
				*this = std::move(*this + rhs);
				return *this;
			}

			constexpr inline auto operator-(Integral auto rhs) const noexcept -> ConstIterator {
				const auto diff = static_cast<size_t>(rhs);
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
					temp.m_ptr = (*temp.m_container_ptr)[temp.m_current_index];
				}
				return temp;
			}

			constexpr inline auto operator-=(Integral auto rhs) noexcept -> ConstIterator& {
				*this = std::move(*this - rhs);
				return *this;
			}

			constexpr inline auto
			operator-(const ConstIterator& rhs) const noexcept -> difference_type {
				return static_cast<std::ptrdiff_t>(m_current_index)
					   - static_cast<std::ptrdiff_t>(rhs.m_current_index);
			}

			constexpr inline auto operator[](Integral auto index) const noexcept -> ConstIterator {
				return std::move(*this + index);
			}

			constexpr inline auto operator>(const ConstIterator& rhs) const noexcept -> bool {
				return m_current_index > rhs.m_current_index;
			}

			constexpr inline auto operator<(const ConstIterator& rhs) const noexcept -> bool {
				return m_current_index < rhs.m_current_index;
			}

			constexpr inline auto operator>=(const ConstIterator& rhs) const noexcept -> bool {
				return m_current_index >= rhs.m_current_index;
			}

			constexpr inline auto operator<=(const ConstIterator& rhs) const noexcept -> bool {
				return m_current_index <= rhs.m_current_index;
			}

		  private:
			pointer m_ptr;
			RingBuffer* m_container_ptr = nullptr;
			size_t m_current_index = 0;
		};

		/// @brief Creates a `RingBuffer` with default capacity
		constexpr RingBuffer() noexcept = default;

		/// @brief Creates a `RingBuffer` with (at least) the given initial capacity
		///
		/// @param intitial_capacity - The initial capacity of the `RingBuffer`
		constexpr explicit RingBuffer(size_t intitial_capacity) noexcept
			: m_buffer(allocate_unique<Element[]>(m_allocator, // NOLINT
												  intitial_capacity + 1,
												  m_allocator)),
			  m_loop_index(intitial_capacity), m_capacity(intitial_capacity + 1) {
		}

		/// @brief Constructs a new `RingBuffer` with the given initial capacity and
		/// fills it with `default_value`
		///
		/// @param intitial_capacity - The initial capacity of the `RingBuffer`
		/// @param default_value - The value to fill the `RingBuffer` with
		constexpr RingBuffer(size_t intitial_capacity,
							 const T& default_value) noexcept requires Copyable<T>
			: m_buffer(allocate_unique<Element[]>(m_allocator, // NOLINT
												  intitial_capacity + 1,
												  m_allocator,
												  default_value)),
			  m_write_index(intitial_capacity),
			  m_start_index(0ULL), // NOLINT
			  m_loop_index(intitial_capacity),
			  m_capacity(intitial_capacity + 1) {
		}

		constexpr RingBuffer(const RingBuffer& buffer) noexcept requires Copyable<T>
			: m_buffer(allocate_unique<Element[]>(m_allocator, // NOLINT
												  buffer.m_capacity,
												  m_allocator)),
			  m_write_index(0ULL), // NOLINT
			  m_start_index(0ULL), // NOLINT
			  m_loop_index(buffer.m_loop_index),
			  m_capacity(buffer.m_capacity) {
			const auto size = buffer.size();
			for(auto i = 0ULL; i < size; ++i) {
				push_back(buffer.m_buffer[i]);
			}
			m_start_index
				= buffer.m_start_index; // NOLINT(cppcoreguidelines-prefer-member-initializer)
			m_write_index
				= buffer.m_write_index; // NOLINT(cppcoreguidelines-prefer-member-initializer)
		}

		constexpr RingBuffer(RingBuffer&& buffer) noexcept
			: m_allocator(buffer.m_allocator), m_buffer(std::move(buffer.m_buffer)),
			  m_write_index(buffer.m_write_index), m_start_index(buffer.m_start_index),
			  m_loop_index(buffer.m_loop_index), m_capacity(buffer.m_capacity) {
			buffer.m_capacity = 0ULL;
			buffer.m_loop_index = 0ULL;
			buffer.m_write_index = 0ULL;
			buffer.m_start_index = 0ULL;
			buffer.m_buffer = nullptr;
		}

		~RingBuffer() noexcept = default;

		/// @brief Returns the element at the given index.
		/// @note This is not checked in the same manner as STL containers:
		/// if index >= capacity, the element at capacity - 1 is returned.
		///
		/// @param index - The index of the desired element
		///
		/// @return The element at the given index, or at capacity - 1 if index >= capacity
		[[nodiscard]] constexpr inline auto at(Integral auto index) noexcept -> Element {
			const auto start = m_start_index.load();
			auto i = get_adjusted_internal_index(index, start);

			if(i > m_loop_index) {
				i = m_loop_index;
			}

			return m_buffer[i]; // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
		}

		/// @brief Returns the first element in the `RingBuffer`
		///
		/// @return The first element
		[[nodiscard]] constexpr inline auto front() noexcept -> Element {
			return m_buffer[m_start_index.load()];
		}

		/// @brief Returns the last element in the `RingBuffer`
		/// @note If <= 1 elements are in the `RingBuffer`, this will be the same as `front`
		///
		/// @return The last element
		[[nodiscard]] constexpr inline auto back() noexcept -> Element {
			const auto start = m_start_index.load();
			const auto write = m_write_index.load();
			const auto index = get_adjusted_internal_index(size(start, write) - 1, start);

			return m_buffer[index];
		}

		/// @brief Returns a pointer to the underlying data in the `RingBuffer`.
		/// @note This is not sorted in any way to match the representation used by the `RingBuffer`
		///
		/// @return A pointer to the underlying data
		[[nodiscard]] constexpr inline auto data() noexcept -> Element* {
			return m_buffer;
		}

		/// @brief Returns whether the `RingBuffer` is empty
		///
		/// @return `true` if the `RingBuffer` is empty, `false` otherwise
		[[nodiscard]] constexpr inline auto empty() const noexcept -> bool {
			return m_write_index.load() == m_start_index.load();
		}

		/// @brief Returns whether the `RingBuffer` is full
		///
		/// @return `true` if the `RingBuffer` is full, `false` otherwise
		[[nodiscard]] constexpr inline auto full() const noexcept -> bool {
			return size() == m_capacity - 1;
		}

		/// @brief Returns the current number of elements in the `RingBuffer`
		///
		/// @return The current number of elements
		[[nodiscard]] constexpr inline auto size() const noexcept -> size_t {
			const auto write = m_write_index.load();
			const auto start = m_start_index.load();
			return write >= start ? write - start : (m_capacity) - (start - write);
		}

		/// @brief Returns the maximum possible number of elements this `RingBuffer` could store
		/// if grown to maximum possible capacity
		///
		/// @return The maximum possible number of storable elements
		[[nodiscard]] constexpr inline auto max_size() const noexcept -> size_t {
			return allocator_traits::max_size(m_allocator);
		}

		/// @brief Returns the current capacity of the `RingBuffer`;
		/// the number of elements it can currently store
		///
		/// @return The current capacity
		[[nodiscard]] constexpr inline auto capacity() const noexcept -> size_t {
			return m_capacity - 1;
		}

		/// @brief Reserves more storage for the `RingBuffer`. If `new_capacity` is > capacity,
		/// then the capacity of the `RingBuffer` will be extended until at least `new_capacity`
		/// elements can be stored.
		/// @note Memory contiguity is maintained, so no **elements** will be lost or invalidated.
		/// However, all iterators and references to elements will be invalidated.
		///
		/// @param new_capacity - The new capacity of the `RingBuffer`
		constexpr inline auto reserve(size_t new_capacity) noexcept -> void {
			// we only need to do anything if `new_capacity` is actually larger than `m_capacity`
			if(new_capacity > m_capacity - 1) {
				auto temp = allocate_unique<Element[]>(m_allocator, // NOLINT
													   new_capacity + 1,
													   m_allocator);
				auto span = gsl::make_span(&temp[0], new_capacity + 1);
				std::copy(begin(), end(), span.begin());
				m_buffer = std::move(temp);
				m_start_index = 0;
				m_write_index = m_loop_index + 1;
				m_loop_index = new_capacity;
				m_capacity = new_capacity + 1;
			}
		}

		/// @brief Erases all elements from the `RingBuffer`
		constexpr inline auto clear() noexcept -> void {
			m_start_index = 0;
			m_write_index = 0;
		}

		/// @brief Inserts the given element at the end of the `RingBuffer`
		/// @note if `size() == capacity()` then this loops and overwrites `front()`
		///
		/// @param value - the element to insert
		constexpr inline auto push_back(const T& value) noexcept -> void requires Copyable<T> {
			// clang-format off
			m_buffer[m_write_index.load()] = Element(m_allocator, value); // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
			// clang-format on

			increment_indices();
		}

		/// @brief Inserts the given element at the end of the `RingBuffer`
		/// @note if `size() == capacity()` then this loops and overwrites `front()`
		///
		/// @param value - the element to insert
		constexpr inline auto push_back(T&& value) noexcept -> void {
			// clang-format off
			m_buffer[m_write_index.load()] = Element(m_allocator, std::move(value)); // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
			// clang-format on

			increment_indices();
		}

		/// @brief Inserts the given element at the end of the `RingBuffer`
		/// @note if `size() == capacity()` then this loops and overwrites `front()`
		///
		/// @param element - the element to insert
		constexpr inline auto push_back(const Element& element) noexcept -> void {
			// clang-format off
			m_buffer[m_write_index.load()] = element; // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
			// clang-format on

			increment_indices();
		}

		/// @brief Inserts the given element at the end of the `RingBuffer`
		/// @note if `size() == capacity()` then this loops and overwrites `front()`
		///
		/// @param element - the element to insert
		constexpr inline auto push_back(Element&& element) noexcept -> void {
			// clang-format off
			m_buffer[m_write_index.load()] = std::move(element); // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
			// clang-format on

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
		requires ConstructibleFrom<T, Args...>
		constexpr inline auto emplace_back(Args&&... args) noexcept -> Element {
			const auto index = m_write_index.load();

			allocator_traits::template construct<Element>(
				m_allocator,
				&m_buffer[index], // NOLINT
				std::allocate_shared<T, Allocator<Element>>(m_allocator,
															std::forward<Args>(args)...));

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
		requires ConstructibleFrom<T, Args...>
		constexpr inline auto
		emplace(const Iterator& position, Args&&... args) noexcept -> Element {
			const auto start = m_start_index.load();
			auto index = get_adjusted_internal_index(position.get_index(), start);

			allocator_traits::template construct<Element>(
				m_allocator,
				&m_buffer[index], // NOLINT
				std::allocate_shared<T, Allocator<Element>>(m_allocator,
															std::forward<Args>(args)...));

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
		constexpr inline auto
		emplace(const ConstIterator& position, Args&&... args) noexcept -> Element {
			const auto start = m_start_index.load();
			auto index = get_adjusted_internal_index(position.get_index(), start);

			allocator_traits::template construct<Element>(
				m_allocator,
				&m_buffer[index], // NOLINT
				std::allocate_shared<T, Allocator<Element>>(m_allocator,
															std::forward<Args>(args)...));

			return m_buffer[index]; // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
		}

		/// @brief Assigns the given element to the position indicated
		/// by the `Iterator` `position`
		/// @note if `size() == capacity()` this drops the last element out of the `RingBuffer`
		///
		/// @param position - `Iterator` indicating where in the `RingBuffer` to place the element
		/// @param element - The element to store in the `RingBuffer`
		constexpr inline auto
		insert(const Iterator& position, const T& element) noexcept -> void requires Copyable<T> {
			insert_internal(position.get_index(), element);
		}

		/// @brief Assigns the given element to the position indicated
		/// by the `Iterator` `position`
		/// @note if `size() == capacity()` this drops the last element out of the `RingBuffer`
		///
		/// @param position - `Iterator` indicating where in the `RingBuffer` to place the element
		/// @param element - The element to store in the `RingBuffer`
		constexpr inline auto insert(const Iterator& position, T&& element) noexcept -> void {
			insert_internal(position.get_index(), std::forward<T>(element));
		}

		/// @brief Assigns the given element to the position indicated
		/// by the `ConstIterator` `position`
		/// @note if `size() == capacity()` this drops the last element out of the `RingBuffer`
		///
		/// @param position - `ConstIterator` indicating where in the `RingBuffer` to place the
		/// element
		/// @param element - The element to store in the `RingBuffer`
		constexpr inline auto insert(const ConstIterator& position, const T& element) noexcept
			-> void requires Copyable<T> {
			insert_internal(position.get_index(), element);
		}

		/// @brief Assigns the given element to the position indicated
		/// by the `ConstIterator` `position`
		/// @note if `size() == capacity()` this drops the last element out of the `RingBuffer`
		///
		/// @param position - `ConstIterator` indicating where in the `RingBuffer` to place the
		/// element
		/// @param element - The element to store in the `RingBuffer`
		constexpr inline auto insert(const ConstIterator& position, T&& element) noexcept -> void {
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
		requires ConstructibleFrom<T, Args...>
		constexpr inline auto
		insert_emplace(const Iterator& position, Args&&... args) noexcept -> Element {
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
		requires ConstructibleFrom<T, Args...>
		constexpr inline auto
		insert_emplace(const ConstIterator& position, Args&&... args) noexcept -> Element {
			return insert_emplace_internal(position.get_index(), std::forward<Args>(args)...);
		}

		/// @brief Erases the element at the given `position`, moving other elements backward
		/// in the buffer to maintain contiguity
		///
		/// @param position - The element to erase
		///
		/// @return `Iterator` pointing to the element after the one erased
		constexpr inline auto erase(const Iterator& position) noexcept -> Iterator {
			return erase_internal(position.get_index());
		}

		/// @brief Erases the element at the given `position`, moving other elements backward
		/// in the buffer to maintain contiguity
		///
		/// @param position - The element to erase
		///
		/// @return `Iterator` pointing to the element after the one erased
		constexpr inline auto erase(const ConstIterator& position) noexcept -> Iterator {
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
		constexpr inline auto
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
		constexpr inline auto
		erase(const ConstIterator& first, const ConstIterator& last) noexcept -> Iterator {
			if(first >= last) {
				return last;
			}

			return erase_internal(first.get_index(), last.get_index());
		}

		/// @brief Removes the last element in the `RingBuffer` and returns it
		///
		/// @return The last element in the `RingBuffer`
		[[nodiscard]] constexpr inline auto pop_back() noexcept -> Element {
			const auto start = m_start_index.load();
			auto write = m_write_index.load();
			const auto index = get_adjusted_internal_index(size(start, write) - 1, start);
			Element back_ = m_buffer[index];

			while(!m_write_index.compare_exchange_weak(write, write - 1)) {
			}
			return back_;
		}

		/// @brief Removes the first element in the `RingBuffer` and returns it
		///
		/// @return The first element in the `RingBuffer`
		[[nodiscard]] constexpr inline auto pop_front() noexcept -> Element {
			const auto start = m_start_index.load();
			Element front_ = m_buffer[start];
			increment_indices_from_start();
			return front_;
		}

		/// @brief Returns a Random Access Bidirectional iterator over the `RingBuffer`,
		/// at the beginning
		///
		/// @return The iterator, at the beginning
		[[nodiscard]] constexpr inline auto begin() -> Iterator {
			const auto start = m_start_index.load();
			Element p = m_buffer[start]; // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)

			return Iterator(p, this, 0ULL);
		}

		/// @brief Returns a Random Access Bidirectional iterator over the `RingBuffer`,
		/// at the end
		///
		/// @return The iterator, at the end
		[[nodiscard]] constexpr inline auto end() -> Iterator {
			const auto start = m_start_index.load();
			const auto write = m_write_index.load();
			Element p = m_buffer[write]; // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)

			return Iterator(p, this, size(start, write));
		}

		/// @brief Returns a Random Access Bidirectional read-only iterator over the `RingBuffer`,
		/// at the beginning
		///
		/// @return The iterator, at the beginning
		[[nodiscard]] constexpr inline auto cbegin() -> ConstIterator {
			const auto start = m_start_index.load();
			Element p = m_buffer[start]; // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)

			return ConstIterator(p, this, 0ULL);
		}

		/// @brief Returns a Random Access Bidirectional read-only iterator over the `RingBuffer`,
		/// at the end
		///
		/// @return The iterator, at the end
		[[nodiscard]] constexpr inline auto cend() -> ConstIterator {
			const auto start = m_start_index.load();
			const auto write = m_write_index.load();
			Element p = m_buffer[write]; // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
			return ConstIterator(p, this, size(start, write));
		}

		/// @brief Unchecked access-by-index operator
		///
		/// @param index - The index to get the corresponding element for
		///
		/// @return - The element at index
		[[nodiscard]] constexpr inline auto operator[](Integral auto index) noexcept -> Element {
			const auto start = m_start_index.load();
			auto i = get_adjusted_internal_index(index, start);

			return m_buffer[i]; // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
		}

		constexpr auto
		operator=(const RingBuffer& buffer) noexcept -> RingBuffer& requires Copyable<T> {
			if(this == &buffer) {
				return *this;
			}
			auto temp = allocate_unique<Element[]>(m_allocator, // NOLINT
												   buffer.m_capacity,
												   m_allocator);
			const auto size = buffer.size();
			for(auto i = 0ULL; i < size; ++i) {
				temp[i] = buffer.m_buffer[i];
			}
			m_buffer = std::move(temp);
			m_capacity = buffer.m_capacity;
			m_start_index = buffer.m_start_index;
			m_write_index = buffer.m_write_index;
			m_loop_index = buffer.m_loop_index;
			return *this;
		}
		constexpr auto operator=(RingBuffer&& buffer) noexcept -> RingBuffer& {
			m_allocator = buffer.m_allocator;
			m_buffer = std::move(buffer.m_buffer);
			m_write_index = buffer.m_write_index;
			m_start_index = buffer.m_start_index;
			m_loop_index = buffer.m_loop_index;
			m_capacity = buffer.m_capacity;
			buffer.m_buffer = nullptr;
			buffer.m_write_index = 0ULL;
			buffer.m_start_index = 0ULL;
			buffer.m_capacity = 0ULL;
			return *this;
		}

	  private:
		static const constexpr size_t DEFAULT_CAPACITY_INTERNAL = DEFAULT_CAPACITY + 1;
		Allocator<Element> m_allocator = Allocator<Element>();
		unique_pointer m_buffer = allocate_unique<Element[]>(m_allocator, // NOLINT
															 DEFAULT_CAPACITY_INTERNAL,
															 m_allocator);
		std::atomic_size_t m_write_index = 0ULL;
		std::atomic_size_t m_start_index = 0ULL;
		size_t m_loop_index = DEFAULT_CAPACITY;
		size_t m_capacity = DEFAULT_CAPACITY_INTERNAL;

		/// @brief Returns the current number of elements in the `RingBuffer`
		///
		/// @return The current number of elements
		[[nodiscard]] constexpr inline auto
		size(UnsignedIntegral auto start, UnsignedIntegral auto write) const noexcept -> size_t {
			return write >= start ? write - start : (m_capacity) - (start - write);
		}

		/// @brief Converts the given `RingBuffer` index into the corresponding index into then
		/// underlying `T` array
		///
		/// @param index - The `RingBuffer` index to convert
		///
		/// @return The corresponding index into the underlying `T` array
		[[nodiscard]] constexpr inline auto
		get_adjusted_internal_index(Integral auto index) const noexcept -> size_t {
			auto i = static_cast<size_t>(index);
			return (m_start_index.load() + i) % (m_capacity);
		}

		/// @brief Converts the given `RingBuffer` index into the corresponding index into then
		/// underlying `T` array
		///
		/// @param index - The `RingBuffer` index to convert
		/// @param start - The `RingBuffer`'s starting internal index
		///
		/// @return The corresponding index into the underlying `T` array
		[[nodiscard]] constexpr inline auto
		get_adjusted_internal_index(Integral auto index, UnsignedIntegral auto start) const noexcept
			-> size_t {
			auto i = static_cast<size_t>(index);
			return (start + i) % (m_capacity);
		}

		/// @brief Converts the given index into the underlying `T` array into
		/// a using facing index into the `RingBuffer`
		///
		/// @param index - The internal index
		///
		/// @return The corresponding user-facing index
		[[nodiscard]] constexpr inline auto
		get_external_index_from_internal(Integral auto index) const noexcept -> size_t {
			auto i = static_cast<size_t>(index);
			const auto start = m_start_index.load();
			if(i >= start && i <= m_loop_index) {
				return i - start;
			}
			else if(i < start) {
				return m_capacity - (start - i);
			}
			else {
				return m_capacity - 1;
			}
		}

		/// @brief Converts the given index into the underlying `T` array into
		/// a using facing index into the `RingBuffer`
		///
		/// @param index - The internal index
		/// @param start - The `RingBuffer`'s starting internal index
		///
		/// @return The corresponding user-facing index
		[[nodiscard]] constexpr inline auto
		get_external_index_from_internal(Integral auto index,
										 UnsignedIntegral auto start) const noexcept -> size_t {
			auto i = static_cast<size_t>(index);
			if(i >= start && i <= m_loop_index) {
				return i - start;
			}
			else if(i < start) {
				return m_capacity - (start - i);
			}
			else {
				return m_capacity - 1;
			}
		}

		/// @brief Used to increment the start and write indices into the underlying `T` array,
		/// and the size property, after pushing an element at the back,
		/// maintaining the logical `RingBuffer` structure
		constexpr inline auto increment_indices() noexcept -> void {
			auto old_start = m_start_index.load();
			auto old_write = m_write_index.load();
			while(!m_write_index.compare_exchange_weak(old_write, (old_write + 1) % m_capacity)) {
			}

			// if write index is at start, we need to push start forward to maintain
			// the "invalid" spacer element for this.end()
			if(old_start == m_write_index.load()) {
				old_start = m_start_index.load();
				while(!m_start_index.compare_exchange_weak(old_start, (old_start + 1) % m_capacity))
				{
				}
			}
		}

		/// @brief Used to increment the start index into the underlying `T` array
		/// and the size property after popping an element from the front,
		/// maintaining the logical `RingBuffer` structure
		constexpr inline auto increment_indices_from_start() noexcept -> void {
			auto start = m_start_index.load();
			const auto func = [&](size_t start_, size_t write) {
				// do this to remove a branch, equivalent to:
				// if (start_ != write) : (start + 1) % m_capacity
				// else : start
				return static_cast<size_t>(start_ != write) * ((start + 1) % m_capacity)
					   + static_cast<size_t>(!(start_ != write)) * start;
			};
			while(!m_start_index.compare_exchange_weak(start, func(start, m_write_index.load()))) {
			}
		}

		/// @brief Inserts the given element at the position indicated
		/// by the `external_index`
		/// @note if `size() == capacity()` this drops the last element out of the `RingBuffer`
		///
		/// @param external_index - The user-facing index into the `RingBuffer` to insert the
		/// element at
		/// @param elem - The element to store in the `RingBuffer`
		constexpr inline auto
		insert_internal(size_t external_index, const T& elem) noexcept -> void {
			const auto start = m_start_index.load();
			const auto write = m_write_index.load();
			auto index = get_adjusted_internal_index(external_index, start);

			if(index == write) {
				emplace_back(elem);
			}
			else {
				const auto size_ = size(start, write);
				auto num_to_move = size_ - external_index;
				auto j = num_to_move - 1;

				if(size_ == m_capacity - 1) [[likely]] { // NOLINT
					num_to_move--;
					j--;
					index++;
				}

				for(auto i = 0ULL; i < num_to_move; ++i, --j) {
					m_buffer[get_adjusted_internal_index(size_ - i, start)] = std::move(
						m_buffer[get_adjusted_internal_index(external_index + j, start)]);
				}

				m_buffer[index] = Element(m_allocator, elem);
				increment_indices();
			}
		}

		/// @brief Inserts the given element at the position indicated
		/// by the `external_index`
		/// @note if `size() == capacity()` this drops the last element out of the `RingBuffer`
		///
		/// @param external_index - The user-facing index into the `RingBuffer` to insert the
		/// element at
		/// @param elem - The element to store in the `RingBuffer`
		constexpr inline auto insert_internal(size_t external_index, T&& elem) noexcept -> void {
			const auto start = m_start_index.load();
			const auto write = m_write_index.load();
			auto index = get_adjusted_internal_index(external_index, start);

			if(index == write) {
				emplace_back(std::forward<T>(elem));
			}
			else {
				const auto size_ = size(start, write);
				auto num_to_move = size_ - external_index;
				auto j = num_to_move - 1;

				if(size_ == m_capacity - 1) [[likely]] { // NOLINT
					num_to_move--;
					j--;
					index++;
				}

				for(auto i = 0ULL; i < num_to_move; ++i, --j) {
					m_buffer[get_adjusted_internal_index(size_ - i, start)] = std::move(
						m_buffer[get_adjusted_internal_index(external_index + j, start)]);
				}

				m_buffer[index] = Element(m_allocator, std::forward<T>(elem));
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
		requires ConstructibleFrom<T, Args...>
		constexpr inline auto
		insert_emplace_internal(size_t external_index, Args&&... args) noexcept -> Element {
			const auto start = m_start_index.load();
			const auto write = m_write_index.load();
			auto index = get_adjusted_internal_index(external_index, start);

			if(index == write) {
				return emplace_back(std::forward<Args>(args)...);
			}
			else {
				const auto size_ = size(start, write);
				auto num_to_move = size_ - external_index;
				auto j = num_to_move - 1;

				if(size_ == m_capacity - 1) [[likely]] { // NOLINT
					num_to_move--;
					j--;
					index++;
				}

				for(auto i = 0ULL; i < num_to_move; ++i, --j) {
					m_buffer[get_adjusted_internal_index(size_ - i, start)] = std::move(
						m_buffer[get_adjusted_internal_index(external_index + j, start)]);
				}

				allocator_traits::template construct<Element>(
					m_allocator,
					&m_buffer[index],
					std::allocate_shared<T, Allocator<Element>>(m_allocator,
																std::forward<Args>(args)...));
				increment_indices();
				return m_buffer[index];
			}
		}

		/// @brief Erases the element at the given index, returning an `Iterator` to the element
		/// after the removed one
		///
		/// @param external_index - The index to the element to remove. This should be a
		/// `RingBuffer` index: IE, not an interal one into the `T` array
		///
		/// @return `Iterator` pointing to the element after the one removed
		[[nodiscard]] constexpr inline auto
		erase_internal(size_t external_index) noexcept -> Iterator {
			const auto start = m_start_index.load();
			auto write = m_write_index.load();
			const auto index = get_adjusted_internal_index(external_index, start);

			if(index == write) [[unlikely]] { // NOLINT
				return end();
			}
			else {
				const auto size_ = size(start, write);
				const auto num_to_move = (size_ - 1) - external_index;
				const auto pos_to_move = external_index + 1;
				const auto pos_to_replace = external_index;
				for(auto i = 0ULL; i < num_to_move; ++i) {
					m_buffer[get_adjusted_internal_index(pos_to_replace + i, start)]
						= m_buffer[get_adjusted_internal_index(pos_to_move + i, start)];
				}
				while(!m_write_index.compare_exchange_weak(write, write - 1)) {
				}

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
		[[nodiscard]] constexpr inline auto
		erase_internal(size_t first, size_t last) noexcept -> Iterator { // NOLINT
			const auto start = m_start_index.load();
			auto write = m_write_index.load();
			const auto size_ = size(start, write);
			const auto last_internal = get_adjusted_internal_index(last, start);
			const auto num_to_remove = (last - first);

			if(last_internal == write) {

				if(write == m_loop_index || write > start) {
					// m_start_index = m_loop_index;
					while(!m_write_index.compare_exchange_weak(write, write - num_to_remove)) {
					}
				}
				else if(write < start) {
					auto num_after_start_index
						= (write > num_to_remove) ? write - num_to_remove : 0ULL;
					auto num_before_start_index = num_to_remove - num_after_start_index;
					if(num_after_start_index > 0) {
						num_after_start_index--;
						while(!m_write_index.compare_exchange_weak(write,
																   m_loop_index
																	   - num_after_start_index)) {
						}
					}
					else {
						while(!m_write_index.compare_exchange_weak(write,
																   write - num_before_start_index))
						{
						}
					}
				}
				return end();
			}
			else {
				const auto num_to_move = size_ - last;
				const auto pos_to_move = last;
				const auto pos_to_replace = first;
				for(auto i = 0ULL; i < num_to_move; ++i) {
					m_buffer[get_adjusted_internal_index(pos_to_replace + i, start)]
						= m_buffer[get_adjusted_internal_index(pos_to_move + i, start)];
				}
				while(!m_write_index.compare_exchange_weak(write, write - num_to_remove)) {
				}

				auto iter = begin() + first;
				return iter;
			}
		}
	};
	IGNORE_PADDING_STOP

} // namespace hyperion::utils
