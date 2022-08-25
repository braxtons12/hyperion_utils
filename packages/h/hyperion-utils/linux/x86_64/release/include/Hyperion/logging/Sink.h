/// @file Sink.h
/// @author Braxton Salyer <braxtonsalyer@gmail.com>
/// @brief Provides basic logging sink implementations
/// @version 0.1
/// @date 2022-07-09
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

#include <Hyperion/Fmt.h>
#include <Hyperion/FmtIO.h>
#include <Hyperion/Ignore.h>
#include <Hyperion/Result.h>
#include <Hyperion/filesystem/File.h>
#include <Hyperion/logging/Config.h>
#include <Hyperion/logging/Entry.h>
#include <Hyperion/mpl/List.h>
#include <cstddef>
#include <filesystem>
#include <type_traits>
#include <vector>

namespace hyperion::logging {
	IGNORE_WEAK_VTABLES_START

	class SinkBase {
	  public:
		SinkBase() noexcept = default;
		SinkBase(const SinkBase&) noexcept = default;
		SinkBase(SinkBase&&) noexcept = default;
		virtual ~SinkBase() noexcept = default;

		virtual constexpr auto sink(const Entry& entry) noexcept -> void = 0;
		virtual constexpr auto sink(Entry&& entry) noexcept -> void = 0;
		[[nodiscard]] virtual constexpr auto get_log_level() const noexcept -> Level = 0;
		virtual constexpr auto set_log_level(Level level) noexcept -> void = 0;

		auto operator=(const SinkBase&) noexcept -> SinkBase& = default;
		auto operator=(SinkBase&&) noexcept -> SinkBase& = default;
	};

	/// @brief Enum indicating whether the sink should style the text when writing it
	/// @ingroup logging
	/// @headerfile "Hyperion/logging/SinkType.h"
	enum class SinkTextStyle : uint8_t {
		Styled = 0,
		NotStyled = 1
	};

	IGNORE_PADDING_START

	/// @brief Basic logging sink that writes to a specified file
	/// @ingroup logging
	/// @headerfile "Hyperion/logging/Sink.h"
	class FileSink : public SinkBase {
	  public:
		static constexpr auto DEFAULT_FILE_NAME = "Hyperion";
		static constexpr auto DEFAULT_FILE_SUBDIRECTORY = "Hyperion";

		/// @brief Delete default constructor
		/// @ingroup logging
		/// @headerfile "Hyperion/logging/Sink.h"
		FileSink() noexcept = delete;
		/// @brief Constructs a `FileSink` that will write to the given file
		///
		/// Constructs a `FileSink` with the default `Level` (`Level::MESSAGE`) that will sink
		/// logging entries at or above that level to the given `fs::File`
		///
		/// @param file - The file to write entries to
		/// @ingroup logging
		/// @headerfile "Hyperion/logging/Sink.h"
		constexpr explicit FileSink(fs::File&& file) noexcept : m_file(std::move(file)) {
		}
		/// @brief Constructs a `FileSink` that will write to the given file
		///
		/// Constructs a `FileSink` that will sink logging entries at or above the given `Level`
		/// to the given `fs::File`
		///
		/// @param file - The file to write entries to
		/// @param level - The to configure this sink for
		/// @ingroup logging
		/// @headerfile "Hyperion/logging/Sink.h"
		constexpr FileSink(fs::File&& file, Level level) noexcept
			: m_file(std::move(file)), m_log_level(level) {
		}
		/// @brief Deleted copy constructor
		/// @ingroup logging
		/// @headerfile "Hyperion/logging/Sink.h"
		FileSink(const FileSink& sink) = delete;
		/// @brief Move constructor
		/// @ingroup logging
		/// @headerfile "Hyperion/logging/Sink.h"
		constexpr FileSink(FileSink&& sink) noexcept = default;
		/// @brief Destructor
		/// @ingroup logging
		/// @headerfile "Hyperion/logging/Sink.h"
		constexpr ~FileSink() noexcept override = default;

