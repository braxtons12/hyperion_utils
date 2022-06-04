
#include <Hyperion/BasicTypes.h>
#include <Hyperion/Concepts.h>
#include <Hyperion/HyperionDef.h>
#include <Hyperion/Memory.h>
#include <Hyperion/Option.h>
#include <atomic>
#include <gsl/gsl>
#include <limits>

namespace hyperion {

	IGNORE_PADDING_START
	template<typename T, template<typename ElementType> typename Allocator = std::allocator>
	class LockFreeQueue {
	  public:
		/// Default capacity of `LockFreeQueue`
		static const constexpr usize DEFAULT_CAPACITY = 16;
		using allocator_traits = std::allocator_traits<Allocator<T>>;
		using unique_pointer = decltype(hyperion::allocate_unique<T[]>( // NOLINT
			std::declval<Allocator<T[]>>(),						   // NOLINT
			DEFAULT_CAPACITY));

		/// @brief Creates a `LockFreeQueue` with default capacity
		constexpr LockFreeQueue() noexcept {
			for(auto index = 0_usize; index < m_capacity; ++index) {
				std::construct_at(std::addressof(m_buffer[index])); // NOLINT
			}
		}

		/// @brief Creates a `LockFreeQueue` with (at least) the given initial capacity
		///
		/// @param initial_capacity - The initial capacity of the `LockFreeQueue`
		constexpr explicit LockFreeQueue(u32 initial_capacity) noexcept
			: m_buffer(hyperion::allocate_unique<T[]>(m_allocator, initial_capacity)), // NOLINT
			  m_capacity(initial_capacity) {
			for(auto index = 0_usize; index < m_capacity; ++index) {
				std::construct_at(std::addressof(m_buffer[index])); // NOLINT
			}
		}

		/// @brief Constructs a new `LockFreeQueue` with the given initial capacity and
		/// fills it with `default_value`
		///
		/// @param initial_capacity - The initial capacity of the `LockFreeQueue`
		/// @param default_value - The value to fill the `LockFreeQueue` with
		constexpr LockFreeQueue(u32 initial_capacity,
								const T& default_value) noexcept
		requires concepts::NoexceptCopyConstructible<T>
		: m_buffer(hyperion::allocate_unique<T[]>(m_allocator, // NOLINT
											 initial_capacity,
											 default_value)),
		  m_capacity(initial_capacity) {

			set_all(0_u32, initial_capacity, initial_capacity);
			for(auto index = 0_usize; index < initial_capacity; ++index) {
				m_buffer[index] = default_value;
			}
		}

		explicit(false) constexpr LockFreeQueue(std::initializer_list<T> values) noexcept
		requires concepts::NoexceptMoveAssignable<T>
		: m_buffer(hyperion::allocate_unique<T[]>(m_allocator, // NOLINT
											 values.size())),
		  m_capacity(values.size()) {

			set_all(0_u32, values.size(), values.size());
			auto index = 0_usize;
			for(auto&& val : values) {
				m_buffer[index] = std::forward<T>(val);
				index++;
			}
		}

		constexpr LockFreeQueue(const LockFreeQueue& buffer) noexcept
		requires concepts::NoexceptCopyConstructible<T>
		: m_buffer(hyperion::allocate_unique<T[]>(m_allocator, // NOLINT
											 buffer.m_capacity)),
		  m_capacity(buffer.m_capacity) {

			const auto size = buffer.size();
			set_all(0_u32, size, size);
			for(auto index = 0_usize; index < size; ++index) {
				m_buffer[index] = buffer[index];
			}
		}

		constexpr LockFreeQueue(LockFreeQueue&& buffer) noexcept
			: m_allocator(buffer.m_allocator),
			  m_buffer(std::move(buffer.m_buffer)),
			  m_capacity(buffer.m_capacity) {

			const auto [read, write, size] = buffer.get_all();
			set_all(read, write, size);
			buffer.set_all(0_u32, 0_u32, 0_u32);
			buffer.m_buffer = nullptr;
		}

		~LockFreeQueue() noexcept
		requires concepts::NoexceptDestructible<T>
		{
			for(auto index = 0_usize; index < m_capacity; ++index) {
				std::destroy_at(std::addressof(m_buffer[index])); // NOLINT
			}
		}

