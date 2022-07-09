/// @file Entry.h
/// @author Braxton Salyer <braxtonsalyer@gmail.com>
/// @brief Logging entry types
/// @version 0.1
/// @date 2022-06-15
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

#include <Hyperion/Concepts.h>
#include <Hyperion/Error.h>
#include <Hyperion/Fmt.h>
#include <Hyperion/HyperionDef.h>
#include <Hyperion/logging/Config.h>
#include <Hyperion/mpl/List.h>
#include <string_view>
#include <variant>

namespace hyperion::logging {

	IGNORE_WEAK_VTABLES_START

	/// @brief Base interface for a Hyperion logging entry
	/// @ingroup logging
	/// @headerfile "Hyperion/logging/Entry.h"
	class EntryBase {
	  public:
		/// @ingroup logging
		EntryBase() noexcept = default;
		/// @ingroup logging
		EntryBase(const EntryBase&) noexcept = default;
		/// @ingroup logging
		EntryBase(EntryBase&&) noexcept = default;
		/// @ingroup logging
		virtual ~EntryBase() noexcept = default;

		/// @brief Returns the logging level associated with this entry
		///
		/// @return the log level associated with this entry
		/// @ingroup logging
		[[nodiscard]] virtual constexpr auto level() const noexcept -> Level = 0;
		/// @brief Returns the text style associated with this entry, if any
		///
		/// @return the text style of this entry
		/// @ingroup logging
		[[nodiscard]] virtual constexpr auto style() const noexcept -> fmt::text_style = 0;
		/// @brief Returns the textual message associated with this entry
		///
		/// @return the text of this entry
		/// @ingroup logging
		[[nodiscard]] virtual constexpr auto entry() const noexcept -> std::string_view = 0;

		/// @ingroup logging
		auto operator=(const EntryBase&) noexcept -> EntryBase& = default;
		/// @ingroup logging
		auto operator=(EntryBase&&) noexcept -> EntryBase& = default;
	};

	/// @brief Entry type for `Level::MESSAGE` log entries
	/// @ingroup logging
	/// @headerfile "Hyperion/logging/Entry.h"
	class MessageEntry final : public EntryBase {
	  public:
		/// @ingroup logging
		MessageEntry() noexcept = delete;
		/// @ingroup logging
		explicit MessageEntry(const std::string_view& entry) noexcept : m_entry(entry) {
		}
		/// @ingroup logging
		template<usize N>
		explicit MessageEntry(const char (&entry)[N]) noexcept : m_entry(entry) { // NOLINT
		}
		/// @ingroup logging
		template<typename... Args>
		explicit MessageEntry(fmt::format_string<Args...>&& format_string, Args&&... args) noexcept
			: m_entry(fmt::format(std::move(format_string), std::forward<Args>(args)...)) {
		}
		/// @ingroup logging
		template<typename CharT, typename... Args>
		explicit MessageEntry(fmt::basic_string_view<CharT>&& format_string,
							  Args&&... args) noexcept
			: m_entry(fmt::format(std::move(format_string), std::forward<Args>(args)...)) {
		}
		/// @ingroup logging
		template<typename String, typename... Args>
		explicit MessageEntry(String&& format_string, Args&&... args) noexcept
			: m_entry(
				fmt::format(std::forward<String>(format_string), std::forward<Args>(args)...)) {
		}
		/// @ingroup logging
		MessageEntry(const MessageEntry& entry) noexcept = default;
		/// @ingroup logging
		MessageEntry(MessageEntry&& entry) noexcept = default;
		/// @ingroup logging
		~MessageEntry() noexcept final = default;

		/// @brief Returns the logging level associated with this entry
		///
		/// @return the log level associated with this entry
		/// @ingroup logging
		[[nodiscard]] inline constexpr auto level() const noexcept -> Level final { // NOLINT
			return Level::MESSAGE;
		}

		/// @brief Returns the text style associated with this entry, if any
		///
		/// @return the text style of this entry
		/// @ingroup logging
		[[nodiscard]] inline constexpr auto
		style() const noexcept -> fmt::text_style final { // NOLINT
			return fmt::fg(fmt::color::white);
		}

		/// @brief Returns the textual message associated with this entry
		///
		/// @return the text of this entry
		/// @ingroup logging
		[[nodiscard]] inline auto entry() const noexcept -> std::string_view final {
			return m_entry;
		}

