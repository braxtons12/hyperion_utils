/// @file Testing.h
/// @author Braxton Salyer <braxtonsalyer@gmail.com>
/// @brief wrapper header for Doctest testing to ensure consistent configuration
/// @version 0.1
/// @date 2022-12-03
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
#define DOCTEST_CONFIG_NO_SHORT_MACRO_NAMES
#define DOCTEST_CONFIG_NO_COMPARISON_WARNING_SUPPRESSION
#define DOCTEST_CONFIG_USE_STD_HEADERS
#define DOCTEST_CONFIG_SUPER_FAST_ASSERTS
#include <doctest/doctest.h>

/// @brief Use to define a doctest test suite.
/// Usage is identical to `DOCTEST_TEST_SUITE()`, this just ignores particular warnings coming from
/// doctest.
/// @ingroup defines
/// @headerfile "Hyperion/Testing.h"
#define TEST_SUITE(...) /** NOLINT(cppcoreguidelines-macro-usage) **/ \
	IGNORE_RESERVED_IDENTIFIERS_START                                 \
	IGNORE_UNUSED_TEMPLATES_START                                     \
	DOCTEST_TEST_SUITE(__VA_ARGS__)                                   \
	/** NOLINT(modernize-use-trailing-return-type) **/                \
	IGNORE_RESERVED_IDENTIFIERS_STOP IGNORE_UNUSED_TEMPLATES_STOP
/// @brief Use to define a doctest test case.
/// Usage is identical to `DOCTEST_TEST_CASE()`, this just ignores particular warnings coming from
/// doctest.
/// @ingroup defines
/// @headerfile "Hyperion/Testing.h"
#define TEST_CASE(...) /** NOLINT(cppcoreguidelines-macro-usage) **/ \
	IGNORE_RESERVED_IDENTIFIERS_START                                \
	IGNORE_UNUSED_TEMPLATES_START                                    \
	/** NOLINTNEXTLINE **/                                           \
	DOCTEST_TEST_CASE(__VA_ARGS__)                                   \
	/** NOLINT **/                                                   \
	IGNORE_RESERVED_IDENTIFIERS_STOP IGNORE_UNUSED_TEMPLATES_STOP
/// @brief Use to define a doctest test subcase.
/// Usage is identical to `DOCTEST_SUBCASE()`, this just ignores particular warnings coming from
/// doctest.
/// @ingroup defines
/// @headerfile "Hyperion/Testing.h"
#define SUBCASE(...) /** NOLINT(cppcoreguidelines-macro-usage) **/ \
	IGNORE_RESERVED_IDENTIFIERS_START                              \
	IGNORE_UNUSED_TEMPLATES_START                                  \
	/** NOLINTNEXTLINE **/                                         \
	DOCTEST_SUBCASE(__VA_ARGS__)                                   \
	/** NOLINT **/                                                 \
	IGNORE_RESERVED_IDENTIFIERS_STOP IGNORE_UNUSED_TEMPLATES_STOP

