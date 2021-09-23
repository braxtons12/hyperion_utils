#pragma once

#include "HyperionUtils/HyperionDef.h"

IGNORE_COMMA_MISUSE_START
#include <boost/ut.hpp>
IGNORE_COMMA_MISUSE_STOP

namespace hyperion::test {
	namespace ut = boost::ut;

	using namespace ut::literals;		  // NOLINT
	using namespace ut::operators::terse; // NOLINT
	using namespace ut;					  // NOLINT

	static constexpr auto TRUE = _b(true);
	static constexpr auto FALSE = _b(false);
} // namespace hyperion::test
