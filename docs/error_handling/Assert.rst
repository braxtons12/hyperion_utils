Assert
******

Hyperion provides a runtime assert similar to the standard :cpp:`assert`, but that supports an 
additional message string using fmtlib and :cpp:`std::format` style string formatting

.. code-block:: cpp
    :caption: Example
    :linenos:

    #include <Hyperion/error/Assert.h>

    void example_function(hyperion::i32 cant_be_arg2, hyperion::i32 cant_be_arg1) noexcept {
        HYPERION_ASSERT(cant_be_arg2 != cant_be_arg1,
                        "example_function called with cant_be_arg2 ({}) equal to cant_be_arg1 ({})",
                        cant_be_arg2,
                        cant_be_arg1);
    }

.. doxygendefine:: HYPERION_ASSERT
