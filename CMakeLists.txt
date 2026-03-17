cmake_minimum_required(VERSION 3.24)
set(CMAKE_POLICY_DEFAULT_CMP0135 NEW)
project(sensor_node_cpp20 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ---------- Symlink compile_commands.json to project root (host builds only) ----------
if(NOT CMAKE_CROSSCOMPILING AND CMAKE_EXPORT_COMPILE_COMMANDS)
    file(RELATIVE_PATH REL_BUILD_DIR ${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR})
    execute_process(
        COMMAND ${CMAKE_COMMAND} -E create_symlink
            ${REL_BUILD_DIR}/compile_commands.json
            ${CMAKE_SOURCE_DIR}/compile_commands.json
    )
endif()

# ---------- Modules ----------
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
include(CompilerWarnings)
include(FetchDeps)

# ---------- Common library (header-only, shared by C++17 and C++20) ----------
add_library(common INTERFACE)
target_include_directories(common INTERFACE common/include)

# =============================================================================
# Demo 1: Concepts + constexpr + static_assert
# =============================================================================

# C++17 version (header-only template library)
add_library(demo1_cpp17 INTERFACE)
target_include_directories(demo1_cpp17 INTERFACE demo1_concepts/cpp17/include)
target_link_libraries(demo1_cpp17 INTERFACE common)
target_compile_features(demo1_cpp17 INTERFACE cxx_std_17)

# C++20 version (header-only template library)
add_library(demo1_cpp20 INTERFACE)
target_include_directories(demo1_cpp20 INTERFACE demo1_concepts/cpp20/include)
target_link_libraries(demo1_cpp20 INTERFACE common)
target_compile_features(demo1_cpp20 INTERFACE cxx_std_20)

# =============================================================================
# Demo 2: Ranges-v3 Sensor Pipeline
# =============================================================================

# C++17 version
add_library(demo2_cpp17 STATIC demo2_ranges/cpp17/src/sensor_pipeline.cpp)
target_include_directories(demo2_cpp17 PUBLIC demo2_ranges/cpp17/include)
target_link_libraries(demo2_cpp17 PUBLIC common)
target_compile_features(demo2_cpp17 PUBLIC cxx_std_17)
set_project_warnings(demo2_cpp17)

# C++20 version
add_library(demo2_cpp20 STATIC demo2_ranges/cpp20/src/sensor_pipeline.cpp)
target_include_directories(demo2_cpp20 PUBLIC demo2_ranges/cpp20/include)
target_link_libraries(demo2_cpp20 PUBLIC common range-v3)
target_compile_features(demo2_cpp20 PUBLIC cxx_std_20)
set_project_warnings(demo2_cpp20)

# =============================================================================
# Demo 3: tl::expected Monadic Chains
# =============================================================================

# C++17 version
add_library(demo3_cpp17 STATIC demo3_expected/cpp17/src/sensor_init.cpp)
target_include_directories(demo3_cpp17 PUBLIC demo3_expected/cpp17/include)
target_link_libraries(demo3_cpp17 PUBLIC common)
target_compile_features(demo3_cpp17 PUBLIC cxx_std_17)
set_project_warnings(demo3_cpp17)

# C++20 version
add_library(demo3_cpp20 STATIC demo3_expected/cpp20/src/sensor_init.cpp)
target_include_directories(demo3_cpp20 PUBLIC demo3_expected/cpp20/include)
target_link_libraries(demo3_cpp20 PUBLIC common tl::expected)
target_compile_features(demo3_cpp20 PUBLIC cxx_std_20)
set_project_warnings(demo3_cpp20)

# =============================================================================
# Demo 4: consteval + bit_cast + constinit + span + GSL
# =============================================================================

# C++17 version
add_library(demo4_cpp17 STATIC demo4_consteval/cpp17/src/calibration_table.cpp)
target_include_directories(demo4_cpp17 PUBLIC demo4_consteval/cpp17/include)
target_link_libraries(demo4_cpp17 PUBLIC common)
target_compile_features(demo4_cpp17 PUBLIC cxx_std_17)
set_project_warnings(demo4_cpp17)

# C++20 version
add_library(demo4_cpp20 STATIC demo4_consteval/cpp20/src/calibration_table.cpp)
target_include_directories(demo4_cpp20 PUBLIC demo4_consteval/cpp20/include)
target_link_libraries(demo4_cpp20 PUBLIC common Microsoft.GSL::GSL)
target_compile_features(demo4_cpp20 PUBLIC cxx_std_20)
set_project_warnings(demo4_cpp20)

# =============================================================================
# ARM firmware targets (cross-compile only)
# =============================================================================

if(CMAKE_CROSSCOMPILING)
    set(LINKER_SCRIPT "${CMAKE_SOURCE_DIR}/linker/STM32F407VG.ld")

    # C++17 firmware
    add_executable(firmware_cpp17
        app/main_cpp17.cpp
        startup/startup_stm32f407.cpp
        startup/syscalls.cpp
    )
    target_link_libraries(firmware_cpp17 PRIVATE
        demo1_cpp17 demo2_cpp17 demo3_cpp17 demo4_cpp17)
    target_link_options(firmware_cpp17 PRIVATE
        -T${LINKER_SCRIPT} -Wl,-Map=firmware_cpp17.map)

    # C++20 firmware
    add_executable(firmware_cpp20
        app/main_cpp20.cpp
        startup/startup_stm32f407.cpp
        startup/syscalls.cpp
    )
    target_link_libraries(firmware_cpp20 PRIVATE
        demo1_cpp20 demo2_cpp20 demo3_cpp20 demo4_cpp20)
    target_link_options(firmware_cpp20 PRIVATE
        -T${LINKER_SCRIPT} -Wl,-Map=firmware_cpp20.map)

    # Post-build: generate .bin and print size
    foreach(target firmware_cpp17 firmware_cpp20)
        add_custom_command(TARGET ${target} POST_BUILD
            COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:${target}>
                    ${target}.bin
            COMMAND ${CMAKE_SIZE} $<TARGET_FILE:${target}>
            COMMENT "Size report for ${target}")
    endforeach()
endif()

# =============================================================================
# LSP support (host only): compile_commands.json entries for app/ files
# =============================================================================

if(NOT CMAKE_CROSSCOMPILING)
    add_library(lsp_app_cpp17 OBJECT EXCLUDE_FROM_ALL app/main_cpp17.cpp)
    target_link_libraries(lsp_app_cpp17 PRIVATE
        demo1_cpp17 demo2_cpp17 demo3_cpp17 demo4_cpp17)

    add_library(lsp_app_cpp20 OBJECT EXCLUDE_FROM_ALL app/main_cpp20.cpp)
    target_link_libraries(lsp_app_cpp20 PRIVATE
        demo1_cpp20 demo2_cpp20 demo3_cpp20 demo4_cpp20)
endif()

# =============================================================================
# Tests (host only)
# =============================================================================

# =============================================================================
# Compilation-time probes (host only, object-only — never linked)
# =============================================================================

if(NOT CMAKE_CROSSCOMPILING)
    add_library(probe_demo1_cpp17 OBJECT EXCLUDE_FROM_ALL bench/probe_demo1_cpp17.cpp)
    target_link_libraries(probe_demo1_cpp17 PRIVATE demo1_cpp17)
    target_compile_features(probe_demo1_cpp17 PRIVATE cxx_std_20)

    add_library(probe_demo1_cpp20 OBJECT EXCLUDE_FROM_ALL bench/probe_demo1_cpp20.cpp)
    target_link_libraries(probe_demo1_cpp20 PRIVATE demo1_cpp20)
    target_compile_features(probe_demo1_cpp20 PRIVATE cxx_std_20)
endif()

if(BUILD_TESTING AND NOT CMAKE_CROSSCOMPILING)
    enable_testing()

    # Demo 1 test
    add_executable(test_demo1 demo1_concepts/test/test_demo1_equivalence.cpp)
    target_include_directories(test_demo1 PRIVATE demo1_concepts/test)
    target_link_libraries(test_demo1 PRIVATE demo1_cpp17 demo1_cpp20 doctest::doctest)
    target_compile_features(test_demo1 PRIVATE cxx_std_20)
    add_test(NAME demo1_equivalence COMMAND test_demo1)

    # Demo 2 test
    add_executable(test_demo2 demo2_ranges/test/test_demo2_equivalence.cpp)
    target_include_directories(test_demo2 PRIVATE demo2_ranges/test)
    target_link_libraries(test_demo2 PRIVATE demo2_cpp17 demo2_cpp20 doctest::doctest)
    target_compile_features(test_demo2 PRIVATE cxx_std_20)
    add_test(NAME demo2_equivalence COMMAND test_demo2)

    # Demo 3 test
    add_executable(test_demo3 demo3_expected/test/test_demo3_equivalence.cpp)
    target_include_directories(test_demo3 PRIVATE demo3_expected/test)
    target_link_libraries(test_demo3 PRIVATE demo3_cpp17 demo3_cpp20 doctest::doctest)
    target_compile_features(test_demo3 PRIVATE cxx_std_20)
    add_test(NAME demo3_equivalence COMMAND test_demo3)

    # Demo 4 test
    add_executable(test_demo4 demo4_consteval/test/test_demo4_equivalence.cpp)
    target_include_directories(test_demo4 PRIVATE demo4_consteval/test)
    target_link_libraries(test_demo4 PRIVATE demo4_cpp17 demo4_cpp20
                          doctest::doctest Microsoft.GSL::GSL)
    target_compile_features(test_demo4 PRIVATE cxx_std_20)
    add_test(NAME demo4_equivalence COMMAND test_demo4)
endif()
