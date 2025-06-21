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
  PROPERTIES EXPORT_NAME "utils"
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
    ARCHIVE DESTINATION ${INSTALL_LIBDIR})

  configure_package_config_file(
    ${Utils_SOURCE_DIR}/cmake/in/UtilsConfig.cmake.in UtilsConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/Utils/cmake)

  export(
    EXPORT UtilsTargets
    FILE ${CMAKE_CURRENT_BINARY_DIR}/UtilsTargets.cmake
    NAMESPACE atom::)

  install(
    EXPORT UtilsTargets
    FILE UtilsTargets.cmake
    NAMESPACE atom::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/Utils/cmake)

  install(FILES ${PROJECT_BINARY_DIR}/UtilsConfig.cmake
          DESTINATION ${CMAKE_INSTALL_LIBDIR}/Utils/cmake)

  install(
    DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/atom/utils
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

include(CTest)
enable_testing()

function(exec_dir dir prefix output_dir as_test)
  set(libs ${ARGN})

  file(GLOB_RECURSE source RELATIVE ${dir}/src ${dir}/src/*.cpp)
  foreach(src ${source})
    get_filename_component(exec_raw ${src} NAME_WE)
    string(REPLACE ".cpp" "" exec_base ${exec_raw})
    string(REPLACE "/" "_" exec_name ${exec_base})
    set(exec_name ${prefix}${exec_name})
    add_executable(${exec_name} ${dir}/src/${src})
    target_include_directories(${exec_name} PRIVATE $<BUILD_INTERFACE:${dir}/include>)
    target_link_libraries(${exec_name} PRIVATE ${libs})
    set_target_properties(${exec_name} PROPERTIES
      RUNTIME_OUTPUT_DIRECTORY ${output_dir}
      RUNTIME_OUTPUT_DIRECTORY_DEBUG ${output_dir}
      RUNTIME_OUTPUT_DIRECTORY_RELEASE ${output_dir})
    if(as_test)
      add_test(NAME ${exec_name} COMMAND ${exec_name})
     endif()
  endforeach()
endfunction()

function(exec_dir_options dir prefix)
  set(options ${ARGN})
  file(GLOB_RECURSE source RELATIVE ${dir}/src ${dir}/src/*.cpp)
  foreach(src ${source})
    get_filename_component(exec_raw ${src} NAME_WE)
    string(REPLACE ".cpp" "" exec_base ${exec_raw})
    string(REPLACE "/" "_" exec_name ${exec_base})
    set(exec_name ${prefix}${exec_name})
    target_compile_options(${exec_name} PRIVATE ${options})
  endforeach()
endfunction()

if(ATOM_UTILS_BUILD_TEST)
  option(ATOM_UTILS_TEST_SEE_ALL_TEMPLATE_BACKTRACE OFF)
  set(test_dir ${Utils_SOURCE_DIR}/test)
  set(test_output_dir ${CMAKE_CURRENT_BINARY_DIR}/test)
  exec_dir(${test_dir} "test-" ${test_output_dir} true Utils)
  if(ATOM_UTILS_TEST_SEE_ALL_TEMPLATE_BACKTRACE)
    exec_dir_options(${test_dir} "test-" ftemplate-backtrace-limit=0)
  endif()
endif()

if(ATOM_UTILS_BUILD_BENCHMARK)
  include(FetchContent)
  FetchContent_Declare(
    googlebenchmark
    GIT_REPOSITORY https://github.com/google/benchmark.git
    GIT_TAG main
    GIT_SHALLOW 1)
  set(BENCHMARK_DOWNLOAD_DEPENDENCIES ON)
  FetchContent_MakeAvailable(googlebenchmark)

  set(benchmark_dir ${Utils_SOURCE_DIR}/benchmark)
  set(benchmark_output_dir ${CMAKE_CURRENT_BINARY_DIR}/benchmark)
  exec_dir(${benchmark_dir} "benchmark-" ${benchmark_output_dir} false Utils benchmark::benchmark)
endif()