		/// @brief Sinks the given entry, writing it to the file associated with this
		///
		/// Sinks the given entry if it is at or above the configured `Level`, writing it to the
		/// file. Otherwise, does nothing
		///
		/// @param entry - The entry to sink
		/// @ingroup logging
		/// @headerfile "Hyperion/logging/Sink.h"
		auto sink(const Entry& entry) noexcept -> void override;

		/// @brief Sinks the given entry, writing it to the file associated with this
		///
		/// Sinks the given entry if it is at or above the configured `Level`, writing it to the
		/// file. Otherwise, does nothing
		///
		/// @param entry - The entry to sink
		/// @ingroup logging
		/// @headerfile "Hyperion/logging/Sink.h"
		auto sink(Entry&& entry) noexcept -> void override;

		/// @brief Returns the currently configured `Level` for this sink
		/// @ingroup logging
		/// @headerfile "Hyperion/logging/Sink.h"
		[[nodiscard]] inline auto get_log_level() const noexcept -> Level override {
			return m_log_level;
		}

		/// @brief Sets the configured `Level` for this sink to the given one
		///
		/// @param level - The `Level` to configure this sink to
		/// @ingroup logging
		/// @headerfile "Hyperion/logging/Sink.h"
		inline auto set_log_level(Level level) noexcept -> void override {
			m_log_level = level;
		}

		/// @brief Creates a file for logging in the system temporary files directory
		///
		/// Attempts to create an `fs::File` with the given root file name (it will be decorated
		/// with a timestamp and ".log" extension) in the given subdirectory of the the system
		/// temporary files directory. Attempts to create the subdirectory if it does not exist.
		///
		/// # Errors
		/// Returns an Error if:
		/// - accessing the system temporary files directory fails
		/// - creating or accessing the given subdirectory fails
		/// - creating a file with the given root file name fails
		///
		/// @param root_file_name - The root of the name for the file, with no timestamps or
		/// extension (i.e. "HyperionLog"). The used file name will have a timestamp prepended and
		/// the ".log" extension appended to this
		/// @param subdirectory_name - The relative subdirectory in the temporary files directory to
		/// create the file in (for "$temp_files_directory/subdirectory_name", just pass
		/// "subdirectory_name", e.g. for the "Hyperion" subdirectory, pass "Hyperion")
		///
		/// @return The `fs::File` on success, `error::SystemError` on error
		/// @ingroup logging
		/// @headerfile "Hyperion/logging/Sink.h"
		[[nodiscard]] static auto
		create_file(const std::string& root_file_name = DEFAULT_FILE_NAME, // NOLINT
					const std::string& subdirectory_name = DEFAULT_FILE_SUBDIRECTORY)
			noexcept -> Result<fs::File>;

		/// @brief Deleted copy-assignment operator
		/// @ingroup logging
		/// @headerfile "Hyperion/logging/Sink.h"
		auto operator=(const FileSink& sink) -> FileSink& = delete;
		/// @brief Move-assignment operator
		/// @ingroup logging
		/// @headerfile "Hyperion/logging/Sink.h"
		constexpr auto operator=(FileSink&& sink) noexcept -> FileSink& = default;

	  private:
		fs::File m_file;
		Level m_log_level = Level::MESSAGE;
	};

