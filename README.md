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

in CMakeLists.txt:

```
FetchContent_Declare(HyperionUtils
	GIT_REPOSITORY "https://github.com/braxtons12/Hyperion-Utils"
	)

FetchContent_MakeAvailable(HyperionUtils)

add_executable(YourExecutable yourSource.cpp)

target_link_libraries(YourExecutable HyperionUtils)
```

and in code:

```
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
We aren't currently *thrilled* about this approach, (or at least, our current *implementation* of this approach)<br>
but it prevents collisions between other googletest builds in Hyperion's libraries.<br>
To run the tests, simply configure and build the test project, then run the resulting "Test" executable.

#### Toolchain Files?

HyperionUtils includes an assortment of CMake toolchain files. These are simply a quick and easy
(and lazy) way to use an associated platform + compiler combination with our preferred optimization
flags and are **NOT** necessary to use HyperionUtils and won't affect your project unless you pass
them as a toolchain file argument to CMake

### Contributing

Feel free to submit issues, pull requests, etc!

HyperionUtils uses the MIT license, so please make sure you agree to its terms before submitting
any code.