		/// @ingroup logging
		auto operator=(const MessageEntry& entry) noexcept -> MessageEntry& = default;
		/// @ingroup logging
		auto operator=(MessageEntry&& entry) noexcept -> MessageEntry& = default;

	  private:
		std::string m_entry;
	};

	/// @brief Entry type for `Level::TRACE` log entries
	/// @ingroup logging
	/// @headerfile "Hyperion/logging/Entry.h"
	class TraceEntry final : public EntryBase {
	  public:
		/// @ingroup logging
		TraceEntry() noexcept = delete;
		/// @ingroup logging
		explicit TraceEntry(const std::string_view& entry) noexcept // NOLINT
			: m_entry(entry) {
		}
		/// @ingroup logging
		template<usize N>
		explicit TraceEntry(const char (&entry)[N]) noexcept : m_entry(entry) { // NOLINT
		}
		/// @ingroup logging
		template<typename... Args>
		explicit TraceEntry(fmt::format_string<Args...>&& format_string, Args&&... args) noexcept
			: m_entry(fmt::format(std::move(format_string), std::forward<Args>(args)...)) {
		}
		/// @ingroup logging
		template<typename CharT, typename... Args>
		explicit TraceEntry(fmt::basic_string_view<CharT>&& format_string, Args&&... args) noexcept
			: m_entry(fmt::format(std::move(format_string), std::forward<Args>(args)...)) {
		}
		/// @ingroup logging
		template<typename String, typename... Args>
		explicit TraceEntry(String&& format_string, Args&&... args) noexcept
			: m_entry(
				fmt::format(std::forward<String>(format_string), std::forward<Args>(args)...)) {
		}
		/// @ingroup logging
		TraceEntry(const TraceEntry& entry) noexcept = default;
		/// @ingroup logging
		TraceEntry(TraceEntry&& entry) noexcept = default;
		/// @ingroup logging
		~TraceEntry() noexcept final = default;

		/// @brief Returns the logging level associated with this entry
		///
		/// @return the log level associated with this entry
		/// @ingroup logging
		[[nodiscard]] inline constexpr auto level() const noexcept -> Level final {
			return Level::TRACE;
		}

		/// @brief Returns the text style associated with this entry, if any
		///
		/// @return the text style of this entry
		/// @ingroup logging
		[[nodiscard]] inline constexpr auto style() const noexcept -> fmt::text_style final {
			return fmt::fg(fmt::color::steel_blue);
		}

		/// @brief Returns the textual message associated with this entry
		///
		/// @return the text of this entry
		/// @ingroup logging
		[[nodiscard]] inline auto entry() const noexcept -> std::string_view final {
			return m_entry;
		}

		/// @ingroup logging
		auto operator=(const TraceEntry& entry) noexcept -> TraceEntry& = default;
		/// @ingroup logging
		auto operator=(TraceEntry&& entry) noexcept -> TraceEntry& = default;

	  private:
		std::string m_entry;
	};

	/// @brief Entry type for `Level::INFO` log entries
	/// @ingroup logging
	/// @headerfile "Hyperion/logging/Entry.h"
	class InfoEntry final : public EntryBase {
	  public:
		/// @ingroup logging
		InfoEntry() noexcept = delete;
		/// @ingroup logging
		explicit InfoEntry(const std::string_view& entry) noexcept // NOLINT
			: m_entry(entry) {
		}
		/// @ingroup logging
		template<usize N>
		explicit InfoEntry(const char (&entry)[N]) noexcept : m_entry(entry) { // NOLINT
		}
		/// @ingroup logging
		template<typename... Args>
		explicit InfoEntry(fmt::format_string<Args...>&& format_string, Args&&... args) noexcept
			: m_entry(fmt::format(std::move(format_string), std::forward<Args>(args)...)) {
		}
		/// @ingroup logging
		template<typename CharT, typename... Args>
		explicit InfoEntry(fmt::basic_string_view<CharT>&& format_string, Args&&... args) noexcept
			: m_entry(fmt::format(std::move(format_string), std::forward<Args>(args)...)) {
		}
		/// @ingroup logging
		template<typename String, typename... Args>
		explicit InfoEntry(String&& format_string, Args&&... args) noexcept
			: m_entry(
				fmt::format(std::forward<String>(format_string), std::forward<Args>(args)...)) {
		}
		/// @ingroup logging
		InfoEntry(const InfoEntry& entry) noexcept = default;
		/// @ingroup logging
		InfoEntry(InfoEntry&& entry) noexcept = default;
		/// @ingroup logging
		~InfoEntry() noexcept final = default;

