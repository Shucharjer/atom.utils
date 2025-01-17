cmake_minimum_required(VERSION 3.05)
set(LIBRARY_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/build)

set(CMAKE_CXX_STANDARD 20)

set(CMAKE_NAMESPACE "Atom::")

project(utils CXX)

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

include_directories(include)

add_library(Utils INTERFACE)
add_library(Atom::Utils ALIAS Utils)

set_target_properties(Utils PROPERTIES LINKER_LANGUAGE CXX)

target_include_directories(Utils INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_sources(Utils INTERFACE
    # the fellow line is a template    
    # $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/>
)

install(
    TARGETS Utils
    EXPORT UtilsTargets
    LIBRARY DESTINATION ${INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${INSTALL_LIBDIR}
    RUNTIME DESTINATION ${INSTALL_LIBDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(
    EXPORT UtilsTargets
    FILE UtilsTargets.cmake
    NAMESPACE Atom::
    DESTINATION cmake
)