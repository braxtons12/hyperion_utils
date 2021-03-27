# HyperionUtils

HyperionUtils is a header-only library of utilities and underlying structures
used in the Hyperion game engine.<br>
It is currently under active development (no stable release yet).

HyperionUtils can be used separately from the engine itself!

Some of the utilities include:

- Logging
- Data Structures
- Basic Error type
- Rust-style algebraic datatypes (alternatives to std::optional and boost::outcome::result/outcome::result)
- Concepts and Type Traits
- Rust-style owning synchronization types

## Getting Started

HyperionUtils uses CMake, and incorporating it into your project is as easy as:

To make HyperionUtils visible:<br>
In CMakeLists.txt:

```cmake
FetchContent_Declare(HyperionUtils
	GIT_REPOSITORY "https://github.com/braxtons12/Hyperion-Utils"
	GIT_TAG origin/master
	)

FetchContent_MakeAvailable(HyperionUtils)
```
Then, if using the entire library or the logging facilities,<br>
e.g., you have either of

```cpp
#include <HyperionUtils/HyperionUtils.h>
```
OR

```cpp
#include <HyperionUtils/Logger.h>
```
in your code, add these link libraries:

```cmake
target_link_libraries(your_target fmt::fmt HyperionUtils)
```

or if using other individual components, just:

```cmake
target_link_libraries(your_target HyperionUtils)
```

and in code:

```cpp
#include "HyperionUtils/HyperionUtils.h"

using hyperion::utils::Option;

struct Thing{};

bool condition = false;
inline auto get_thing() -> Option<Thing> {
	if(condition) {
		return Some(Thing{});
	}
	else {
		return None();
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

#### Toolchain Files?

HyperionUtils includes an assortment of CMake toolchain files. These are simply a quick and easy
(and lazy) way to use an associated platform + compiler combination with our preferred optimization
flags and are **NOT** necessary to use HyperionUtils and won't affect your project unless you pass
them as a toolchain file argument to CMake

### Contributing

Feel free to submit issues, pull requests, etc!

HyperionUtils uses the MIT license, so please make sure you agree to its terms before submitting
any code.
