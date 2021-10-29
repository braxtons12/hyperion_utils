#pragma once

#include <Hyperion/Error.h>
#include <Hyperion/Option.h>
#include <Hyperion/Result.h>

#include "microTest.h"

namespace hyperion::test {

	const suite ResultTests = [] { // NOLINT
		"okAsMutValue"_test = [] {
			Result<bool> ok = Ok(true);

			expect(ok.is_ok() == TRUE);
			expect(static_cast<bool>(ok) == TRUE);
			expect(ok.is_err() == FALSE);

			auto& gotten_mut = ok.as_mut();
			expect(gotten_mut == TRUE);
			gotten_mut = false;
			auto& gotten_mut2 = ok.as_mut();
			expect(gotten_mut2 == FALSE);
		};

		"okAsMutPointer"_test = [] {
			auto value = true;
			Result<bool*> ok = Ok(&value);

			auto*& gotten_mut = ok.as_mut();
			expect((gotten_mut != nullptr) == TRUE);
			expect(*gotten_mut == TRUE);
			*gotten_mut = false;
			gotten_mut = ok.as_mut();
			expect((gotten_mut != nullptr) == TRUE);
			expect(*gotten_mut == FALSE);
		};

		"errAsMutValue"_test = [] {
			auto error = error::SystemError(2);
			Result<bool> err = Err(std::move(error));

			expect(err.is_err() == TRUE);
			expect(err.is_ok() == FALSE);
			expect(static_cast<bool>(err) == FALSE);
			expect(aborts([&] { ignore(err.as_mut()); }));
		};

		"errAsMutPointer"_test = [] {
			auto error = error::SystemError(2);
			Result<bool*> err = Err(std::move(error));

			expect(aborts([&] { ignore(err.as_mut()); }));
		};

		"okAsConstValue"_test = [] {
			Result<bool> ok = Ok(true);

			const auto& gotten_mut = ok.as_const();
			expect(gotten_mut == TRUE);
		};

		"okAsConstPointer"_test = [] {
			auto value = true;
			Result<bool*> ok = Ok(&value);

			const auto* gotten_mut = ok.as_const();
			expect((gotten_mut != nullptr) == TRUE);
			expect(*gotten_mut == TRUE);
		};

		"errAsConstValue"_test = [] {
			auto error = error::SystemError(2);
			Result<bool> err = Err(std::move(error));

			expect(aborts([&] { ignore(err.as_const()); }));
		};

		"errAsConstPointer"_test = [] {
			auto error = error::SystemError(2);
			Result<bool*> err = Err(std::move(error));

			expect(aborts([&] { ignore(err.as_const()); }));
		};

		"okUnwrapValue"_test = [] {
			auto value = true;
			Result<bool> ok = Ok(value);

			expect(ok.unwrap() == TRUE);
		};

		"okUnwrapPointer"_test = [] {
			auto value = true;
			Result<bool*> ok = Ok(&value);

			auto* ptr = ok.unwrap();
			expect((ptr != nullptr) == TRUE);
			expect(*ptr == TRUE);
		};

		"errUnwrapValue"_test = [] {
			auto error = error::SystemError(2);
			Result<bool> err = Err(std::move(error));

			expect(aborts([&] { ignore(err.unwrap()); }));
		};

		"errUnwrapPointer"_test = [] {
			auto error = error::SystemError(2);
			Result<bool*> err = Err(std::move(error));

			expect(aborts([&] { ignore(err.unwrap()); }));
		};

		"okUnwrapOrValue"_test = [] {
			auto value = true;
			Result<bool> ok = Ok(value);

			expect(ok.unwrap_or(false) == TRUE);
		};

		"okUnwrapOrPointer"_test = [] {
			auto value = true;
			Result<bool*> ok = Ok(&value);

			auto value2 = false;
			expect(*(ok.unwrap_or(&value2)) == TRUE);
		};

		"errUnwrapOrValue"_test = [] {
			auto error = error::SystemError(2);
			Result<bool> err = Err(std::move(error));

			expect(err.unwrap_or(false) == FALSE);
		};

		"errUnwrapOrPointer"_test = [] {
			auto error = error::SystemError(2);
			Result<bool*> err = Err(std::move(error));

			auto value2 = false;
			expect(*(err.unwrap_or(&value2)) == FALSE);
		};

		"okUnwrapOrElseValue"_test = [] {
			auto value = true;
			Result<bool> ok = Ok(value);

			expect(ok.unwrap_or_else([]() noexcept -> bool { return false; }) == TRUE);
		};

		"okUnwrapOrElsePointer"_test = [] {
			auto value = true;
			Result<bool*> ok = Ok(&value);

			auto value2 = false;
			expect(*ok.unwrap_or_else([&]() noexcept -> bool* { return &value2; }) == TRUE);
		};

		"errUnwrapOrElseValue"_test = [] {
			auto error = error::SystemError(2);
			Result<bool> err = Err(std::move(error));

			expect(err.unwrap_or_else([]() noexcept -> bool { return false; }) == FALSE);
		};

		"errUnwrapOrElsePointer"_test = [] {
			auto error = error::SystemError(2);
			Result<bool*> err = Err(std::move(error));

			auto value = false;
			expect(*err.unwrap_or_else([&]() noexcept -> bool* { return &value; }) == FALSE);
		};

		"okUnwrapErrValue"_test = [] {
			auto value = true;
			Result<bool> ok = Ok(value);

			expect(aborts([&] { ignore(ok.unwrap_err()); }));
		};

		"okUnwrapErrPointer"_test = [] {
			auto value = true;
			Result<bool*> ok = Ok(&value);

			expect(aborts([&] { ignore(ok.unwrap_err()); }));
		};

		"errUnwrapErrValue"_test = [] {
			auto error = error::SystemError(2);
			Result<bool> err = Err(std::move(error));

			expect(err.unwrap_err().value() == 2_i);
		};

		"errUnwrapErrPointer"_test = [] {
			auto error = error::SystemError(2);
			Result<bool, error::SystemError*> err = Err(&error);

			expect(err.unwrap_err()->value() == 2_i);
		};

		"okokValue"_test = [] {
			auto value = true;
			Result<bool> ok = Ok(value);

			auto maybe_ok = ok.ok();
			expect(maybe_ok.is_some() == TRUE);
			expect(maybe_ok.unwrap() == TRUE);
		};

		"okokPointer"_test = [] {
			auto value = true;
			Result<bool*> ok = Ok(&value);

			auto maybe_ok = ok.ok();
			expect(maybe_ok.is_some() == TRUE);
			auto* unwrapped = maybe_ok.unwrap();
			expect((unwrapped != nullptr) == TRUE);
			expect(*unwrapped == TRUE);
		};

		"errOkValue"_test = [] {
			auto error = error::SystemError(2);
			Result<bool> err = Err(std::move(error));

			auto maybe_ok = err.ok();
			expect(maybe_ok.is_none() == TRUE);
		};

		"errOkPointer"_test = [] {
			auto error = error::SystemError(2);
			Result<bool, error::SystemError*> err = Err(&error);

			auto maybe_ok = err.ok();
			expect(maybe_ok.is_none() == TRUE);
		};

		"okErrValue"_test = [] {
			auto value = true;
			Result<bool> ok = Ok(value);

			auto maybe_err = ok.err();
			expect(maybe_err.is_none() == TRUE);
		};

		"okErrPointer"_test = [] {
			auto value = true;
			Result<bool*> ok = Ok(&value);

			auto maybe_err = ok.err();
			expect(maybe_err.is_none() == TRUE);
		};

		"errerrValue"_test = [] {
			auto error = error::SystemError(2);
			Result<bool> err = Err(std::move(error));

			auto maybe_ok = err.err();
			expect(maybe_ok.is_some() == TRUE);
			expect(maybe_ok.unwrap().value() == 2_i);
		};

		"errerrPointer"_test = [] {
			auto error = error::SystemError(2);
			Result<bool, error::SystemError*> err = Err(&error);

			auto maybe_ok = err.err();
			expect(maybe_ok.is_some() == TRUE);
			expect(maybe_ok.unwrap()->value() == 2_i);
		};

		"okMapValue"_test = [] {
			auto value = true;
			Result<bool> ok = Ok(value);

			auto maybe_ok = ok.map([](const bool _val) noexcept -> int {
				ignore(_val);
				return 2;
			});
			expect(maybe_ok.is_ok() == TRUE);
			expect(maybe_ok.unwrap() == 2_i);
		};

		"okMapPointer"_test = [] {
			auto value = true;
			Result<bool*> ok = Ok(&value);

			auto maybe_ok = ok.map([](const bool* _val) noexcept -> int {
				ignore(_val);
				return 2;
			});
			expect(maybe_ok.is_ok() == TRUE);
			expect(maybe_ok.unwrap() == 2_i);
		};

		"errMapValue"_test = [] {
			auto error = error::SystemError(2);
			Result<bool> err = Err(std::move(error));

			auto maybe_ok = err.map([](const bool _val) noexcept -> int {
				ignore(_val);
				return 4;
			});
			expect(maybe_ok.is_err() == TRUE);
			expect(maybe_ok.unwrap_err().value() == 2_i);
		};

		"errMapPointer"_test = [] {
			auto error = error::SystemError(2);
			Result<bool, error::SystemError*> err = Err(&error);

			auto maybe_ok = err.map([](const bool _val) noexcept -> int {
				ignore(_val);
				return 4;
			});
			expect(maybe_ok.is_err() == TRUE);
			expect(maybe_ok.unwrap_err()->value() == 2_i);
		};

		"okMapOrValue"_test = [] {
			auto value = true;
			Result<bool> ok = Ok(value);

			expect(ok.map_or(
					   [](const bool _val) noexcept -> int {
						   ignore(_val);
						   return 2;
					   },
					   4)
				   == 2_i);
		};

		"okMapOrPointer"_test = [] {
			auto value = true;
			Result<bool*> ok = Ok(&value);

			expect(ok.map_or(
					   [](const bool* _val) noexcept -> int {
						   ignore(_val);
						   return 2;
					   },
					   4)
				   == 2_i);
		};

		"errMapOrValue"_test = [] {
			auto error = error::SystemError(2);
			Result<bool> err = Err(std::move(error));

			expect(err.map_or(
					   [](const bool _val) noexcept -> int {
						   ignore(_val);
						   return 2;
					   },
					   4)
				   == 4_i);
		};

		"errMapOrPointer"_test = [] {
			auto error = error::SystemError(2);
			Result<bool, error::SystemError*> err = Err(&error);

			expect(err.map_or(
					   [](const bool _val) noexcept -> int {
						   ignore(_val);
						   return 2;
					   },
					   4)
				   == 4_i);
		};

		"okMapOrElseValue"_test = [] {
			auto value = true;
			Result<bool> ok = Ok(value);

			expect(ok.map_or_else(
					   [](const bool _val) noexcept -> int {
						   ignore(_val);
						   return 2;
					   },
					   []() { return 4; })
				   == 2_i);
		};

		"okMapOrElsePointer"_test = [] {
			auto value = true;
			Result<bool*> ok = Ok(&value);

			expect(ok.map_or_else(
					   [](const bool* _val) noexcept -> int {
						   ignore(_val);
						   return 2;
					   },
					   []() { return 4; })
				   == 2_i);
		};

		"errMapOrElseValue"_test = [] {
			auto error = error::SystemError(2);
			Result<bool> err = Err(std::move(error));

			expect(err.map_or_else(
					   [](const bool _val) noexcept -> int {
						   ignore(_val);
						   return 2;
					   },
					   []() { return 4; })
				   == 4_i);
		};

		"errMapOrElsePointer"_test = [] {
			auto error = error::SystemError(2);
			Result<bool, error::SystemError*> err = Err(&error);

			expect(err.map_or_else(
					   [](const bool _val) noexcept -> int {
						   ignore(_val);
						   return 2;
					   },
					   []() { return 4; })
				   == 4_i);
		};

		"okMapErrValue"_test = [] {
			auto value = true;
			Result<bool> ok = Ok(value);

			auto maybe_ok = ok.map_err(
				[]([[maybe_unused]] const error::SystemError& _val) noexcept -> error::SystemError {
					return {3};
				});
			expect(maybe_ok.is_ok() == TRUE);
			expect(maybe_ok.unwrap() == TRUE);
		};

		"okMapErrPointer"_test = [] {
			auto value = true;
			Result<bool*> ok = Ok(&value);

			auto maybe_ok = ok.map_err(
				[]([[maybe_unused]] const error::SystemError& _val) noexcept -> error::SystemError {
					return {3};
				});
			expect(maybe_ok.is_ok() == TRUE);
			expect(*(maybe_ok.unwrap()) == TRUE);
		};

		"errMapErrValue"_test = [] {
			auto error = error::SystemError(2);
			Result<bool> err = Err(std::move(error));

			auto maybe_ok = err.map_err(
				[]([[maybe_unused]] const error::SystemError& _val) noexcept -> error::SystemError {
					return {3};
				});
			expect(maybe_ok.is_err() == TRUE);
			expect(maybe_ok.unwrap_err().value() == 3_i);
		};

		"errMapErrPointer"_test = [] {
			auto error = error::SystemError(2);
			Result<bool, error::SystemError*> err = Err(&error);

			auto maybe_ok
				= err.map_err([](const error::SystemError* _val) noexcept -> error::SystemError {
					  ignore(_val);
					  return {3};
				  });
			expect(maybe_ok.is_err() == TRUE);
			expect(maybe_ok.unwrap_err().value() == 3_i);
		};

		"okMovePointer"_test = [] {
			// NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
			auto value = true;
			Result<bool*> ok = Ok(&value);

			constexpr auto ok_move_test = [](Result<bool*>&& o) noexcept -> void {
				expect(o.is_ok() == TRUE);
				expect(o == TRUE);
				expect(o.is_err() == FALSE);
				expect(*(o.unwrap()) == TRUE);
			};
			ok_move_test(std::move(ok));
		};

		"errMovePointer"_test = [] {
			auto error = error::SystemError(2);
			Result<bool*, error::SystemError*> err = Err(&error);

			constexpr auto err_move_test
				= [](Result<bool*, error::SystemError*>&& e) noexcept -> void {
				expect(e.is_ok() == FALSE);
				expect(e == FALSE);
				expect(e.is_err() == TRUE);
				expect(e.unwrap_err()->value() == 2_i);
			};
			err_move_test(std::move(err));
		};
	};
} // namespace hyperion::test
