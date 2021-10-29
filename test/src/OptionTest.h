#pragma once

#include <Hyperion/Error.h>
#include <Hyperion/Option.h>
#include <Hyperion/Result.h>

#include "microTest.h"

namespace hyperion::test {

	const suite OptionTests = [] { // NOLINT
		"someMapping"_test = [] {
			auto some = Some(true);

			expect(some.is_some() == TRUE);
			expect(some.is_none() == FALSE);
			expect(some.map([](const bool some_value) noexcept -> bool { return some_value; })
					   .is_some()
				   == TRUE);
			expect(some.map([](const bool some_value) noexcept -> bool { return some_value; })
					   .is_none()
				   == FALSE);
			expect(some.map_or([](const bool some_value) noexcept -> bool { return some_value; },
							   false)
				   == TRUE);
			expect(
				some.map_or_else([](const bool some_value) noexcept -> bool { return some_value; },
								 []() noexcept -> bool { return false; })
				== TRUE);
		};

		"noneMapping"_test = [] {
			Option<bool> none = None();

			expect(none.is_none() == TRUE);
			expect(none.is_some() == FALSE);
			expect(none.map([](const bool some_value) noexcept -> bool { return some_value; })
					   .is_none()
				   == TRUE);
			expect(none.map([](const bool some_value) noexcept -> bool { return some_value; })
					   .is_some()
				   == FALSE);
			expect(none.map_or([](const bool some_value) noexcept -> bool { return some_value; },
							   false)
				   == FALSE);
			expect(
				none.map_or_else([](const bool some_value) noexcept -> bool { return some_value; },
								 []() noexcept -> bool { return false; })
				== FALSE);
		};

		"someOkOrValue"_test = [] {
			auto some = Some(true);
			auto error = error::SystemError(2);

			expect(some.is_some() == TRUE);
			expect(some.is_none() == FALSE);

			auto res = some.ok_or(std::move(error));
			expect(res.is_ok() == TRUE);
			expect(res.unwrap() == TRUE);
		};

		"someOkOrPointer"_test = [] {
			// NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
			auto* value = new bool(true);
			auto some = Some(value);
			auto error = error::SystemError(2);

			expect(some.is_some() == TRUE);
			expect(some.is_none() == FALSE);

			auto res = some.ok_or(std::move(error));
			expect(res.is_ok() == TRUE);
			auto ok = res.ok();
			expect(ok.is_some() == TRUE);
			auto* unwrapped = ok.unwrap();
			expect((unwrapped != nullptr) == TRUE);
			expect(*unwrapped == TRUE);
		};

		"noneOkOrValue"_test = [] {
			Option<bool> none = None();
			auto error = error::SystemError(2);

			expect(none.is_none() == TRUE);
			expect(none.is_some() == FALSE);

			auto res = none.ok_or(std::move(error));
			expect(res.is_err() == TRUE);
			expect(res.unwrap_err().value() == 2_i);
		};

		"someOkOrElseValue"_test = [] {
			auto some = Some(true);

			expect(some.is_some() == TRUE);
			expect(some.is_none() == FALSE);

			auto res = some.ok_or_else([]() { return error::SystemError(2); });
			expect(res.is_ok() == TRUE);
			expect(res.unwrap() == TRUE);
		};

		"someOkOrElsePointer"_test = [] {
			// NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
			auto* value = new bool(true);
			auto some = Some(value);

			expect(some.is_some() == TRUE);
			expect(some.is_none() == FALSE);

			auto res = some.ok_or_else([]() { return error::SystemError(2); });
			expect(res.is_ok() == TRUE);
			auto* unwrapped = res.unwrap();
			expect((unwrapped != nullptr) == TRUE);
			expect(*unwrapped == TRUE);
		};

		"noneOkOrElseValue"_test = [] {
			Option<bool> none = None();

			expect(none.is_none() == TRUE);
			expect(none.is_some() == FALSE);

			auto res = none.ok_or_else([]() { return error::SystemError(2); });
			expect(res.is_err() == TRUE);
			expect(none.is_some() == FALSE);
			expect(res.unwrap_err().value() == 2_i);
		};

		"someUnwrapValue"_test = [] {
			auto some = Some(true);

			expect(some.is_some() == TRUE);
			expect(some.is_none() == FALSE);
			expect(some.unwrap() == TRUE);
		};

		"someUnwrapPointer"_test = [] {
			// NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
			auto* value = new bool(true);
			auto some = Some(value);

			expect(some.is_some() == TRUE);
			expect(some.is_none() == FALSE);

			auto* unwrapped = some.unwrap();
			expect((unwrapped != nullptr) == TRUE);
			expect(*unwrapped == TRUE);
		};

		"noneUnwrap"_test = [] {
			Option<bool> none = None();

			expect(none.is_none() == TRUE);
			expect(none.is_some() == FALSE);

			expect(aborts([&] { ignore(none.unwrap()); }));
		};

		"someUnwrapOrValue"_test = [] {
			auto some = Some(true);

			expect(some.is_some() == TRUE);
			expect(some.is_none() == FALSE);
			expect(some.unwrap_or(false) == TRUE);
		};

		"someUnwrapOrPointer"_test = [] {
			// NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
			auto* some_value = new bool(true);
			// NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
			auto* none_value = new bool(false);
			auto some = Some(some_value);

			expect(some.is_some() == TRUE);
			expect(some.is_none() == FALSE);
			auto* unwrapped = some.unwrap_or(none_value);
			expect((unwrapped != nullptr) == TRUE);
			expect(*unwrapped == TRUE);
		};

		"noneUnwrapOrValue"_test = [] {
			Option<bool> none = None();

			expect(none.is_none() == TRUE);
			expect(none.is_some() == FALSE);
			expect(none.unwrap_or(false) == FALSE);
		};

		"noneUnwrapOrPointer"_test = [] {
			// NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
			auto* none_value = new bool(false);
			Option<bool*> none = None();

			expect(none.is_none() == TRUE);
			expect(none.is_some() == FALSE);
			auto* unwrapped = none.unwrap_or(none_value);
			expect((unwrapped != nullptr) == TRUE);
			expect(*unwrapped == FALSE);
		};

		"someUnwrapOrElseValue"_test = [] {
			auto some = Some(true);

			expect(some.is_some() == TRUE);
			expect(some.is_none() == FALSE);
			expect(some.unwrap_or_else([]() { return false; }) == TRUE);
		};

		"someUnwrapOrElsePointer"_test = [] {
			// NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
			auto* some_value = new bool(true);
			// NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
			auto* none_value = new bool(false);
			auto some = Some(some_value);

			expect(some.is_some() == TRUE);
			expect(some.is_none() == FALSE);
			auto* unwrapped = some.unwrap_or_else([none_value]() { return none_value; });
			expect((unwrapped != nullptr) == TRUE);
			expect(*unwrapped == TRUE);
		};

		"noneUnwrapOrElseValue"_test = [] {
			Option<bool> none = None();

			expect(none.is_none() == TRUE);
			expect(none.is_some() == FALSE);
			expect(none.unwrap_or_else([]() { return false; }) == FALSE);
		};

		"noneUnwrapOrElsePointer"_test = [] {
			Option<bool*> none = None();
			// NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
			auto* none_value = new bool(false);

			expect(none.is_none() == TRUE);
			expect(none.is_some() == FALSE);
			auto* unwrapped = none.unwrap_or_else([none_value]() { return none_value; });
			expect((unwrapped != nullptr) == TRUE);
			expect(*unwrapped == FALSE);
		};

		"someAsMutValue"_test = [] {
			auto some = Some(true);

			expect(some.is_some() == TRUE);
			expect(some.is_none() == FALSE);
			auto& gotten_mut = some.as_mut();
			expect(gotten_mut == TRUE);
			gotten_mut = false;
			auto& gotten_mut2 = some.as_mut();
			expect(gotten_mut2 == FALSE);
		};

		"someAsMutPointer"_test = [] {
			// NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
			auto* some_value = new bool(true);
			auto some = Some(some_value);

			expect(some.is_some() == TRUE);
			expect(some.is_none() == FALSE);
			auto* gotten_mut = some.as_mut();
			expect(*gotten_mut == TRUE);
			*gotten_mut = false;
			gotten_mut = some.as_mut();
			expect(*gotten_mut == FALSE);
		};

		"noneAsMutValue"_test = [] {
			Option<bool> none = None();

			expect(none.is_none() == TRUE);
			expect(none.is_some() == FALSE);
			expect(aborts([&] { ignore(none.as_mut()); }));
		};

		"noneAsMutPointer"_test = [] {
			Option<bool*> none = None();

			expect(none.is_none() == TRUE);
			expect(none.is_some() == FALSE);
			expect(aborts([&] { ignore(none.as_mut()); }));
		};

		"someAsConstValue"_test = [] {
			auto some = Some(true);

			expect(some.is_some() == TRUE);
			expect(some.is_none() == FALSE);
			const auto& gotten_const = some.as_const();
			expect(gotten_const == TRUE);
		};

		"someAsConstPointer"_test = [] {
			// NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
			auto* some_value = new bool(true);
			auto some = Some(some_value);

			expect(some.is_some() == TRUE);
			expect(some.is_none() == FALSE);
			const auto* gotten_const = some.as_const();
			expect(*gotten_const == TRUE);
		};

		"noneAsConstValue"_test = [] {
			Option<bool> none = None();

			expect(none.is_none() == TRUE);
			expect(none.is_some() == FALSE);
			expect(aborts([&] { ignore(none.as_const()); }));
		};

		"noneAsConstPointer"_test = [] {
			Option<bool*> none = None();

			expect(none.is_none() == TRUE);
			expect(none.is_some() == FALSE);
			expect(aborts([&] { ignore(none.as_const()); }));
		};

		"someMovePointer"_test = [] {
			// NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
			auto* some_value = new bool(true);
			{
				auto some = Some(some_value);

				constexpr auto some_move_test = [](Option<bool*>&& s) noexcept -> void {
					expect(s.is_some() == TRUE);
					expect(s.is_none() == FALSE);
					auto* unwrapped = s.unwrap();
					expect((unwrapped != nullptr) == TRUE);
					expect(*unwrapped == TRUE);
				};

				some_move_test(std::move(some));
			}
		};

		"noneMovePointer"_test = [] {
			Option<bool*> none = None();
			constexpr auto none_move_test = [](Option<bool*>&& n) noexcept -> void {
				expect(n.is_none() == TRUE);
				expect(n.is_some() == FALSE);
				expect(aborts([&] { ignore(n.unwrap()); }));
			};
			none_move_test(std::move(none));
		};
	};
} // namespace hyperion::test
