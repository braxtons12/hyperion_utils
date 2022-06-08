/// @file Utils.h
/// @author Braxton Salyer <braxtonsalyer@gmail.com>
/// @brief Hyperion Utils is a Hyperion Engine library constisting of various core vocabulary types,
/// such as `Option<T>`, `Result<T, E>` and `Error<StatusCodeDomain>`; owning synchronization types,
/// such as `ReadWriteLock<T>`; alternatives to certain standard library types, such as
/// `UniquePtr<T, Deleter>`; custom File type in `fs::File`; Meta-programming utilites in
/// `hyperion::mpl`; and a robust, compile-time configured logging system; among other utilites.
/// @version 0.1
/// @date 2022-06-07
///
/// MIT License
/// @copyright Copyright (c) 2022 Braxton Salyer <braxtonsalyer@gmail.com>
///
/// Permission is hereby granted, free of charge, to any person obtaining a copy
/// of this software and associated documentation files (the "Software"), to deal
/// in the Software without restriction, including without limitation the rights
/// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
/// copies of the Software, and to permit persons to whom the Software is
/// furnished to do so, subject to the following conditions:
///
/// The above copyright notice and this permission notice shall be included in all
/// copies or substantial portions of the Software.
///
/// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
/// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
/// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
/// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
/// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
/// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
/// SOFTWARE.
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