#if !defined(DOCTEST_CONFIG_DISABLE)
	/// @brief Use to do a doctest test assertion.
	/// Usage is identical to `DOCTEST_CHECK()`, this just ignores particular warnings coming from
	/// doctest.
	/// @ingroup defines
	/// @headerfile "Hyperion/Testing.h"
	#define CHECK(...) /** NOLINT(cppcoreguidelines-macro-usage) **/                               \
		IGNORE_RESERVED_IDENTIFIERS_START IGNORE_UNUSED_TEMPLATES_START DOCTEST_CHECK(__VA_ARGS__) \
		IGNORE_RESERVED_IDENTIFIERS_STOP IGNORE_UNUSED_TEMPLATES_STOP
	/// @brief Use to do a doctest test assertion.
	/// Usage is identical to `DOCTEST_CHECK_FALSE()`, this just ignores particular warnings coming
	/// from doctest.
	/// @ingroup defines
	/// @headerfile "Hyperion/Testing.h"
	#define CHECK_FALSE(...) /** NOLINT(cppcoreguidelines-macro-usage) **/                   \
		IGNORE_RESERVED_IDENTIFIERS_START IGNORE_UNUSED_TEMPLATES_START DOCTEST_CHECK_FALSE( \
			__VA_ARGS__)                                                                     \
		IGNORE_RESERVED_IDENTIFIERS_STOP IGNORE_UNUSED_TEMPLATES_STOP
	/// @brief Use to do a doctest test assertion.
	/// Usage is identical to `DOCTEST_CHECK_EQ()`, this just ignores particular warnings coming
	/// from doctest.
	/// @ingroup defines
	/// @headerfile "Hyperion/Testing.h"
	#define CHECK_EQ(...) /** NOLINT(cppcoreguidelines-macro-usage) **/                   \
						  /** NOLINTNEXTLINE(cppcoreguidelines-macro-usage) **/           \
		IGNORE_RESERVED_IDENTIFIERS_START IGNORE_UNUSED_TEMPLATES_START DOCTEST_CHECK_EQ( \
			__VA_ARGS__)                                                                  \
		IGNORE_RESERVED_IDENTIFIERS_STOP IGNORE_UNUSED_TEMPLATES_STOP
	/// @brief Use to do a doctest test assertion.
	/// Usage is identical to `DOCTEST_CHECK_NE()`, this just ignores particular warnings coming
	/// from doctest.
	/// @ingroup defines
	/// @headerfile "Hyperion/Testing.h"
	#define CHECK_NE(...) /** NOLINT(cppcoreguidelines-macro-usage) **/                   \
		IGNORE_RESERVED_IDENTIFIERS_START IGNORE_UNUSED_TEMPLATES_START DOCTEST_CHECK_NE( \
			__VA_ARGS__)                                                                  \
		IGNORE_RESERVED_IDENTIFIERS_STOP IGNORE_UNUSED_TEMPLATES_STOP
	/// @brief Use to do a doctest test assertion.
	/// Usage is identical to `DOCTEST_CHECK_GT()`, this just ignores particular warnings coming
	/// from doctest.
	/// @ingroup defines
	/// @headerfile "Hyperion/Testing.h"
	#define CHECK_GT(...) /** NOLINT(cppcoreguidelines-macro-usage) **/                   \
		IGNORE_RESERVED_IDENTIFIERS_START IGNORE_UNUSED_TEMPLATES_START DOCTEST_CHECK_GT( \
			__VA_ARGS__)                                                                  \
		IGNORE_RESERVED_IDENTIFIERS_STOP IGNORE_UNUSED_TEMPLATES_STOP
	/// @brief Use to do a doctest test assertion.
	/// Usage is identical to `DOCTEST_CHECK_LT()`, this just ignores particular warnings coming
	/// from doctest.
	/// @ingroup defines
	/// @headerfile "Hyperion/Testing.h"
	#define CHECK_LT(...) /** NOLINT(cppcoreguidelines-macro-usage) **/                   \
		IGNORE_RESERVED_IDENTIFIERS_START IGNORE_UNUSED_TEMPLATES_START DOCTEST_CHECK_LT( \
			__VA_ARGS__)                                                                  \
		IGNORE_RESERVED_IDENTIFIERS_STOP IGNORE_UNUSED_TEMPLATES_STOP
	/// @brief Use to do a doctest test assertion.
	/// Usage is identical to `DOCTEST_CHECK_GE()`, this just ignores particular warnings coming
	/// from doctest.
	/// @ingroup defines
	/// @headerfile "Hyperion/Testing.h"
	#define CHECK_GE(...) /** NOLINT(cppcoreguidelines-macro-usage) **/                   \
		IGNORE_RESERVED_IDENTIFIERS_START IGNORE_UNUSED_TEMPLATES_START DOCTEST_CHECK_GE( \
			__VA_ARGS__)                                                                  \
		IGNORE_RESERVED_IDENTIFIERS_STOP IGNORE_UNUSED_TEMPLATES_STOP
	/// @brief Use to do a doctest test assertion.
	/// Usage is identical to `DOCTEST_CHECK_LE()`, this just ignores particular warnings coming
	/// from doctest.
	/// @ingroup defines
	/// @headerfile "Hyperion/Testing.h"
	#define CHECK_LE(...) /** NOLINT(cppcoreguidelines-macro-usage) **/                   \
		IGNORE_RESERVED_IDENTIFIERS_START IGNORE_UNUSED_TEMPLATES_START DOCTEST_CHECK_LE( \
			__VA_ARGS__)                                                                  \
		IGNORE_RESERVED_IDENTIFIERS_STOP IGNORE_UNUSED_TEMPLATES_STOP

	/// @brief Use to do a doctest test assertion.
	/// Usage is identical to `DOCTEST_REQUIRE()`, this just ignores particular warnings coming from
	/// doctest.
	/// @ingroup defines
	/// @headerfile "Hyperion/Testing.h"
	#define REQUIRE(...) /** NOLINT(cppcoreguidelines-macro-usage) **/                   \
		IGNORE_RESERVED_IDENTIFIERS_START IGNORE_UNUSED_TEMPLATES_START DOCTEST_REQUIRE( \
			__VA_ARGS__)                                                                 \
		IGNORE_RESERVED_IDENTIFIERS_STOP IGNORE_UNUSED_TEMPLATES_STOP
	/// @brief Use to do a doctest test assertion.
	/// Usage is identical to `DOCTEST_REQUIRE_FALSE()`, this just ignores particular warnings
	/// coming from doctest.
	/// @ingroup defines
	/// @headerfile "Hyperion/Testing.h"
	#define REQUIRE_FALSE(...) /** NOLINT(cppcoreguidelines-macro-usage) **/                   \
		IGNORE_RESERVED_IDENTIFIERS_START IGNORE_UNUSED_TEMPLATES_START DOCTEST_REQUIRE_FALSE( \
			__VA_ARGS__)                                                                       \
		IGNORE_RESERVED_IDENTIFIERS_STOP IGNORE_UNUSED_TEMPLATES_STOP
	/// @brief Use to do a doctest test assertion.
	/// Usage is identical to `DOCTEST_REQUIRE_EQ()`, this just ignores particular warnings coming
	/// from doctest.
	/// @ingroup defines
	/// @headerfile "Hyperion/Testing.h"
	#define REQUIRE_EQ(...) /** NOLINT(cppcoreguidelines-macro-usage) **/                   \
							/** NOLINTNEXTLINE(cppcoreguidelines-macro-usage) **/           \
		IGNORE_RESERVED_IDENTIFIERS_START IGNORE_UNUSED_TEMPLATES_START DOCTEST_REQUIRE_EQ( \
			__VA_ARGS__)                                                                    \
		IGNORE_RESERVED_IDENTIFIERS_STOP IGNORE_UNUSED_TEMPLATES_STOP
	/// @brief Use to do a doctest test assertion.
	/// Usage is identical to `DOCTEST_REQUIRE_NE()`, this just ignores particular warnings coming
	/// from doctest.
	/// @ingroup defines
	/// @headerfile "Hyperion/Testing.h"
	#define REQUIRE_NE(...) /** NOLINT(cppcoreguidelines-macro-usage) **/                   \
		IGNORE_RESERVED_IDENTIFIERS_START IGNORE_UNUSED_TEMPLATES_START DOCTEST_REQUIRE_NE( \
			__VA_ARGS__)                                                                    \
		IGNORE_RESERVED_IDENTIFIERS_STOP IGNORE_UNUSED_TEMPLATES_STOP
	/// @brief Use to do a doctest test assertion.
	/// Usage is identical to `DOCTEST_REQUIRE_GT()`, this just ignores particular warnings coming
	/// from doctest.
	/// @ingroup defines
	/// @headerfile "Hyperion/Testing.h"
	#define REQUIRE_GT(...) /** NOLINT(cppcoreguidelines-macro-usage) **/                   \
		IGNORE_RESERVED_IDENTIFIERS_START IGNORE_UNUSED_TEMPLATES_START DOCTEST_REQUIRE_GT( \
			__VA_ARGS__)                                                                    \
		IGNORE_RESERVED_IDENTIFIERS_STOP IGNORE_UNUSED_TEMPLATES_STOP
	/// @brief Use to do a doctest test assertion.
	/// Usage is identical to `DOCTEST_REQUIRE_LT()`, this just ignores particular warnings coming
	/// from doctest.
	/// @ingroup defines
	/// @headerfile "Hyperion/Testing.h"
	#define REQUIRE_LT(...) /** NOLINT(cppcoreguidelines-macro-usage) **/                   \
		IGNORE_RESERVED_IDENTIFIERS_START IGNORE_UNUSED_TEMPLATES_START DOCTEST_REQUIRE_LT( \
			__VA_ARGS__)                                                                    \
		IGNORE_RESERVED_IDENTIFIERS_STOP IGNORE_UNUSED_TEMPLATES_STOP
	/// @brief Use to do a doctest test assertion.
	/// Usage is identical to `DOCTEST_REQUIRE_GE()`, this just ignores particular warnings coming
	/// from doctest.
	/// @ingroup defines
	/// @headerfile "Hyperion/Testing.h"
	#define REQUIRE_GE(...) /** NOLINT(cppcoreguidelines-macro-usage) **/                   \
		IGNORE_RESERVED_IDENTIFIERS_START IGNORE_UNUSED_TEMPLATES_START DOCTEST_REQUIRE_GE( \
			__VA_ARGS__)                                                                    \
		IGNORE_RESERVED_IDENTIFIERS_STOP IGNORE_UNUSED_TEMPLATES_STOP
	/// @brief Use to do a doctest test assertion.
	/// Usage is identical to `DOCTEST_REQUIRE_LE()`, this just ignores particular warnings coming
	/// from doctest.
	/// @ingroup defines
	/// @headerfile "Hyperion/Testing.h"
	#define REQUIRE_LE(...) /** NOLINT(cppcoreguidelines-macro-usage) **/                   \
		IGNORE_RESERVED_IDENTIFIERS_START IGNORE_UNUSED_TEMPLATES_START DOCTEST_REQUIRE_LE( \
			__VA_ARGS__)                                                                    \
		IGNORE_RESERVED_IDENTIFIERS_STOP IGNORE_UNUSED_TEMPLATES_STOP

