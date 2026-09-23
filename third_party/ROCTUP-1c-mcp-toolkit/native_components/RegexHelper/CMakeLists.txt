cmake_minimum_required(VERSION 3.16)
project(RegexHelper LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

include(FetchContent)

FetchContent_Declare(
    pcre2
    URL https://github.com/PCRE2Project/pcre2/releases/download/pcre2-10.46/pcre2-10.46.tar.gz
    URL_HASH SHA256=8d28d7f2c3b970c3a4bf3776bcbb5adfc923183ce74bc8df1ebaad8c1985bd07
)

set(PCRE2_BUILD_PCRE2_8  OFF CACHE BOOL "" FORCE)
set(PCRE2_BUILD_PCRE2_16 ON  CACHE BOOL "" FORCE)
set(PCRE2_BUILD_PCRE2_32 OFF CACHE BOOL "" FORCE)
set(PCRE2_BUILD_PCRE2GREP OFF CACHE BOOL "" FORCE)
set(PCRE2_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(PCRE2_STATIC_PIC ON CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(pcre2)

add_library(RegexHelper SHARED
    src/dllmain.cpp
    src/component.cpp
)

target_include_directories(RegexHelper PRIVATE include src)
target_link_libraries(RegexHelper PRIVATE pcre2-16-static)

set_target_properties(RegexHelper PROPERTIES PREFIX "")

if(MSVC)
    set_target_properties(RegexHelper PROPERTIES
        LINK_FLAGS "/DEF:${CMAKE_CURRENT_SOURCE_DIR}/src/RegexHelper.def")
    target_compile_definitions(RegexHelper PRIVATE
        _WINDOWS WIN32_LEAN_AND_MEAN NOMINMAX _CRT_SECURE_NO_WARNINGS
        PCRE2_CODE_UNIT_WIDTH=16)
    target_compile_options(RegexHelper PRIVATE /W3 /utf-8 /EHsc)
else()
    target_compile_definitions(RegexHelper PRIVATE PCRE2_CODE_UNIT_WIDTH=16)
    target_compile_options(RegexHelper PRIVATE -Wall -Wextra -fexceptions)
endif()

# ---- Smoke test (Linux only) ----
option(REGEX_HELPER_SMOKE_TEST "Build Linux smoke test" OFF)
if(REGEX_HELPER_SMOKE_TEST)
    add_executable(regex_smoke_test
        tests/smoke_test.cpp
        src/component.cpp
    )
    target_include_directories(regex_smoke_test PRIVATE include src)
    target_link_libraries(regex_smoke_test PRIVATE pcre2-16-static)
    target_compile_definitions(regex_smoke_test PRIVATE PCRE2_CODE_UNIT_WIDTH=16)
endif()
