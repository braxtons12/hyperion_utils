# HyperionUtils

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


### Quick Start

For how to get started using hyperion-utils, check out the [Quick Start Guide](https://braxtons12.github.io/Hyperion-Utils/quick_start.html).

### Documentation

You can also find the rest of the project documentation [here](https://braxtons12.github.io/Hyperion-Utils/)

### Example

After following the Quick Start Guide, you'll be able to use hyperion-utils in your project.
A basic example of what hyperion-utils can do is below:

```cpp

#include "HyperionUtils/HyperionUtils.h"

using hyperion::Option;
using hyperion::Some;
using hyperion::None;
using hyperion::MESSAGE;
using hyperion::i32;
using hyperion::operator""_i32;

using LogParams = hyperion::logging::DefaultParameters;

struct Thing{i32 x = 0;, i32 y = 2; };

bool condition = true;
inline auto get_thing() -> Option<Thing> {
	if(condition) {
		return Some(Thing{.x=42_i32});
	}
	else {
		return None();
	}
}

inline auto log_thing() -> void {
    if(auto thing = get_thing()) {
        // Logging with the global logger can return an error if the logger hasn't been initialized,
        // or the logging policy implies logging can fail (for example, if using an ayschronous
        // logger that discards entries when the queue is full). Because of this, the default
        // logging functions return a hyperion::Result.
        // For this simple example, we'll just ignore it
        ignore(MESSAGE(
                "{}", // format string, see libfmt
                thing.unwrap().x); //the stuff we want to log.
    }
    else {
        ignore(ERROR("thing was None!"));
    }
}

auto main(i32 argc, char** argv) -> i32 {
    auto file_sink = hyperion::logging::FileSink::create_file()
                     .and_then(hyperion::logging::make_sink<hyperion::logging::FileSink, hyperion::fs::File&&>)
                     .expect("Failed to create the example logging file!");
    auto sinks = hyperion::logging::Sinks();
    sinks.push_back(std::move(file_sink));
    auto logger = hyperion::make_unique<hyperion::Logger<Parameters>>(std::move(sinks));
    hyperion::GlobalLog::set_global_logger(std::move(logger));

    log_thing();
}

```

### Testing

Tests are currently setup as a separate target build and use the Doctest testing framework.
To run the tests, simply configure the project and build the test target, then run the resulting executable:<br>

```sh

cmake -B build -G "Ninja"
cmake --build build --target HyperionUtilsTest
./build/HyperionUtilsTest

```

### Contributing

Feel free to submit issues, pull requests, etc!<br>
When contributing code, please following the project `.clang-format` (except in judicious cases of
templates or requires clauses ruining formatting), use trailing returns types, use assign-init over direct-init
(parenthesis or braced init), and prefer simplicity and correctness over performance by default

### License
HyperionUtils uses the MIT license.

#### What are these toolchain files?

HyperionUtils includes an assortment of CMake toolchain files. These are simply a quick and easy
(and lazy) way for us to use an associated platform + compiler combination with our preferred
optimization flags, without hard coding them in the project `CMakeLists.txt`. They're **NOT**
necessary to use HyperionUtils and they won't affect your project unless you pass them as a
toolchain file argument to CMake