#else
	/// @brief Use to do a doctest test assertion.
	/// Usage is identical to `DOCTEST_CHECK()`, this just ignores particular warnings coming from
	/// doctest.
	/// @ingroup defines
	/// @headerfile "Hyperion/Testing.h"
	#define CHECK(...)		 /** NOLINT(cppcoreguidelines-macro-usage) **/ hyperion::ignore(__VA_ARGS__)
	/// @brief Use to do a doctest test assertion.
	/// Usage is identical to `DOCTEST_CHECK_FALSE()`, this just ignores particular warnings coming
	/// from doctest.
	/// @ingroup defines
	/// @headerfile "Hyperion/Testing.h"
	#define CHECK_FALSE(...) /** NOLINT(cppcoreguidelines-macro-usage) **/ \
		hyperion::ignore(__VA_ARGS__)
	/// @brief Use to do a doctest test assertion.
	/// Usage is identical to `DOCTEST_CHECK_EQ()`, this just ignores particular warnings coming
	/// from doctest.
	/// @ingroup defines
	/// @headerfile "Hyperion/Testing.h"
	#define CHECK_EQ(...) /** NOLINT(cppcoreguidelines-macro-usage) **/ \
		hyperion::ignore(__VA_ARGS__)
	/// @brief Use to do a doctest test assertion.
	/// Usage is identical to `DOCTEST_CHECK_NE()`, this just ignores particular warnings coming
	/// from doctest.
	/// @ingroup defines
	/// @headerfile "Hyperion/Testing.h"
	#define CHECK_NE(...) /** NOLINT(cppcoreguidelines-macro-usage) **/ \
		hyperion::ignore(__VA_ARGS__)
	/// @brief Use to do a doctest test assertion.
	/// Usage is identical to `DOCTEST_CHECK_GT()`, this just ignores particular warnings coming
	/// from doctest.
	/// @ingroup defines
	/// @headerfile "Hyperion/Testing.h"
	#define CHECK_GT(...) /** NOLINT(cppcoreguidelines-macro-usage) **/ \
		hyperion::ignore(__VA_ARGS__)
	/// @brief Use to do a doctest test assertion.
	/// Usage is identical to `DOCTEST_CHECK_LT()`, this just ignores particular warnings coming
	/// from doctest.
	/// @ingroup defines
	/// @headerfile "Hyperion/Testing.h"
	#define CHECK_LT(...) /** NOLINT(cppcoreguidelines-macro-usage) **/ \
		hyperion::ignore(__VA_ARGS__)
	/// @brief Use to do a doctest test assertion.
	/// Usage is identical to `DOCTEST_CHECK_GE()`, this just ignores particular warnings coming
	/// from doctest.
	/// @ingroup defines
	/// @headerfile "Hyperion/Testing.h"
	#define CHECK_GE(...) /** NOLINT(cppcoreguidelines-macro-usage) **/ \
		hyperion::ignore(__VA_ARGS__)
	/// @brief Use to do a doctest test assertion.
	/// Usage is identical to `DOCTEST_CHECK_LE()`, this just ignores particular warnings coming
	/// from doctest.
	/// @ingroup defines
	/// @headerfile "Hyperion/Testing.h"
	#define CHECK_LE(...) /** NOLINT(cppcoreguidelines-macro-usage) **/ \
		hyperion::ignore(__VA_ARGS__)

	/// @brief Use to do a doctest test assertion.
	/// Usage is identical to `DOCTEST_REQUIRE()`, this just ignores particular warnings coming from
	/// doctest.
	/// @ingroup defines
	/// @headerfile "Hyperion/Testing.h"
	#define REQUIRE(...)	   /** NOLINT(cppcoreguidelines-macro-usage) **/ hyperion::ignore(__VA_ARGS__)
	/// @brief Use to do a doctest test assertion.
	/// Usage is identical to `DOCTEST_REQUIRE_FALSE()`, this just ignores particular warnings
	/// coming from doctest.
	/// @ingroup defines
	/// @headerfile "Hyperion/Testing.h"
	#define REQUIRE_FALSE(...) /** NOLINT(cppcoreguidelines-macro-usage) **/ \
		hyperion::ignore(__VA_ARGS__)
	/// @brief Use to do a doctest test assertion.
	/// Usage is identical to `DOCTEST_REQUIRE_EQ()`, this just ignores particular warnings coming
	/// from doctest.
	/// @ingroup defines
	/// @headerfile "Hyperion/Testing.h"
	#define REQUIRE_EQ(...) /** NOLINT(cppcoreguidelines-macro-usage) **/ \
		hyperion::ignore(__VA_ARGS__)
	/// @brief Use to do a doctest test assertion.
	/// Usage is identical to `DOCTEST_REQUIRE_NE()`, this just ignores particular warnings coming
	/// from doctest.
	/// @ingroup defines
	/// @headerfile "Hyperion/Testing.h"
	#define REQUIRE_NE(...) /** NOLINT(cppcoreguidelines-macro-usage) **/ \
		hyperion::ignore(__VA_ARGS__)
	/// @brief Use to do a doctest test assertion.
	/// Usage is identical to `DOCTEST_REQUIRE_GT()`, this just ignores particular warnings coming
	/// from doctest.
	/// @ingroup defines
	/// @headerfile "Hyperion/Testing.h"
	#define REQUIRE_GT(...) /** NOLINT(cppcoreguidelines-macro-usage) **/ \
		hyperion::ignore(__VA_ARGS__)
	/// @brief Use to do a doctest test assertion.
	/// Usage is identical to `DOCTEST_REQUIRE_LT()`, this just ignores particular warnings coming
	/// from doctest.
	/// @ingroup defines
	/// @headerfile "Hyperion/Testing.h"
	#define REQUIRE_LT(...) /** NOLINT(cppcoreguidelines-macro-usage) **/ \
		hyperion::ignore(__VA_ARGS__)
	/// @brief Use to do a doctest test assertion.
	/// Usage is identical to `DOCTEST_REQUIRE_GE()`, this just ignores particular warnings coming
	/// from doctest.
	/// @ingroup defines
	/// @headerfile "Hyperion/Testing.h"
	#define REQUIRE_GE(...) /** NOLINT(cppcoreguidelines-macro-usage) **/ \
		hyperion::ignore(__VA_ARGS__)
	/// @brief Use to do a doctest test assertion.
	/// Usage is identical to `DOCTEST_REQUIRE_LE()`, this just ignores particular warnings coming
	/// from doctest.
	/// @ingroup defines
	/// @headerfile "Hyperion/Testing.h"
	#define REQUIRE_LE(...) /** NOLINT(cppcoreguidelines-macro-usage) **/ \
		hyperion::ignore(__VA_ARGS__)
#endif // !defined(DOCTEST_CONFIG_DISABLE)


