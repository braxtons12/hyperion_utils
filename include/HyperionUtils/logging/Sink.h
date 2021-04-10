#pragma once

#include <cstddef>
#include <filesystem>
#include <type_traits>
#include <vector>

#include "../OptionAndResult.h"
#include "Entry.h"
#include "SinkBase.h"
#include "fmtIncludes.h"

namespace hyperion::utils {

	/// @brief Enum indicating whether the sink should style the text when writing it
	enum class SinkTextStyle : uint8_t
	{
		Styled = 0,
		NotStyled = 1
	};

	/// @brief Possible error categories that can occur when creating a file for a `FileSink`
	enum class FileCreationErrorCategory : uint8_t
	{
		DirectoryCreationFailed = 0,
		FileCreationFailed = 1,
		TempDirectoryAccessFailed = 2
	};

	IGNORE_PADDING_START
	IGNORE_WEAK_VTABLES_START
	/// @brief Error type for communicating file creation errors for a `FileSink`
	class FileCreationError final : public Error {
	  public:
		FileCreationError() noexcept {
			Error::m_message = "Error creating logging file"s;
		}
		FileCreationError(FileCreationErrorCategory category) noexcept // NOLINT
			: m_category(category) {
			if(category == FileCreationErrorCategory::DirectoryCreationFailed) {
				Error::m_message = "Error creating logging directory"s;
			}
			else if(category == FileCreationErrorCategory::FileCreationFailed) {
				Error::m_message = "Error creating logging file"s;
			}
			else {
				Error::m_message = "Error accessing temporary directory"s;
			}
		}
		FileCreationError(const std::error_code& error_code,
						  FileCreationErrorCategory category) noexcept
			: Error(error_code), m_category(category) {
		}
		FileCreationError(FileCreationErrorCategory category, const std::string& message) noexcept
			: Error(message), m_category(category) {
		}
		FileCreationError(FileCreationErrorCategory category, std::string&& message) noexcept
			: Error(std::forward<std::string>(message)), m_category(category) {
		}
		FileCreationError(const FileCreationError& error) noexcept = default;
		FileCreationError(FileCreationError&& error) noexcept = default;
		~FileCreationError() noexcept final = default;

		[[nodiscard]] inline auto category() const noexcept -> FileCreationErrorCategory {
			return m_category;
		}

		auto operator=(const FileCreationError& error) noexcept -> FileCreationError& = default;
		auto operator=(FileCreationError&& error) noexcept -> FileCreationError& = default;

	  private:
		FileCreationErrorCategory m_category = FileCreationErrorCategory::FileCreationFailed;
	};
	IGNORE_WEAK_VTABLES_STOP
	IGNORE_PADDING_STOP

	/// Alias for the file type used internally be `FileSink`s
	using OutputFilePointer = std::unique_ptr<fmt::ostream>;

	/// @brief Logging Sink type to sink to a file
	///
	/// @tparam Style - Whether the text should be styled or not
	template<SinkTextStyle Style = SinkTextStyle::NotStyled>
	class FileSink final : public SinkBase<FileSink<Style>> {
	  public:
		static constexpr auto DEFAULT_FILE_NAME = "HyperionLog";
		static constexpr auto DEFAULT_FILE_SUBDIRECTORY = "Hyperion";

		FileSink() noexcept = delete;
		explicit FileSink(OutputFilePointer&& file) noexcept // NOLINT
			: m_file(std::forward<OutputFilePointer>(file)) {
		}
		FileSink(const FileSink& sink) noexcept = delete;
		FileSink(FileSink&& sink) noexcept = default;
		~FileSink() noexcept {
			m_file->close();
		}

		/// @brief Sinks the given entry, writing it to the file associated with this
		///
		/// @param entry - The entry to sink
		inline auto sink_entry(const Entry& entry) noexcept -> void {
			if constexpr(Style == SinkTextStyle::Styled) {
				m_file->print(entry.style(), entry.entry());
			}
			else {
				m_file->print(entry.entry());
			}
		}

		/// @brief Sinks the given entry, writing it to the file associated with this
		///
		/// @param entry - The entry to sink
		inline auto sink_entry(Entry&& entry) noexcept -> void {
			if constexpr(Style == SinkTextStyle::Styled) {
				m_file->print(entry.style(), entry.entry());
			}
			else {
				m_file->print(entry.entry());
			}
		}