		/// @brief Returns the logging level associated with this entry
		///
		/// @return the log level associated with this entry
		/// @ingroup logging
		[[nodiscard]] inline constexpr auto level() const noexcept -> Level final {
			return Level::INFO;
		}

		/// @brief Returns the text style associated with this entry, if any
		///
		/// @return the text style of this entry
		/// @ingroup logging
		[[nodiscard]] inline constexpr auto style() const noexcept -> fmt::text_style final {
			return fmt::fg(fmt::color::light_green) | fmt::emphasis::italic;
		}

		/// @brief Returns the textual message associated with this entry
		///
		/// @return the text of this entry
		/// @ingroup logging
		[[nodiscard]] inline auto entry() const noexcept -> std::string_view final {
			return m_entry;
		}

		/// @ingroup logging
		auto operator=(const InfoEntry& entry) noexcept -> InfoEntry& = default;
		/// @ingroup logging
		auto operator=(InfoEntry&& entry) noexcept -> InfoEntry& = default;

	  private:
		std::string m_entry;
	};

	/// @brief Entry type for `Level::WARN` log entries
	/// @ingroup logging
	/// @headerfile "Hyperion/logging/Entry.h"
	class WarnEntry final : public EntryBase {
	  public:
		/// @ingroup logging
		WarnEntry() noexcept = delete;
		/// @ingroup logging
		explicit WarnEntry(const std::string_view& entry) noexcept // NOLINT
			: m_entry(entry) {
		}
		/// @ingroup logging
		template<usize N>
		explicit WarnEntry(const char (&entry)[N]) noexcept : m_entry(entry) { // NOLINT
		}
		/// @ingroup logging
		template<typename... Args>
		explicit WarnEntry(fmt::format_string<Args...>&& format_string, Args&&... args) noexcept
			: m_entry(fmt::format(std::move(format_string), std::forward<Args>(args)...)) {
		}
		/// @ingroup logging
		template<typename CharT, typename... Args>
		explicit WarnEntry(fmt::basic_string_view<CharT>&& format_string, Args&&... args) noexcept
			: m_entry(fmt::format(std::move(format_string), std::forward<Args>(args)...)) {
		}
		/// @ingroup logging
		template<typename String, typename... Args>
		explicit WarnEntry(String&& format_string, Args&&... args) noexcept
			: m_entry(
				fmt::format(std::forward<String>(format_string), std::forward<Args>(args)...)) {
		}
		/// @ingroup logging
		WarnEntry(const WarnEntry& entry) noexcept = default;
		/// @ingroup logging
		WarnEntry(WarnEntry&& entry) noexcept = default;
		/// @ingroup logging
		~WarnEntry() noexcept final = default;

		/// @brief Returns the logging level associated with this entry
		///
		/// @return the log level associated with this entry
		/// @ingroup logging
		[[nodiscard]] inline constexpr auto level() const noexcept -> Level final {
			return Level::WARN;
		}

		/// @brief Returns the text style associated with this entry, if any
		///
		/// @return the text style of this entry
		/// @ingroup logging
		[[nodiscard]] inline constexpr auto style() const noexcept -> fmt::text_style final {
			return fmt::fg(fmt::color::orange) | fmt::emphasis::bold;
		}

		/// @brief Returns the textual message associated with this entry
		///
		/// @return the text of this entry
		/// @ingroup logging
		[[nodiscard]] inline auto entry() const noexcept -> std::string_view final {
			return m_entry;
		}

		/// @ingroup logging
		auto operator=(const WarnEntry& entry) noexcept -> WarnEntry& = default;
		/// @ingroup logging
		auto operator=(WarnEntry&& entry) noexcept -> WarnEntry& = default;

	  private:
		std::string m_entry;
	};

