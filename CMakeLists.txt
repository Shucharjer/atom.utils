cmake_minimum_required(VERSION 3.15)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_CXX_STANDARD 20)

project(
  Utils
  DESCRIPTION
    "A headers-only util library in C++20, including reflection, structures, and some magic."
  HOMEPAGE_URL "https://github.com/Shucharjer/atom.utils"
  LANGUAGES CXX)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif()

# CMake

list(INSERT CMAKE_MODULE_PATH 0 ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules)

option(ATOM_USE_LIBCPP OFF)
option(ATOM_Utils_INCLUDE_HEADERS OFF)
option(ATOM_Utils_INSTALL ON)
option(ATOM_UTILS_BUILD_DOCS "Build documentation for atom.utils" OFF)
option(ATOM_UTILS_BUILD_TEST "Build test for atom.utils" OFF)
option(ATOM_UTILS_BUILD_BENCHMARK OFF)

if(ATOM_USE_LIBCPP)
  if(NOT WIN32)

    cmake_push_check_state()

    set(CMAKE_REQUIRED_FLAGS "${CMAKE_REQUIRED_FLAGS} -stdlib=libc++")

    check_cxx_source_compiles(
      "
            #include <type_traits>
            int main() { return std::is_same_v<int, char>; }
            "
      ATOM_USE_LIBCPP_COMPILE_SUCCESS)

    cmake_pop_check_state()
  endif()

  add_compile_options(-stdlib=libc++)
endif()

include(GNUInstallDirs)

add_library(Utils INTERFACE)
add_library(Atom::Utils ALIAS Utils)

