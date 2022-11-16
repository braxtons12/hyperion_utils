Panic
*****

Hyperion provides a facility for "failing fast" when an irrecoverable error occurs. This is similar
to :cpp:`std::terminate()`, but more closely models Rust's :rust:`panic!`, and is aptly named so,
:cpp:`panic`. :cpp:`panic` will print info about the panic site as well as a runtime-formatted
error message using fmtlib and :cpp:`std::format` style string formatting before calling the
registered panic handler. The default panic handler will terminate the program, but it can be
overridden to provide custom behavior (for example, showing an error dialog in a GUI application).

.. code-block:: cpp
    :caption: Example
    :linenos:

    #include <Hyperion/error/Panic.h>

    // Input can't be 24 through 42, or bad things will happen
    void example_function(hyperion::i32 input) {
        if(input >= 24 && input <= 42) {
            panic("example_function called with input value in the range [24, 42]\ninput was {}",
                  input);
        }
    }

.. doxygenclass:: hyperion::error::Panic
   :members:
   :undoc-members:

.. doxygendefine:: panic
