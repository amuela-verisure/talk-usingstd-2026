include(FetchContent)

# tl::expected - single-header expected implementation (C++23 polyfill)
FetchContent_Declare(
    tl_expected
    GIT_REPOSITORY https://github.com/TartanLlama/expected.git
    GIT_TAG        v1.1.0
    GIT_SHALLOW    TRUE
)

# range-v3 - ranges library (C++23 std::ranges polyfill)
FetchContent_Declare(
    range_v3
    GIT_REPOSITORY https://github.com/ericniebler/range-v3.git
    GIT_TAG        0.12.0
    GIT_SHALLOW    TRUE
)

# Microsoft GSL - Guidelines Support Library (gsl::narrow, gsl::span)
FetchContent_Declare(
    gsl
    GIT_REPOSITORY https://github.com/microsoft/GSL.git
    GIT_TAG        v4.0.0
    GIT_SHALLOW    TRUE
)

# doctest - lightweight C++ testing framework (host builds only)
FetchContent_Declare(
    doctest
    GIT_REPOSITORY https://github.com/doctest/doctest.git
    GIT_TAG        v2.4.11
    GIT_SHALLOW    TRUE
)

FetchContent_MakeAvailable(tl_expected range_v3 gsl)

if(NOT CMAKE_CROSSCOMPILING)
    set(CMAKE_WARN_DEPRECATED OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(doctest)
    set(CMAKE_WARN_DEPRECATED ON CACHE BOOL "" FORCE)
endif()
