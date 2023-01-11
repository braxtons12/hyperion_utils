Filesystem
**********

Hyperion enhances standard filesystem facilities with additional types to make file I/O safer and
more efficient. These generally all revolve around :cpp:`hyperion::fs::File`, Hyperion's type
handing file access and I/O. 

.. toctree::
    :caption: Modules

    File

.. code-block:: cpp
    :caption: Example
    :linenos:

    #include <Hyperion/Filesystem.h>

    auto example() -> void {
        using namespace hyperion::fs;
        auto maybe_file = File::open("MyFile.txt",
                                     File::OpenOptions{.type = File::AccessType::Write,
                                                       .modifier = File::AccessModifier::None});
        auto file = maybe_file.expect("Failed to open MyFile.txt");
        file.println("Hello World! This is the secret of life: {}!", 42);
    }

