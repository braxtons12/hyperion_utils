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

Currently, Hyperion doesn't have a dedicated repository for its XMake pages, so you'll need to
clone the repository or add it as a submodule first:

.. code-block:: bash

    git clone https://github.com/braxtons12/Hyperion-Utils hyperion_utils --recursive

.. code-block:: lua
    :caption: xmake.lua
    :linenos:

    -- add the hyperion-utils export directory as an XMake repository
    add_repositories("local-repo PATH_TO_CLONED_HYPERION_UTILS/export")
    -- add hyperion-utils as a required dependency
    add_requires("hyperion-utils", {
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

In the future, Hyperion will have a dedicated repository for its xmake packages, which will let you skip the cloning step 