		/// @brief Creates an `OutputFilePointer` with the given `file_name` inside the
		/// subdirectory `subdirectory_name` of the system temporary files directory
		///
		/// # Errors
		/// Returns an Error if:
		/// - accessing the system temporary files directory fails
		/// - creating or accessing the given subdirectory fails
		/// - creating a file with the given `file_name` fails
		///
		/// @param root_file_name - The relative root file name (JUST the file name, ie: "Hyperion")
		/// of the the output file. This will have a timestamp prepended and the ".log" extension
		/// appended
		/// @param subdirectory_name - The relative subdirectory (JUST the subdirectory name, ie:
		/// "Hyperion") to store the file in
		///
		/// @return The `OutputFilePointer` on success, `FileCreationError` on error
		[[nodiscard]] inline static auto
		create_file(const std::string& root_file_name = DEFAULT_FILE_NAME,
					const std::string& subdirectory_name = DEFAULT_FILE_SUBDIRECTORY) noexcept
			-> Result<OutputFilePointer, FileCreationError> {
			return get_temp_directory()
				.map<std::filesystem::path>([&](std::filesystem::path& temp_directory) {
					temp_directory.append(subdirectory_name);
					return create_subdirectory(temp_directory);
				})
				.template map<OutputFilePointer>([&](std::filesystem::path& temp_directory) {
					const auto time_string = create_time_stamp();
					temp_directory.append(time_string + " "s + root_file_name);
					temp_directory.replace_extension("log"s);
					try {
						return Ok(std::make_unique<fmt::ostream>(
							fmt::output_file(temp_directory.string())));
					}
					catch(const fmt::system_error& error) {
						return Err(FileCreationError(
							std::error_code(error.error_code(), std::generic_category()),
							FileCreationErrorCategory::FileCreationFailed));
					}
				});
		}

		auto operator=(const FileSink& sink) noexcept -> FileSink& = default;
		auto operator=(FileSink&& sink) noexcept -> FileSink& = default;

	  private:
		OutputFilePointer m_file;

		/// @brief Returns the system temporary files directory
		///
		/// # Errors
		/// Returns an error if accessing the temporary files directory fails
		///
		/// @return The `std::filesystem::path` to the temporary files directory on success,
		/// `FileCreationError` on error
		[[nodiscard]] inline static auto
		get_temp_directory() noexcept -> Result<std::filesystem::path, FileCreationError> {
			std::error_code err_code;
			auto temp_dir = std::filesystem::temp_directory_path(err_code);

			if(err_code.value() != 0) {
				return Err(FileCreationError(err_code,
											 FileCreationErrorCategory::TempDirectoryAccessFailed));
			}
			else {
				return Ok(temp_dir);
			}
		}

		/// @brief Creates the subdirectory for the given absolute subdirectory path
		///
		/// # Errors
		/// Returns an error if creating the subdirectory fails
		///
		/// @param subdirectory_path - The absolute path for the subdirectory
		///
		/// @return The given `std::filesystem::path` to the subdirectory on success,
		/// `FileCreationError` on error
		[[nodiscard]] inline static auto
		create_subdirectory(const std::filesystem::path& subdirectory_path) noexcept
			-> Result<std::filesystem::path, FileCreationError> {
			std::error_code err_code;
			std::filesystem::create_directory(subdirectory_path, err_code);

			if(err_code.value() != 0) {
				return Err(FileCreationError(err_code,
											 FileCreationErrorCategory::DirectoryCreationFailed));
			}
			else {
				return Ok(subdirectory_path);
			}
		}

		/// @brief Creates a time stamp in the format
		/// [Year-Month-Day|Hour-Minute-Second] for the current local time
		///
		/// @return The time stamp
		[[nodiscard]] inline static auto create_time_stamp() noexcept -> std::string {
			return fmt::format("[{:%Y-%m-%d|%H-%M-%S}]", fmt::localtime(std::time(nullptr)));
		}
	};

	/// @brief Logging Sink type to sink to stdout
	///
	/// @tparam Style - Whether the text should be styled
	template<SinkTextStyle Style = SinkTextStyle::Styled>
	class StdoutSink final : public SinkBase<StdoutSink<Style>> {
	  public:
		StdoutSink() noexcept = default;
		StdoutSink(const StdoutSink& sink) noexcept = delete;
		StdoutSink(StdoutSink&& sink) noexcept = default;
		~StdoutSink() noexcept = default;

		/// @brief Sinks the given entry, writing it to stdout
		///
		/// @param entry - The entry to sink
		inline auto sink_entry(const Entry& entry) noexcept -> void {
			if constexpr(Style == SinkTextStyle::Styled) {
				fmt::print(stdout, entry.style(), entry.entry());
			}
			else {
				fmt::print(stdout, entry.entry());
			}
		}

