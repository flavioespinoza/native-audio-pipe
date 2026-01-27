# FindGTest.cmake
# Locate Google Test framework
#
# This module defines:
#   GTEST_FOUND - System has Google Test
#   GTEST_INCLUDE_DIRS - Google Test include directories
#   GTEST_LIBRARIES - Libraries needed to use Google Test
#   GTEST_MAIN_LIBRARIES - Main library for standalone tests

include(FetchContent)

# Option to use system GTest or fetch it
option(NAP_USE_SYSTEM_GTEST "Use system-installed Google Test" OFF)

if(NAP_USE_SYSTEM_GTEST)
    find_package(GTest QUIET)
    if(GTEST_FOUND)
        message(STATUS "Found system Google Test")
    else()
        message(STATUS "System Google Test not found, will fetch")
    endif()
endif()

if(NOT GTEST_FOUND)
    message(STATUS "Fetching Google Test from GitHub...")

    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG        v1.14.0
    )

    # Prevent overriding parent project's compiler/linker settings on Windows
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

    FetchContent_MakeAvailable(googletest)

    set(GTEST_FOUND TRUE)
    set(GTEST_LIBRARIES gtest)
    set(GTEST_MAIN_LIBRARIES gtest_main)
    set(GTEST_INCLUDE_DIRS ${googletest_SOURCE_DIR}/googletest/include)

    message(STATUS "Google Test fetched successfully")
endif()

mark_as_advanced(
    GTEST_INCLUDE_DIRS
    GTEST_LIBRARIES
    GTEST_MAIN_LIBRARIES
)
