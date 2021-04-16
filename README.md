# HyperionUtils

HyperionUtils is a header-only C++20 library of utilities, data-structures, and monads
used in the Hyperion game engine.<br>
It is currently under active development and has not yet hit a release point.

HyperionUtils is not dependent on the engine and can be used freely separately from any other
Hyperion projects.

Some of the features of HyperionUtils include:

- Robust, Compile-time configurable logging (in progress)
- Datastructures, such as queues supporting lock-free concurrency (initial implementation complete; API unlikely to change, implementation subject to change)
- Error handling facilities similar to Rust and boost::outcome (initial implementation complete; API unlikely to change, implementation subject to change)
- Option monad based on Rust's Option (complete)
- Concepts, Type Traits, and meta-programming functions (in progress; Those already implemented unlikely to change)
- Rust-style owning synchronization types (in progress)

## Getting Started

HyperionUtils uses CMake, and incorporating it into your project is easy!

There are several ways to use HyperionUtils, and the necessary additions to your `CMakeLists.txt`
will change depending on your choice. This is because HyperionUtils depends on
[fmt](https://github.com/fmtlib/fmt) in the logging code

In any case, you'll first need to add HyperionUtils to your CMake project.
In `CMakeLists.txt`:

```cmake
FetchContent_Declare(HyperionUtils
	GIT_REPOSITORY "https://github.com/braxtons12/Hyperion-Utils"
	GIT_TAG origin/master
	)

FetchContent_MakeAvailable(HyperionUtils)
```

1. If you wish to only include indiviual headers and aren't using the logging facilities
	1. Include whatever headers you wish to use in your code
	2. In `CMakeLists.txt`:

```cmake
target_link_libraries(your_target HyperionUtils)
```

2. If you wish to only include individual headers and _are_ using the logging facilities
	1. Include whatever headers you wish to use in your code
	2. In `CMakeLists.txt`:

```cmake
target_link_libraries(your_target fmt::fmt HyperionUtils)
```

3. If you want to just include the main header, `HyperionUtils/HyperionUtils.h`
	2. If you don't want to use the logging facilities, configure your `CMakeLists.txt` like 1-i above
	and include `HyperionUtils/HyperionUtils.h`
	1. If you want to use the logging facilities, configure your `CMakeLists.txt` like in 2-i above,
	then define `HYPERION_INCLUDE_LOGGING_BY_DEFAULT` in your compiler flags or prior to including
	`HyperionUtils/HyperionUtils.h`, eg:

```cpp
#define HYPERION_INCLUDE_LOGGING_BY_DEFAULT
```

### Example

In `CMakeLists.txt`

```cmake
FetchContent_Declare(HyperionUtils
	GIT_REPOSITORY "https://github.com/braxtons12/Hyperion-Utils"
	GIT_TAG origin/master
	)

FetchContent_MakeAvailable(HyperionUtils)

### Add your target, etc...

target_link_libraries(your_target fmt::fmt HyperionUtils)

```

In your code:

```cpp

#include "HyperionUtils/HyperionUtils.h"

using hyperion::utils::Option;
using hyperion::utils::MESSAGE;

using LogParams = hyperion::utils::DefaultLogParameters;

struct Thing{int x = 0;, int y = 2; };

bool condition = false;
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
        // The default logging policy will bail if the queue is full instead of blocking, and thus
        // returns a Result indicating whether it succeded. We'll just ignore it
        ignore(MESSAGE<LogParams>(
                None(), // Optional thread identifier
                "{}", // format string, see fmt
                thing.unwrap().x); //the stuff we want to log
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
templates ruining things), use trailing returns types, use assign-init over direct-init
(parenthesis or braced init), and prefer simplicity and correctness over performance by default

### License
HyperionUtils uses the MIT license.

#### What are these toolchain files?

HyperionUtils includes an assortment of CMake toolchain files. These are simply a quick and easy
(and lazy) way for us to use an associated platform + compiler combination with our preferred
optimization flags, without hard coding them in the project `CMakeLists.txt`. They're **NOT**
necessary to use HyperionUtils and they won't affect your project unless you pass them as a
toolchain file argument to CMake
