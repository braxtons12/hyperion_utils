StatusCode
**********

Overview
--------

This module provides Hyperion's basic low-level functionality for representing errors.
This functionality includes defining the basic requirements for status code domain types,
status codes, and the machinery for working with them.

Status code domains (in code, :cpp:`StatusCodeDomain` s) represent domains, or categories, from which
potential errors can occur. For example, :cpp:`PosixDomain`, :cpp:`Win32Domain`, and :cpp:`NTDomain`
are all :cpp:`StatusCodeDomain` s that represent errors that can occur from POSIX functions and
Windows's Win32 and NT subsystems, respectively. :cpp:`StatusCodeDomain` s provide the functionality
to convert from low-level error codes from their respective domain to textual error messages those
codes represent, check whether a given code represents success or failure, and convert a code from
its domain to the :cpp:`GenericDomain`'s :cpp:`hyperion::error::Errno` (if applicable).

.. toctree::
    :caption: Modules

    status_codes/StatusCodeDomain
    status_codes/StatusCode
    status_codes/PosixDomain
    status_codes/Win32Domain
    status_codes/NTDomain
    status_codes/GenericDomain
    status_codes/SystemDomain

Examples
""""""""

.. code-block:: cpp
    :caption: Using the System Domain
    :linenos:

    // The system domain is the status code domain associated with general system library errors
    // (ie errors from the POSIX and C standard libraries on UNIX, or the Win32 library on Windows).
    #include <Hyperion/error/SystemDomain.h>
    auto example_checked_add(hyperion::i32 lhs, hyperion::i32 rhs, hyperion::i32* out_val)
        noexcept -> hyperion::error::SystemStatusCode {

        // if rhs + lhs > std::numeric_limits<i32>::max(), the operation will overflow, so we return
        // an error code
        if((std::numeric_limits<hyperion::i32>::max() - lhs) > rhs) {
            //
            if constexpr(std::same_as<hyperion::error::Win32Domain, hyperion::error::SystemDomain>) {
                return hyperion::error::SystemStatusCode(ERROR_INVALID_PARAMETER)
            }
            else {
                return hyperion::error::SystemStatusCode(EDOM)
            }
        
        }

        *out_val = lhs + rhs;
        return hyperion::error::SystemStatusCode(
            hyperion::error::SystemStatusCode::domain_type::success_value());
    }

    auto use_checked_add() noexcept -> void {
        using hyperion::operator""_i32;
        const auto lhs = 18_i32; 
        const auto rhs = 24_i32;
        auto out = 0_i32;

        auto result = example_checked_add(lhs, rhs, &out);
        if(hyperion::error::SystemDomain::is_error(result)) {
            hyperion::eprintln("checked add was an error:\nerror {}: {}",
                               result.code(), result.message());
            return;
        }

        hyperion::println("checked add result: {}", out);
    }

.. code-block:: cpp
   :caption: Creating Your Own :cpp:`StatusCodeDomain`
   :linenos:

    #include <Hyperion/Error.h>    
    namespace my_namespace {
        // create an enum we use to represent our low-level status codes
        enum class MyErrorCategory {
            0 = Success,
            1 = BadThing,
            2 = ReallyBadThing,
            -1 = Unknown
        };
    } // namespace my_namespace

    // hyperion provides this macro to automate the generation of the boilerplate needed for a
    // `StatusCodeDomain`. This automatically generates the class definition, overloaded definitions
    // of `make_status_code_domain`, shorthand aliases for `StatusCode`, `ErrorCode`, and `Error`
    // instantiations for the domain, and a static assert ensuring `StatusCodeDomain`'s requirements
    // are met by the generated code
    STATUS_CODE_DOMAIN(
        MyErrorDomain, // The name of the `StatusCodeDomain` class
        My, // The root-name of the `StatusCodeDomain`, used to prefix aliases for
                 // `StatusCode<MyErrorDomain>` and `ErrorCode<MyErrorDomain>`, ie:
                 // `using MyStatusCode = StatusCode<MyErrorDomain>;`
        my_namespace, // the namespace to place the `StatusCodeDomain` class declaration in
        my_namespace::MyErrorCategory, // the value type of the `StatusCodeDomain`
        true, // whether codes of this domain are convertible to codes of the generic domain
              // generally, you should at least provide conversions for your success and unknown
              // values, but it isn't necessary
        "029ddb93-2245-ab31-bc89-928db01aefb5", // The UUID of the domain
        "MyErrorDomain", // The name of the domain class, as a string
        my_namespace::MyErrorCategory::Success, // The domain's success value
        my_namespace::MyErrorCategory::Unknown, // The domain's official value for unknown errors
        // Function converting a `value_type` (ie a `my_namespace::MyErrorCategory`) into its
        // associated error message
        [](value_type code) noexcept -> std::string_view {
            if(code == value_type::Success) {
                return "Success";
            }
            if(code == value_type::BadThing) {
                return "A Bad Thing Happened!";
            }
            if(code == value_type::ReallyBadThing) {
                return "A Really Bad Thing Happened!";
            }

            return "Unknown error";
        },
        // if codes of this domain are convertible to codes in the generic domain, provide a
        // function converting a `value_type` to a `hyperion::error::GenericStatusCode`.
        // Otherwise, this parameter can be omitted.
        [](value_type code) noexcept -> hyperion::error::GenericStatusCode {
            if(code == value_type::Success) {
                return make_status_code(hyperion::error::Errno::Success);
            }
            
            return make_status_code(hyperion::error::Errno::Unknown);
        });

    // specialize `status_code_enum_info` to register our `my_namespace::MyErrorCategory` enum as
    // the value type for a `StatusCodeDomain`, specifically, our `my_namespace::MyErrorDomain` domain
    template<>
    struct hyperion::error::status_code_enum_info<my_namespace::MyErrorCategory> {
        using domain_type [[maybe_unused]] = my_namespace::MyErrorDomain;
        static constexpr bool value = true;
    }

