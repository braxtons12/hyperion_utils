#pragma once

#include <Hyperion/Concepts.h>
#include <Hyperion/Error.h>
#include <Hyperion/Format.h>
#include <Hyperion/HyperionDef.h>
#include <Hyperion/logging/Config.h>
#include <Hyperion/mpl/List.h>
#include <string_view>
#include <variant>

namespace hyperion {

	/// @brief Requirements for what constitutes a log entry type
	template<typename T>
	concept EntryType = requires(T val) {
		{
			val.level()
		}
		noexcept->concepts::Same<LogLevel>;
		{
			val.style()
			} -> concepts::Same<fmt::text_style>;
		{
			val.entry()
		}
		noexcept->concepts::Stringable;
	};

	/// @brief Entry type for `LogLevel::MESSAGE` log entries
	class MessageEntry final {
	  public:
		MessageEntry() noexcept = delete;
		explicit MessageEntry(const std::string_view& entry) noexcept : m_entry(entry) {
		}
		template<typename... Args>
		explicit MessageEntry(fmt::format_string<Args...>&& format_string, Args&&... args) noexcept
			: m_entry(fmt::format(std::move(format_string), std::forward<Args>(args)...)) {
		}
		MessageEntry(const MessageEntry& entry) noexcept = default;
		MessageEntry(MessageEntry&& entry) noexcept = default;
		~MessageEntry() noexcept = default;

		/// @brief Returns the `LogLevel` associated with this entry
		///
		/// @return The `LogLevel` of this entry
		[[nodiscard]] inline constexpr auto level() const noexcept -> LogLevel { // NOLINT
			return LogLevel::MESSAGE;
		}

		/// @brief Returns the text style of this entry
		///
		/// @return The `fmt::text_style` of this entry
		[[nodiscard]] inline constexpr auto
		style() const noexcept -> fmt::text_style { // NOLINT
			return fmt::fg(fmt::color::white);
		}

		/// @brief Returns the text entry for this
		///
		/// @return The text entry
		[[nodiscard]] inline auto entry() const noexcept -> std::string_view {
			return m_entry;
		}

		auto operator=(const MessageEntry& entry) noexcept -> MessageEntry& = default;
		auto operator=(MessageEntry&& entry) noexcept -> MessageEntry& = default;

	  private:
		std::string m_entry;
	};

	/// @brief Entry type for `LogLevel::TRACE` log entries
	class TraceEntry final {
	  public:
		TraceEntry() noexcept = delete;
		explicit TraceEntry(const std::string_view& entry) noexcept // NOLINT
			: m_entry(entry) {
		}
		template<typename... Args>
		explicit TraceEntry(fmt::format_string<Args...>&& format_string, Args&&... args) noexcept
			: m_entry(fmt::format(std::move(format_string), std::forward<Args>(args)...)) {
		}
		TraceEntry(const TraceEntry& entry) noexcept = default;
		TraceEntry(TraceEntry&& entry) noexcept = default;
		~TraceEntry() noexcept = default;

		/// @brief Returns the `LogLevel` associated with this entry
		///
		/// @return The `LogLevel` of this entry
		[[nodiscard]] inline constexpr auto level() const noexcept -> LogLevel { // NOLINT
			return LogLevel::TRACE;
		}

		/// @brief Returns the text style of this entry
		///
		/// @return The `fmt::text_style` of this entry
		[[nodiscard]] inline constexpr auto
		style() const noexcept -> fmt::text_style { // NOLINT
			return fmt::fg(fmt::color::steel_blue);
		}

		/// @brief Returns the text entry for this
		///
		/// @return The text entry
		[[nodiscard]] inline auto entry() const noexcept -> std::string_view {
			return m_entry;
		}

		auto operator=(const TraceEntry& entry) noexcept -> TraceEntry& = default;
		auto operator=(TraceEntry&& entry) noexcept -> TraceEntry& = default;

	  private:
		std::string m_entry;
	};

