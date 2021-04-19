# HyperionUtils

HyperionUtils is a header-only C++20 library of utilities, data-structures, and monads
used in the Hyperion game engine.<br>
It is currently under active development and has not yet hit a release point.

HyperionUtils is not dependent on the engine and can be used freely separately from any other
Hyperion projects.

Some of the features of HyperionUtils include:

- Robust, Compile-time configurable logging (in progress)
- Datastructures, such as queues supporting lock-free concurrency (initial implementation complete; API unlikely to change, implementation subject to change)
- Error handling facilities similar to Rust and boost::outcome with equivalent semantics and API to Rust (complete)
- Option monad based on Rust's Option with equivalent semantics and API to Rust (complete)
- Concepts, Type Traits, and meta-programming functions (in progress; Those already implemented unlikely to change)
- Rust-style owning synchronization types (in progress)

## Documentation

You can view the documentation [here](https://braxtons12.github.io/Hyperion-Utils/)

## Getting Started

HyperionUtils uses CMake, and incorporating it into your project is easy!

HyperionUtils depends on [fmt](https://github.com/fmtlib/fmt), so you will have to link to fmt in
your target.

First, setup your CMake project.
In `CMakeLists.txt`:

```cmake

FetchContent_Declare(HyperionUtils
	GIT_REPOSITORY "https://github.com/braxtons12/Hyperion-Utils"
	GIT_TAG origin/master
	)

FetchContent_MakeAvailable(HyperionUtils)

### Setup your target......

target_link_libraries(your_target fmt::fmt HyperionUtils)

```

Then, include your desired headers, either the main header, `HyperionUtils/HyperionUtils.h`, for everything,
or individual ones for granular imports. If you want to use `Option` or `Result`, please include them
through `Monads.h` or the main header instead of individually, as the API surface in each is
dependent on the other

### Example

First, import HyperionUtils.
In `CMakeLists.txt`:

```cmake

FetchContent_Declare(HyperionUtils
	GIT_REPOSITORY "https://github.com/braxtons12/Hyperion-Utils"
	GIT_TAG origin/master
	)

FetchContent_MakeAvailable(HyperionUtils)

### Add your target, etc...

target_link_libraries(your_target fmt::fmt HyperionUtils)

```

Then, in your code:

```cpp

#include "HyperionUtils/HyperionUtils.h"

using hyperion::Option;
using hyperion::Some;
using hyperion::None;
using hyperion::MESSAGE;

using LogParams = hyperion::DefaultLogParameters;

struct Thing{int x = 0;, int y = 2; };

bool condition = true;
inline auto get_thing() -> Option<Thing> {
	if(condition) {
		return Some(Thing{.x=42});
	}
	else {
		return None();
	}
}

inline auto log_thing() -> void {
    if(auto thing = get_thing()) {
        // Logging can return an error depending on the logging policy, or if the
		// configured log level is higher than the logging entry we tried to queue.
		// Thus, MESSAGE returns a Result to communicate this. For this simple
		// example, we'll just ignore it
        ignore(MESSAGE<LogParams>(
                None(), // Optional thread identifier
                "{}", // format string, see fmt
                thing.unwrap().x); //the stuff we want to log.
				// If thing were None, this would call std::terminate
    }
}

```

### Testing

Tests are currently setup as an isolated project in the "test" subdirectory.<br>
This prevents collisions between other googletest builds in Hyperion's other sublibraries.<br>
To run the tests, simply configure and build the test project, then run the resulting "Test" executable:<br>

```sh

cmake -B build -G "Ninja"
cmake --build build
./build/Test

```

### Contributing

Feel free to submit issues, pull requests, etc!<br>
When contributing code, please following the project `.clang-format` (except in judicious cases of
templates or requires clauses ruining things), use trailing returns types, use assign-init over direct-init
(parenthesis or braced init), and prefer simplicity and correctness over performance by default

### License
HyperionUtils uses the MIT license.

#### What are these toolchain files?

HyperionUtils includes an assortment of CMake toolchain files. These are simply a quick and easy
(and lazy) way for us to use an associated platform + compiler combination with our preferred
optimization flags, without hard coding them in the project `CMakeLists.txt`. They're **NOT**
necessary to use HyperionUtils and they won't affect your project unless you pass them as a
toolchain file argument to CMake
