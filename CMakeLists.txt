cmake_minimum_required(VERSION 3.05)
set(LIBRARY_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/build)

if (NOT CMAKE_CXX_STANDARD)
set(CMAKE_CXX_STANDARD 20)
endif()

set(CMAKE_NAMESPACE "Atom::")

project(Utils
    DESCRIPTION "A headers-only util library in C++20, including reflection, structures, and some magic."
    HOMEPAGE_URL "https://github.com/Shucharjer/atom.utils"
    LANGUAGES C CXX
)

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

# CMake

list(INSERT CMAKE_MODULE_PATH 0 ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules)

add_library(Utils INTERFACE)
add_library(Atom::Utils ALIAS Utils)

set_target_properties(Utils PROPERTIES LINKER_LANGUAGE CXX)

target_include_directories(Utils INTERFACE
    $<BUILD_INTERFACE:${Utils_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

option(Utils_INCLUDE_HEADERS OFF)

if (Utils_INCLUDE_HEADERS)
    target_sources(Utils INTERFACE
        # the fellow line is a template    
        # $<BUILD_INTERFACE:${Utils_SOURCE_DIR}/include/>
        $<BUILD_INTERFACE:${Utils_SOURCE_DIR}/include/auxiliary.hpp>
        $<BUILD_INTERFACE:${Utils_SOURCE_DIR}/include/auxiliary/singleton.hpp>
        $<BUILD_INTERFACE:${Utils_SOURCE_DIR}/include/concepts.hpp>
        $<BUILD_INTERFACE:${Utils_SOURCE_DIR}/include/concpets/allocator.hpp>
        $<BUILD_INTERFACE:${Utils_SOURCE_DIR}/include/concepts/mempool.hpp>
        $<BUILD_INTERFACE:${Utils_SOURCE_DIR}/include/concepts/ranges.hpp>
        $<BUILD_INTERFACE:${Utils_SOURCE_DIR}/include/concepts/type.hpp>
        $<BUILD_INTERFACE:${Utils_SOURCE_DIR}/include/concepts/uconceptdef.hpp>
        $<BUILD_INTERFACE:${Utils_SOURCE_DIR}/include/core.hpp>
        $<BUILD_INTERFACE:${Utils_SOURCE_DIR}/include/core/closure.hpp>
        $<BUILD_INTERFACE:${Utils_SOURCE_DIR}/include/core/langdef.hpp>
        $<BUILD_INTERFACE:${Utils_SOURCE_DIR}/include/core/pair.hpp>
        $<BUILD_INTERFACE:${Utils_SOURCE_DIR}/include/core/pipeline.hpp>
        $<BUILD_INTERFACE:${Utils_SOURCE_DIR}/include/core/tuple.hpp>
        $<BUILD_INTERFACE:${Utils_SOURCE_DIR}/include/core/type_traits.hpp>
        $<BUILD_INTERFACE:${Utils_SOURCE_DIR}/include/core/type.hpp>
        $<BUILD_INTERFACE:${Utils_SOURCE_DIR}/include/memory.hpp>
        $<BUILD_INTERFACE:${Utils_SOURCE_DIR}/include/memory/allocator.hpp>
        $<BUILD_INTERFACE:${Utils_SOURCE_DIR}/include/memory/around_ptr.hpp>
        $<BUILD_INTERFACE:${Utils_SOURCE_DIR}/include/memory/pool.hpp>
        $<BUILD_INTERFACE:${Utils_SOURCE_DIR}/include/memory/storage.hpp>
        $<BUILD_INTERFACE:${Utils_SOURCE_DIR}/include/misc.hpp>
        $<BUILD_INTERFACE:${Utils_SOURCE_DIR}/include/misc/timer.hpp>
        $<BUILD_INTERFACE:${Utils_SOURCE_DIR}/include/ranges.hpp>
        $<BUILD_INTERFACE:${Utils_SOURCE_DIR}/include/ranges/iterator.hpp>
        $<BUILD_INTERFACE:${Utils_SOURCE_DIR}/include/ranges/to.hpp>
        $<BUILD_INTERFACE:${Utils_SOURCE_DIR}/include/reflection.hpp>
        $<BUILD_INTERFACE:${Utils_SOURCE_DIR}/include/signal.hpp>
        $<BUILD_INTERFACE:${Utils_SOURCE_DIR}/include/signal/delegate.hpp>
        $<BUILD_INTERFACE:${Utils_SOURCE_DIR}/include/signal/sink.hpp>
        $<BUILD_INTERFACE:${Utils_SOURCE_DIR}/include/signal/dispatcher.hpp>
        $<BUILD_INTERFACE:${Utils_SOURCE_DIR}/include/structures.hpp>
        $<BUILD_INTERFACE:${Utils_SOURCE_DIR}/include/structures/dense_map.hpp>
        $<BUILD_INTERFACE:${Utils_SOURCE_DIR}/include/structures/dense_set.hpp>
        $<BUILD_INTERFACE:${Utils_SOURCE_DIR}/include/structures/linear.hpp>
        $<BUILD_INTERFACE:${Utils_SOURCE_DIR}/include/structures/map.hpp>
        $<BUILD_INTERFACE:${Utils_SOURCE_DIR}/include/structures/set.hpp>
        $<BUILD_INTERFACE:${Utils_SOURCE_DIR}/include/structures/string.hpp>
        $<BUILD_INTERFACE:${Utils_SOURCE_DIR}/include/structures/tstring.hpp>
        $<BUILD_INTERFACE:${Utils_SOURCE_DIR}/include/thread.hpp>
        $<BUILD_INTERFACE:${Utils_SOURCE_DIR}/include/thread/corotine.hpp>
        $<BUILD_INTERFACE:${Utils_SOURCE_DIR}/include/thread/lock_keeper.hpp>
        $<BUILD_INTERFACE:${Utils_SOURCE_DIR}/include/thread/lock.hpp>
        $<BUILD_INTERFACE:${Utils_SOURCE_DIR}/include/thread/thread_pool.hpp>
    )
endif()

include(JoinPaths)

set(Utils_PKGCONFIG ${CMAKE_CURRENT_BINARY_DIR}/utils.pc)

join_paths(Utils_PKGCONFIG_INCLUDEDIR "\${prefix}" "CMAKE_INSTALL_INCLUDEDIR")

configure_file(
    ${Utils_SOURCE_DIR}/cmake/in/utils.pc.in
    ${Utils_PKGCONFIG}
    @ONLY
)

install(
    FILES ${Utils_PKGCONFIG}
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig
)

include(CMakePackageConfigHelpers)

install(
    TARGETS Utils
    EXPORT UtilsTargets
    LIBRARY DESTINATION ${INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${INSTALL_LIBDIR}
    RUNTIME DESTINATION ${INSTALL_LIBDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

configure_package_config_file(
    ${Utils_SOURCE_DIR}/cmake/in/UtilsConfig.cmake.in
    UtilsConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/Utils/cmake
)

export(
    EXPORT UtilsTargets
    FILE ${CMAKE_CURRENT_BINARY_DIR}/UtilsTargets.cmake
    NAMESPACE Atom::
)

install(
    EXPORT UtilsTargets
    FILE UtilsTargets.cmake
    NAMESPACE Atom::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/Utils/cmake
)

install(
    FILES ${PROJECT_BINARY_DIR}/UtilsConfig.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/Utils/cmake
)

install(
    DIRECTORY
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    FILES_MATCHING
    PATTERN "*.h"
    PATTERN "*.hpp"
)

export(PACKAGE Utils)

option(BUILD_UTILS_DOCS OFF)

if(BUILD_UTILS_DOCS)
    find_package(Doxygen REQUIRED dot)
    if (DOXYGEN_FOUND)
        include(FetchContent)

        FetchContent_Declare(
            doxygen-awesome-css
            GIT_REPOSITORY https://github.com/jothepro/doxygen-awesome-css
            GIT_TAG main
            GIT_SHALLOW 1
        )

        FetchContent_GetProperties(doxygen-awesome-css)

        if(NOT doxygen-awesome-css_POPULATED)
            FetchContent_Populate(doxygen-awesome-css)
            set(doxygen-awesome-css_INCLUDE_DIR ${doxygen-awesome-css_SOURCE_DIR})
        endif()

        set(DOXY_SOURCE_DIRECTORY ${Utils_SOURCE_DIR}/include)
        set(DOXY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/docs)
        set(DOXY_CSS_DIRECTORY ${doxygen-awesome-css_INCLUDE_DIR})

        configure_file(doxy.in doxy.cfg @ONLY)

        add_custom_target(
            documents
            COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/doxy.cfg
            WORKING_DIRECTORY ${Utils_SOURCE_DIR}
            VERBATIM
        )
    else()
        message(FATAL_ERROR "Required to build documents but doxygen is not found!")
    endif()
endif()

option(BUILD_UTILS_TEST OFF)

if(BUILD_UTILS_TEST)

set(UTILS_TEST_DIR "${Utils_SOURCE_DIR}/test")

function(BUILD_EXECUTABLE_FOR TEST_EXECUTABLE_NAME TEST_SOURCES)
    add_executable(${TEST_EXECUTABLE_NAME} ${TEST_SOURCES})
    target_link_libraries(${TEST_EXECUTABLE_NAME} PRIVATE Atom::Utils)
    add_test(NAME ${TEST_EXECUTABLE_NAME} COMMAND ${TEST_EXECUTABLE_NAME})
endfunction()

BUILD_EXECUTABLE_FOR(core ${UTILS_TEST_DIR}/core.cpp)
BUILD_EXECUTABLE_FOR(memory ${UTILS_TEST_DIR}/memory.cpp)
BUILD_EXECUTABLE_FOR(ranges ${UTILS_TEST_DIR}/ranges.cpp)
BUILD_EXECUTABLE_FOR(reflection ${UTILS_TEST_DIR}/reflection.cpp)
BUILD_EXECUTABLE_FOR(signal ${UTILS_TEST_DIR}/signal.cpp)
BUILD_EXECUTABLE_FOR(structures ${UTILS_TEST_DIR}/structures.cpp)
BUILD_EXECUTABLE_FOR(thread ${UTILS_TEST_DIR}/thread.cpp)

endif()