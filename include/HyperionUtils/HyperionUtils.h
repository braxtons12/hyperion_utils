#pragma once

#include "BasicTypes.h"
#include "ChangeDetector.h"
#include "Concepts.h"
#include "Error.h"
#include "Ignore.h"
#include "LockFreeQueue.h"
#include "Logger.h"
#include "Macros.h"
#include "OptionAndResult.h"
#include "RingBuffer.h"
#include "Span.h"
#include "TypeTraits.h"

using hyperion::utils::Err;	 // NOLINT
using hyperion::utils::None; // NOLINT
using hyperion::utils::Ok;	 // NOLINT
using hyperion::utils::Some; // NOLINT

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
