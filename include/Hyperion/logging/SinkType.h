/// @file SinkType.h
/// @author Braxton Salyer <braxtonsalyer@gmail.com>
/// @brief Requirements for a `SinkType`
/// @version 0.1
/// @date 2021-10-29
///
/// MIT License
/// @copyright Copyright (c) 2021 Braxton Salyer <braxtonsalyer@gmail.com>
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
#include <Hyperion/logging/Config.h>
#include <Hyperion/logging/Entry.h>

namespace hyperion {
	/// @brief Requirements for what constitutes a logging sink type
	/// @ingroup logging
	/// @headerfile "Hyperion/logging/SinkType.h"
	template<typename Sink>
	concept SinkType
		= requires(Sink sink, const Entry& entry_lval, Entry&& entry_rval, LogLevel level) {
		{
			sink.sink_entry(entry_lval)
			} -> concepts::Same<void>;
		noexcept(sink.sink_entry(entry_lval));

		{
			sink.sink_entry(entry_rval)
			} -> concepts::Same<void>;
		noexcept(sink.sink_entry(entry_rval));
		{
			sink.get_log_level()
			} -> concepts::Same<LogLevel>;
		noexcept(sink.get_log_level());
		{
			sink.set_log_level(level)
			} -> concepts::Same<void>;
		noexcept(sink.set_log_level(level));
	};

	/// @brief Enum indicating whether the sink should style the text when writing it
	/// @ingroup logging
	/// @headerfile "Hyperion/logging/SinkType.h"
	enum class SinkTextStyle : uint8_t {
		Styled = 0,
		NotStyled = 1
	};
} // namespace hyperion
