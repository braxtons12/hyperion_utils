#pragma once

#include <variant>

#include "../Concepts.h"
#include "../Macros.h"
#include "Config.h"
#include "fmtIncludes.h"

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
		[[nodiscard]] inline constexpr auto entry() const noexcept -> std::string_view {
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
		explicit HYPERION_CONSTEXPR_STRINGS
		MessageEntry(const std::string& entry) noexcept // NOLINT
			: m_entry(entry) {
		}
		explicit HYPERION_CONSTEXPR_STRINGS MessageEntry(std::string&& entry) noexcept
			: m_entry(std::forward<std::string>(entry)) {
		}
		template<typename S, typename... Args, typename Char = fmt::char_t<S>>
		explicit HYPERION_CONSTEXPR_STRINGS
		MessageEntry(const S& format_string, Args&&... args) noexcept
			: m_entry(fmt::format(format_string, std::forward<Args>(args)...)) {
		}
		HYPERION_CONSTEXPR_STRINGS MessageEntry(const MessageEntry& entry) noexcept = default;
		HYPERION_CONSTEXPR_STRINGS MessageEntry(MessageEntry&& entry) noexcept = default;
		HYPERION_CONSTEXPR_STRINGS ~MessageEntry() noexcept = default;

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
		[[nodiscard]] inline constexpr auto log_entry() const noexcept -> std::string_view {
			return m_entry;
		}

		HYPERION_CONSTEXPR_STRINGS auto
		operator=(const MessageEntry& entry) noexcept -> MessageEntry& = default;
		HYPERION_CONSTEXPR_STRINGS auto
		operator=(MessageEntry&& entry) noexcept -> MessageEntry& = default;

	  private:
		std::string m_entry;
	};

	/// @brief Entry type for `LogLevel::TRACE` log entries
	class TraceEntry final : public EntryBase<TraceEntry> {
	  public:
		TraceEntry() noexcept = delete;
		explicit HYPERION_CONSTEXPR_STRINGS TraceEntry(const std::string& entry) noexcept // NOLINT
			: m_entry(entry) {
		}
		explicit HYPERION_CONSTEXPR_STRINGS TraceEntry(std::string&& entry) noexcept
			: m_entry(std::forward<std::string>(entry)) {
		}
		template<typename S, typename... Args, typename Char = fmt::char_t<S>>
		explicit HYPERION_CONSTEXPR_STRINGS
		TraceEntry(const S& format_string, Args&&... args) noexcept
			: m_entry(fmt::format(format_string, std::forward<Args>(args)...)) {
		}
		HYPERION_CONSTEXPR_STRINGS TraceEntry(const TraceEntry& entry) noexcept = default;
		HYPERION_CONSTEXPR_STRINGS TraceEntry(TraceEntry&& entry) noexcept = default;
		HYPERION_CONSTEXPR_STRINGS ~TraceEntry() noexcept = default;

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
		[[nodiscard]] inline constexpr auto log_entry() const noexcept -> std::string_view {
			return m_entry;
		}

		HYPERION_CONSTEXPR_STRINGS auto
		operator=(const TraceEntry& entry) noexcept -> TraceEntry& = default;
		HYPERION_CONSTEXPR_STRINGS auto
		operator=(TraceEntry&& entry) noexcept -> TraceEntry& = default;

	  private:
		std::string m_entry;
	};

	/// @brief Entry type for `LogLevel::INFO` log entries
	class InfoEntry final : public EntryBase<InfoEntry> {
	  public:
		InfoEntry() noexcept = delete;
		explicit HYPERION_CONSTEXPR_STRINGS InfoEntry(const std::string& entry) noexcept // NOLINT
			: m_entry(entry) {
		}
		explicit HYPERION_CONSTEXPR_STRINGS InfoEntry(std::string&& entry) noexcept
			: m_entry(std::forward<std::string>(entry)) {
		}
		template<typename S, typename... Args, typename Char = fmt::char_t<S>>
		explicit HYPERION_CONSTEXPR_STRINGS
		InfoEntry(const S& format_string, Args&&... args) noexcept
			: m_entry(fmt::format(format_string, std::forward<Args>(args)...)) {
		}
		HYPERION_CONSTEXPR_STRINGS InfoEntry(const InfoEntry& entry) noexcept = default;
		HYPERION_CONSTEXPR_STRINGS InfoEntry(InfoEntry&& entry) noexcept = default;
		HYPERION_CONSTEXPR_STRINGS ~InfoEntry() noexcept = default;

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
		[[nodiscard]] inline constexpr auto log_entry() const noexcept -> std::string_view {
			return m_entry;
		}

		HYPERION_CONSTEXPR_STRINGS auto
		operator=(const InfoEntry& entry) noexcept -> InfoEntry& = default;
		HYPERION_CONSTEXPR_STRINGS auto
		operator=(InfoEntry&& entry) noexcept -> InfoEntry& = default;

	  private:
		std::string m_entry;
	};

	/// @brief Entry type for `LogLevel::WARN` log entries
	class WarnEntry final : public EntryBase<WarnEntry> {
	  public:
		WarnEntry() noexcept = delete;
		explicit HYPERION_CONSTEXPR_STRINGS WarnEntry(const std::string& entry) noexcept // NOLINT
			: m_entry(entry) {
		}
		explicit HYPERION_CONSTEXPR_STRINGS WarnEntry(std::string&& entry) noexcept
			: m_entry(std::forward<std::string>(entry)) {
		}
		template<typename S, typename... Args, typename Char = fmt::char_t<S>>
		explicit HYPERION_CONSTEXPR_STRINGS
		WarnEntry(const S& format_string, Args&&... args) noexcept
			: m_entry(fmt::format(format_string, std::forward<Args>(args)...)) {
		}
		HYPERION_CONSTEXPR_STRINGS WarnEntry(const WarnEntry& entry) noexcept = default;
		HYPERION_CONSTEXPR_STRINGS WarnEntry(WarnEntry&& entry) noexcept = default;
		HYPERION_CONSTEXPR_STRINGS ~WarnEntry() noexcept = default;

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
		[[nodiscard]] inline constexpr auto log_entry() const noexcept -> std::string_view {
			return m_entry;
		}

		HYPERION_CONSTEXPR_STRINGS auto
		operator=(const WarnEntry& entry) noexcept -> WarnEntry& = default;
		HYPERION_CONSTEXPR_STRINGS auto
		operator=(WarnEntry&& entry) noexcept -> WarnEntry& = default;

	  private:
		std::string m_entry;
	};

	/// @brief Entry type for `LogLevel::ERROR` log entries
	class ErrorEntry final : public EntryBase<ErrorEntry> {
	  public:
		ErrorEntry() noexcept = delete;
		explicit HYPERION_CONSTEXPR_STRINGS ErrorEntry(const std::string& entry) noexcept // NOLINT
			: m_entry(entry) {
		}
		explicit HYPERION_CONSTEXPR_STRINGS ErrorEntry(std::string&& entry) noexcept
			: m_entry(std::forward<std::string>(entry)) {
		}
		template<typename S, typename... Args, typename Char = fmt::char_t<S>>
		explicit HYPERION_CONSTEXPR_STRINGS
		ErrorEntry(const S& format_string, Args&&... args) noexcept
			: m_entry(fmt::format(format_string, std::forward<Args>(args)...)) {
		}
		HYPERION_CONSTEXPR_STRINGS ErrorEntry(const ErrorEntry& entry) noexcept = default;
		HYPERION_CONSTEXPR_STRINGS ErrorEntry(ErrorEntry&& entry) noexcept = default;
		HYPERION_CONSTEXPR_STRINGS ~ErrorEntry() noexcept = default;

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
		[[nodiscard]] inline constexpr auto log_entry() const noexcept -> std::string_view {
			return m_entry;
		}

		HYPERION_CONSTEXPR_STRINGS auto
		operator=(const ErrorEntry& entry) noexcept -> ErrorEntry& = default;
		HYPERION_CONSTEXPR_STRINGS auto
		operator=(ErrorEntry&& entry) noexcept -> ErrorEntry& = default;

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

		HYPERION_CONSTEXPR_STRINGS Entry() noexcept : m_inner(MessageEntry("DefaultMessage\n")) {
		}
		explicit HYPERION_CONSTEXPR_STRINGS Entry(const EntryType auto& entry) noexcept
			: m_inner(entry) {
		}
		explicit HYPERION_CONSTEXPR_STRINGS
		Entry(EntryType auto&& entry) noexcept // NOLINT(bugprone-forwarding-reference-overload)
											   // the forwarding reference here is fine because it's
											   // constrained by the concept
			: m_inner(std::forward<decltype(entry)>(entry)) {
		}
		template<EntryType T, typename... Args>
		explicit HYPERION_CONSTEXPR_STRINGS
		Entry(std::in_place_type_t<T> tag, Args&&... args) noexcept
			: m_inner(tag, std::forward<Args>(args)...) {
		}
		HYPERION_CONSTEXPR_STRINGS Entry(const Entry& entry) noexcept = default;
		HYPERION_CONSTEXPR_STRINGS Entry(Entry&& entry) noexcept = default;
		HYPERION_CONSTEXPR_STRINGS ~Entry() noexcept = default;

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
		[[nodiscard]] inline constexpr auto entry() const noexcept -> std::string_view {
			return std::visit([](const auto& entry) { return entry.entry(); }, m_inner);
		}

		[[nodiscard]] inline constexpr auto valid() const noexcept -> bool {
			return !m_inner.valueless_by_exception();
		}

		HYPERION_CONSTEXPR_STRINGS auto operator=(const Entry& entry) noexcept -> Entry& = default;
		HYPERION_CONSTEXPR_STRINGS auto operator=(Entry&& entry) noexcept -> Entry& = default;

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
	requires ConstructibleFrom<T, Args...>
	inline HYPERION_CONSTEXPR_STRINGS auto make_entry(Args&&... args) noexcept -> Entry {
		return Entry(std::in_place_type_t<T>(), std::forward<Args>(args)...);
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
