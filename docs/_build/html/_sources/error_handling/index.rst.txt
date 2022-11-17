Error Handling
**************

Hyperion provides robust methods for communicating, propagating, and handling errors, from low-level
basic functionality like simple status code types to high-level ergonomics, including monadic
sum-types similar to Rust's :rust:`Option` and :rust:`Result` and C++17's :cpp:`std::optional` and
C++23's :cpp:`std::expected`. Create custom error code categories in
:cpp:`hyperion::error::StatusCodeDomain` s, create codes from those categories via
:cpp:`hyperion::error::StatusCode<hyperion::error::StatusCodeDomain>` s, package them in
:cpp:`hyperion::error::Error`, and communicate and propagate them via :cpp:`hyperion::Result<T, E>`.
Communicate optional values or obvious failure cases (for example, in a checked math function)
with :cpp:`hyperion::Option<T>`. Assert valid pre and post conditions at runtime with
:cpp:`HYPERION_ASSERT` and fail fast and gracefully when irrecoverable errors occur with :cpp:`panic`.

.. toctree::
    :caption: Modules

    StatusCode
    Error
    Option
    Result
    Assert
    Panic
 
.. code-block:: cpp
    :caption: Example
    :linenos:

    #include <Hyperion/Error.h>
    #include <Hyperion/Result.h>
    #include <Hyperion/Option.h>

    using hyperion::i32;
    using hyperion::operator""_i32;
    using hyperion::Err;
    using hyperion::Ok;
    using hyperion::None;
    using hyperion::Some;

    // import calculation returns an i32 on success, but can fail with a POSIX error code
    auto important_calculation(i32 input) -> hyperion::Result<i32, hyperion::error::PosixError>;
    // checked_add does a basic range-checked add, returning None if the result would be out of

    // range
    auto checked_add(i32 lhs, i32 rhs) -> hyperion::Option<i32> {
        if(std::numeric_limits<i32>::max() - lhs < rhs || std::numeric_limits<i32>::min() + lhs > rhs) {
            return None();
        }

        return Some(lhs + rhs);
    } 

    auto important_calculation2(i32 input) -> hyperion::Result<i32, hyperion::error::PosixError> {
        // input can't be the meaning of life
        HYPERION_ASSERT(input != 42_i32,
                        "Invalid input for input in important_calculation2: {}",
                        input);
        /// Get the results of our calculations, check if they were successful, then return the
        /// result or error
        auto res = important_calculation(input);
        auto sum = checked_add(input, 42_i32);
        
        // if the checked add was out of range, return an EDOM error to communicate that
        if(sum.is_none()) {
            return Err(hyperion::error::PosixError(EDOM));
        }

        if(res.is_err()) {
            return res;
        }

        return Ok(sum.unwrap() * res.unwrap());

        // Alternatively, `Option` and `Result` both have shorthands for these operations, `ok_or`
        // and `and_then`, respectively, and we could have used them like this:

        // if checked_add returned `Some(value)`, return `Ok(value)`, otherwise return
        // `Err(hyperion::error::PosixError(EDOM)`
        checked_add(input, 42_i32)
            .ok_or(hyperion::error::PosixError(EDOM))
            // If the result of the previous calls was `Ok(lhs)`, call the lambda and return that
            // value as a `Result`. Otherwise, propagate the previous error
            .and_then([input](auto lhs) {
                // If `important_calculation2` was `Ok(rhs)`, call the lambda and return that value
                // as a `Result`. Otherwise, propagate the previous error
                important_calculation2(input).and_then([lhs](auto rhs) {
                    return lhs * rhs;
                });
            });
    }

    auto get_important_value() -> i32 {
        // get the result of `important_calculation2`, fail if it's an error, return the result
        // if it was successful
        auto result = important_calculation2(63_i32);

        // if the result of important_calculation2 is an error, we don't have a way to handle it
        // so we trigger a panic
        if(result.is_err()) {
            panic("get_important_value failed! We can't continue!");
        }

        return result.unwrap();

        // Alternatively, `Result` has a shorthand for this pattern, `expect` and we could have
        // called it instead of manually checking for the error like this:
        return important_calculation2(63_i32)
                    .expect("get_important_value failed! We can't continue");
    }