	/// @brief Basic logging sink that writes to `stdout`
	///
	/// @tparam Style - Whether the text should be styled
	/// @ingroup logging
	/// @headerfile "Hyperion/logging/Sink.h"
	template<SinkTextStyle Style = SinkTextStyle::Styled>
	class StdoutSink final : public SinkBase {
	  public:
		/// @brief Constructs a `StdoutSink` that will log entries at or above the default
		/// `Level` (`Level::ERROR`)
		/// @ingroup logging
		/// @headerfile "Hyperion/logging/Sink.h"
		constexpr StdoutSink() noexcept = default;
		/// @brief Constructs a `StdoutSink` that will log entries at or above the given `Level`
		///
		/// @param level - The `Level` to configure this sink for
		/// @ingroup logging
		/// @headerfile "Hyperion/logging/Sink.h"
		explicit constexpr StdoutSink(Level level) noexcept : m_log_level(level) {
		}
		/// @brief Copy constructor
		/// @ingroup logging
		/// @headerfile "Hyperion/logging/Sink.h"
		constexpr StdoutSink(const StdoutSink& sink) noexcept = default;
		/// @brief Move constructor
		/// @ingroup logging
		/// @headerfile "Hyperion/logging/Sink.h"
		constexpr StdoutSink(StdoutSink&& sink) noexcept = default;
		/// @brief Destructor
		/// @ingroup logging
		/// @headerfile "Hyperion/logging/Sink.h"
		constexpr ~StdoutSink() noexcept final = default;

		/// @brief Sinks the given entry, writing it to `stdout`
		///
		/// Sinks the given entry if it is at or above the configured `Level`, writing it to
		/// `stdout`. Otherwise, does nothing
		///
		/// @param entry - The entry to sink
		/// @ingroup logging
		/// @headerfile "Hyperion/logging/Sink.h"
		auto sink(const Entry& entry) noexcept -> void final;

		/// @brief Sinks the given entry, writing it to `stdout`
		///
		/// Sinks the given entry if it is at or above the configured `Level`, writing it to
		/// `stdout`. Otherwise, does nothing
		///
		/// @param entry - The entry to sink
		/// @ingroup logging
		/// @headerfile "Hyperion/logging/Sink.h"
		auto sink(Entry&& entry) noexcept -> void final;

		/// @brief Returns the currently configured `Level` for this sink
		/// @ingroup logging
		/// @headerfile "Hyperion/logging/Sink.h"
		[[nodiscard]] inline auto get_log_level() const noexcept -> Level final {
			return m_log_level;
		}

		/// @brief Sets the configured `Level` for this sink to the given one
		///
		/// @param level - The `Level` to configure this sink to
		/// @ingroup logging
		/// @headerfile "Hyperion/logging/Sink.h"
		inline auto set_log_level(Level level) noexcept -> void final {
			m_log_level = level;
		}

		/// @brief Copy-assignment operator
		/// @ingroup logging
		/// @headerfile "Hyperion/logging/Sink.h"
		constexpr auto operator=(const StdoutSink& sink) noexcept -> StdoutSink& = default;
		/// @brief Move-assignment operator
		/// @ingroup logging
		/// @headerfile "Hyperion/logging/Sink.h"
		constexpr auto operator=(StdoutSink&& sink) noexcept -> StdoutSink& = default;

	  private:
		Level m_log_level = Level::ERROR;
	};

	/// @brief Basic logging sink that writes to `stderr`
	///
	/// @tparam Style - Whether the text should be styled
	/// @ingroup logging
	/// @headerfile "Hyperion/logging/Sink.h"
	template<SinkTextStyle Style = SinkTextStyle::Styled>
	class StderrSink final : public SinkBase {
	  public:
		/// @brief Constructs a `StderrSink` that will log entries at or above the default
		/// `Level` (`Level::ERROR`)
		/// @ingroup logging
		/// @headerfile "Hyperion/logging/Sink.h"
		constexpr StderrSink() noexcept = default;
		/// @brief Constructs a `StderrSink` that will log entries at or above the given `Level`
		///
		/// @param level - The `Level` to configure this sink for
		/// @ingroup logging
		/// @headerfile "Hyperion/logging/Sink.h"
		explicit constexpr StderrSink(Level level) noexcept : m_log_level(level) {
		}
		/// @brief Copy constructor
		/// @ingroup logging
		/// @headerfile "Hyperion/logging/Sink.h"
		constexpr StderrSink(const StderrSink& sink) noexcept = default;
		/// @brief Move constructor
		/// @ingroup logging
		/// @headerfile "Hyperion/logging/Sink.h"
		constexpr StderrSink(StderrSink&& sink) noexcept = default;
		/// @brief Destructor
		/// @ingroup logging
		/// @headerfile "Hyperion/logging/Sink.h"
		constexpr ~StderrSink() noexcept final = default;

