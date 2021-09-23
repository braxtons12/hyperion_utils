/// @brief HyperionUtils is a Hyperion Engine library consisting of various basic functionalities,
/// types, and utilities.
///
/// HyperionUtils contains Hyperion's logging, error handling, and meta-programming facilities,
/// as well as various datastructures and replacements or wrappers for `std` types such as `Variant`
/// to replace `std::variant` or `Span` to wrap `gsl::span`
#pragma once

#include "BasicTypes.h"
#include "ChangeDetector.h"
#include "Concepts.h"
#include "Error.h"
#include "FmtIO.h"
#include "HyperionDef.h"
#include "Ignore.h"
#include "LockFreeQueue.h"
#include "Logger.h"
#include "MPL.h"
#include "Memory.h"
#include "Option.h"
#include "Result.h"
#include "RingBuffer.h"
#include "Span.h"
#include "Synchronization.h"
#include "TypeTraits.h"

using hyperion::Err;  // NOLINT
using hyperion::None; // NOLINT
using hyperion::Ok;	  // NOLINT
using hyperion::Some; // NOLINT

// clang-format off
using hyperion::u8;	   // NOLINT
using hyperion::u16;   // NOLINT
using hyperion::u32;   // NOLINT
using hyperion::u64;   // NOLINT
using hyperion::usize; // NOLINT
using hyperion::umax;  // NOLINT

using hyperion::i8;	   // NOLINT
using hyperion::i16;   // NOLINT
using hyperion::i32;   // NOLINT
using hyperion::i64;   // NOLINT
using hyperion::imax;  // NOLINT

using hyperion::f32; // NOLINT
using hyperion::f64; // NOLINT
// using hyperion::fmax;  // NOLINT

using hyperion::operator""_u8; // NOLINT
using hyperion::operator""_u16; // NOLINT
using hyperion::operator""_u32; // NOLINT
using hyperion::operator""_u64; // NOLINT
using hyperion::operator""_usize; // NOLINT
using hyperion::operator""_umax; // NOLINT

using hyperion::operator""_i8; // NOLINT
using hyperion::operator""_i16; // NOLINT
using hyperion::operator""_i32; // NOLINT
using hyperion::operator""_i64; // NOLINT
using hyperion::operator""_imax; // NOLINT

using hyperion::operator""_f32; // NOLINT
using hyperion::operator""_f64; // NOLINT
using hyperion::operator""_fmax; // NOLINT
// clang-format on