		/// @brief Returns whether the `LockFreeQueue` is empty
		///
		/// @return `true` if the `LockFreeQueue` is empty, `false` otherwise
		[[nodiscard]] inline constexpr auto empty() const noexcept -> bool {
			return size() == 0;
		}

		/// @brief Returns whether the `LockFreeQueue` is full
		///
		/// @return `true` if the `LockFreeQueue` is full, `false` otherwise
		[[nodiscard]] inline constexpr auto full() const noexcept -> bool {
			return size() == m_capacity;
		}

		/// @brief Returns the current number of elements in the `LockFreeQueue`
		///
		/// @return The current number of elements
		[[nodiscard]] inline constexpr auto size() const noexcept -> usize {
			return m_size.load(std::memory_order_acquire);
		}

		/// @brief Returns the maximum possible number of elements this `LockFreeQueue` could store
		/// if grown to maximum possible capacity
		///
		/// @return The maximum possible number of storable elements
		[[nodiscard]] inline constexpr auto max_size() const noexcept -> usize {
			return std::numeric_limits<u32>::max() - 1;
		}

		/// @brief Returns the current capacity of the `LockFreeQueue`;
		/// the number of elements it can currently store
		///
		/// @return The current capacity
		[[nodiscard]] inline constexpr auto capacity() const noexcept -> usize {
			return m_capacity;
		}

		/// @brief Reserves more storage for the `LockFreeQueue`. If `new_capacity` is > capacity,
		/// then the capacity of the `LockFreeQueue` will be extended until at least `new_capacity`
		/// elements can be stored.
		/// @note Memory contiguity is maintained, so no **elements** will be lost or invalidated.
		/// However, all iterators and references to elements will be invalidated.
		///
		/// @param new_capacity - The new capacity of the `LockFreeQueue`
		inline constexpr auto reserve(u32 new_capacity) noexcept -> void {
			// we only need to do anything if `new_capacity` is actually larger than `m_capacity`
			if(new_capacity > m_capacity) {
				std::atomic_thread_fence(std::memory_order_acquire);
				auto temp = hyperion::allocate_unique<T[],			  // NOLINT
												 Allocator<T[]>>( // NOLINT
					m_allocator,
					new_capacity); // NOLINT
				auto span = gsl::make_span(std::addressof(temp[0]), new_capacity);
				const auto read = m_read;
				for(auto i = 0_usize; i < m_capacity; ++i) {
					const auto index = (read + i) % m_capacity;
					span[i] = m_buffer[gsl::narrow_cast<std::ptrdiff_t>(index)];
				}

				m_buffer = std::move(temp);
				m_read = 0_u32;
				const auto old_capacity = m_capacity;
				m_capacity = new_capacity;
				m_write.store(old_capacity - 1, std::memory_order_relaxed);
				m_max_read.store(old_capacity - 1, std::memory_order_release);
			}
		}

		/// @brief Erases all elements from the `LockFreeQueue`
		inline constexpr auto clear() noexcept -> void {
			m_buffer = hyperion::allocate_unique<T[]>(m_allocator, m_capacity); // NOLINT
			set_all(0_u32, 0_u32, 0_i32);
		}

		/// @brief Inserts the given element at the end of the `LockFreeQueue`
		/// @note if `size() == capacity()` then this blocks until an element has been removed
		/// (opening up a space for the to-be-pushed entry)
		///
		/// @param value - the element to insert
		template<typename U>
		requires concepts::Same<T, std::remove_cvref_t<U>>
				 inline constexpr auto push_back(U&& value) noexcept -> void
				 requires concepts::NoexceptConstructibleFrom<T, U>
		{
			while(!increment_size()) {
				// block until we won't overwrite
			}

			const auto write = m_write.fetch_add(1, std::memory_order_acquire) % m_capacity;

			m_buffer[write] = std::forward<U>(value);

			ignore(m_max_read.fetch_add(1, std::memory_order_release));
		}

		/// @brief Inserts the given element at the end of the `LockFreeQueue` if the queue is not
		/// full
		///
		/// @param value - the element to insert
		///
		/// @return `true` if successfully pushed, `false` if the queue was full
		template<typename U>
		requires concepts::Same<T, std::remove_cvref_t<U>>
				 inline constexpr auto try_push_back(U&& value) noexcept -> bool
				 requires concepts::NoexceptAssignable<T, U>
		{
			if(!increment_size()) {
				return false;
			}

			const auto write = m_write.fetch_add(1, std::memory_order_acquire) % m_capacity;

			m_buffer[write] = std::forward<U>(value);

			ignore(m_max_read.fetch_add(1, std::memory_order_release));
			return true;
		}

