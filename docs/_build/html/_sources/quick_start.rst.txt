Quick Start Guide
*****************

hyperion-utils supports both CMake and XMake, and incorporating it in your project is quick and easy.

CMake
-----

hyperion-utils is easily incorporated in a CMake project with FetchContent:

.. code-block:: cmake
    :caption: CMakeLists.txt
    :linenos:

    # Include FetchContent so we can use it
    include(FetchContent)
    # Declare the dependency on hyperion-utils and make it available for use
    FetchContent_Declare(HyperionUtils
        GIT_REPOSITORY "https://github.com/braxtons12/Hyperion-Utils"
        GIT_TAG "origin/master")

    FetchContent_MakeAvailable(HyperionUtils)
    # For this example, we create an executable target
    add_executable(MyExecutable "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp")
    # Add hyperion-utils as a dependency to our executable, "MyExecutable"
    target_link_libraries(MyExecutable PRIVATE Hyperion::Utils)

XMake
-----

.. code-block:: lua
    :caption: xmake.lua
    :linenos:

    -- add the hyperion-packages git repository as an XMake repository
    add_repositories("hyperion_utils https://github.com/braxtons12/hyperion-packages.git")
    -- add hyperion-utils as a required dependency
    add_requires("hyperion-utils", {
        -- Don't verify the git commit. This is necessary because hyperion-utils hasn't reached a
        -- official release yet, and so doesn't have a stable git commit to track.
        -- This allows pulling directly from $HEAD
        verify = false,
        configs = {
            -- optional, enables profiling with Tracy, defaults to false
            hyperion_tracy_enable = true
        }
    })
    
    -- For this example, we create an executable target
    target("my_executable")
        set_kind("binary")
        -- Add hyperion-utils as a dependency to our executable, "my_executable"
        add_packages("hyperion-utils")
