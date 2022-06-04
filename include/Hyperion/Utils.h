/// @brief Hyperion is a Hyperion Engine library consisting of various basic
/// functionalities, types, and utilities.
///
/// Hyperion contains Hyperion's logging, error handling, and meta-programming
/// facilities, as well as various datastructures and replacements or wrappers
/// for `std` types such as `Variant` to replace `std::variant` or `Span` to
/// wrap `gsl::span`
#pragma once

#include <Hyperion/BasicTypes.h>
#include <Hyperion/ChangeDetector.h>
#include <Hyperion/Concepts.h>
#include <Hyperion/Error.h>
#include <Hyperion/FmtIO.h>
#include <Hyperion/HyperionDef.h>
#include <Hyperion/Ignore.h>
#include <Hyperion/Logger.h>
#include <Hyperion/MPL.h>
#include <Hyperion/Memory.h>
#include <Hyperion/Option.h>
#include <Hyperion/Result.h>
#include <Hyperion/RingBuffer.h>
#include <Hyperion/Span.h>
#include <Hyperion/Synchronization.h>
#include <Hyperion/TypeTraits.h>
#include <Hyperion/test/OptionAndResultTests.h>

using hyperion::Err;  // NOLINT(misc-unused-using-decls,
					  // google-global-names-in-headers)
using hyperion::None; // NOLINT(misc-unused-using-decls,
					  // google-global-names-in-headers)
using hyperion::Ok;	  // NOLINT(misc-unused-using-decls,
					  // google-global-names-in-headers)
using hyperion::Some; // NOLINT(misc-unused-using-decls,
					  // google-global-names-in-headers)

using hyperion::u16;   // NOLINT(misc-unused-using-decls, google-global-names-in-headers)
using hyperion::u32;   // NOLINT(misc-unused-using-decls, google-global-names-in-headers)
using hyperion::u64;   // NOLINT(misc-unused-using-decls, google-global-names-in-headers)
using hyperion::u8;	   // NOLINT(misc-unused-using-decls, google-global-names-in-headers)
using hyperion::umax;  // NOLINT(misc-unused-using-decls, google-global-names-in-headers)
using hyperion::usize; // NOLINT(misc-unused-using-decls, google-global-names-in-headers)

using hyperion::i16;  // NOLINT(misc-unused-using-decls, google-global-names-in-headers)
using hyperion::i32;  // NOLINT(misc-unused-using-decls, google-global-names-in-headers)
using hyperion::i64;  // NOLINT(misc-unused-using-decls, google-global-names-in-headers)
using hyperion::i8;	  // NOLINT(misc-unused-using-decls, google-global-names-in-headers)
using hyperion::imax; // NOLINT(misc-unused-using-decls, google-global-names-in-headers)

using hyperion::f32;  // NOLINT(misc-unused-using-decls, google-global-names-in-headers)
using hyperion::f64;  // NOLINT(misc-unused-using-decls, google-global-names-in-headers)

using hyperion::operator""_u8;	  // NOLINT(misc-unused-using-decls, google-global-names-in-headers)
using hyperion::operator""_u16;	  // NOLINT(misc-unused-using-decls, google-global-names-in-headers)
using hyperion::operator""_u32;	  // NOLINT(misc-unused-using-decls, google-global-names-in-headers)
using hyperion::operator""_u64;	  // NOLINT(misc-unused-using-decls, google-global-names-in-headers)
using hyperion::operator""_usize; // NOLINT(misc-unused-using-decls, google-global-names-in-headers)
using hyperion::operator""_umax;  // NOLINT(misc-unused-using-decls, google-global-names-in-headers)

using hyperion::operator""_i8;	 // NOLINT(misc-unused-using-decls, google-global-names-in-headers)
using hyperion::operator""_i16;	 // NOLINT(misc-unused-using-decls, google-global-names-in-headers)
using hyperion::operator""_i32;	 // NOLINT(misc-unused-using-decls, google-global-names-in-headers)
using hyperion::operator""_i64;	 // NOLINT(misc-unused-using-decls, google-global-names-in-headers)
using hyperion::operator""_imax; // NOLINT(misc-unused-using-decls, google-global-names-in-headers)

using hyperion::operator""_f32;	 // NOLINT(misc-unused-using-decls, google-global-names-in-headers)
using hyperion::operator""_f64;	 // NOLINT(misc-unused-using-decls, google-global-names-in-headers)
using hyperion::operator""_fmax; // NOLINT(misc-unused-using-decls, google-global-names-in-headers)
