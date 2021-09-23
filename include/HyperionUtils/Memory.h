/// @file Memory.h
/// @author Braxton Salyer <braxtonsalyer@gmail.com>
/// @brief This includes Hyperion's equivalents and extensions to the C++ standard library's
/// `<memory>`
/// @version 0.1
/// @date 2021-08-27
///
/// MIT License
/// @copyright Copyright (c) 2021 Braxton Salyer <braxtonsalyer@gmail.com>
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

#include "memory/UniquePtr.h"

///	@defgroup memory Memory
/// Hyperion provides equivalents and extensions to the C++ standard library's
/// `<memory>`. These include things like `constexpr` equivalents to `std:unique_ptr`
/// and `std::shared_ptr` and allocator aware factory functions our `UniquePtr` and `SharedPtr`,
/// among other things.
/// @headerfile "HyperionUtils/Memory.h"