	/// @brief Entry type for `LogLevel::INFO` log entries
	class InfoEntry final {
	  public:
		InfoEntry() noexcept = delete;
		explicit InfoEntry(const std::string_view& entry) noexcept // NOLINT
			: m_entry(entry) {
		}
		template<typename... Args>
		explicit InfoEntry(fmt::format_string<Args...>&& format_string, Args&&... args) noexcept
			: m_entry(fmt::format(std::move(format_string), std::forward<Args>(args)...)) {
		}
		InfoEntry(const InfoEntry& entry) noexcept = default;
		InfoEntry(InfoEntry&& entry) noexcept = default;
		~InfoEntry() noexcept = default;

		/// @brief Returns the `LogLevel` associated with this entry
		///
		/// @return The `LogLevel` of this entry
		[[nodiscard]] inline constexpr auto level() const noexcept -> LogLevel { // NOLINT
			return LogLevel::INFO;
		}

		/// @brief Returns the text style of this entry
		///
		/// @return The `fmt::text_style` of this entry
		[[nodiscard]] inline constexpr auto
		style() const noexcept -> fmt::text_style { // NOLINT
			return fmt::fg(fmt::color::light_green) | fmt::emphasis::italic;
		}

		/// @brief Returns the text entry for this
		///
		/// @return The text entry
		[[nodiscard]] inline auto entry() const noexcept -> std::string_view {
			return m_entry;
		}

		auto operator=(const InfoEntry& entry) noexcept -> InfoEntry& = default;
		auto operator=(InfoEntry&& entry) noexcept -> InfoEntry& = default;

	  private:
		std::string m_entry;
	};

	/// @brief Entry type for `LogLevel::WARN` log entries
	class WarnEntry final {
	  public:
		WarnEntry() noexcept = delete;
		explicit WarnEntry(const std::string_view& entry) noexcept // NOLINT
			: m_entry(entry) {
		}
		template<typename... Args>
		explicit WarnEntry(fmt::format_string<Args...>&& format_string, Args&&... args) noexcept
			: m_entry(fmt::format(std::move(format_string), std::forward<Args>(args)...)) {
		}
		WarnEntry(const WarnEntry& entry) noexcept = default;
		WarnEntry(WarnEntry&& entry) noexcept = default;
		~WarnEntry() noexcept = default;

		/// @brief Returns the `LogLevel` associated with this entry
		///
		/// @return The `LogLevel` of this entry
		[[nodiscard]] inline constexpr auto level() const noexcept -> LogLevel { // NOLINT
			return LogLevel::WARN;
		}

		/// @brief Returns the text style of this entry
		///
		/// @return The `fmt::text_style` of this entry
		[[nodiscard]] inline constexpr auto
		style() const noexcept -> fmt::text_style { // NOLINT
			return fmt::fg(fmt::color::orange) | fmt::emphasis::bold;
		}

		/// @brief Returns the text entry for this
		///
		/// @return The text entry
		[[nodiscard]] inline auto entry() const noexcept -> std::string_view {
			return m_entry;
		}

		auto operator=(const WarnEntry& entry) noexcept -> WarnEntry& = default;
		auto operator=(WarnEntry&& entry) noexcept -> WarnEntry& = default;

	  private:
		std::string m_entry;
	};

	/// @brief Entry type for `LogLevel::ERROR` log entries
	class ErrorEntry final {
	  public:
		ErrorEntry() noexcept = delete;
		explicit ErrorEntry(const std::string_view& entry) noexcept // NOLINT
			: m_entry(entry) {
		}
		template<typename... Args>
		explicit ErrorEntry(fmt::format_string<Args...>&& format_string, Args&&... args) noexcept
			: m_entry(fmt::format(std::move(format_string), std::forward<Args>(args)...)) {
		}
		ErrorEntry(const ErrorEntry& entry) noexcept = default;
		ErrorEntry(ErrorEntry&& entry) noexcept = default;
		~ErrorEntry() noexcept = default;

		/// @brief Returns the `LogLevel` associated with this entry
		///
		/// @return The `LogLevel` of this entry
		[[nodiscard]] inline constexpr auto level() const noexcept -> LogLevel { // NOLINT
			return LogLevel::ERROR;
		}