		/// @brief Inserts the given element at the end of the `LockFreeQueue`, overwriting the
		/// most recent entry if the queue is full
		///
		/// @param value - the element to insert
		template<typename U>
		requires concepts::Same<T, std::remove_cvref_t<U>>
				 inline constexpr auto force_push_back(U&& value) noexcept -> void
				 requires concepts::NoexceptAssignable<T, U>
		{
			auto write = 0_u32;
			if(!increment_size()) {
				write = m_max_read.fetch_sub(1, std::memory_order_acquire) % m_capacity;
			}
			else {
				write = m_write.fetch_add(1, std::memory_order_acquire) % m_capacity;
			}

			m_buffer[write] = std::forward<U>(value);

			ignore(m_max_read.fetch_add(1, std::memory_order_release));
		}

		/// @brief Constructs the given element in place at the end of the `LockFreeQueue`
		/// @note if `size() == capacity()` then this blocks until an element has been removed
		/// (opening up a space for the to-be-pushed entry)
		///
		/// @tparam Args - The types of the element's constructor arguments
		/// @param args - The constructor arguments for the element
		template<typename... Args>
		requires concepts::NoexceptConstructibleFrom<T, Args...>
		inline constexpr auto emplace_back(Args&&... args) noexcept -> void {

			while(!increment_size()) {
				// block until we won't overwrite
			}

			const auto write = m_write.fetch_add(1, std::memory_order_acquire) % m_capacity;

			m_buffer[write] = T(std::forward<Args>(args)...);

			ignore(m_max_read.fetch_add(1, std::memory_order_release));
		}

		/// @brief Constructs the given element in place at the end of the `LockFreeQueue` if the
		/// queue was not full
		///
		/// @tparam Args - The types of the element's constructor arguments
		/// @param args - The constructor arguments for the element
		///
		/// @return `true` if successfully pushed, `false` if the queue was full
		template<typename... Args>
		requires concepts::NoexceptConstructibleFrom<T, Args...>
		inline constexpr auto
		try_emplace_back(Args&&... args) noexcept -> bool {
			if(!increment_size()) {
				return false;
			}

			const auto write = m_write.fetch_add(1, std::memory_order_acquire) % m_capacity;

			m_buffer[write] = T(std::forward<Args>(args)...);

			ignore(m_max_read.fetch_add(1, std::memory_order_release));
			return true;
		}

		/// @brief Constructs the given element in place at the end of the `LockFreeQueue`,
		/// overwriting the last element in the queue if the queue was full
		///
		/// @tparam Args - The types of the element's constructor arguments
		/// @param args - The constructor arguments for the element
		template<typename... Args>
		requires concepts::NoexceptConstructibleFrom<T, Args...>
		inline constexpr auto force_emplace_back(Args&&... args) noexcept -> void {
			auto write = 0_u32;
			if(!increment_size()) {
				write = m_max_read.fetch_sub(1, std::memory_order_acquire) % m_capacity;
			}
			else {
				write = m_write.fetch_add(1, std::memory_order_acquire) % m_capacity;
			}

			m_buffer[write] = T(std::forward<Args>(args)...);

			ignore(m_max_read.fetch_add(1, std::memory_order_release));
		}

		/// @brief Returns the first element in the `LockFreeQueue`
		///
		/// @return The first element
		[[nodiscard]] inline constexpr auto front() noexcept -> Option<T> {
			if(m_read >= m_max_read.load(std::memory_order_acquire)) {
				return None();
			}

			return Some(m_buffer[m_read]);
		}

		/// @brief Removes the first element in the `LockFreeQueue` and returns it
		///
		/// @return The first element in the `LockFreeQueue`
		[[nodiscard]] inline constexpr auto pop_front() noexcept -> hyperion::Option<T>
		requires concepts::NoexceptMoveConstructible<T>
		{
			if(m_read >= m_max_read.load(std::memory_order_acquire)) {
				return None();
			}

			const auto read = m_read % m_capacity;
			m_read++;
			auto ret = Some(std::move(m_buffer[read]));
			ignore(m_size.fetch_sub(1, std::memory_order_release));
			return ret;
		}