		/// @brief Sinks the given entry, writing it to `stderr`
		///
		/// Sinks the given entry if it is at or above the configured `Level`, writing it to
		/// `stderr`. Otherwise, does nothing
		///
		/// @param entry - The entry to sink
		/// @ingroup logging
		/// @headerfile "Hyperion/logging/Sink.h"
		auto sink(const Entry& entry) noexcept -> void final;

		/// @brief Sinks the given entry, writing it to `stderr`
		///
		/// Sinks the given entry if it is at or above the configured `Level`, writing it to
		/// `stderr`. Otherwise, does nothing
		///
		/// @param entry - The entry to sink
		/// @ingroup logging
		/// @headerfile "Hyperion/logging/Sink.h"
		auto sink(Entry&& entry) noexcept -> void final;

		/// @brief Returns the currently configured `Level` for this sink
		/// @ingroup logging
		/// @headerfile "Hyperion/logging/Sink.h"
		[[nodiscard]] inline auto get_log_level() const noexcept -> Level final {
			return m_log_level;
		}

		/// @brief Sets the configured `Level` for this sink to the given one
		///
		/// @param level - The `Level` to configure this sink to
		/// @ingroup logging
		/// @headerfile "Hyperion/logging/Sink.h"
		inline auto set_log_level(Level level) noexcept -> void final {
			m_log_level = level;
		}

		/// @brief Copy-assignment operator
		/// @ingroup logging
		/// @headerfile "Hyperion/logging/Sink.h"
		constexpr auto operator=(const StderrSink& sink) noexcept -> StderrSink& = default;
		/// @brief Move-assignment operator
		/// @ingroup logging
		/// @headerfile "Hyperion/logging/Sink.h"
		constexpr auto operator=(StderrSink&& sink) noexcept -> StderrSink& = default;

	  private:
		Level m_log_level = Level::INFO;
	};
	IGNORE_PADDING_STOP

	IGNORE_WEAK_VTABLES_START

	/// @brief Creates a `Sink` of the given `SinkType` from the given arguments
	/// Constructs the underlying `SinkType` in place in the `Sink`
	///
	/// @tparam T - The `SinkType` to create the `Sink` as
	/// @tparam Args - The types of the arguments for constructing the `SinkType`
	/// @param args - The arguments to pass to the `SinkType`'s constructor
	///
	/// @return A `Sink`
	/// @ingroup logging
	/// @headerfile "Hyperion/logging/Sink.h"
	template<typename T, typename... Args>
	requires concepts::Derived<T, SinkBase> && concepts::ConstructibleFrom<T, Args...>
	inline auto make_sink(Args&&... args) noexcept -> hyperion::UniquePtr<SinkBase> {
		return hyperion::make_unique<T>(std::forward<Args>(args)...);
	}

	/// @brief Basic dynamically-sized contiguous container to store `Sink`s in
	/// @ingroup logging
	/// @headerfile "Hyperion/logging/Sink.h"
	class Sinks {
	  public:
		using size_type = std::vector<hyperion::UniquePtr<SinkBase>>::size_type;
		using iterator = std::vector<hyperion::UniquePtr<SinkBase>>::iterator;
		using const_iterator = std::vector<hyperion::UniquePtr<SinkBase>>::const_iterator;
		using reverse_iterator = std::vector<hyperion::UniquePtr<SinkBase>>::reverse_iterator;
		using const_reverse_iterator
			= std::vector<hyperion::UniquePtr<SinkBase>>::const_reverse_iterator;

