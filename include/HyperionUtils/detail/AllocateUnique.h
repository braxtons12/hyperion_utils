/// @brief Implementation of `allocate_unique` for `std::unique_ptr`. Should be analogous to
///

#pragma once

#include <memory>

#include "../BasicTypes.h"
#include "../Concepts.h"
#include "../Macros.h"

namespace hyperion::utils::detail {

	/// @brief Concept that determines if `T` is allocatable by `Allocator`
	template<typename T,
			 typename Allocator = std::allocator<T>,
			 typename U = std::remove_cv_t<std::remove_all_extents_t<T>>>
	concept Allocatable = requires(Allocator alloc) {
		// clang-format off
		concepts::Same<std::decay_t<U>,typename std::allocator_traits<Allocator>::value_type> ||
			concepts::Derived<U, typename std::allocator_traits<Allocator>::value_type>;
		// clang-format on
		std::allocator_traits<Allocator>::allocate(alloc, 1_usize);
	};

	IGNORE_PADDING_START
	/// @brief Custom deleter class for `std::unique_ptr` when `T` is a single element or
	/// compile-time 1-dimensional array
	///
	/// @tparam T - The allocated type
	/// @tparam Allocator - The allocator to use
	// clang-format off
	template<typename T,
			 usize N,
			 typename Allocator = std::allocator<T>,
			 typename U = std::remove_cv_t<std::remove_all_extents_t<T>>>
	requires Allocatable<U, Allocator> &&(N != 0)
	class UniqueDeleterStaticSize {
	//clang-format on
	  public:
		using Alloc = typename std::allocator_traits<Allocator>::template rebind_alloc<U>;
		using traits = std::allocator_traits<Alloc>;
		using pointer = typename traits::pointer;

		UniqueDeleterStaticSize() = delete;
		constexpr explicit UniqueDeleterStaticSize(const Allocator& allocator) noexcept
			: m_allocator(allocator) {
		}
		constexpr UniqueDeleterStaticSize(
			const UniqueDeleterStaticSize& deleter) noexcept = default;
		constexpr UniqueDeleterStaticSize(UniqueDeleterStaticSize&& deleter) noexcept = default;
		constexpr ~UniqueDeleterStaticSize() noexcept = default;

		inline constexpr auto operator()(pointer p) const {
			Alloc allocator(m_allocator);
			auto i = N >= 1 ? N - 1 : 0;
			do {
				traits::destroy(allocator, std::addressof(*p) + i);
				--i;
			} while(i != 0);

			traits::deallocate(allocator, p, N);
		}

		// clang-format off
		constexpr auto operator=(const UniqueDeleterStaticSize& deleter) noexcept
			-> UniqueDeleterStaticSize& = default;
		constexpr auto operator=(UniqueDeleterStaticSize&& deleter) noexcept
			-> UniqueDeleterStaticSize& = default;
		// clang-format on

	  private:
		Alloc m_allocator;
	};

	/// @brief Custom deleter class for `std::unique_ptr` when `T` is an element in
	/// a run-time 1-dimensional array
	///
	/// @tparam T - The allocated type
	/// @tparam Allocator - The allocator to use
	template<typename T,
			 typename Allocator = std::allocator<T>,
			 typename U = std::remove_cv_t<std::remove_all_extents_t<T>>>
	requires Allocatable<U, Allocator>
	class UniqueDeleterDynSize {
	  public:
		using Alloc = typename std::allocator_traits<Allocator>::template rebind_alloc<U>;
		using traits = std::allocator_traits<Alloc>;
		using pointer = typename traits::pointer;

		UniqueDeleterDynSize() = delete;
		constexpr UniqueDeleterDynSize(const Allocator& allocator, usize num_elements) noexcept
			: m_allocator(allocator), m_num_elements(num_elements) {
		}
		constexpr UniqueDeleterDynSize(const UniqueDeleterDynSize& deleter) noexcept = default;
		constexpr UniqueDeleterDynSize(UniqueDeleterDynSize&& deleter) noexcept = default;
		constexpr ~UniqueDeleterDynSize() noexcept = default;

