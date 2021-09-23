#pragma once

#include <string_view>
#include <variant>

#include "../Concepts.h"
#include "../Format.h"
#include "../HyperionDef.h"
#include "Config.h"

namespace hyperion {

	/// @brief Base CRTP for logging entry types
	///
	/// @tparam T - The entry type
	template<typename T>
	class EntryBase;

	/// @brief Requirements for what constitutes a log entry type
	template<typename T>
	concept EntryType = requires(T val) {
		requires concepts::Derived<T, EntryBase<T>>;

		{
			val.log_level()
		}
		noexcept->concepts::Same<LogLevel>;
		{
			val.log_style()
			} -> concepts::Same<fmt::text_style>;
		{
			val.log_entry()
		}
		noexcept->concepts::Same<std::string_view>;
	};

	/// @brief Base CRTP for logging entry types
	///
	/// @tparam T - The entry type
	template<typename T>
	class EntryBase {
	  public:
		/// @brief Returns the `LogLevel` associated with this entry
		///
		/// @return The `LogLevel` of this entry
		[[nodiscard]] inline constexpr auto level() const noexcept -> LogLevel {
			return underlying().log_level();
		}

		/// @brief Returns the text style of this entry
		///
		/// @return The `fmt::text_style` of this entry
		[[nodiscard]] inline constexpr auto style() const noexcept -> fmt::text_style { // NOLINT
			return underlying().log_style();
		}

		/// @brief Returns the text entry for this
		///
		/// @return The text entry
		[[nodiscard]] inline auto entry() const noexcept -> std::string_view {
			return underlying().log_entry();
		}

	  private:
		[[nodiscard]] inline constexpr auto underlying() const noexcept -> const EntryType auto& {
			return static_cast<const T&>(*this);
		}

		[[nodiscard]] inline constexpr auto underlying() noexcept -> EntryType auto& {
			return static_cast<T&>(*this);
		}

		constexpr EntryBase() noexcept = default;
		constexpr EntryBase(const EntryBase& entry) noexcept = default;
		constexpr EntryBase(EntryBase&& entry) noexcept = default;
		constexpr ~EntryBase() noexcept = default;
		constexpr auto operator=(const EntryBase& entry) noexcept -> EntryBase& = default;
		constexpr auto operator=(EntryBase&& entry) noexcept -> EntryBase& = default;

		friend T;
	};

	/// @brief Entry type for `LogLevel::MESSAGE` log entries
	class MessageEntry final : public EntryBase<MessageEntry> {
	  public:
		MessageEntry() noexcept = delete;
		explicit MessageEntry(const std::string_view& entry) noexcept : m_entry(entry) {
		}
		explicit MessageEntry(std::string_view&& entry) noexcept : m_entry(entry) {
		}
		template<typename... Args>
		explicit MessageEntry(fmt::format_string<Args...>&& format_string, Args&&... args) noexcept
			: m_entry(fmt::format(format_string, std::forward<Args>(args)...)) {
		}
		MessageEntry(const MessageEntry& entry) noexcept = default;
		MessageEntry(MessageEntry&& entry) noexcept = default;
		~MessageEntry() noexcept = default;

		/// @brief Returns the `LogLevel` associated with this entry
		///
		/// @return The `LogLevel` of this entry
		[[nodiscard]] inline constexpr auto log_level() const noexcept -> LogLevel { // NOLINT
			return LogLevel::MESSAGE;
		}

		/// @brief Returns the text style of this entry
		///
		/// @return The `fmt::text_style` of this entry
		[[nodiscard]] inline constexpr auto
		log_style() const noexcept -> fmt::text_style { // NOLINT
			return fmt::fg(fmt::color::white);
		}

		/// @brief Returns the text entry for this
		///
		/// @return The text entry
		[[nodiscard]] inline auto log_entry() const noexcept -> std::string_view {
			return m_entry;
		}

		auto operator=(const MessageEntry& entry) noexcept -> MessageEntry& = default;
		auto operator=(MessageEntry&& entry) noexcept -> MessageEntry& = default;

	  private:
		std::string m_entry;
	};

	/// @brief Entry type for `LogLevel::TRACE` log entries
	class TraceEntry final : public EntryBase<TraceEntry> {
	  public:
		TraceEntry() noexcept = delete;
		explicit TraceEntry(const std::string_view& entry) noexcept // NOLINT
			: m_entry(entry) {
		}
		explicit TraceEntry(std::string_view&& entry) noexcept : m_entry(entry) {
		}
		template<typename... Args>
		explicit TraceEntry(fmt::format_string<Args...>&& format_string, Args&&... args) noexcept
			: m_entry(fmt::format(format_string, std::forward<Args>(args)...)) {
		}
		TraceEntry(const TraceEntry& entry) noexcept = default;
		TraceEntry(TraceEntry&& entry) noexcept = default;
		~TraceEntry() noexcept = default;

		/// @brief Returns the `LogLevel` associated with this entry
		///
		/// @return The `LogLevel` of this entry
		[[nodiscard]] inline constexpr auto log_level() const noexcept -> LogLevel { // NOLINT
			return LogLevel::TRACE;
		}