target_include_directories(
  Utils INTERFACE $<BUILD_INTERFACE:${Utils_SOURCE_DIR}/include>
                  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

set_target_properties(
  Utils
  PROPERTIES EXPORT_NAME "atom.utils"
             INTERFACE_COMPILE_DEFINITIONS "ATOM_NAMESPACE=Atom"
             LINKER_LANGUAGE CXX)

include(CheckCXXSourceCompiles)
include(CMakePushCheckState)

if(ATOM_Utils_INCLUDE_HEADERS)
  target_sources(
    Utils
    INTERFACE
      # the fellow line is a template
      # $<BUILD_INTERFACE:${Utils_SOURCE_DIR}/include/>
      $<BUILD_INTERFACE:${Utils_SOURCE_DIR}/include/auxiliary.hpp>
      $<BUILD_INTERFACE:${Utils_SOURCE_DIR}/include/auxiliary/singleton.hpp>
      $<BUILD_INTERFACE:${Utils_SOURCE_DIR}/include/concepts/ranges.hpp>
      $<BUILD_INTERFACE:${Utils_SOURCE_DIR}/include/concepts/type.hpp>
      $<BUILD_INTERFACE:${Utils_SOURCE_DIR}/include/core.hpp>
      $<BUILD_INTERFACE:${Utils_SOURCE_DIR}/include/core/type_traits.hpp>
      $<BUILD_INTERFACE:${Utils_SOURCE_DIR}/include/memory.hpp>
      $<BUILD_INTERFACE:${Utils_SOURCE_DIR}/include/memory/storage.hpp>
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
      $<BUILD_INTERFACE:${Utils_SOURCE_DIR}/include/structures/tstring.hpp>
      $<BUILD_INTERFACE:${Utils_SOURCE_DIR}/include/thread.hpp>
      $<BUILD_INTERFACE:${Utils_SOURCE_DIR}/include/thread/coroutine.hpp>
      $<BUILD_INTERFACE:${Utils_SOURCE_DIR}/include/thread/lock_keeper.hpp>
      $<BUILD_INTERFACE:${Utils_SOURCE_DIR}/include/thread/lock.hpp>
      $<BUILD_INTERFACE:${Utils_SOURCE_DIR}/include/thread/thread_pool.hpp>)
endif()

if(ATOM_Utils_INSTALL)

  if(WIN32)
    include(JoinPaths)
  elseif(UNIX)
    function(join_paths joined_path first_path_segment)
      set(temp_path "${first_path_segment}")
      foreach(current_segment IN LISTS ARGN)
        if(NOT ("${current_segment}" STREQUAL ""))
          if(IS_ABSOLUTE "${current_segment}")
            set(temp_path "${current_segment}")
          else()
            set(temp_path "${temp_path}/${current_segment}")
          endif()
        endif()
      endforeach()
      set(${joined_path}
          "${temp_path}"
          PARENT_SCOPE)
    endfunction()
  endif()

  set(Utils_PKGCONFIG ${CMAKE_CURRENT_BINARY_DIR}/utils.pc)

  join_paths(Utils_PKGCONFIG_INCLUDEDIR "\${prefix}" "CMAKE_INSTALL_INCLUDEDIR")

  configure_file(${Utils_SOURCE_DIR}/cmake/in/utils.pc.in ${Utils_PKGCONFIG}
                 @ONLY)

  install(FILES ${Utils_PKGCONFIG}
          DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)

  include(CMakePackageConfigHelpers)

  install(
    TARGETS Utils
    EXPORT UtilsTargets
    LIBRARY DESTINATION ${INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${INSTALL_LIBDIR}
    RUNTIME DESTINATION ${INSTALL_LIBDIR}
    INCLUDES
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

  configure_package_config_file(
    ${Utils_SOURCE_DIR}/cmake/in/UtilsConfig.cmake.in UtilsConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/Utils/cmake)

  export(
    EXPORT UtilsTargets
    FILE ${CMAKE_CURRENT_BINARY_DIR}/UtilsTargets.cmake
    NAMESPACE Atom::)

  install(
    EXPORT UtilsTargets
    FILE UtilsTargets.cmake
    NAMESPACE Atom::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/Utils/cmake)

  install(FILES ${PROJECT_BINARY_DIR}/UtilsConfig.cmake
          DESTINATION ${CMAKE_INSTALL_LIBDIR}/Utils/cmake)

  install(
    DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    FILES_MATCHING
    PATTERN "*.h"
    PATTERN "*.hpp")

  export(PACKAGE Utils)

endif()

if(ATOM_UTILS_BUILD_DOCS)
  find_package(Doxygen REQUIRED dot)
  if(DOXYGEN_FOUND)
    include(FetchContent)

    FetchContent_Declare(
      doxygen-awesome-css
      GIT_REPOSITORY https://github.com/jothepro/doxygen-awesome-css
      GIT_TAG main
      GIT_SHALLOW 1)
    FetchContent_MakeAvailable(doxygen-awesome-css)

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
      VERBATIM)
  else()
    message(FATAL_ERROR "Required to build documents but doxygen is not found!")
  endif()
endif()

if(ATOM_UTILS_BUILD_TEST)
  include(CTest)
  enable_testing()
  option(SEE_ALL_TEMPLATE_BACKTRACE OFF)

  function(add_utils_test test_name)
    set(ATOM_UTILS_TEST_DIR "${Utils_SOURCE_DIR}/test")
    set(ATOM_UTILS_TEST_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/test)
    add_executable(test_${test_name} ${ATOM_UTILS_TEST_DIR}/${test_name}.cpp)
    target_link_libraries(test_${test_name} PRIVATE Atom::Utils)
    set_target_properties(
      test_${test_name}
      PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${ATOM_UTILS_TEST_OUTPUT_DIR}
                 RUNTIME_OUTPUT_DIRECTORY_DEBUG ${ATOM_UTILS_TEST_OUTPUT_DIR}
                 RUNTIME_OUTPUT_DIRECTORY_RELEASE ${ATOM_UTILS_TEST_OUTPUT_DIR})
    add_test(NAME ${test_name} COMMAND test_${test_name})

    if(SEE_ALL_TEMPLATE_BACKTRACE)
      target_compile_options(${TEST_EXECUTABLE_NAME}
                             PRIVATE -ftemplate-backtrace-limit=0)
    endif()
  endfunction()

  set(ATOM_UTILS_TESTS
      core
      memory
      meta
      ranges
      reflection
      signal
      structures
      thread)

  foreach(test IN LISTS ATOM_UTILS_TESTS)
    add_utils_test(${test})
  endforeach()

endif()

if(ATOM_UTILS_BUILD_BENCHMARK)
  set(ATOM_Utils_BENCHMARK_DIR "${Utils_SOURCE_DIR}/benchmark")

  include(FetchContent)
  FetchContent_Declare(
    googlebenchmark
    GIT_REPOSITORY https://github.com/google/benchmark.git
    GIT_TAG main
    GIT_SHALLOW 1)
  set(BENCHMARK_DOWNLOAD_DEPENDENCIES ON)
  FetchContent_MakeAvailable(googlebenchmark)

  function(add_utils_benchmark benchmark_name)
    set(ATOM_UTILS_BENCHMARK_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/benchmark)
    file(GLOB BENCHMARK_SRCS
         "${ATOM_Utils_BENCHMARK_DIR}/${benchmark_name}.cpp"
         "${ATOM_Utils_BENCHMARK_DIR}/${benchmark_name}.s")
    add_executable(benchmark-${benchmark_name} ${BENCHMARK_SRCS})
    target_link_libraries(benchmark-${benchmark_name}
                          PRIVATE Utils benchmark::benchmark)
    set_target_properties(
      benchmark-${benchmark_name}
      PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${ATOM_UTILS_BENCHMARK_OUTPUT_DIR}
                 RUNTIME_OUTPUT_DIRECTORY_DEBUG
                 ${ATOM_UTILS_BENCHMARK_OUTPUT_DIR}
                 RUNTIME_OUTPUT_DIRECTORY_RELEASE
                 ${ATOM_UTILS_BENCHMARK_OUTPUT_DIR})
  endfunction()

  add_utils_benchmark(delegate)
  add_utils_benchmark(memory)
endif()