		/// @brief Constructs a default (empty) `Sinks`
		/// @ingroup logging
		/// @headerfile "Hyperion/logging/Sink.h"
		constexpr Sinks() noexcept = default;
		/// @brief Constructs a `Sinks` from an array of rvalue `Sink`s.
		/// This allows for braced-initialization of a `Sinks` even though `Sink`s
		/// are not copyable.
		///
		/// @tparam N - The size of the array
		/// @param sinks - The array of sinks to initialize from
		/// @ingroup logging
		/// @headerfile "Hyperion/logging/Sink.h"
		template<size_t N>
		explicit Sinks(hyperion::UniquePtr<SinkBase> (&&sinks)[N]) noexcept { // NOLINT
			m_sinks.reserve(N);
			for(auto&& sink : sinks) {
				m_sinks.push_back(std::move(sink));
			}
		}
		/// @brief Deleted copy-constructor
		/// @ingroup logging
		/// @headerfile "Hyperion/logging/Sink.h"
		Sinks(const Sinks& sinks) noexcept = delete;
		/// @brief Move-constructor
		/// @ingroup logging
		/// @headerfile "Hyperion/logging/Sink.h"
		Sinks(Sinks&& sinks) noexcept = default;
		/// @brief Destructor
		/// @ingroup logging
		/// @headerfile "Hyperion/logging/Sink.h"
		~Sinks() noexcept = default;

		/// @brief Adds a `Sink` to the end of the container
		///
		/// @param sink - The `Sink` to add
		/// @ingroup logging
		/// @headerfile "Hyperion/logging/Sink.h"
		inline auto push_back(hyperion::UniquePtr<SinkBase>&& sink) noexcept -> void {
			m_sinks.push_back(std::move(sink));
		}

		/// @brief Constructs a `Sink` in place at the end of the container
		///
		/// @tparam Args - The types of the arguments to pass to `Sink`'s constructor
		/// @param args - The arguments to pass to `Sink`'s constructor
		///
		/// @return A reference to the constructed `Sink`
		/// @ingroup logging
		/// @headerfile "Hyperion/logging/Sink.h"
		template<typename T, typename... Args>
		requires concepts::Derived<T, SinkBase> && concepts::ConstructibleFrom<T, Args...>
		inline auto emplace_back(Args&&... args) noexcept -> hyperion::UniquePtr<SinkBase>& {
			return m_sinks.emplace_back(make_sink<T>(std::forward<Args>(args)...));
		}

		/// @brief Returns a reference to the `Sink` at the beginning of the container
		///
		/// @return A reference to the first `Sink`
		/// @ingroup logging
		/// @headerfile "Hyperion/logging/Sink.h"
		[[nodiscard]] inline auto front() noexcept -> hyperion::UniquePtr<SinkBase>& {
			return m_sinks.front();
		}
		/// @brief Returns a reference to the `Sink` at the beginning of the container
		///
		/// @return A reference to the first `Sink`
		/// @ingroup logging
		/// @headerfile "Hyperion/logging/Sink.h"
		[[nodiscard]] inline auto front() const noexcept -> const hyperion::UniquePtr<SinkBase>& {
			return m_sinks.front();
		}
		/// @brief Returns a reference to the `Sink` at the end of the container
		///
		/// @return A reference to the last `Sink`
		/// @ingroup logging
		/// @headerfile "Hyperion/logging/Sink.h"
		[[nodiscard]] inline auto back() noexcept -> hyperion::UniquePtr<SinkBase>& {
			return m_sinks.back();
		}
		/// @brief Returns a reference to the `Sink` at the end of the container
		///
		/// @return A reference to the last `Sink`
		/// @ingroup logging
		/// @headerfile "Hyperion/logging/Sink.h"
		[[nodiscard]] inline auto back() const noexcept -> const hyperion::UniquePtr<SinkBase>& {
			return m_sinks.back();
		}