		constexpr auto operator=(const LockFreeQueue& buffer) noexcept -> LockFreeQueue&
		requires concepts::NoexceptCopyConstructible<T>
		{
			std::atomic_thread_fence(std::memory_order_acquire);
			if(this == &buffer) {
				return *this;
			}
			auto temp = hyperion::allocate_unique<T[]>(m_allocator, buffer.m_capacity); // NOLINT
			const auto size = buffer.size();

			const auto buf_read = buffer.m_indices.get_read();
			for(auto index = 0_usize; index < size; ++index) {
				allocator_traits::construct(m_allocator,
											std::addressof(temp[index]),
											buffer[index]);
			}

			m_buffer = std::move(temp);
			m_capacity = buffer.m_capacity;
			set_all(0_u32, size, size);
			std::atomic_thread_fence(std::memory_order_release);
			return *this;
		}

		constexpr auto operator=(LockFreeQueue&& buffer) noexcept -> LockFreeQueue& {
			std::atomic_thread_fence(std::memory_order_acquire);
			m_allocator = buffer.m_allocator;
			m_buffer = std::move(buffer.m_buffer);
			const auto [read, write, size] = buffer.get_all();
			set_all(read, write, size);
			m_capacity = buffer.m_capacity;
			buffer.m_buffer = nullptr;
			buffer.set_all(0_u32, 0_u32, 0_u32);
			buffer.m_capacity = 0_usize;
			std::atomic_thread_fence(std::memory_order_release);
			return *this;
		}

	  private:
		u32 m_read = 0_u32;
		std::atomic<u32> m_write = 0_u32;
		std::atomic<u32> m_max_read = 0_u32;
		std::atomic<u32> m_size = 0_u32;

		[[nodiscard]] inline auto get_all() const noexcept -> std::tuple<u32, u32, u32> {
			return {m_read, m_write.load(), m_size.load()};
		}

		// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
		inline auto set_all(u32 read, u32 write, u32 size) noexcept -> void {
			std::atomic_thread_fence(std::memory_order_acquire);
			m_read = read;
			m_max_read.store(size, std::memory_order_relaxed);
			m_write.store(write, std::memory_order_relaxed);
			m_size.store(size, std::memory_order_release);
		}

		inline auto increment_size() noexcept -> bool {
			if(const auto _size = m_size.fetch_add(1, std::memory_order_acquire);
				_size >= m_capacity) {
				m_size.fetch_sub(1, std::memory_order_release);
				return false;
			}

			return true;
		}

		[[no_unique_address]] Allocator<T> m_allocator = Allocator<T>();
		unique_pointer m_buffer = hyperion::allocate_unique<T[]>(m_allocator, // NOLINT
															DEFAULT_CAPACITY);
		u32 m_capacity = DEFAULT_CAPACITY;
	};

	IGNORE_PADDING_STOP

	namespace detail::lock_free_queue::test {
		IGNORE_UNUSED_MEMBER_FUNCTIONS_START
		class TestClass {
		  public:
			TestClass() noexcept = default;
			explicit TestClass(i32 val) noexcept : m_ptr(std::make_shared<i32>(val)) {
			}
			TestClass(const TestClass&) noexcept = default;
			TestClass(TestClass&&) noexcept = default;
			~TestClass() noexcept = default;
			auto operator=(const TestClass&) noexcept -> TestClass& = default;
			auto operator=(TestClass&&) noexcept -> TestClass& = default;

			auto operator=(i32 val) noexcept -> TestClass& {
				if(m_ptr != nullptr) {
					*m_ptr = val;
				}
				else {
					m_ptr = std::make_shared<i32>(val);
				}

				return *this;
			}

			// NOLINTNEXTLINE(google-explicit-constructor, hicpp-explicit-conversions))
			explicit(false) operator i32() const noexcept {
				return *m_ptr;
			}

			auto operator<=>(const TestClass& rhs) const noexcept -> std::strong_ordering {
				return *m_ptr <=> *(rhs.m_ptr);
			}

			auto operator<=>(i32 rhs) const noexcept -> std::strong_ordering {
				return *m_ptr <=> rhs;
			}

			auto operator==(const TestClass& rhs) const noexcept -> bool {
				return (*this <=> rhs) == std::strong_ordering::equal;
			}

