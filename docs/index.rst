hyperion-utils
**************

hyperion-utils is a foundational library for the Hyperion game engine, containing many of the
"vocabulary"-level types, data structures, and core utilities used internally by the engine.

It's currently under active development and has not yet hit a stable release point.

hyperion-utils is not dependent on other parts of the engine and can be used freely, separately from
any other Hyperion projects.

Some of the notable features include:

- Robust, compile-time configurable logging (logging level, sinks, global and local loggers,
  threading support)
- Unique data structures (ringbuffer, lock-free multi-producer, single consumer queue)
- Robust error handling facilities similar to :cpp:`boost::outcome` and Rust,
  and semantics and an API closely matching Rust's :rust:`Result<T, E>` and :rust:`Option<T>`
- Various meta-programming facilities including custom Concepts, Type Traits, and a small
  meta-programming library
- Synchronization primitives similar to Rust's owning synchronization types like :rust:`Mutex<T>` and :rust:`RwLock<T>`
- :cpp:`Enum<Types...>`, a C++20 alternative to std::variant with improved API, compile-time, and performance,
  designed to more closely model algebraic data types from other languages


See the :doc:`Quick Start Guide <quick_start>` for how to get started using hyperion-utils.

For an overview of each module, see the links in the left sidebar or below.

.. toctree::
    :caption: Getting Started

    quick_start

.. toctree::
    :caption: Quick Reference

    genindex

.. toctree::
    :caption: Error Handling

    error_handling/index

.. toctree::
    :caption: Filesystem Access

    filesystem/index

.. toctree::
    :caption: Memory Utilities and Smart Pointers

    memory/index

.. toctree::
    :caption: Synchronization Utilities

    synchronization/index

.. toctree::
    :caption: Enum
    
    enum