		/// @brief Returns the number of `Sink`s in the container
		///
		/// @return The number of `Sink`s
		/// @ingroup logging
		/// @headerfile "Hyperion/logging/Sink.h"
		[[nodiscard]] inline auto size() const noexcept -> size_type {
			return m_sinks.size();
		}

		/// @brief Returns the maximum possible number of `Sink`s this can store
		///
		/// @return The maximum number of `Sink`s
		/// @ingroup logging
		/// @headerfile "Hyperion/logging/Sink.h"
		[[nodiscard]] inline auto max_size() const noexcept -> size_type {
			return m_sinks.max_size();
		}

		/// @brief Returns the current capacity of the container
		///
		/// @return The capacity of the container
		/// @ingroup logging
		/// @headerfile "Hyperion/logging/Sink.h"
		[[nodiscard]] inline auto capacity() const noexcept -> size_type {
			return m_sinks.capacity();
		}

		/// @brief Returns whether the container is currently empty
		///
		/// @return `true` if the container is empty, `false` otherwise
		/// @ingroup logging
		/// @headerfile "Hyperion/logging/Sink.h"
		[[nodiscard]] inline auto empty() const noexcept -> bool {
			return m_sinks.empty();
		}

		/// @brief Returns a reference to the `Sink` at the given `index`
		///
		/// @param index - The index of the desired `Sink`
		///
		/// @return A reference to the associated `Sink`
		/// @ingroup logging
		/// @headerfile "Hyperion/logging/Sink.h"
		inline auto
		at(concepts::UnsignedIntegral auto index) noexcept -> hyperion::UniquePtr<SinkBase>& {
			return m_sinks.at(static_cast<size_type>(index));
		}
		/// @brief Returns a reference to the `Sink` at the given `index`
		///
		/// @param index - The index of the desired `Sink`
		///
		/// @return A reference to the associated `Sink`
		/// @ingroup logging
		/// @headerfile "Hyperion/logging/Sink.h"
		inline auto at(concepts::UnsignedIntegral auto index) const noexcept
			-> const hyperion::UniquePtr<SinkBase>& {
			return m_sinks.at(static_cast<size_type>(index));
		}

		/// @brief Returns an iterator pointing to the beginning of the container
		///
		/// @return An iterator at the beginning of the container
		/// @ingroup logging
		/// @headerfile "Hyperion/logging/Sink.h"
		[[nodiscard]] inline auto begin() noexcept -> iterator {
			return m_sinks.begin();
		}
		/// @brief Returns an iterator pointing to the beginning of the container
		///
		/// @return An iterator at the beginning of the container
		/// @ingroup logging
		/// @headerfile "Hyperion/logging/Sink.h"
		[[nodiscard]] inline auto begin() const noexcept -> const_iterator {
			return m_sinks.begin();
		}
		/// @brief Returns an iterator pointing to the end of the container
		///
		/// @return An iterator at the end of the container
		/// @ingroup logging
		/// @headerfile "Hyperion/logging/Sink.h"
		[[nodiscard]] inline auto end() noexcept -> iterator {
			return m_sinks.end();
		}
		/// @brief Returns an iterator pointing to the end of the container
		///
		/// @return An iterator at the end of the container
		/// @ingroup logging
		/// @headerfile "Hyperion/logging/Sink.h"
		[[nodiscard]] inline auto end() const noexcept -> const_iterator {
			return m_sinks.end();
		}

		/// @brief Returns an iterator pointing to the beginning of the container
		///
		/// @return An iterator at the beginning of the container
		/// @ingroup logging
		/// @headerfile "Hyperion/logging/Sink.h"
		[[nodiscard]] inline auto cbegin() const noexcept -> const_iterator {
			return m_sinks.cbegin();
		}
		/// @brief Returns an iterator pointing to the end of the container
		///
		/// @return An iterator at the end of the container
		/// @ingroup logging
		/// @headerfile "Hyperion/logging/Sink.h"
		[[nodiscard]] inline auto cend() const noexcept -> const_iterator {
			return m_sinks.cend();
		}