		inline constexpr auto operator()(pointer p) const {
			Alloc allocator(m_allocator);
			auto i = m_num_elements >= 1 ? m_num_elements - 1 : 0;
			do {
				traits::destroy(allocator, std::addressof(*p) + i);
				--i;
			} while(i != 0);

			traits::deallocate(allocator, p, m_num_elements);
		}

		// clang-format off
		constexpr auto operator=(const UniqueDeleterDynSize& deleter) noexcept
			-> UniqueDeleterDynSize& = default;
		constexpr auto operator=(UniqueDeleterDynSize&& deleter) noexcept
			-> UniqueDeleterDynSize& = default;
		// clang-format on

	  private:
		Alloc m_allocator;
		usize m_num_elements = 1;
	};

	/// @brief Allocates and constructs new `std::unique_ptr<T>`,
	/// where `T` is a compile-time 1-dimensional array
	///
	/// @tparam T - The type to allocate
	/// @tparam Allocator - The allocator type to use
	/// @param alloc - The allocator to use
	/// @param args - The arguments to construct the `T`, if `T` is a single element, or the
	/// arguments to construct the default value for all elements in the array if `T` is an array
	///
	/// @return a new `std::unique_ptr`
	template<typename T,
			 usize N,
			 typename Allocator = std::allocator<T>,
			 typename... Args,
			 typename U = std::remove_cv_t<std::remove_all_extents_t<T>>>
	requires concepts::ConstructibleFrom<U, Args...> && Allocatable<U, Allocator> &&(N != 0)
		[[nodiscard]] inline constexpr auto allocate_unique(const Allocator& alloc, Args&&... args)
			-> std::unique_ptr<T[N], UniqueDeleterStaticSize<T, N, Allocator>> { // NOLINT

		using Alloc = typename std::allocator_traits<Allocator>::template rebind_alloc<U>;
		using traits = std::allocator_traits<Alloc>;
		using Deleter = UniqueDeleterStaticSize<T, N, Allocator>;

		Alloc allocator(alloc);
		auto p = traits::allocate(allocator, N);

		try {
			auto i = 0_usize;
			do {
				traits::construct(allocator, std::addressof(*p) + i, std::forward<Args>(args)...);
				i++;
			} while(i < N);

			return std::unique_ptr<T[N], Deleter>(p, Deleter(allocator)); // NOLINT
		}
		catch(std::exception& e) {
			traits::deallocate(allocator, p, N);
			throw e;
		}
		catch(...) {
			traits::deallocate(allocator, p, N);
			throw;
		}
	}

	/// @brief Allocates and constructs new `std::unique_ptr<T>`,
	/// where `T` is a compile-time 1-dimensional array
	///
	/// @tparam T - The type to allocate
	/// @tparam Allocator - The allocator type to use
	/// @param alloc - The allocator to use
	///
	/// @return a new `std::unique_ptr`
	template<typename T,
			 usize N,
			 typename Allocator = std::allocator<T>,
			 typename U = std::remove_cv_t<std::remove_all_extents_t<T>>>
	requires concepts::DefaultConstructible<U> && Allocatable<U, Allocator> &&(N != 0)
		[[nodiscard]] inline constexpr auto allocate_unique(const Allocator& alloc)
			-> std::unique_ptr<T[N], UniqueDeleterStaticSize<T, N, Allocator>> { // NOLINT

		using Alloc = typename std::allocator_traits<Allocator>::template rebind_alloc<U>;
		using traits = std::allocator_traits<Alloc>;
		using Deleter = UniqueDeleterStaticSize<T, N, Allocator>;

		Alloc allocator(alloc);
		auto p = traits::allocate(allocator, N);

		try {
			auto i = 0_usize;
			do {
				traits::construct(allocator, std::addressof(*p) + i);
				i++;
			} while(i < N);

			return std::unique_ptr<T[N], Deleter>(p, Deleter(allocator)); // NOLINT
		}
		catch(std::exception& e) {
			traits::deallocate(allocator, p, N);
			throw e;
		}
		catch(...) {
			traits::deallocate(allocator, p, N);
			throw;
		}
	}

