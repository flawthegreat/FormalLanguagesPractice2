cmake_minimum_required(VERSION 3.10)

project(flp)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

if(UNIX)
    include(GNUInstallDirs)
else()
    message(
        FATAL_ERROR 
        "Could not set installation directories: unsupported platform"
    )
endif()

set(
    FL_SOURCES
    "${flp_SOURCE_DIR}/Source/FL/Common.cpp"
    "${flp_SOURCE_DIR}/Source/FL/Grammar.cpp"
    "${flp_SOURCE_DIR}/Source/FL/ContextFreeGrammar.cpp"
    "${flp_SOURCE_DIR}/Source/FL/CYK.cpp"
)

set(
    FL_HEADERS
    "${flp_SOURCE_DIR}/Source/FL/Common.hpp"
    "${flp_SOURCE_DIR}/Source/FL/Constants.hpp"
    "${flp_SOURCE_DIR}/Source/FL/Grammar.hpp"
    "${flp_SOURCE_DIR}/Source/FL/ContextFreeGrammar.hpp"
    "${flp_SOURCE_DIR}/Source/FL/CYK.hpp"
)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${flp_SOURCE_DIR}/Build/lib")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${flp_SOURCE_DIR}/Build/bin")

include_directories("${flp_SOURCE_DIR}/Source")
add_library(FL STATIC ${FL_SOURCES} ${FL_HEADERS})
add_executable(flp "${flp_SOURCE_DIR}/Source/Application.cpp")
target_link_libraries(flp PUBLIC FL)

if (COVERAGE STREQUAL "true")
    find_package(GTest REQUIRED)
    set(
        flp_test_SOURCES
        "${flp_SOURCE_DIR}/Tests/TestMain.cpp"
        "${flp_SOURCE_DIR}/Tests/TestGrammar.cpp"
        "${flp_SOURCE_DIR}/Tests/TestContextFreeGrammar.cpp"
        "${flp_SOURCE_DIR}/Tests/TestCYK.cpp"
    )
    add_executable(flp_test ${flp_test_SOURCES})
    target_include_directories(flp_test PRIVATE "${GTEST_INCLUDE_DIR}")
    target_link_libraries(flp_test FL ${GTEST_BOTH_LIBRARIES} pthread)

    set(CMAKE_MODULE_PATH "${flp_SOURCE_DIR}/CMakeModules")
    set(CMAKE_BUILD_TYPE "Debug")
    include(CodeCoverage)
    append_coverage_compiler_flags()
    set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -O0")
    setup_target_for_coverage_gcovr_html(
        NAME CoverageReport
        EXECUTABLE flp_test
        EXCLUDE "${flp_SOURCE_DIR}/Tests"
        OUT_DIR "${flp_SOURCE_DIR}/Build"
    )
endif()

foreach(FILE ${RE_HEADERS})
    file(
        COPY ${FILE} 
        DESTINATION ${flp_SOURCE_DIR}/Build/include/FL/
    )
endforeach(FILE)

install(TARGETS flp RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