		/// @brief Sinks the given entry, writing it to stdout
		///
		/// @param entry - The entry to sink
		inline auto sink_entry(Entry&& entry) noexcept -> void {
			if constexpr(Style == SinkTextStyle::Styled) {
				fmt::print(stdout, entry.style(), entry.entry());
			}
			else {
				fmt::print(stdout, entry.entry());
			}
		}

		auto operator=(const StdoutSink& sink) noexcept -> StdoutSink& = delete;
		auto operator=(StdoutSink&& sink) noexcept -> StdoutSink& = default;
	};

	/// @brief Logging Sink Type to sink to stderr
	///
	/// @tparam Style - Whether the text should be styled
	template<SinkTextStyle Style = SinkTextStyle::Styled>
	class StderrSink final : public SinkBase<StderrSink<Style>> {
	  public:
		StderrSink() noexcept = default;
		StderrSink(const StderrSink& sink) noexcept = default;
		StderrSink(StderrSink&& sink) noexcept = default;
		~StderrSink() noexcept = default;

		/// @brief Sinks the given entry, writing it to stderr
		///
		/// @param entry - The entry to sink
		inline auto sink_entry(const Entry& entry) noexcept -> void {
			if constexpr(Style == SinkTextStyle::Styled) {
				fmt::print(stderr, entry.style(), entry.entry());
			}
			else {
				fmt::print(stderr, entry.entry());
			}
		}

		/// @brief Sinks the given entry, writing it to stderr
		///
		/// @param entry - The entry to sink
		inline auto sink_entry(Entry&& entry) noexcept -> void {
			if constexpr(Style == SinkTextStyle::Styled) {
				fmt::print(stderr, entry.style(), entry.entry());
			}
			else {
				fmt::print(stderr, entry.entry());
			}
		}

		auto operator=(const StderrSink& sink) noexcept -> StderrSink& = delete;
		auto operator=(StderrSink&& sink) noexcept -> StderrSink& = default;
	};

#ifndef HYPERION_LOGGING_SINKS
	/// List of Sink types to sink logging entries to
	#define HYPERION_LOGGING_SINKS FileSink<>, StdoutSink<>, StderrSink<> // NOLINT
#endif

	/// @brief Universal Hyperion logging Sink type.
	/// This class is compile-time polymorphic and can take the form of any sink meeting
	/// the requirements of `SinkType` and listed by the `HYPERION_LOGGING_SINKS` macro.
	///
	/// The user may write their own custom `SinkType`s. In this case,
	/// they should define `HYPERION_LOGGING_SINKS` to be the comma separated list
	/// of their desired possible Sink types BEFORE including this header
	/// (`HyperionUtils/logging/Sinks.h`) and/or the HyperionUtils logging header
	/// (`HyperionUtils/Logger.h`) and/or the global HyperionUtils header
	/// (`HyperionUtils/HyperionUtils.h`).
	/// The default value of this macro is: `FileSink<>, StdoutSink<>, StderrSink<>`
	///
	/// TODO: replace std::variant with our own custom variant-like type that can't be valueless
	class Sink {
	  public:
		using variant_type = std::variant<HYPERION_LOGGING_SINKS>;

		/// @brief Constructs this with its current value being the given `SinkType`
		///
		/// @param sink - The sink to use as the current value
		explicit Sink(SinkType auto&& sink) noexcept // NOLINT
													 // (bugprone-forwarding-reference-overload)
													 // the forwarding reference here is fine
													 // because it's constrained by the concept
			: m_inner(std::forward<decltype(sink)>(sink)) {
		}

		/// @brief Constructs this with its current value being an in-place constructed
		/// `SinkType` constructed from the arguments `args`
		///
		/// @tparam T - The `SinkType` to use for the current value
		/// @tparam Args - The types of the arguments to construct this's value from
		/// @param tag - Tag type for template type deduction
		/// @param args - The arguments to pass to `T`'s constructor
		template<SinkType T, typename... Args>
		explicit Sink(std::in_place_type_t<T> tag, Args&&... args) noexcept
			: m_inner(tag, std::forward<Args>(args)...) {
		}
		Sink(const Sink& sink) noexcept = delete;
		Sink(Sink&& sink) noexcept = default;
		~Sink() noexcept = default;