	/// @brief Allocates and default-constructs a new `std::unique_ptr<T>`,
	/// where `T` is a single element or a run-time 1-dimensional array
	///
	/// @tparam T - The type to allocate
	/// @tparam Allocator - The allocator type to use
	/// @param alloc - The allocator to use
	/// @param N - The number of elements in the array
	/// @param args - The arguments to construct the default value for all elements in the array
	///
	/// @return a new `std::unique_ptr`
	// clang-format off
	template<typename T,
			 typename Allocator = std::allocator<T>,
			 typename... Args,
			 typename U = std::remove_cv_t<std::remove_all_extents_t<T>>>
	requires concepts::ConstructibleFrom<U, Args...> && Allocatable<U, Allocator>
	[[nodiscard]] inline constexpr auto
	allocate_unique(const Allocator& alloc, usize N, Args&&... args)
		-> std::unique_ptr<T, UniqueDeleterDynSize<T,
								typename std::allocator_traits<Allocator>::template rebind_alloc<U>>>
	{
		// clang-format on

		using Alloc = typename std::allocator_traits<Allocator>::template rebind_alloc<U>;
		using traits = std::allocator_traits<Alloc>;
		using Deleter = UniqueDeleterDynSize<T, Alloc>;

		Alloc allocator(alloc);
		auto p = traits::allocate(allocator, N);

		try {
			auto i = 0_usize;
			do {
				traits::construct(allocator, std::addressof(*p) + i, std::forward<Args>(args)...);
				++i;
			} while(i < N);

			return std::unique_ptr<T, Deleter>(p, Deleter(allocator, N));
		}
		catch(std::exception& e) {
			traits::deallocate(allocator, p, N);
			throw e;
		}
		catch(...) {
			traits::deallocate(allocator, p, N);
			throw;
		}
	}

	/// @brief Allocates and default-constructs a new `std::unique_ptr<T>`,
	/// where `T` is a single element or a run-time 1-dimensional array
	///
	/// @tparam T - The type to allocate
	/// @tparam Allocator - The allocator type to use
	/// @param alloc - The allocator to use
	/// @param N - The number of elements in the array
	///
	/// @return a new `std::unique_ptr`
	// clang-format off
	template<typename T,
			 typename Allocator = std::allocator<T>,
			 typename U = std::remove_cv_t<std::remove_all_extents_t<T>>>
	requires concepts::DefaultConstructible<U> && Allocatable<U, Allocator>
	[[nodiscard]] inline constexpr auto
	allocate_unique(const Allocator& alloc, usize N)
		-> std::unique_ptr<T, UniqueDeleterDynSize<T,
								typename std::allocator_traits<Allocator>::template rebind_alloc<U>>>
	{
		// clang-format on

		using Alloc = typename std::allocator_traits<Allocator>::template rebind_alloc<U>;
		using traits = std::allocator_traits<Alloc>;
		using Deleter = UniqueDeleterDynSize<T, Alloc>;

		Alloc allocator(alloc);
		auto p = traits::allocate(allocator, N);

		try {
			auto i = 0_usize;
			do {
				traits::construct(allocator, std::addressof(*p) + i);
				++i;
			} while(i < N);

			return std::unique_ptr<T, Deleter>(p, Deleter(allocator, N));
		}
		catch(std::exception& e) {
			traits::deallocate(allocator, p, N);
			throw e;
		}
		catch(...) {
			traits::deallocate(allocator, p, N);
			throw;
		}
	}

	IGNORE_PADDING_STOP
} // namespace hyperion::utils::detail