	/// @brief Entry type for `Level::ERROR` log entries
	/// @ingroup logging
	/// @headerfile "Hyperion/logging/Entry.h"
	class ErrorEntry final : public EntryBase {
	  public:
		/// @ingroup logging
		ErrorEntry() noexcept = delete;
		/// @ingroup logging
		explicit ErrorEntry(const std::string_view& entry) noexcept // NOLINT
			: m_entry(entry) {
		}
		/// @ingroup logging
		template<usize N>
		explicit ErrorEntry(const char (&entry)[N]) noexcept : m_entry(entry) { // NOLINT
		}
		/// @ingroup logging
		template<typename... Args>
		explicit ErrorEntry(fmt::format_string<Args...>&& format_string, Args&&... args) noexcept
			: m_entry(fmt::format(std::move(format_string), std::forward<Args>(args)...)) {
		}
		/// @ingroup logging
		template<typename CharT, typename... Args>
		explicit ErrorEntry(fmt::basic_string_view<CharT>&& format_string, Args&&... args) noexcept
			: m_entry(fmt::format(std::move(format_string), std::forward<Args>(args)...)) {
		}
		/// @ingroup logging
		template<typename String, typename... Args>
		explicit ErrorEntry(String&& format_string, Args&&... args) noexcept
			: m_entry(
				fmt::format(std::forward<String>(format_string), std::forward<Args>(args)...)) {
		}
		/// @ingroup logging
		ErrorEntry(const ErrorEntry& entry) noexcept = default;
		/// @ingroup logging
		ErrorEntry(ErrorEntry&& entry) noexcept = default;
		/// @ingroup logging
		~ErrorEntry() noexcept final = default;

		/// @brief Returns the logging level associated with this entry
		///
		/// @return the log level associated with this entry
		/// @ingroup logging
		[[nodiscard]] inline constexpr auto level() const noexcept -> Level final {
			return Level::ERROR;
		}

		/// @brief Returns the text style associated with this entry, if any
		///
		/// @return the text style of this entry
		/// @ingroup logging
		[[nodiscard]] inline constexpr auto style() const noexcept -> fmt::text_style final {
			return fmt::fg(fmt::color::red) | fmt::emphasis::bold;
		}

		/// @brief Returns the textual message associated with this entry
		///
		/// @return the text of this entry
		/// @ingroup logging
		[[nodiscard]] inline auto entry() const noexcept -> std::string_view final {
			return m_entry;
		}

		/// @ingroup logging
		auto operator=(const ErrorEntry& entry) noexcept -> ErrorEntry& = default;
		/// @ingroup logging
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
	/// @ingroup logging
	/// @headerfile "Hyperion/logging/Entry.h"
	class Entry : private std::variant<MessageEntry, TraceEntry, InfoEntry, WarnEntry, ErrorEntry> {
	  public:
		using rep = std::variant<MessageEntry, TraceEntry, InfoEntry, WarnEntry, ErrorEntry>;
		using types_list = mpl::list<MessageEntry, TraceEntry, InfoEntry, WarnEntry, ErrorEntry>;

		/// @ingroup logging
		Entry() noexcept : rep(MessageEntry("DefaultMessage")) {
		}

		/// @ingroup logging
		template<typename T>
		requires concepts::Derived<T, EntryBase>
		explicit Entry(T&& entry) noexcept : rep(std::forward<T>(entry)) { // NOLINT
		}

		/// @ingroup logging
		template<typename T, typename... Args>
		requires concepts::Derived<T, EntryBase> && mpl::contains_v<T, types_list>
		explicit Entry(std::in_place_type_t<T> tag,
					   fmt::format_string<Args...>&& fmt,
					   Args&&... args) noexcept
			: rep(tag, std::move(fmt), std::forward<Args>(args)...) {
		}

		/// @ingroup logging
		template<typename T, typename CharT, typename... Args>
		requires concepts::Derived<T, EntryBase> && mpl::contains_v<T, types_list>
		explicit Entry(std::in_place_type_t<T> tag,
					   fmt::basic_string_view<CharT>&& fmt,
					   Args&&... args) noexcept
			: rep(tag, std::move(fmt), std::forward<Args>(args)...) {
		}

		/// @ingroup logging
		template<typename T, typename String, typename... Args>
		requires concepts::Derived<T, EntryBase> && mpl::contains_v<T, types_list>
		explicit Entry(std::in_place_type_t<T> tag, String&& fmt, Args&&... args) noexcept
			: rep(tag, std::forward<String>(fmt), std::forward<Args>(args)...) {
		}