		/// @brief Sinks the given entry, writing it to the output location
		/// corresponding with the current value of this
		///
		/// @param entry - The entry to sink
		inline constexpr auto sink(const Entry& entry) noexcept -> void {
			std::visit([&](auto& sink) { sink.sink(entry); }, m_inner);
		}

		/// @brief Sinks the given entry, writing it to the output location
		/// corresponding with the current value of this
		///
		/// @param entry - The entry to sink
		inline constexpr auto sink(Entry&& entry) noexcept -> void {
			std::visit([&](auto& sink) { sink.sink(std::forward<Entry>(entry)); }, m_inner);
		}

		auto operator=(const Sink& sink) noexcept -> Sink& = delete;
		auto operator=(Sink&& sink) noexcept -> Sink& = default;

	  private:
		variant_type m_inner;
	};

	template<SinkType T, typename... Args>
	requires ConstructibleFrom<T, Args...>
	inline auto make_sink(Args&&... args) noexcept -> Sink {
		return Sink(std::in_place_type_t<T>(), std::forward<Args>(args)...);
	}

	/// @brief Basic container to store `Sink`s in
	class Sinks {
	  public:
		using size_type = std::vector<Sink>::size_type;
		using iterator = std::vector<Sink>::iterator;
		using const_iterator = std::vector<Sink>::const_iterator;
		using reverse_iterator = std::vector<Sink>::reverse_iterator;
		using const_reverse_iterator = std::vector<Sink>::const_reverse_iterator;

		/// @brief Constructs a `Sinks` from an array of rvalue `Sink`s.
		/// This allows for braced-initialization of a `Sinks` even though `Sink`s
		/// are not copyable.
		///
		/// @tparam N - The size of the array
		/// @param sinks - The array of sinks to initialize from
		template<size_t N>
		explicit Sinks(Sink(&&sinks)[N]) noexcept : m_sinks(sinks) { // NOLINT
		}
		Sinks(const Sinks& sinks) noexcept = delete;
		Sinks(Sinks&& sinks) noexcept = default;
		~Sinks() noexcept = default;

		/// @brief Adds a `Sink` to the end of the container
		///
		/// @param sink - The `Sink` to add
		inline auto push_back(Sink&& sink) noexcept -> void {
			m_sinks.push_back(std::forward<Sink>(sink));
		}

		/// @brief Constructs a `Sink` in place at the end of the container
		///
		/// @tparam Args - The types of the arguments to pass to `Sink`'s constructor
		/// @param args - The arguments to pass to `Sink`'s constructor
		///
		/// @return A reference to the new `Sink`
		template<typename... Args>
		requires concepts::ConstructibleFrom<Sink, Args...>
		inline auto emplace_back(Args&&... args) noexcept -> Sink& {
			return m_sinks.emplace_back(std::forward<Args>(args)...);
		}

		/// @brief Returns a reference to the `Sink` at the beginning of the container
		///
		/// @return A reference to the first `Sink`
		[[nodiscard]] inline auto front() noexcept -> Sink& {
			return m_sinks.front();
		}
		/// @brief Returns a reference to the `Sink` at the beginning of the container
		///
		/// @return A reference to the first `Sink`
		[[nodiscard]] inline auto front() const noexcept -> const Sink& {
			return m_sinks.front();
		}
		/// @brief Returns a reference to the `Sink` at the end of the container
		///
		/// @return A reference to the last `Sink`
		[[nodiscard]] inline auto back() noexcept -> Sink& {
			return m_sinks.back();
		}
		/// @brief Returns a reference to the `Sink` at the end of the container
		///
		/// @return A reference to the last `Sink`
		[[nodiscard]] inline auto back() const noexcept -> const Sink& {
			return m_sinks.back();
		}

		/// @brief Returns the number of `Sink`s in the container
		///
		/// @return The number of `Sink`s
		[[nodiscard]] inline auto size() const noexcept -> size_type {
			return m_sinks.size();
		}

		/// @brief Returns the maximum possible number of `Sink`s this can store
		///
		/// @return The maximum number of `Sink`s
		[[nodiscard]] inline auto max_size() const noexcept -> size_type {
			return m_sinks.max_size();
		}

		/// @brief Returns the current capacity of the container
		///
		/// @return The capacity of the container
		[[nodiscard]] inline auto capacity() const noexcept -> size_type {
			return m_sinks.capacity();
		}

		/// @brief Returns whether the container is currently empty
		///
		/// @return `true` if the container is empty, `false` otherwise
		[[nodiscard]] inline auto empty() const noexcept -> bool {
			return m_sinks.empty();
		}

