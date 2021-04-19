#pragma once

#include "../Concepts.h"
#include "Entry.h"

namespace hyperion {
	/// @brief Base CRTP for logging sink types
	///
	/// @tparam T - The sink type
	template<typename T>
	class SinkBase;

	/// @brief Requirements for what constitutes a log sink type
	template<typename T>
	concept SinkType = requires(T val, const Entry& entry_lval, Entry&& entry_rval) {
		requires concepts::Derived<T, SinkBase<T>>;

		{
			val.sink_entry(entry_lval)
		}
		noexcept->concepts::Same<void>;

		{
			val.sink_entry(entry_rval)
		}
		noexcept->concepts::Same<void>;
	};

	/// @brief Base CRTP for logging sink types
	///
	/// @tparam T - The sink type
	template<typename T>
	class SinkBase {
	  public:
		/// @brief Sinks the given log entry,
		/// writing it to the output location associated with this sink.
		///
		/// @param entry - The log entry to sink
		inline constexpr auto sink(const Entry& entry) noexcept -> void {
			underlying().sink_entry(entry);
		}

		/// @brief Sinks the given log entry,
		/// writing it to the output location associated with this sink.
		///
		/// @param entry - The log entry to sink
		inline constexpr auto sink(Entry&& entry) noexcept -> void {
			underlying().sink_entry(std::forward<Entry>(entry));
		}

	  private:
		[[nodiscard]] inline constexpr auto underlying() const noexcept -> const SinkType auto& {
			return static_cast<const T&>(*this);
		}

		[[nodiscard]] inline constexpr auto underlying() noexcept -> SinkType auto& {
			return static_cast<T&>(*this);
		}

		constexpr SinkBase() noexcept = default;
		constexpr SinkBase(const SinkBase& sink) noexcept = default;
		constexpr SinkBase(SinkBase&& sink) noexcept = default;
		constexpr ~SinkBase() noexcept = default;
		constexpr auto operator=(const SinkBase& sink) noexcept -> SinkBase& = default;
		constexpr auto operator=(SinkBase&& sink) noexcept -> SinkBase& = default;

		friend T;
	};
} // namespace hyperion