			auto operator!=(const TestClass& rhs) const noexcept -> bool {
				return (*this <=> rhs) != std::strong_ordering::equal;
			}

			auto operator==(i32 rhs) const noexcept -> bool {
				return (*this <=> rhs) == std::strong_ordering::equal;
			}

			auto operator!=(i32 rhs) const noexcept -> bool {
				return (*this <=> rhs) != std::strong_ordering::equal;
			}

			std::shared_ptr<i32> m_ptr;
		};
		IGNORE_UNUSED_MEMBER_FUNCTIONS_STOP

	} // namespace detail::lock_free_queue::test

//	// NOLINTNEXTLINE
//	TEST_SUITE("LockFreeQueue") {
//		// NOLINTNEXTLINE
//		TEST_CASE("DefaultConstructed") {
//			auto buffer = LockFreeQueue<detail::lock_free_queue::test::TestClass>();
//
//			constexpr auto capacity
//				= LockFreeQueue<detail::lock_free_queue::test::TestClass>::DEFAULT_CAPACITY;
//
//			SUBCASE("accessors") {
//				CHECK_EQ(buffer.size(), 0_usize);
//				CHECK_EQ(buffer.capacity(), capacity);
//				CHECK(buffer.empty());
//			}
//
//			SUBCASE("push_back") {
//				for(auto i = 0_i32; i < static_cast<i32>(capacity); ++i) {
//					buffer.push_back(detail::lock_free_queue::test::TestClass(i));
//				}
//
//				SUBCASE("at") {
//					for(auto i = 0_i32; i < static_cast<i32>(capacity); ++i) {
//						auto front = buffer.pop_front();
//						CHECK(front.is_some());
//						CHECK_EQ(front.unwrap(), i);
//					}
//				}
//
//				SUBCASE("looping") {
//					for(auto i = static_cast<i32>(capacity) - 1_i32; i >= 0; --i) {
//						buffer.push_back(detail::lock_free_queue::test::TestClass(i));
//					}
//
//					auto value = static_cast<i32>(capacity) - 1_i32;
//					for(auto i = 0_usize; i < capacity; ++i, --value) {
//						auto front = buffer.pop_front();
//						CHECK(front.is_some());
//						CHECK_EQ(front.unwrap(), value);
//					}
//				}
//			}
//
//			SUBCASE("emplace_back") {
//				for(auto i = 0_i32; i < static_cast<i32>(capacity); ++i) {
//					buffer.emplace_back(i);
//				}
//
//				SUBCASE("at") {
//					for(auto i = 0_i32; i < static_cast<i32>(capacity); ++i) {
//						auto front = buffer.pop_front();
//						CHECK(front.is_some());
//						CHECK_EQ(front.unwrap(), i);
//					}
//				}
//
//				SUBCASE("looping") {
//					for(auto i = static_cast<i32>(capacity) - 1_i32; i >= 0; --i) {
//						buffer.emplace_back(i);
//					}
//
//					auto value = static_cast<i32>(capacity) - 1_i32;
//					for(auto i = 0_usize; i < capacity; ++i, --value) {
//						auto front = buffer.pop_front();
//						CHECK(front.is_some());
//						CHECK_EQ(front.unwrap(), value);
//					}
//				}
//			}
//
//			SUBCASE("reserve") {
//				for(auto i = 0_i32; i < static_cast<i32>(capacity); ++i) {
//					buffer.emplace_back(i);
//				}
//
//				const auto new_capacity = capacity * 2_usize;
//				buffer.reserve(new_capacity);
//
//				CHECK_EQ(buffer.capacity(), new_capacity);
//				CHECK_EQ(buffer.size(), capacity);
//			}
//
//			SUBCASE("front") {
//				buffer.emplace_back(2);
//				CHECK_EQ(buffer.front().as_cref(), 2);
//			}
//
//			SUBCASE("pop_front") {
//				buffer.emplace_back(1);
//				buffer.emplace_back(2);
//
//				CHECK_EQ(buffer.size(), 2_usize);
//				CHECK_EQ(buffer.front().as_cref(), 1);
//
//				auto front = buffer.pop_front();
//
//				CHECK(front.is_some());
//				CHECK_EQ(front.unwrap(), 1);
//
//				CHECK_EQ(buffer.size(), 1_usize);
//				CHECK_EQ(buffer.front().as_cref(), 2);
//			}
//		}
//	}

} // namespace hyperion
