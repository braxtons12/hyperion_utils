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
#include <tuple>

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

				m_current_index--;
				m_ptr = &(*m_container_ptr)[m_current_index];
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

				m_current_index--;
				m_ptr = &(*m_container_ptr)[m_current_index];
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

		constexpr RingBuffer(std::initializer_list<T> values) noexcept requires Copyable<T>
			: m_buffer(allocate_unique<T[]>(m_allocator, // NOLINT
											values.size() + 1)),
			  m_loop_index(values.size()),
			  m_capacity(values.size() + 1) {

			auto end_ = values.end();
			for(auto iter = values.begin(); iter != end_; ++iter) {
				push_back(std::move(*iter));
			}
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
			return m_write_index >= m_start_index ? (m_write_index - m_start_index) :
													  (m_capacity - (m_start_index - m_write_index));
		}

		/// @brief Returns the maximum possible number of elements this `RingBuffer` could store
		/// if grown to maximum possible capacity
		///
		/// @return The maximum possible number of storable elements
		[[nodiscard]] constexpr inline auto max_size() const noexcept -> size_t {
			return allocator_traits::max_size(m_allocator) - 1;
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
													&m_buffer[m_write_index], // NOLINT
													std::forward<Args>(args)...);

			increment_indices();

			auto index = (m_write_index == 0ULL ? m_loop_index : m_write_index - 1);
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
													&m_buffer[index], // NOLINT
													std::forward<Args>(args)...);

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
													&m_buffer[index], // NOLINT
													std::forward<Args>(args)...);

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
			decrement_write();
			return back_;
		}

		/// @brief Removes the first element in the `RingBuffer` and returns it
		///
		/// @return The first element in the `RingBuffer`
		[[nodiscard]] constexpr inline auto pop_front() noexcept -> T requires Copyable<T> {
			T front_ = front();
			increment_start();
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
		constexpr inline auto increment_start() noexcept -> void {
			if(m_start_index != m_write_index) {
				m_start_index = (m_start_index + 1) % (m_capacity);
			}
		}

		/// @brief Used to decrement the write index into the underlying `T` array
		/// when popping an element from the back
		constexpr inline auto decrement_write() noexcept -> void {
			if(m_write_index == 0ULL) {
				m_write_index = m_capacity - 1;
			}
			else {
				m_write_index--;
			}
		}

		constexpr inline auto decrement_write_n(UnsignedIntegral auto n) noexcept -> void {
			auto amount_to_decrement = static_cast<size_t>(n);
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
		constexpr inline auto
		insert_internal(size_t external_index, const T& elem) noexcept -> void {
			auto index = get_adjusted_internal_index(external_index);

			if(index == m_write_index) {
				emplace_back(elem);
			}
			else {
				const auto size_ = size();
				auto num_to_move = size_ - external_index;
				auto j = num_to_move - 1;

				// if we're full, drop the last element in the buffer
				if(size_ == m_capacity - 1) [[likely]] { // NOLINT
					num_to_move--;
					j--;
					index++;
				}

				for(auto i = 0ULL; i < num_to_move; ++i, --j) {
					if constexpr(Movable<T>) {
						m_buffer[get_adjusted_internal_index(size_ - i)]
							= std::move(m_buffer[get_adjusted_internal_index(external_index + j)]);
					}
					else {
						m_buffer[get_adjusted_internal_index(size_ - i)]
							= m_buffer[get_adjusted_internal_index(external_index + j)];
					}
				}

				m_buffer[index] = elem;
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
			auto index = get_adjusted_internal_index(external_index);

			if(index == m_write_index) {
				emplace_back(std::forward<T>(elem));
			}
			else {
				const auto size_ = size();
				auto num_to_move = size_ - external_index;
				auto j = num_to_move - 1;

				// if we're full, drop the last element in the buffer
				if(size_ == m_capacity - 1) [[likely]] { // NOLINT
					num_to_move--;
					j--;
					index++;
				}

				for(auto i = 0ULL; i < num_to_move; ++i, --j) {
					if constexpr(Movable<T>) {
						m_buffer[get_adjusted_internal_index(size_ - i)]
							= std::move(m_buffer[get_adjusted_internal_index(external_index + j)]);
					}
					else {
						m_buffer[get_adjusted_internal_index(size_ - i)]
							= m_buffer[get_adjusted_internal_index(external_index + j)];
					}
				}

				m_buffer[index] = std::forward<T>(elem);
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
		insert_emplace_internal(size_t external_index, Args&&... args) noexcept -> T& {
			auto index = get_adjusted_internal_index(external_index);

			if(index == m_write_index) {
				return emplace_back(std::forward<Args>(args)...);
			}
			else {
				const auto size_ = size();
				auto num_to_move = size_ - external_index;
				auto j = num_to_move - 1;

				// if we're full, drop the last element in the buffer
				if(size_ == m_capacity - 1) [[likely]] { // NOLINT
					num_to_move--;
					j--;
					index++;
				}

				for(auto i = 0ULL; i < num_to_move; ++i, --j) {
					if constexpr(Movable<T>) {
						m_buffer[get_adjusted_internal_index(size_ - i)]
							= std::move(m_buffer[get_adjusted_internal_index(external_index + j)]);
					}
					else {
						m_buffer[get_adjusted_internal_index(size_ - i)]
							= m_buffer[get_adjusted_internal_index(external_index + j)];
					}
				}

				allocator_traits::template construct<T>(m_allocator,
														&m_buffer[index],
														std::forward<Args>(args)...);

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
			const auto index = get_adjusted_internal_index(external_index);

			if(index == m_write_index) [[unlikely]] { // NOLINT
				return end();
			}
			else {
				const auto size_ = size();
				auto num_to_move = (size_ - 1) - external_index;
				const auto pos_to_move = external_index + 1;
				const auto pos_to_replace = external_index;
				for(auto i = 0ULL; i < num_to_move; ++i) {
					if constexpr(Movable<T>) {
						m_buffer[get_adjusted_internal_index(pos_to_replace + i)]
							= std::move(m_buffer[get_adjusted_internal_index(pos_to_move + i)]);
					}
					else {
						m_buffer[get_adjusted_internal_index(pos_to_replace + i)]
							= m_buffer[get_adjusted_internal_index(pos_to_move + i)];
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
		[[nodiscard]] constexpr inline auto
		erase_internal(size_t first, size_t last) noexcept -> Iterator {
			const auto size_ = size();
			const auto last_internal = get_adjusted_internal_index(last);
			const auto num_to_remove = (last - first);

			if(last_internal > m_write_index) {
				if(m_write_index > m_start_index) {
					decrement_write_n(num_to_remove);
				}
				else if(m_write_index < m_start_index) {
					auto num_after_start_index
						= (m_write_index > num_to_remove ? m_write_index - num_to_remove : 0ULL);
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
			else {
				const auto num_to_move = size_ - last;
				const auto pos_to_move = last;
				const auto pos_to_replace = first;
				for(auto i = 0ULL; i < num_to_move; ++i) {
					if constexpr(Movable<T>) {
						m_buffer[get_adjusted_internal_index(pos_to_replace + i)]
							= std::move(m_buffer[get_adjusted_internal_index(pos_to_move + i)]);
					}
					else {
						m_buffer[get_adjusted_internal_index(pos_to_replace + i)]
							= m_buffer[get_adjusted_internal_index(pos_to_move + i)];
					}
				}
				decrement_write_n(num_to_remove);

				return begin() + first;
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
		using index_type = uint32_t;

		/// Default capacity of `RingBuffer`
		static const constexpr index_type DEFAULT_CAPACITY = 16;

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
			constexpr ~Element() noexcept = default;

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
										index_type currentIndex) noexcept
				: m_ptr(ptr), m_container_ptr(containerPtr), m_current_index(currentIndex) {
			}
			constexpr Iterator(const Iterator& iter) noexcept = default;
			constexpr Iterator(Iterator&& iter) noexcept = default;
			~Iterator() noexcept = default;

			/// @brief Returns the index in the `RingBuffer` that corresponds
			/// to the element this iterator points to
			///
			/// @return The index corresponding with the element this points to
			[[nodiscard]] constexpr inline auto get_index() const noexcept -> index_type {
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
				const auto diff = static_cast<index_type>(rhs);
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
				const auto diff = static_cast<index_type>(rhs);
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
			index_type m_current_index = 0;
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
											 index_type currentIndex) noexcept
				: m_ptr(ptr), m_container_ptr(containerPtr), m_current_index(currentIndex) {
			}
			constexpr ConstIterator(const ConstIterator& iter) noexcept = default;
			constexpr ConstIterator(ConstIterator&& iter) noexcept = default;
			~ConstIterator() noexcept = default;

			/// @brief Returns the index in the `RingBuffer` that corresponds
			/// to the element this iterator points to
			///
			/// @return The index corresponding with the element this points to
			[[nodiscard]] constexpr inline auto get_index() const noexcept -> index_type {
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
				const auto diff = static_cast<index_type>(rhs);
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
				const auto diff = static_cast<index_type>(rhs);
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
			index_type m_current_index = 0;
		};

		/// @brief Creates a `RingBuffer` with default capacity
		constexpr RingBuffer() noexcept = default;

		/// @brief Creates a `RingBuffer` with (at least) the given initial capacity
		///
		/// @param intitial_capacity - The initial capacity of the `RingBuffer`
		constexpr explicit RingBuffer(index_type intitial_capacity) noexcept
			: m_buffer(allocate_unique<Element[]>(m_allocator, // NOLINT
												  intitial_capacity + 1,
												  m_allocator)),
			  m_state(intitial_capacity + 1) {
		}

		/// @brief Constructs a new `RingBuffer` with the given initial capacity and
		/// fills it with `default_value`
		///
		/// @param intitial_capacity - The initial capacity of the `RingBuffer`
		/// @param default_value - The value to fill the `RingBuffer` with
		constexpr RingBuffer(index_type intitial_capacity,
							 const T& default_value) noexcept requires Copyable<T>
			: m_buffer(allocate_unique<Element[]>(m_allocator, // NOLINT
												  intitial_capacity + 1,
												  m_allocator,
												  default_value)),
			  m_state(intitial_capacity + 1, 0U, intitial_capacity) {
		}

		constexpr RingBuffer(const RingBuffer& buffer) noexcept requires Copyable<T>
			: m_buffer(allocate_unique<Element[]>(m_allocator, // NOLINT
												  buffer.m_capacity,
												  m_allocator)),
			  m_state(buffer.m_capacity) {
			const auto size = buffer.size();
			for(auto i = 0U; i < size; ++i) {
				push_back(buffer.m_buffer[i]);
			}
			m_state = buffer.m_state;
		}

		constexpr RingBuffer(RingBuffer&& buffer) noexcept
			: m_allocator(buffer.m_allocator), m_buffer(std::move(buffer.m_buffer)),
			  m_state(buffer.m_state) {
			buffer.m_state.update(0U, 0U, 0U);
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
			const auto i = m_state.adjusted_index(static_cast<index_type>(index));

			return m_buffer[i]; // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
		}

		/// @brief Returns the first element in the `RingBuffer`
		///
		/// @return The first element
		[[nodiscard]] constexpr inline auto front() noexcept -> Element {
			return m_buffer
				[m_state.start()]; // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
		}

		/// @brief Returns the last element in the `RingBuffer`
		/// @note If <= 1 elements are in the `RingBuffer`, this will be the same as `front`
		///
		/// @return The last element
		[[nodiscard]] constexpr inline auto back() noexcept -> Element {
			const auto index = m_state.back();

			return m_buffer[index]; // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
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
			return m_state.empty();
		}

		/// @brief Returns whether the `RingBuffer` is full
		///
		/// @return `true` if the `RingBuffer` is full, `false` otherwise
		[[nodiscard]] constexpr inline auto full() const noexcept -> bool {
			return m_state.full();
		}

		/// @brief Returns the current number of elements in the `RingBuffer`
		///
		/// @return The current number of elements
		[[nodiscard]] constexpr inline auto size() const noexcept -> index_type {
			return m_state.size();
		}

		/// @brief Returns the maximum possible number of elements this `RingBuffer` could store
		/// if grown to maximum possible capacity
		///
		/// @return The maximum possible number of storable elements
		[[nodiscard]] constexpr inline auto max_size() const noexcept -> index_type {
			return m_state.max_size();
		}

		/// @brief Returns the current capacity of the `RingBuffer`;
		/// the number of elements it can currently store
		///
		/// @return The current capacity
		[[nodiscard]] constexpr inline auto capacity() const noexcept -> index_type {
			return m_state.capacity() - 1;
		}

		/// @brief Reserves more storage for the `RingBuffer`. If `new_capacity` is > capacity,
		/// then the capacity of the `RingBuffer` will be extended until at least `new_capacity`
		/// elements can be stored.
		/// @note Memory contiguity is maintained, so no **elements** will be lost or invalidated.
		/// However, all iterators and references to elements will be invalidated.
		///
		/// @param new_capacity - The new capacity of the `RingBuffer`
		constexpr inline auto reserve(index_type new_capacity) noexcept -> void {
			auto capacity_ = m_state.capacity();

			// we only need to do anything if `new_capacity` is actually larger than `m_capacity`
			if(new_capacity > capacity_ - 1) {
				auto temp = allocate_unique<Element[]>(m_allocator, // NOLINT
													   new_capacity + 1,
													   m_allocator);
				auto span = gsl::make_span(&temp[0], new_capacity + 1);
				std::copy(begin(), end(), span.begin());
				m_buffer = std::move(temp);
				m_state.update(0U, capacity_, new_capacity + 1);
			}
		}

		/// @brief Erases all elements from the `RingBuffer`
		constexpr inline auto clear() noexcept -> void {
			m_state.clear();
		}

		/// @brief Inserts the given element at the end of the `RingBuffer`
		/// @note if `size() == capacity()` then this loops and overwrites `front()`
		///
		/// @param value - the element to insert
		constexpr inline auto push_back(const T& value) noexcept -> void requires Copyable<T> {
			m_buffer[m_state.write()] // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
				= Element(m_allocator, value);

			m_state.increment_indices();
		}

		/// @brief Inserts the given element at the end of the `RingBuffer`
		/// @note if `size() == capacity()` then this loops and overwrites `front()`
		///
		/// @param value - the element to insert
		constexpr inline auto push_back(T&& value) noexcept -> void {
			m_buffer[m_state.write()] // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
				= Element(m_allocator, std::forward<T>(value));

			m_state.increment_indices();
		}

		/// @brief Inserts the given element at the end of the `RingBuffer`
		/// @note if `size() == capacity()` then this loops and overwrites `front()`
		///
		/// @param element - the element to insert
		constexpr inline auto push_back(const Element& element) noexcept -> void {
			m_buffer[m_state.write()] // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
				= element;

			m_state.increment_indices();
		}

		/// @brief Inserts the given element at the end of the `RingBuffer`
		/// @note if `size() == capacity()` then this loops and overwrites `front()`
		///
		/// @param element - the element to insert
		constexpr inline auto push_back(Element&& element) noexcept -> void {
			m_buffer[m_state.write()] // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
				= std::forward<Element>(element);
			m_state.increment_indices();
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
			const auto index = m_state.write();

			allocator_traits::template construct<Element>(
				m_allocator,
				&m_buffer[index], // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
				std::allocate_shared<T, Allocator<Element>>(m_allocator,
															std::forward<Args>(args)...));

			m_state.increment_indices();

			return m_buffer[index]; // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
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
			const auto index = m_state.get_adjusted_internal_index(position.get_index());

			allocator_traits::template construct<Element>(
				m_allocator,
				&m_buffer[index], // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
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
			const auto index = m_state.get_adjusted_internal_index(position.get_index());

			allocator_traits::template construct<Element>(
				m_allocator,
				&m_buffer[index], // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
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
			Element back_ = back();
			m_state.decrement_write();
			return back_;
		}

		/// @brief Removes the first element in the `RingBuffer` and returns it
		///
		/// @return The first element in the `RingBuffer`
		[[nodiscard]] constexpr inline auto pop_front() noexcept -> Element {
			Element front_ = front();
			m_state.increment_start();
			return front_;
		}

		/// @brief Returns a Random Access Bidirectional iterator over the `RingBuffer`,
		/// at the beginning
		///
		/// @return The iterator, at the beginning
		[[nodiscard]] constexpr inline auto begin() -> Iterator {
			// clang-format off
			Element p = m_buffer[m_state.start()]; // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
			// clang-format on

			return Iterator(p, this, 0U);
		}

		/// @brief Returns a Random Access Bidirectional iterator over the `RingBuffer`,
		/// at the end
		///
		/// @return The iterator, at the end
		[[nodiscard]] constexpr inline auto end() -> Iterator {
			// clang-format off
			Element p = m_buffer[m_state.write()]; // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
			// clang-format on

			return Iterator(p, this, m_state.size());
		}

		/// @brief Returns a Random Access Bidirectional read-only iterator over the `RingBuffer`,
		/// at the beginning
		///
		/// @return The iterator, at the beginning
		[[nodiscard]] constexpr inline auto cbegin() -> ConstIterator {
			// clang-format off
			Element p = m_buffer[m_state.start()]; // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
			// clang-format on

			return ConstIterator(p, this, 0U);
		}

		/// @brief Returns a Random Access Bidirectional read-only iterator over the `RingBuffer`,
		/// at the end
		///
		/// @return The iterator, at the end
		[[nodiscard]] constexpr inline auto cend() -> ConstIterator {
			// clang-format off
			Element p = m_buffer[m_state.write()]; // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
			// clang-format on

			return ConstIterator(p, this, m_state.size());
		}

		/// @brief Unchecked access-by-index operator
		///
		/// @param index - The index to get the corresponding element for
		///
		/// @return - The element at index
		[[nodiscard]] constexpr inline auto operator[](Integral auto index) noexcept -> Element {
			const auto i = m_state.adjusted_index(static_cast<index_type>(index));

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
			for(auto i = 0U; i < size; ++i) {
				temp[i]					  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
					= buffer.m_buffer[i]; // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
			}
			m_buffer = std::move(temp);
			m_state = m_buffer.m_state;
			return *this;
		}
		constexpr auto operator=(RingBuffer&& buffer) noexcept -> RingBuffer& {
			m_allocator = buffer.m_allocator;
			m_buffer = std::move(buffer.m_buffer);
			m_state = m_buffer.m_state;
			buffer.m_buffer = nullptr;
			buffer.m_state.update(0U, 0U, 0U);
			return *this;
		}

	  private:
		static const constexpr uint32_t DEFAULT_CAPACITY_INTERNAL = DEFAULT_CAPACITY + 1;

		class State {
		  public:
			using merged_type = uint64_t;
			using atomic_index_type = std::atomic<index_type>;
			using atomic_merged_type = std::atomic<merged_type>;

			constexpr State() noexcept = default;
			explicit constexpr State(index_type capacity) noexcept : m_capacity(capacity) {
			}
			explicit constexpr State(index_type capacity,
									 index_type start,
									 index_type write) noexcept
				: m_indices(merge_indices(start, write)), m_capacity(capacity) {
			}
			constexpr State(const State& state) noexcept
				: m_indices(state.m_indices), m_capacity(state.m_capacity) {
			}
			constexpr State(State&& state) noexcept
				: m_indices(state.m_indices), m_capacity(state.m_capacity) {
			}
			constexpr ~State() noexcept = default;

			inline constexpr auto
			update(index_type start, index_type write, index_type capacity) noexcept -> void {
				auto indices = m_indices.load();
				while(!m_indices.compare_exchange_weak(indices, merge_indices(start, write))) {
				}

				auto capacity_ = m_capacity.load();
				while(!m_capacity.compare_exchange_weak(capacity_, capacity)) {
				}
			}

			[[nodiscard]] inline constexpr auto start() const noexcept -> index_type {
				return start(m_indices.load());
			}

			[[nodiscard]] inline constexpr auto write() const noexcept -> index_type {
				return write(m_indices.load());
			}

			[[nodiscard]] inline constexpr auto
			indices() const noexcept -> std::tuple<index_type, index_type> {
				const auto val = m_indices.load();
				return {start(val), write(val)};
			}

			[[nodiscard]] inline constexpr auto capacity() const noexcept -> index_type {
				return m_capacity.load();
			}

			[[nodiscard]] inline constexpr auto max_size() const noexcept -> index_type {
				return std::numeric_limits<index_type>::max() - 1;
			}

			[[nodiscard]] inline constexpr auto loop_index() const noexcept -> index_type {
				return m_capacity.load() - 1;
			}

			[[nodiscard]] inline constexpr auto size() const noexcept -> index_type {
				const auto indices = m_indices.load();
				const auto capacity_ = m_capacity.load();
				const auto start_ = start(indices);
				const auto write_ = write(indices);
				return write_ >= start_ ? (write_ - start_) : (capacity_ - (start_ - write_));
			}

			[[nodiscard]] inline constexpr auto
			size(UnsignedIntegral auto start,
				 UnsignedIntegral auto write,
				 UnsignedIntegral auto capacity) const noexcept -> index_type {
				const auto start_ = static_cast<index_type>(start);
				const auto write_ = static_cast<index_type>(write);
				const auto capacity_ = static_cast<index_type>(capacity);
				return static_cast<index_type>(write_ >= start_ ? (write_ - start_) :
																	(capacity_ - (start_ - write_)));
			}

			[[nodiscard]] inline constexpr auto empty() const noexcept -> bool {
				return size() == 0U;
			}

			[[nodiscard]] inline constexpr auto full() const noexcept -> bool {
				const auto indices = m_indices.load();
				const auto capacity_ = m_capacity.load();
				const auto start_ = start(indices);
				const auto write_ = write(indices);
				const auto size_
					= write_ >= start_ ? (write_ - start_) : (capacity_ - (start_ - write_));
				return size_ == capacity_ - 1;
			}

			inline constexpr auto clear() noexcept -> void {
				auto indices = m_indices.load();
				while(!m_indices.compare_exchange_weak(indices, merge_indices(0U, 0U))) {
				}
			}

			[[nodiscard]] inline constexpr auto
			adjusted_index(UnsignedIntegral auto index) const noexcept -> index_type {
				const auto i = static_cast<index_type>(index);
				return (start() + i) % capacity();
			}

			[[nodiscard]] inline constexpr auto
			adjusted_index(UnsignedIntegral auto index,
						   UnsignedIntegral auto start,
						   UnsignedIntegral auto capacity) const noexcept -> index_type {
				const auto i = static_cast<index_type>(index);
				const auto start_ = static_cast<index_type>(start);
				const auto capacity_ = static_cast<index_type>(capacity);
				return (start_ + i) % (capacity_);
			}

			[[nodiscard]] inline constexpr auto back() const noexcept -> index_type {
				const auto indices = m_indices.load();
				const auto capacity_ = m_capacity.load();

				return adjusted_index(size(start(indices), write(indices), capacity_) - 1,
									  start(indices),
									  capacity_);
			}

			inline constexpr auto increment_indices() noexcept -> void {
				auto indices = m_indices.load();
				const auto capacity_ = m_capacity.load();
				if(start(indices) == ((write(indices) + 1) % capacity_)) [[likely]] { // NOLINT

					while(!m_indices.compare_exchange_weak(
						indices,
						merge_indices((start(indices) + 1) % capacity_,
									  (write(indices) + 1) % capacity_)))
					{ }
				}
				else {
					while(!(m_indices.compare_exchange_weak(
						indices,
						merge_indices(start(indices), (write(indices) + 1) % capacity_))))
					{ }
				}
			}

			inline constexpr auto increment_start() noexcept -> void {
				auto indices = m_indices.load();
				const auto capacity_ = m_capacity.load();
				if(start(indices) != (write(indices))) {
					while(!(m_indices.compare_exchange_weak(
						indices,
						merge_indices((start(indices) + 1) % capacity_, write(indices)))))
					{ }
				}
			}

			inline constexpr auto decrement_write() noexcept -> void {
				auto indices = m_indices.load();
				if(write(indices) == 0U) {
					const auto capacity_ = m_capacity.load();
					while(!m_indices.compare_exchange_weak(
						indices,
						merge_indices(start(indices), capacity_ - 1))) {
					}
				}
				else {
					while(!m_indices.compare_exchange_weak(
						indices,
						merge_indices(start(indices), write(indices) - 1))) {
					}
				}
			}

			inline constexpr auto decrement_write_n(UnsignedIntegral auto n) noexcept -> void {
				auto indices = m_indices.load();
				const auto capacity_ = m_capacity.load();

				auto amount_to_decrement = static_cast<index_type>(n);
				if(amount_to_decrement > write(indices)) {
					amount_to_decrement -= write(indices);
					while(!m_indices.compare_exchange_weak(
						indices,
						merge_indices(start(indices), (capacity_ - 1) - amount_to_decrement)))
					{ }
				}
				else {
					while(!m_indices.compare_exchange_weak(
						indices,
						merge_indices(start(indices), write(indices) - amount_to_decrement)))
					{ }
				}
			}

			inline constexpr auto set_write(UnsignedIntegral auto index) noexcept -> void {
				const auto index_ = static_cast<index_type>(index);
				auto indices = m_indices.load();
				while(!m_indices.compare_exchange_weak(indices,
													   merge_indices(start(indices), index_))) {
				}
			}

			constexpr auto operator=(const State& state) noexcept -> State& {
				if(&state == this) {
					return *this;
				}
				m_indices.store(state.m_indices);
				m_capacity.store(state.m_capacity);

				return *this;
			}
			constexpr auto operator=(State&& state) noexcept -> State& {
				m_indices.store(state.m_indices);
				m_capacity.store(state.m_capacity);
				return *this;
			}

		  private:
			static constexpr uint8_t START_SHIFT = 32U;
			static constexpr merged_type MASK = 0x0000'0000'FFFF'FFFFU;
			atomic_merged_type m_indices = 0ULL;
			atomic_index_type m_capacity = DEFAULT_CAPACITY_INTERNAL;

			[[nodiscard]] static inline constexpr auto
			merge_indices(index_type start, index_type write) noexcept -> merged_type {
				return (static_cast<merged_type>(start) << START_SHIFT) | write;
			}

			[[nodiscard]] static inline constexpr auto
			start(merged_type indices) noexcept -> index_type {
				return (indices >> START_SHIFT) & MASK;
			}

			[[nodiscard]] static inline constexpr auto
			write(merged_type indices) noexcept -> index_type {
				return indices & MASK;
			}
		};

		Allocator<Element> m_allocator = Allocator<Element>();
		unique_pointer m_buffer = allocate_unique<Element[]>(m_allocator, // NOLINT
															 DEFAULT_CAPACITY_INTERNAL,
															 m_allocator);
		State m_state = State();

		/// @brief Inserts the given element at the position indicated
		/// by the `external_index`
		/// @note if `size() == capacity()` this drops the last element out of the `RingBuffer`
		///
		/// @param external_index - The user-facing index into the `RingBuffer` to insert the
		/// element at
		/// @param elem - The element to store in the `RingBuffer`
		constexpr inline auto
		insert_internal(index_type external_index, const T& elem) noexcept -> void {
			const auto [start, write] = m_state.indices();
			const auto capacity_ = m_state.capacity();
			auto index = m_state.adjusted_index(external_index, start, capacity_);

			if(index == write) {
				emplace_back(elem);
			}
			else {
				const auto size_ = m_state.size(start, write, capacity_);
				auto num_to_move = size_ - external_index;
				auto j = num_to_move - 1;

				if(size_ == capacity_ - 1) [[likely]] { // NOLINT
					num_to_move--;
					j--;
					index++;
				}

				for(auto i = 0U; i < num_to_move; ++i, --j) {
					m_buffer[m_state.adjusted_index(size_ - i, start, capacity_)] = std::move(
						m_buffer[m_state.adjusted_index(external_index + j, start, capacity_)]);
				}

				m_buffer[index] = Element(m_allocator, elem);
				m_state.increment_indices();
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
		insert_internal(index_type external_index, T&& elem) noexcept -> void {
			const auto [start, write] = m_state.indices();
			const auto capacity_ = m_state.capacity();
			auto index = m_state.adjusted_index(external_index, start, capacity_);

			if(index == write) {
				emplace_back(std::forward<T>(elem));
			}
			else {
				const auto size_ = m_state.size(start, write, capacity_);
				auto num_to_move = size_ - external_index;
				auto j = num_to_move - 1;

				if(size_ == capacity_ - 1) [[likely]] { // NOLINT
					num_to_move--;
					j--;
					index++;
				}

				for(auto i = 0U; i < num_to_move; ++i, --j) {
					m_buffer[m_state.adjusted_index(size_ - i, start, capacity_)] = std::move(
						m_buffer[m_state.adjusted_index(external_index + j, start, capacity_)]);
				}

				m_buffer[index] = Element(m_allocator, std::forward<T>(elem));
				m_state.increment_indices();
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
		insert_emplace_internal(index_type external_index, Args&&... args) noexcept -> Element {
			const auto [start, write] = m_state.indices();
			const auto capacity_ = m_state.capacity();
			auto index = m_state.adjusted_index(external_index, start, capacity_);

			if(index == write) {
				return emplace_back(std::forward<Args>(args)...);
			}
			else {
				const auto size_ = m_state.size(start, write, capacity_);
				auto num_to_move = size_ - external_index;
				auto j = num_to_move - 1;

				if(size_ == capacity_ - 1) [[likely]] { // NOLINT
					num_to_move--;
					j--;
					index++;
				}

				for(auto i = 0U; i < num_to_move; ++i, --j) {
					m_buffer[m_state.adjusted_index(size_ - i, start, capacity_)] = std::move(
						m_buffer[m_state.adjusted_index(external_index + j, start, capacity_)]);
				}

				allocator_traits::template construct<Element>(
					m_allocator,
					&m_buffer[index],
					std::allocate_shared<T, Allocator<Element>>(m_allocator,
																std::forward<Args>(args)...));
				m_state.increment_indices();
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
		erase_internal(index_type external_index) noexcept -> Iterator {
			const auto [start, write] = m_state.indices();
			const auto capacity_ = m_state.capacity();
			const auto index = m_state.adjusted_index(external_index, start, capacity_);

			if(index == write) [[unlikely]] { // NOLINT
				return end();
			}
			else {
				// const auto size_ = size(start, write);
				const auto size_ = m_state.size(start, write, capacity_);
				const auto num_to_move = (size_ - 1) - external_index;
				const auto pos_to_move = external_index + 1;
				const auto pos_to_replace = external_index;
				for(auto i = 0U; i < num_to_move; ++i) {
					m_buffer[m_state.adjusted_index(pos_to_replace + i, start, capacity_)]
						= m_buffer[m_state.adjusted_index(pos_to_move + i, start, capacity_)];
				}
				m_state.decrement_write();

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
		erase_internal(index_type first, index_type last) noexcept -> Iterator { // NOLINT
			const auto [start, write] = m_state.indices();
			const auto capacity_ = m_state.capacity();
			const auto size_ = m_state.size(start, write, capacity_);
			const auto last_internal = m_state.adjusted_index(last, start, capacity_);
			const auto num_to_remove = (last - first);

			if(last_internal == write) {

				if(write > start) {
					m_state.decrement_write_n(num_to_remove);
				}
				else if(write < start) {
					auto num_after_start_index
						= (write > num_to_remove) ? write - num_to_remove : 0U;
					auto num_before_start_index = num_to_remove - num_after_start_index;
					if(num_after_start_index > 0) {
						num_after_start_index--;
						m_state.set_write(m_state.loop_index() - num_after_start_index);
					}
					else {
						m_state.decrement_write_n(num_before_start_index);
					}
				}
				return end();
			}
			else {
				const auto num_to_move = size_ - last;
				const auto pos_to_move = last;
				const auto pos_to_replace = first;
				for(auto i = 0U; i < num_to_move; ++i) {
					m_buffer[m_state.adjusted_index(pos_to_replace + i, start, capacity_)]
						= m_buffer[m_state.adjusted_index(pos_to_move + i, start, capacity_)];
				}
				m_state.decrement_write_n(num_to_remove);

				return begin() + first;
			}
		}
	};
	IGNORE_PADDING_STOP

} // namespace hyperion::utils
