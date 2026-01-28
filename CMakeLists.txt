cmake_minimum_required(VERSION 3.16)

project(native-audio-pipe
    VERSION 1.0.0
    DESCRIPTION "Enterprise-Grade C++ Audio Engine"
    LANGUAGES CXX
)

# C++ Standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Build options
option(NAP_BUILD_TESTS "Build unit tests" ON)
option(NAP_BUILD_EXAMPLES "Build examples" OFF)
option(NAP_ENABLE_SIMD "Enable SIMD optimizations" ON)

# Module path for custom Find modules
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

# Platform-specific settings
if(APPLE)
    find_package(CoreAudio)
endif()

# Compiler warnings
if(MSVC)
    add_compile_options(/W4 /WX-)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# SIMD flags
if(NAP_ENABLE_SIMD)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        add_compile_options(-msse4.1)
    endif()
endif()

# Core library sources
set(NAP_CORE_SOURCES
    # Graph
    src/core/graph/AudioGraph.cpp
    src/core/graph/ConnectionManager.cpp
    src/core/graph/ExecutionSorter.cpp
    src/core/graph/FeedbackLoopDetector.cpp
    # Memory
    src/core/memory/CircularBuffer.cpp
    src/core/memory/AudioBlockAllocator.cpp
    src/core/memory/PoolAllocator.cpp
    # Threading
    src/core/threading/WorkerThread.cpp
    src/core/threading/TaskQueue.cpp
    src/core/threading/SpinLock.cpp
    src/core/threading/ThreadBarrier.cpp
    # Parameters (Phase 2)
    src/core/parameters/FloatParameter.cpp
    src/core/parameters/IntParameter.cpp
    src/core/parameters/BoolParameter.cpp
    src/core/parameters/EnumParameter.cpp
    src/core/parameters/TriggerParameter.cpp
    src/core/parameters/ParameterGroup.cpp
    # Serialization (Phase 2)
    src/core/serialization/JsonSerializer.cpp
    src/core/serialization/BinarySerializer.cpp
    src/core/serialization/PresetManager.cpp
    src/core/serialization/StateVector.cpp
)

# Driver sources (Phase 2)
set(NAP_DRIVER_SOURCES
    src/drivers/PulseAudioDriver.cpp
    src/drivers/AlsaDriver.cpp
    src/drivers/JackDriver.cpp
    src/drivers/ASIODriver.cpp
    src/drivers/DummyDriver.cpp
)

# Hardware Driver Layer subdirectories (Phase 3)
set(NAP_DRIVER_PLATFORM_SOURCES
    # ASIO (Windows)
    src/drivers/asio/ASIODriver.cpp
    src/drivers/asio/ASIOBufferSwitch.cpp
    # CoreAudio (macOS)
    src/drivers/coreaudio/CoreAudioDriver.cpp
    src/drivers/coreaudio/CoreAudioDeviceList.cpp
    # ALSA (Linux)
    src/drivers/alsa/AlsaDriver.cpp
    src/drivers/alsa/AlsaMidiDriver.cpp
    # JACK (Linux Pro Audio)
    src/drivers/jack/JackDriver.cpp
    src/drivers/jack/JackClient.cpp
    # Dummy/Null (Testing/CI)
    src/drivers/dummy/NullAudioDriver.cpp
)

# Event & MIDI System (Phase 3)
set(NAP_EVENT_SOURCES
    src/events/MidiMessage.cpp
    src/events/SysexMessage.cpp
)

# Benchmark sources (Phase 3)
set(NAP_BENCHMARK_SOURCES
    benchmarks/nodes/Bench_GainNode.cpp
    benchmarks/nodes/Bench_BiQuadFilter.cpp
    benchmarks/nodes/Bench_ReverbConvolution.cpp
    benchmarks/nodes/Bench_DelayLine.cpp
    benchmarks/nodes/Bench_Oscillator.cpp
    benchmarks/nodes/Bench_Mixer.cpp
    benchmarks/nodes/Bench_Splitter.cpp
    benchmarks/nodes/Bench_Compressor.cpp
    benchmarks/nodes/Bench_Limiter.cpp
    benchmarks/nodes/Bench_Gate.cpp
)

# DSP Utility sources (Phase 2)
set(NAP_DSP_SOURCES
    src/utils/dsp/FastFourierTransform.cpp
    src/utils/dsp/WindowFunctions.cpp
    src/utils/dsp/Resampler.cpp
    src/utils/dsp/DCBlocker.cpp
)