		/// @brief Returns the text style of this entry
		///
		/// @return The `fmt::text_style` of this entry
		[[nodiscard]] inline constexpr auto
		log_style() const noexcept -> fmt::text_style { // NOLINT
			return fmt::fg(fmt::color::steel_blue);
		}

		/// @brief Returns the text entry for this
		///
		/// @return The text entry
		[[nodiscard]] inline auto log_entry() const noexcept -> std::string_view {
			return m_entry;
		}

		auto operator=(const TraceEntry& entry) noexcept -> TraceEntry& = default;
		auto operator=(TraceEntry&& entry) noexcept -> TraceEntry& = default;

	  private:
		std::string m_entry;
	};

	/// @brief Entry type for `LogLevel::INFO` log entries
	class InfoEntry final : public EntryBase<InfoEntry> {
	  public:
		InfoEntry() noexcept = delete;
		explicit InfoEntry(const std::string_view& entry) noexcept // NOLINT
			: m_entry(entry) {
		}
		explicit InfoEntry(std::string_view&& entry) noexcept : m_entry(entry) {
		}
		template<typename... Args>
		explicit InfoEntry(fmt::format_string<Args...>&& format_string, Args&&... args) noexcept
			: m_entry(fmt::format(format_string, std::forward<Args>(args)...)) {
		}
		InfoEntry(const InfoEntry& entry) noexcept = default;
		InfoEntry(InfoEntry&& entry) noexcept = default;
		~InfoEntry() noexcept = default;

		/// @brief Returns the `LogLevel` associated with this entry
		///
		/// @return The `LogLevel` of this entry
		[[nodiscard]] inline constexpr auto log_level() const noexcept -> LogLevel { // NOLINT
			return LogLevel::INFO;
		}

		/// @brief Returns the text style of this entry
		///
		/// @return The `fmt::text_style` of this entry
		[[nodiscard]] inline constexpr auto
		log_style() const noexcept -> fmt::text_style { // NOLINT
			return fmt::fg(fmt::color::light_green) | fmt::emphasis::italic;
		}

		/// @brief Returns the text entry for this
		///
		/// @return The text entry
		[[nodiscard]] inline auto log_entry() const noexcept -> std::string_view {
			return m_entry;
		}

		auto operator=(const InfoEntry& entry) noexcept -> InfoEntry& = default;
		auto operator=(InfoEntry&& entry) noexcept -> InfoEntry& = default;

	  private:
		std::string m_entry;
	};

	/// @brief Entry type for `LogLevel::WARN` log entries
	class WarnEntry final : public EntryBase<WarnEntry> {
	  public:
		WarnEntry() noexcept = delete;
		explicit WarnEntry(const std::string_view& entry) noexcept // NOLINT
			: m_entry(entry) {
		}
		explicit WarnEntry(std::string_view&& entry) noexcept : m_entry(entry) {
		}
		template<typename... Args>
		explicit WarnEntry(fmt::format_string<Args...>&& format_string, Args&&... args) noexcept
			: m_entry(fmt::format(format_string, std::forward<Args>(args)...)) {
		}
		WarnEntry(const WarnEntry& entry) noexcept = default;
		WarnEntry(WarnEntry&& entry) noexcept = default;
		~WarnEntry() noexcept = default;

		/// @brief Returns the `LogLevel` associated with this entry
		///
		/// @return The `LogLevel` of this entry
		[[nodiscard]] inline constexpr auto log_level() const noexcept -> LogLevel { // NOLINT
			return LogLevel::INFO;
		}

		/// @brief Returns the text style of this entry
		///
		/// @return The `fmt::text_style` of this entry
		[[nodiscard]] inline constexpr auto
		log_style() const noexcept -> fmt::text_style { // NOLINT
			return fmt::fg(fmt::color::orange) | fmt::emphasis::bold;
		}

		/// @brief Returns the text entry for this
		///
		/// @return The text entry
		[[nodiscard]] inline auto log_entry() const noexcept -> std::string_view {
			return m_entry;
		}

		auto operator=(const WarnEntry& entry) noexcept -> WarnEntry& = default;
		auto operator=(WarnEntry&& entry) noexcept -> WarnEntry& = default;

	  private:
		std::string m_entry;
	};

	/// @brief Entry type for `LogLevel::ERROR` log entries
	class ErrorEntry final : public EntryBase<ErrorEntry> {
	  public:
		ErrorEntry() noexcept = delete;
		explicit ErrorEntry(const std::string_view& entry) noexcept // NOLINT
			: m_entry(entry) {
		}
		explicit ErrorEntry(std::string_view&& entry) noexcept : m_entry(entry) {
		}
		template<typename... Args>
		explicit ErrorEntry(fmt::format_string<Args...>&& format_string, Args&&... args) noexcept
			: m_entry(fmt::format(format_string, std::forward<Args>(args)...)) {
		}
		ErrorEntry(const ErrorEntry& entry) noexcept = default;
		ErrorEntry(ErrorEntry&& entry) noexcept = default;
		~ErrorEntry() noexcept = default;