		/// @ingroup logging
		Entry(const Entry& entry) noexcept = default;
		/// @ingroup logging
		Entry(Entry&& entry) noexcept = default;

		/// @ingroup logging
		~Entry() noexcept = default;

		/// @brief Returns the logging level associated with this entry
		///
		/// @return the log level associated with this entry
		/// @ingroup logging
		[[nodiscard]] inline constexpr auto level() const noexcept -> Level {
			return std::visit([](const auto& entry) { return entry.level(); },
							  *static_cast<const rep*>(this));
		}

		/// @brief Returns the text style associated with this entry, if any
		///
		/// @return the text style of this entry
		/// @ingroup logging
		[[nodiscard]] inline constexpr auto style() const noexcept -> fmt::text_style {
			return std::visit([](const auto& entry) { return entry.style(); },
							  *static_cast<const rep*>(this));
		}

		/// @brief Returns the textual message associated with this entry
		///
		/// @return the text of this entry
		/// @ingroup logging
		[[nodiscard]] inline auto entry() const noexcept -> std::string_view {
			return std::visit([](const auto& entry) { return entry.entry(); },
							  *static_cast<const rep*>(this));
		}

		/// @brief Returns whether this entry is currently valid (it has not been made "valueless-by-exception")
		///
		/// @return whether the entry is valid
		/// @ingroup logging
		[[nodiscard]] inline constexpr auto valid() const noexcept -> bool {
			return !(this->valueless_by_exception());
		}

		/// @ingroup logging
		auto operator=(const Entry& entry) noexcept -> Entry& = default;
		/// @ingroup logging
		auto operator=(Entry&& entry) noexcept -> Entry& = default;
	};

	IGNORE_PADDING_STOP

	/// @brief Constructs an Entry with the given underlying entry type from `args`
	///
	/// @tparam T - The underlying entry type
	/// @tparam Args - The constructor arguments types
	///
	/// @return an `Entry`
	/// @ingroup logging
	/// @headerfile "Hyperion/logging/Entry.h"
	template<typename T, typename... Args>
	requires concepts::Derived<T, EntryBase> && mpl::contains_v<T, Entry::types_list>
	inline auto make_entry(fmt::format_string<Args...>&& fmt, Args&&... args) noexcept -> Entry {
		HYPERION_PROFILE_FUNCTION();
		return Entry(std::in_place_type_t<T>(), std::move(fmt), std::forward<Args>(args)...);
	}

	/// @brief Constructs an Entry with the given underlying entry type from `args`
	///
	/// @tparam T - The underlying entry type
	/// @tparam Args - The constructor arguments types
	///
	/// @ingroup logging
	/// @headerfile "Hyperion/logging/Entry.h"
	template<typename T, typename CharT, typename... Args>
	requires concepts::Derived<T, EntryBase> && mpl::contains_v<T, Entry::types_list>
	inline auto make_entry(fmt::basic_string_view<CharT>&& fmt, Args&&... args) noexcept -> Entry {
		HYPERION_PROFILE_FUNCTION();
		return Entry(std::in_place_type_t<T>(), std::move(fmt), std::forward<Args>(args)...);
	}

	/// @brief Constructs an Entry with the given underlying entry type from `args`
	///
	/// @tparam T - The underlying entry type
	/// @tparam Args - The constructor arguments types
	///
	/// @ingroup logging
	/// @headerfile "Hyperion/logging/Entry.h"
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
	/// @ingroup logging
	/// @headerfile "Hyperion/logging/Entry.h"
	template<Level Level>
	struct entry_level { };

	template<>
	struct entry_level<Level::MESSAGE> {
		using type = MessageEntry;
	};

	template<>
	struct entry_level<Level::TRACE> {
		using type = TraceEntry;
	};

	template<>
	struct entry_level<Level::INFO> {
		using type = InfoEntry;
	};

	template<>
	struct entry_level<Level::WARN> {
		using type = WarnEntry;
	};

	template<>
	struct entry_level<Level::ERROR> {
		using type = ErrorEntry;
	};

	/// @brief Alias for `entry_level<Level>::type`
	///
	/// @tparam Level - The logging level
	/// @ingroup logging
	/// @headerfile "Hyperion/logging/Entry.h"
	template<Level Level>
	using entry_level_t = typename entry_level<Level>::type;

} // namespace hyperion::logging