# Node sources
set(NAP_NODE_SOURCES
    # Math nodes
    src/nodes/math/GainNode.cpp
    src/nodes/math/PanNode.cpp
    src/nodes/math/MixerNode.cpp
    src/nodes/math/SplitterNode.cpp
    src/nodes/math/InverterNode.cpp
    src/nodes/math/SummerNode.cpp
    # Source nodes
    src/nodes/source/SineOscillator.cpp
    src/nodes/source/WhiteNoise.cpp
    src/nodes/source/PinkNoise.cpp
    src/nodes/source/FileStreamReader.cpp
    src/nodes/source/ImpulseGenerator.cpp
    # Effect nodes
    src/nodes/effect/SimpleDelay.cpp
    src/nodes/effect/BiQuadFilter.cpp
    src/nodes/effect/HardClipper.cpp
    src/nodes/effect/SoftClipper.cpp
    src/nodes/effect/BitCrusher.cpp
    src/nodes/effect/RingModulator.cpp
    src/nodes/effect/Phaser.cpp
    src/nodes/effect/Flanger.cpp
    src/nodes/effect/Chorus.cpp
    src/nodes/effect/ReverbConvolution.cpp
    # Utility nodes
    src/nodes/utility/MeterNode.cpp
    src/nodes/utility/ScopeNode.cpp
    src/nodes/utility/DCBlockerNode.cpp
)

# Skip platform driver stubs on CI / cross-platform builds
option(NAP_BUILD_PLATFORM_DRIVERS "Build platform-specific driver stubs" OFF)

# Create the main library
add_library(native-audio-pipe STATIC
    ${NAP_CORE_SOURCES}
    ${NAP_NODE_SOURCES}
    ${NAP_DRIVER_SOURCES}
    ${NAP_DSP_SOURCES}
    ${NAP_EVENT_SOURCES}
)
if(NAP_BUILD_PLATFORM_DRIVERS)
    target_sources(native-audio-pipe PRIVATE ${NAP_DRIVER_PLATFORM_SOURCES})
endif()

# Benchmark library (Phase 3)
option(NAP_BUILD_BENCHMARKS "Build performance benchmarks" ON)
if(NAP_BUILD_BENCHMARKS)
    add_library(nap-benchmarks STATIC ${NAP_BENCHMARK_SOURCES})
    target_include_directories(nap-benchmarks
        PUBLIC
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/benchmarks>
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
    )
    target_link_libraries(nap-benchmarks PRIVATE native-audio-pipe)
endif()

target_include_directories(native-audio-pipe
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
        $<INSTALL_INTERFACE:include>
)

# Link platform libraries
if(APPLE AND COREAUDIO_FOUND)
    target_link_libraries(native-audio-pipe PRIVATE ${COREAUDIO_LIBRARIES})
endif()

# Threading support
find_package(Threads REQUIRED)
target_link_libraries(native-audio-pipe PRIVATE Threads::Threads)

# Tests
if(NAP_BUILD_TESTS)
    enable_testing()
    include(cmake/FindGTest.cmake)

    # Collect all test sources
    file(GLOB_RECURSE NAP_TEST_SOURCES
        tests/unit/core/graph/*.cpp
        tests/unit/core/memory/*.cpp
        tests/unit/core/threading/*.cpp
        tests/unit/core/parameters/*.cpp
        tests/unit/core/serialization/*.cpp
        tests/unit/nodes/math/*.cpp
        tests/unit/nodes/source/*.cpp
        tests/unit/nodes/effect/*.cpp
        tests/unit/nodes/utility/*.cpp
        tests/unit/drivers/*.cpp
        tests/unit/utils/dsp/*.cpp
        tests/unit/events/*.cpp
    )

    add_executable(nap_tests ${NAP_TEST_SOURCES})

    target_link_libraries(nap_tests
        PRIVATE
            native-audio-pipe
            ${GTEST_LIBRARIES}
            ${GTEST_MAIN_LIBRARIES}
            Threads::Threads
    )

    target_include_directories(nap_tests
        PRIVATE
            ${GTEST_INCLUDE_DIRS}
            ${CMAKE_CURRENT_SOURCE_DIR}/src
            ${CMAKE_CURRENT_SOURCE_DIR}/benchmarks
    )

    # Register tests with CTest
    include(GoogleTest)
    gtest_discover_tests(nap_tests)
endif()

# Installation
include(GNUInstallDirs)

install(TARGETS native-audio-pipe
    EXPORT native-audio-pipe-targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(DIRECTORY src/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/nap
    FILES_MATCHING PATTERN "*.h"
)

install(EXPORT native-audio-pipe-targets
    FILE native-audio-pipe-config.cmake
    NAMESPACE nap::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/native-audio-pipe
)

# Summary
message(STATUS "")
message(STATUS "native-audio-pipe configuration:")
message(STATUS "  Version:       ${PROJECT_VERSION}")
message(STATUS "  C++ Standard:  ${CMAKE_CXX_STANDARD}")
message(STATUS "  Build tests:   ${NAP_BUILD_TESTS}")
message(STATUS "  Build benchmarks: ${NAP_BUILD_BENCHMARKS}")
message(STATUS "  SIMD:          ${NAP_ENABLE_SIMD}")
if(APPLE)
    message(STATUS "  CoreAudio:     ${COREAUDIO_FOUND}")
endif()
message(STATUS "")