		/// @brief Returns the text style of this entry
		///
		/// @return The `fmt::text_style` of this entry
		[[nodiscard]] inline constexpr auto
		style() const noexcept -> fmt::text_style { // NOLINT
			return fmt::fg(fmt::color::red) | fmt::emphasis::bold;
		}

		/// @brief Returns the text entry for this
		///
		/// @return The text entry
		[[nodiscard]] inline auto entry() const noexcept -> std::string_view {
			return m_entry;
		}

		auto operator=(const ErrorEntry& entry) noexcept -> ErrorEntry& = default;
		auto operator=(ErrorEntry&& entry) noexcept -> ErrorEntry& = default;

	  private:
		std::string m_entry;
	};

	IGNORE_PADDING_START
	/// @brief Compile-time polymorphic type encapsulating the various possible log entry types.
	/// This can be one of `MessageEntry`, `TraceEntry`, `InfoEntry`, `WarnEntry`, `ErrorEntry`
	/// at any one point in time, and will dispatch to the correct type when queried.
	///
	/// TODO: replace std::variant with our own custom variant-like type that can't be valueless
	class Entry : private std::variant<MessageEntry, TraceEntry, InfoEntry, WarnEntry, ErrorEntry> {
	  public:
		using rep = std::variant<MessageEntry, TraceEntry, InfoEntry, WarnEntry, ErrorEntry>;
		using types_list = mpl::list<MessageEntry, TraceEntry, InfoEntry, WarnEntry, ErrorEntry>;

		Entry() noexcept : rep(MessageEntry("DefaultMessage\n")) {
		}
		explicit Entry(const EntryType auto& entry) noexcept : rep(entry) {
		}
		explicit Entry(				// NOLINT(bugprone-forwarding-reference-overload)
			EntryType auto&& entry) // the forwarding reference here is fine because it's
			noexcept				// constrained by the concept
			requires mpl::contains_v<std::remove_cvref_t<decltype(entry)>, types_list>
			: rep(std::forward<decltype(entry)>(entry)) {
		}
		template<EntryType T, typename... Args>
		requires mpl::contains_v<T, types_list>
		explicit Entry(std::in_place_type_t<T> tag,
					   fmt::format_string<Args...>&& fmt,
					   Args&&... args) noexcept
			: rep(tag, std::move(fmt), std::forward<Args>(args)...) {
		}
		Entry(const Entry& entry) noexcept = default;
		Entry(Entry&& entry) noexcept = default;
		~Entry() noexcept = default;

		/// @brief Returns the `LogLevel` associated with this entry
		///
		/// @return The `LogLevel` of this entry
		[[nodiscard]] inline constexpr auto level() const noexcept -> LogLevel {
			return std::visit([](const auto& entry) { return entry.level(); },
							  *static_cast<const rep*>(this));
		}

		/// @brief Returns the text style of this entry
		///
		/// @return The `fmt::text_style` of this entry
		[[nodiscard]] inline constexpr auto style() const noexcept -> fmt::text_style {
			return std::visit([](const auto& entry) { return entry.style(); },
							  *static_cast<const rep*>(this));
		}

		/// @brief Returns the text entry for this
		///
		/// @return The text entry
		[[nodiscard]] inline auto entry() const noexcept -> std::string_view {
			return std::visit([](const auto& entry) { return entry.entry(); },
							  *static_cast<const rep*>(this));
		}

		[[nodiscard]] inline constexpr auto valid() const noexcept -> bool {
			return !(this->valueless_by_exception());
		}

		auto operator=(const Entry& entry) noexcept -> Entry& = default;
		auto operator=(Entry&& entry) noexcept -> Entry& = default;
	};
	IGNORE_PADDING_STOP

	/// @brief Constructs an Entry with the given underlying entry type from `args`
	///
	/// @tparam T - The underlying entry type
	/// @tparam Args - The constructor arguments types
	///
	/// @return an `Entry`
	template<EntryType T, typename... Args>
	requires mpl::contains_v<T, Entry::types_list>
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
