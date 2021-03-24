SET(CMAKE_SYSTEM_NAME Windows)
SET(CMAKE_C_COMPILER clang)
SET(CMAKE_CXX_COMPILER clang++)
SET(CMAKE_RC_COMPILER llvm-rc)

SET(CMAKE_LINKER "-fuse-ld=lld-link.exe")
SET(TARGET_ARCH "x86_64-pc-windows-msvc")

SET(CMAKE_C_FLAGS_DEBUG "")
SET(CMAKE_C_FLAGS_DEBUG "-O0 -g -fms-extensions -fms-compatibility ${CMAKE_LINKER} -target ${TARGET_ARCH}")
SET(CMAKE_CXX_FLAGS_DEBUG "")
SET(CMAKE_CXX_FLAGS_DEBUG "-O0 -g -fms-extensions -fms-compatibility -fdelayed-template-parsing ${CMAKE_LINKER} -target ${TARGET_ARCH}")

SET(CMAKE_C_FLAGS_RELEASE "")
SET(CMAKE_C_FLAGS_RELEASE "-flto -Ofast -ffast-math -DNDEBUG -fms-extensions -fms-compatibility ${CMAKE_LINKER} -target ${TARGET_ARCH}")
SET(CMAKE_CXX_FLAGS_RELEASE "")
SET(CMAKE_CXX_FLAGS_RELEASE "-flto -Ofast -ffast-math -DNDEBUG -fms-extensions -fms-compatibility -fdelayed-template-parsing ${CMAKE_LINKER} -target ${TARGET_ARCH}")