		/// @brief Returns the `LogLevel` associated with this entry
		///
		/// @return The `LogLevel` of this entry
		[[nodiscard]] inline constexpr auto log_level() const noexcept -> LogLevel { // NOLINT
			return LogLevel::INFO;
		}

		/// @brief Returns the text style of this entry
		///
		/// @return The `fmt::text_style` of this entry
		[[nodiscard]] inline constexpr auto
		log_style() const noexcept -> fmt::text_style { // NOLINT
			return fmt::fg(fmt::color::red) | fmt::emphasis::bold;
		}

		/// @brief Returns the text entry for this
		///
		/// @return The text entry
		[[nodiscard]] inline auto log_entry() const noexcept -> std::string_view {
			return m_entry;
		}

		auto operator=(const ErrorEntry& entry) noexcept -> ErrorEntry& = default;
		auto operator=(ErrorEntry&& entry) noexcept -> ErrorEntry& = default;

	  private:
		std::string m_entry;
	};

	/// @brief Compile-time polymorphic type encapsulating the various possible log entry types.
	/// This can be one of `MessageEntry`, `TraceEntry`, `InfoEntry`, `WarnEntry`, `ErrorEntry`
	/// at any one point in time, and will dispatch to the correct type when queried.
	///
	/// TODO: replace std::variant with our own custom variant-like type that can't be valueless
	class Entry {
	  public:
		using variant_type
			= std::variant<MessageEntry, TraceEntry, InfoEntry, WarnEntry, ErrorEntry>;

		Entry() noexcept : m_inner(MessageEntry("DefaultMessage\n")) {
		}
		explicit Entry(const EntryType auto& entry) noexcept : m_inner(entry) {
		}
		explicit Entry(				// NOLINT(bugprone-forwarding-reference-overload)
			EntryType auto&& entry) // the forwarding reference here is fine because it's
			noexcept				// constrained by the concept
			: m_inner(std::forward<decltype(entry)>(entry)) {
		}
		template<EntryType T, typename... Args>
		explicit Entry(std::in_place_type_t<T> tag,
					   fmt::format_string<Args...>&& fmt,
					   Args&&... args) noexcept
			: m_inner(tag, std::move(fmt), std::forward<Args>(args)...) {
		}
		Entry(const Entry& entry) noexcept = default;
		Entry(Entry&& entry) noexcept = default;
		~Entry() noexcept = default;

		/// @brief Returns the `LogLevel` associated with this entry
		///
		/// @return The `LogLevel` of this entry
		[[nodiscard]] inline constexpr auto level() const noexcept -> LogLevel {
			return std::visit([](const auto& entry) { return entry.level(); }, m_inner);
		}

		/// @brief Returns the text style of this entry
		///
		/// @return The `fmt::text_style` of this entry
		[[nodiscard]] inline constexpr auto style() const noexcept -> fmt::text_style {
			return std::visit([](const auto& entry) { return entry.style(); }, m_inner);
		}

		/// @brief Returns the text entry for this
		///
		/// @return The text entry
		[[nodiscard]] inline auto entry() const noexcept -> std::string_view {
			return std::visit([](const auto& entry) { return entry.entry(); }, m_inner);
		}

		[[nodiscard]] inline constexpr auto valid() const noexcept -> bool {
			return !m_inner.valueless_by_exception();
		}

		auto operator=(const Entry& entry) noexcept -> Entry& = default;
		auto operator=(Entry&& entry) noexcept -> Entry& = default;

	  private:
		variant_type m_inner = variant_type(MessageEntry("DefaultMessage\n"));
	};

	using concepts::ConstructibleFrom;

	/// @brief Constructs an Entry with the given underlying entry type from `args`
	///
	/// @tparam T - The underlying entry type
	/// @tparam Args - The constructor arguments types
	///
	/// @return an `Entry`
	template<EntryType T, typename... Args>
	// requires ConstructibleFrom<T, Args...>
	inline auto make_entry(fmt::format_string<Args...>&& fmt, Args&&... args) noexcept -> Entry {
		return Entry(std::in_place_type_t<T>(), std::move(fmt), std::forward<Args>(args)...);
	}

	/// @brief Tag type for logging level of an entry
	///
	/// @tparam Level - The logging level
	template<LogLevel Level>
	struct entry_level { };

	/// @brief Alias for `entry_level<Level>::type`
	///
	/// @tparam Level - The logging level
	template<LogLevel Level>
	using entry_level_t = typename entry_level<Level>::type;

	template<>
	struct entry_level<LogLevel::MESSAGE> {
		using type = MessageEntry;
	};

	template<>
	struct entry_level<LogLevel::TRACE> {
		using type = TraceEntry;
	};

	template<>
	struct entry_level<LogLevel::INFO> {
		using type = InfoEntry;
	};

	template<>
	struct entry_level<LogLevel::WARN> {
		using type = WarnEntry;
	};

	template<>
	struct entry_level<LogLevel::ERROR> {
		using type = ErrorEntry;
	};
} // namespace hyperion
