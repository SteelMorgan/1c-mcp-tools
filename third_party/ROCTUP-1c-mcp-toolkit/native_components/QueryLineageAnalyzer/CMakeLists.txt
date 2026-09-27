cmake_minimum_required(VERSION 3.16)
project(QueryLineageAnalyzer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

set(NLOHMANN_JSON_HEADER "${CMAKE_CURRENT_SOURCE_DIR}/include/nlohmann/json.hpp")
if(NOT EXISTS "${NLOHMANN_JSON_HEADER}")
    file(MAKE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/include/nlohmann")
    set(NLOHMANN_JSON_VERSION "v3.12.0")
    message(STATUS "Downloading nlohmann/json ${NLOHMANN_JSON_VERSION}...")
    file(DOWNLOAD
        "https://raw.githubusercontent.com/nlohmann/json/${NLOHMANN_JSON_VERSION}/single_include/nlohmann/json.hpp"
        "${NLOHMANN_JSON_HEADER}"
        STATUS DOWNLOAD_STATUS
    )
    list(GET DOWNLOAD_STATUS 0 STATUS_CODE)
    if(NOT STATUS_CODE EQUAL 0)
        list(GET DOWNLOAD_STATUS 1 ERROR_MSG)
        file(REMOVE "${NLOHMANN_JSON_HEADER}")
        message(FATAL_ERROR "Failed to download nlohmann/json: ${ERROR_MSG}")
    endif()
endif()

add_library(${PROJECT_NAME} SHARED
    src/dllmain.cpp
    src/component.cpp
    src/tokenizer.cpp
    src/parser.cpp
    src/lineage_resolver.cpp
    src/schema_enricher.cpp
    src/json_utils.cpp
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

if(WIN32)
    target_compile_definitions(${PROJECT_NAME} PRIVATE
        _WINDOWS
        WIN32_LEAN_AND_MEAN
        NOMINMAX
        _CRT_SECURE_NO_WARNINGS
    )
    set_target_properties(${PROJECT_NAME} PROPERTIES
        PREFIX ""
        OUTPUT_NAME "QueryLineageAnalyzer"
        LINK_FLAGS "/DEF:${CMAKE_CURRENT_SOURCE_DIR}/src/QueryLineageAnalyzer.def"
    )
else()
    set_target_properties(${PROJECT_NAME} PROPERTIES
        PREFIX ""
        OUTPUT_NAME "QueryLineageAnalyzer"
    )
endif()

if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W3 /utf-8 /EHsc)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra)
endif()

option(QUERY_LINEAGE_BUILD_TESTS "Build QueryLineageAnalyzer native tests" OFF)

if(QUERY_LINEAGE_BUILD_TESTS)
    enable_testing()

    add_executable(test_query_lineage
        tests/test_main.cpp
        src/tokenizer.cpp
        src/parser.cpp
        src/lineage_resolver.cpp
        src/schema_enricher.cpp
        src/json_utils.cpp
        src/component.cpp
    )

    target_include_directories(test_query_lineage PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    )

    if(WIN32)
        target_compile_definitions(test_query_lineage PRIVATE
            _WINDOWS
            WIN32_LEAN_AND_MEAN
            NOMINMAX
            _CRT_SECURE_NO_WARNINGS
        )
    endif()

    if(MSVC)
        target_compile_options(test_query_lineage PRIVATE /W3 /utf-8 /EHsc)
    else()
        target_compile_options(test_query_lineage PRIVATE -Wall -Wextra)
    endif()

    add_test(NAME test_query_lineage COMMAND test_query_lineage)
endif()