		/// @brief Returns a reference to the `Sink` at the given `index`
		///
		/// @param index - The index of the desired `Sink`
		///
		/// @return A reference to the associated `Sink`
		inline auto at(concepts::UnsignedIntegral auto index) noexcept -> Sink& {
			return m_sinks.at(static_cast<size_type>(index));
		}
		/// @brief Returns a reference to the `Sink` at the given `index`
		///
		/// @param index - The index of the desired `Sink`
		///
		/// @return A reference to the associated `Sink`
		inline auto at(concepts::UnsignedIntegral auto index) const noexcept -> const Sink& {
			return m_sinks.at(static_cast<size_type>(index));
		}

		/// @brief Returns an iterator pointing to the beginning of the container
		///
		/// @return An iterator at the beginning of the container
		[[nodiscard]] inline auto begin() noexcept -> iterator {
			return m_sinks.begin();
		}
		/// @brief Returns an iterator pointing to the beginning of the container
		///
		/// @return An iterator at the beginning of the container
		[[nodiscard]] inline auto begin() const noexcept -> const_iterator {
			return m_sinks.begin();
		}
		/// @brief Returns an iterator pointing to the end of the container
		///
		/// @return An iterator at the end of the container
		[[nodiscard]] inline auto end() noexcept -> iterator {
			return m_sinks.end();
		}
		/// @brief Returns an iterator pointing to the end of the container
		///
		/// @return An iterator at the end of the container
		[[nodiscard]] inline auto end() const noexcept -> const_iterator {
			return m_sinks.end();
		}

		/// @brief Returns an iterator pointing to the beginning of the container
		///
		/// @return An iterator at the beginning of the container
		[[nodiscard]] inline auto cbegin() const noexcept -> const_iterator {
			return m_sinks.cbegin();
		}
		/// @brief Returns an iterator pointing to the end of the container
		///
		/// @return An iterator at the end of the container
		[[nodiscard]] inline auto cend() const noexcept -> const_iterator {
			return m_sinks.cend();
		}

		/// @brief Returns a reverse iterator pointing to the beginning of the container
		///
		/// @return A reverse iterator at the beginning of the container
		[[nodiscard]] inline auto rbegin() noexcept -> reverse_iterator {
			return m_sinks.rbegin();
		}
		/// @brief Returns a reverse iterator pointing to the beginning of the container
		///
		/// @return A reverse iterator at the beginning of the container
		[[nodiscard]] inline auto rbegin() const noexcept -> const_reverse_iterator {
			return m_sinks.rbegin();
		}
		/// @brief Returns a reverse iterator pointing to the end of the container
		///
		/// @return A reverse iterator at the end of the container
		[[nodiscard]] inline auto rend() noexcept -> reverse_iterator {
			return m_sinks.rend();
		}
		/// @brief Returns a reverse iterator pointing to the end of the container
		///
		/// @return A reverse iterator at the end of the container
		[[nodiscard]] inline auto rend() const noexcept -> const_reverse_iterator {
			return m_sinks.rend();
		}

		/// @brief Returns a reverse iterator pointing to the beginning of the container
		///
		/// @return A reverse iterator at the beginning of the container
		[[nodiscard]] inline auto crbegin() const noexcept -> const_reverse_iterator {
			return m_sinks.crbegin();
		}
		/// @brief Returns a reverse iterator pointing to the end of the container
		///
		/// @return A reverse iterator at the end of the container
		[[nodiscard]] inline auto crend() const noexcept -> const_reverse_iterator {
			return m_sinks.crend();
		}

		/// @brief Returns a reference to the `Sink` at the given `index`
		///
		/// @param index - The index of the desired `Sink`
		///
		/// @return A reference to the associated `Sink`
		inline auto operator[](concepts::UnsignedIntegral auto index) noexcept -> Sink& {
			return m_sinks[static_cast<size_type>(index)];
		}
		/// @brief Returns a reference to the `Sink` at the given `index`
		///
		/// @param index - The index of the desired `Sink`
		///
		/// @return A reference to the associated `Sink`
		inline auto
		operator[](concepts::UnsignedIntegral auto index) const noexcept -> const Sink& {
			return m_sinks[static_cast<size_type>(index)];
		}
		auto operator=(const Sinks& sinks) noexcept -> Sinks& = delete;
		auto operator=(Sinks&& sinks) noexcept -> Sinks& = default;

	  private:
		std::vector<Sink> m_sinks = std::vector<Sink>();
	};
} // namespace hyperion::utils
