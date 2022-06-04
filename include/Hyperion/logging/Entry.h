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

	IGNORE_WEAK_VTABLES_START

	class EntryBase {
	  public:
		EntryBase() noexcept = default;
		EntryBase(const EntryBase&) noexcept = default;
		EntryBase(EntryBase&&) noexcept = default;
		virtual ~EntryBase() noexcept = default;

		[[nodiscard]] virtual constexpr auto level() const noexcept -> LogLevel = 0;
		[[nodiscard]] virtual constexpr auto style() const noexcept -> fmt::text_style = 0;
		[[nodiscard]] virtual constexpr auto entry() const noexcept -> std::string_view = 0;

		auto operator=(const EntryBase&) noexcept -> EntryBase& = default;
		auto operator=(EntryBase&&) noexcept -> EntryBase& = default;
	};

	/// @brief Entry type for `LogLevel::MESSAGE` log entries
	class MessageEntry final : public EntryBase {
	  public:
		MessageEntry() noexcept = delete;
		explicit MessageEntry(const std::string_view& entry) noexcept : m_entry(entry) {
		}
		template<usize N>
		explicit MessageEntry(const char (&entry)[N]) noexcept : m_entry(entry) { // NOLINT
		}
		template<typename... Args>
		explicit MessageEntry(fmt::format_string<Args...>&& format_string, Args&&... args) noexcept
			: m_entry(fmt::format(std::move(format_string), std::forward<Args>(args)...)) {
		}
		template<typename CharT, typename... Args>
		explicit MessageEntry(fmt::basic_string_view<CharT>&& format_string,
							  Args&&... args) noexcept
			: m_entry(fmt::format(std::move(format_string), std::forward<Args>(args)...)) {
		}
		template<typename String, typename... Args>
		explicit MessageEntry(String&& format_string, Args&&... args) noexcept
			: m_entry(
				fmt::format(std::forward<String>(format_string), std::forward<Args>(args)...)) {
		}
		MessageEntry(const MessageEntry& entry) noexcept = default;
		MessageEntry(MessageEntry&& entry) noexcept = default;
		~MessageEntry() noexcept final = default;

		/// @brief Returns the `LogLevel` associated with this entry
		///
		/// @return The `LogLevel` of this entry
		[[nodiscard]] inline constexpr auto level() const noexcept -> LogLevel final { // NOLINT
			return LogLevel::MESSAGE;
		}

		/// @brief Returns the text style of this entry
		///
		/// @return The `fmt::text_style` of this entry
		[[nodiscard]] inline constexpr auto
		style() const noexcept -> fmt::text_style final { // NOLINT
			return fmt::fg(fmt::color::white);
		}

		/// @brief Returns the text entry for this
		///
		/// @return The text entry
		[[nodiscard]] inline auto entry() const noexcept -> std::string_view final {
			return m_entry;
		}

		auto operator=(const MessageEntry& entry) noexcept -> MessageEntry& = default;
		auto operator=(MessageEntry&& entry) noexcept -> MessageEntry& = default;

	  private:
		std::string m_entry;
	};

	/// @brief Entry type for `LogLevel::TRACE` log entries
	class TraceEntry final : public EntryBase {
	  public:
		TraceEntry() noexcept = delete;
		explicit TraceEntry(const std::string_view& entry) noexcept // NOLINT
			: m_entry(entry) {
		}
		template<usize N>
		explicit TraceEntry(const char (&entry)[N]) noexcept : m_entry(entry) { // NOLINT
		}
		template<typename... Args>
		explicit TraceEntry(fmt::format_string<Args...>&& format_string, Args&&... args) noexcept
			: m_entry(fmt::format(std::move(format_string), std::forward<Args>(args)...)) {
		}
		template<typename CharT, typename... Args>
		explicit TraceEntry(fmt::basic_string_view<CharT>&& format_string, Args&&... args) noexcept
			: m_entry(fmt::format(std::move(format_string), std::forward<Args>(args)...)) {
		}
		template<typename String, typename... Args>
		explicit TraceEntry(String&& format_string, Args&&... args) noexcept
			: m_entry(
				fmt::format(std::forward<String>(format_string), std::forward<Args>(args)...)) {
		}
		TraceEntry(const TraceEntry& entry) noexcept = default;
		TraceEntry(TraceEntry&& entry) noexcept = default;
		~TraceEntry() noexcept final = default;

		/// @brief Returns the `LogLevel` associated with this entry
		///
		/// @return The `LogLevel` of this entry
		[[nodiscard]] inline constexpr auto level() const noexcept -> LogLevel final {
			return LogLevel::TRACE;
		}

		/// @brief Returns the text style of this entry
		///
		/// @return The `fmt::text_style` of this entry
		[[nodiscard]] inline constexpr auto style() const noexcept -> fmt::text_style final {
			return fmt::fg(fmt::color::steel_blue);
		}

		/// @brief Returns the text entry for this
		///
		/// @return The text entry
		[[nodiscard]] inline auto entry() const noexcept -> std::string_view final {
			return m_entry;
		}

		auto operator=(const TraceEntry& entry) noexcept -> TraceEntry& = default;
		auto operator=(TraceEntry&& entry) noexcept -> TraceEntry& = default;

	  private:
		std::string m_entry;
	};

	/// @brief Entry type for `LogLevel::INFO` log entries
	class InfoEntry final : public EntryBase {
	  public:
		InfoEntry() noexcept = delete;
		explicit InfoEntry(const std::string_view& entry) noexcept // NOLINT
			: m_entry(entry) {
		}
		template<usize N>
		explicit InfoEntry(const char (&entry)[N]) noexcept : m_entry(entry) { // NOLINT
		}
		template<typename... Args>
		explicit InfoEntry(fmt::format_string<Args...>&& format_string, Args&&... args) noexcept
			: m_entry(fmt::format(std::move(format_string), std::forward<Args>(args)...)) {
		}
		template<typename CharT, typename... Args>
		explicit InfoEntry(fmt::basic_string_view<CharT>&& format_string, Args&&... args) noexcept
			: m_entry(fmt::format(std::move(format_string), std::forward<Args>(args)...)) {
		}
		template<typename String, typename... Args>
		explicit InfoEntry(String&& format_string, Args&&... args) noexcept
			: m_entry(
				fmt::format(std::forward<String>(format_string), std::forward<Args>(args)...)) {
		}
		InfoEntry(const InfoEntry& entry) noexcept = default;
		InfoEntry(InfoEntry&& entry) noexcept = default;
		~InfoEntry() noexcept final = default;

		/// @brief Returns the `LogLevel` associated with this entry
		///
		/// @return The `LogLevel` of this entry
		[[nodiscard]] inline constexpr auto level() const noexcept -> LogLevel final {
			return LogLevel::INFO;
		}

		/// @brief Returns the text style of this entry
		///
		/// @return The `fmt::text_style` of this entry
		[[nodiscard]] inline constexpr auto style() const noexcept -> fmt::text_style final {
			return fmt::fg(fmt::color::light_green) | fmt::emphasis::italic;
		}

		/// @brief Returns the text entry for this
		///
		/// @return The text entry
		[[nodiscard]] inline auto entry() const noexcept -> std::string_view final {
			return m_entry;
		}

		auto operator=(const InfoEntry& entry) noexcept -> InfoEntry& = default;
		auto operator=(InfoEntry&& entry) noexcept -> InfoEntry& = default;

	  private:
		std::string m_entry;
	};

	/// @brief Entry type for `LogLevel::WARN` log entries
	class WarnEntry final : public EntryBase {
	  public:
		WarnEntry() noexcept = delete;
		explicit WarnEntry(const std::string_view& entry) noexcept // NOLINT
			: m_entry(entry) {
		}
		template<usize N>
		explicit WarnEntry(const char (&entry)[N]) noexcept : m_entry(entry) { // NOLINT
		}
		template<typename... Args>
		explicit WarnEntry(fmt::format_string<Args...>&& format_string, Args&&... args) noexcept
			: m_entry(fmt::format(std::move(format_string), std::forward<Args>(args)...)) {
		}
		template<typename CharT, typename... Args>
		explicit WarnEntry(fmt::basic_string_view<CharT>&& format_string, Args&&... args) noexcept
			: m_entry(fmt::format(std::move(format_string), std::forward<Args>(args)...)) {
		}
		template<typename String, typename... Args>
		explicit WarnEntry(String&& format_string, Args&&... args) noexcept
			: m_entry(
				fmt::format(std::forward<String>(format_string), std::forward<Args>(args)...)) {
		}
		WarnEntry(const WarnEntry& entry) noexcept = default;
		WarnEntry(WarnEntry&& entry) noexcept = default;
		~WarnEntry() noexcept final = default;

		/// @brief Returns the `LogLevel` associated with this entry
		///
		/// @return The `LogLevel` of this entry
		[[nodiscard]] inline constexpr auto level() const noexcept -> LogLevel final {
			return LogLevel::WARN;
		}

		/// @brief Returns the text style of this entry
		///
		/// @return The `fmt::text_style` of this entry
		[[nodiscard]] inline constexpr auto style() const noexcept -> fmt::text_style final {
			return fmt::fg(fmt::color::orange) | fmt::emphasis::bold;
		}

		/// @brief Returns the text entry for this
		///
		/// @return The text entry
		[[nodiscard]] inline auto entry() const noexcept -> std::string_view final {
			return m_entry;
		}

		auto operator=(const WarnEntry& entry) noexcept -> WarnEntry& = default;
		auto operator=(WarnEntry&& entry) noexcept -> WarnEntry& = default;

	  private:
		std::string m_entry;
	};

	/// @brief Entry type for `LogLevel::ERROR` log entries
	class ErrorEntry final : public EntryBase {
	  public:
		ErrorEntry() noexcept = delete;
		explicit ErrorEntry(const std::string_view& entry) noexcept // NOLINT
			: m_entry(entry) {
		}
		template<usize N>
		explicit ErrorEntry(const char (&entry)[N]) noexcept : m_entry(entry) { // NOLINT
		}
		template<typename... Args>
		explicit ErrorEntry(fmt::format_string<Args...>&& format_string, Args&&... args) noexcept
			: m_entry(fmt::format(std::move(format_string), std::forward<Args>(args)...)) {
		}
		template<typename CharT, typename... Args>
		explicit ErrorEntry(fmt::basic_string_view<CharT>&& format_string, Args&&... args) noexcept
			: m_entry(fmt::format(std::move(format_string), std::forward<Args>(args)...)) {
		}
		template<typename String, typename... Args>
		explicit ErrorEntry(String&& format_string, Args&&... args) noexcept
			: m_entry(
				fmt::format(std::forward<String>(format_string), std::forward<Args>(args)...)) {
		}
		ErrorEntry(const ErrorEntry& entry) noexcept = default;
		ErrorEntry(ErrorEntry&& entry) noexcept = default;
		~ErrorEntry() noexcept final = default;

		/// @brief Returns the `LogLevel` associated with this entry
		///
		/// @return The `LogLevel` of this entry
		[[nodiscard]] inline constexpr auto level() const noexcept -> LogLevel final {
			return LogLevel::ERROR;
		}

		/// @brief Returns the text style of this entry
		///
		/// @return The `fmt::text_style` of this entry
		[[nodiscard]] inline constexpr auto style() const noexcept -> fmt::text_style final {
			return fmt::fg(fmt::color::red) | fmt::emphasis::bold;
		}

		/// @brief Returns the text entry for this
		///
		/// @return The text entry
		[[nodiscard]] inline auto entry() const noexcept -> std::string_view final {
			return m_entry;
		}

		auto operator=(const ErrorEntry& entry) noexcept -> ErrorEntry& = default;
		auto operator=(ErrorEntry&& entry) noexcept -> ErrorEntry& = default;

	  private:
		std::string m_entry;
	};

	IGNORE_WEAK_VTABLES_STOP

	IGNORE_PADDING_START

	/// @brief Compile-time polymorphic type encapsulating the various possible log entry types.
	/// This can be one of `MessageEntry`, `TraceEntry`, `InfoEntry`, `WarnEntry`, `ErrorEntry`
	/// at any one point in time, and will dispatch to the correct type when queried.
	///
	/// TODO(braxtons12): replace std::variant with our own custom variant-like type that can't be
	/// valueless
	class Entry : private std::variant<MessageEntry, TraceEntry, InfoEntry, WarnEntry, ErrorEntry> {
	  public:
		using rep = std::variant<MessageEntry, TraceEntry, InfoEntry, WarnEntry, ErrorEntry>;
		using types_list = mpl::list<MessageEntry, TraceEntry, InfoEntry, WarnEntry, ErrorEntry>;

		Entry() noexcept : rep(MessageEntry("DefaultMessage")) {
		}

		template<typename T>
		requires concepts::Derived<T, EntryBase>
		explicit Entry(T&& entry) noexcept : rep(std::forward<T>(entry)) { // NOLINT
		}

		template<typename T, typename... Args>
		requires concepts::Derived<T, EntryBase> && mpl::contains_v<T, types_list>
		explicit Entry(std::in_place_type_t<T> tag,
					   fmt::format_string<Args...>&& fmt,
					   Args&&... args) noexcept
			: rep(tag, std::move(fmt), std::forward<Args>(args)...) {
		}

		template<typename T, typename CharT, typename... Args>
		requires concepts::Derived<T, EntryBase> && mpl::contains_v<T, types_list>
		explicit Entry(std::in_place_type_t<T> tag,
					   fmt::basic_string_view<CharT>&& fmt,
					   Args&&... args) noexcept
			: rep(tag, std::move(fmt), std::forward<Args>(args)...) {
		}

		template<typename T, typename String, typename... Args>
		requires concepts::Derived<T, EntryBase> && mpl::contains_v<T, types_list>
		explicit Entry(std::in_place_type_t<T> tag, String&& fmt, Args&&... args) noexcept
			: rep(tag, std::forward<String>(fmt), std::forward<Args>(args)...) {
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
	template<typename T, typename... Args>
	requires concepts::Derived<T, EntryBase> && mpl::contains_v<T, Entry::types_list>
	inline auto make_entry(fmt::format_string<Args...>&& fmt, Args&&... args) noexcept -> Entry {
		HYPERION_PROFILE_FUNCTION();
		return Entry(std::in_place_type_t<T>(), std::move(fmt), std::forward<Args>(args)...);
	}

	template<typename T, typename CharT, typename... Args>
	requires concepts::Derived<T, EntryBase> && mpl::contains_v<T, Entry::types_list>
	inline auto make_entry(fmt::basic_string_view<CharT>&& fmt, Args&&... args) noexcept -> Entry {
		HYPERION_PROFILE_FUNCTION();
		return Entry(std::in_place_type_t<T>(), std::move(fmt), std::forward<Args>(args)...);
	}

	template<typename T, typename String, typename... Args>
	requires concepts::Derived<T, EntryBase> && mpl::contains_v<T, Entry::types_list>
	inline auto make_entry(String&& fmt, Args&&... args) noexcept -> Entry {
		HYPERION_PROFILE_FUNCTION();
		return Entry(std::in_place_type_t<T>(),
					 std::forward<String>(fmt),
					 std::forward<Args>(args)...);
	}

	/// @brief Tag type for logging level of an entry
	///
	/// @tparam Level - The logging level
	template<LogLevel Level>
	struct entry_level { };

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

	/// @brief Alias for `entry_level<Level>::type`
	///
	/// @tparam Level - The logging level
	template<LogLevel Level>
	using entry_level_t = typename entry_level<Level>::type;

} // namespace hyperion