		/// @brief Returns a reverse iterator pointing to the beginning of the container
		///
		/// @return A reverse iterator at the beginning of the container
		/// @ingroup logging
		/// @headerfile "Hyperion/logging/Sink.h"
		[[nodiscard]] inline auto rbegin() noexcept -> reverse_iterator {
			return m_sinks.rbegin();
		}
		/// @brief Returns a reverse iterator pointing to the beginning of the container
		///
		/// @return A reverse iterator at the beginning of the container
		/// @ingroup logging
		/// @headerfile "Hyperion/logging/Sink.h"
		[[nodiscard]] inline auto rbegin() const noexcept -> const_reverse_iterator {
			return m_sinks.rbegin();
		}
		/// @brief Returns a reverse iterator pointing to the end of the container
		///
		/// @return A reverse iterator at the end of the container
		/// @ingroup logging
		/// @headerfile "Hyperion/logging/Sink.h"
		[[nodiscard]] inline auto rend() noexcept -> reverse_iterator {
			return m_sinks.rend();
		}
		/// @brief Returns a reverse iterator pointing to the end of the container
		///
		/// @return A reverse iterator at the end of the container
		/// @ingroup logging
		/// @headerfile "Hyperion/logging/Sink.h"
		[[nodiscard]] inline auto rend() const noexcept -> const_reverse_iterator {
			return m_sinks.rend();
		}

		/// @brief Returns a reverse iterator pointing to the beginning of the container
		///
		/// @return A reverse iterator at the beginning of the container
		/// @ingroup logging
		/// @headerfile "Hyperion/logging/Sink.h"
		[[nodiscard]] inline auto crbegin() const noexcept -> const_reverse_iterator {
			return m_sinks.crbegin();
		}
		/// @brief Returns a reverse iterator pointing to the end of the container
		///
		/// @return A reverse iterator at the end of the container
		/// @ingroup logging
		/// @headerfile "Hyperion/logging/Sink.h"
		[[nodiscard]] inline auto crend() const noexcept -> const_reverse_iterator {
			return m_sinks.crend();
		}

		/// @brief Returns a reference to the `Sink` at the given `index`
		///
		/// @param index - The index of the desired `Sink`
		///
		/// @return A reference to the associated `Sink`
		/// @ingroup logging
		/// @headerfile "Hyperion/logging/Sink.h"
		inline auto operator[](concepts::UnsignedIntegral auto index) noexcept
			-> hyperion::UniquePtr<SinkBase>& {
			return m_sinks[static_cast<size_type>(index)];
		}
		/// @brief Returns a reference to the `Sink` at the given `index`
		///
		/// @param index - The index of the desired `Sink`
		///
		/// @return A reference to the associated `Sink`
		/// @ingroup logging
		/// @headerfile "Hyperion/logging/Sink.h"
		inline auto operator[](concepts::UnsignedIntegral auto index) const noexcept
			-> const hyperion::UniquePtr<SinkBase>& {
			return m_sinks[static_cast<size_type>(index)];
		}
		/// @brief Deleted copy-assignment operator
		/// @ingroup logging
		/// @headerfile "Hyperion/logging/Sink.h"
		auto operator=(const Sinks& sinks) noexcept -> Sinks& = delete;
		/// @brief Move-assignment operator
		/// @ingroup logging
		/// @headerfile "Hyperion/logging/Sink.h"
		auto operator=(Sinks&& sinks) noexcept -> Sinks& = default;

	  private:
		std::vector<hyperion::UniquePtr<SinkBase>> m_sinks
			= std::vector<hyperion::UniquePtr<SinkBase>>();
	};

} // namespace hyperion::logging
