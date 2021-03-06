cmake_minimum_required(VERSION 3.10)

set(PROJECT_NAME currender)
project(${PROJECT_NAME} LANGUAGES CXX VERSION 0.0.1 DESCRIPTION "A CPU renderer for computer vision")

set(CMAKE_VERBOSE_MAKEFILE TRUE)

# .lib
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/lib)

# .dll and .exe
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin) 

if (WIN32)
# option for Visual Studio
# -EHsc (enable proper Exxeption Handling) needs to avoid C4530
# -Wall is too noisy so that set -W4. 
# https://docs.microsoft.com/en-us/cpp/build/reference/compiler-option-warning-level?view=vs-2017
# "However, for a new project, it may be best to use /W4 in all compilations;
# this will ensure the fewest possible hard-to-find code defects."
set(CMAKE_CXX_FLAGS "-std=c++14 -W4 -EHsc")

else()
# g++ option for *nix
set(CMAKE_CXX_FLAGS  "-std=c++14 -Wall -O2")

endif()

get_directory_property(hasParent PARENT_DIRECTORY)
if(hasParent)
  message(STATUS "Has a parent scope.")
else()
  message(STATUS "Doesn't have a parent scope.")
endif()

function(set_with_parent ARG_NAME ARG_PATH ARG_TEXT)
    set(${ARG_NAME} ${ARG_PATH} CACHE PATH ${ARG_TEXT} FORCE)
    if (hasParent)
      set(${ARG_NAME} ${ARG_PATH} PARENT_SCOPE)
    endif()
endfunction(set_with_parent)

set(Currender_LIBS)
set(Currender_INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR};${CMAKE_CURRENT_SOURCE_DIR}/include;${CMAKE_CURRENT_SOURCE_DIR}/src")

# third_party directries
if(NOT DEFINED NANORT_INSTALL_DIR)
  set_with_parent(NANORT_INSTALL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/third_party/nanort" "NanoRT installed directory")
endif()
message("NANORT_INSTALL_DIR: ${NANORT_INSTALL_DIR}")

if(NOT DEFINED UGU_INSTALL_DIR)
  set_with_parent(UGU_INSTALL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/third_party/ugu" "UGU installed directory")
endif()
message("UGU_INSTALL_DIR: ${UGU_INSTALL_DIR}")

add_subdirectory(${UGU_INSTALL_DIR})

# inherit definitions from ugu
# todo: find better way...
if(UGU_USE_STB)
  add_definitions(-DUGU_USE_STB)
endif()
if(UGU_USE_OPENCV)
  add_definitions(-DUGU_USE_OPENCV)
  # need to find again here otherwise linking fails.
  # ${OpenCV_LIBS} contains only names (no directory) of all modules
  # while full path of world module is linked in VS project...
  find_package(OpenCV REQUIRED)
  add_definitions(-DUGU_USE_OPENCV)
  set(Currender_LIBS ${Currender_LIBS} ${OpenCV_LIBS})
endif()
if(UGU_USE_TINYOBJLOADER)
  add_definitions(-DUGU_USE_TINYOBJLOADER)
endif()
if(UGU_USE_LODEPNG)
  add_definitions(-DUGU_USE_LODEPNG)
endif()
if(UGU_USE_TINYCOLORMAP)
  add_definitions(-DUGU_USE_TINYCOLORMAP)
endif()
if(UGU_USE_OPENMP)
  add_definitions(-DUGU_USE_OPENMP)
endif()

# switch for optional libraries
option(CURRENDER_USE_OPENMP "Use OpenMP to enable parallelization" ON)
message("CURRENDER_USE_OPENMP: ${CURRENDER_USE_OPENMP}")
if(CURRENDER_USE_OPENMP)
  add_definitions(-DCURRENDER_USE_OPENMP)
endif()

option(CURRENDER_USE_NANORT "Use NanoRT for Raytracer" ON)
message("CURRENDER_USE_NANORT: ${CURRENDER_USE_NANORT}")
if(CURRENDER_USE_NANORT)
  add_definitions(-DCURRENDER_USE_NANORT)
  set(Currender_INCLUDE_DIRS ${Currender_INCLUDE_DIRS} ${NANORT_INSTALL_DIR})
endif()

# For OpenMP
if(CURRENDER_USE_OPENMP)
  find_package(OpenMP REQUIRED)
  if(OpenMP_FOUND)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
  endif()
endif()

set(Currender_LIB ${PROJECT_NAME})
add_library(${Currender_LIB}
  STATIC
  include/currender/renderer.h
  include/currender/raytracer.h
  include/currender/rasterizer.h

  src/raytracer.cc
  src/rasterizer.cc
  src/pixel_shader.h
  src/util_private.h
  src/util_private.cc
)

set(Currender_LIBS ${Currender_LIBS} ${Currender_LIB} ${Ugu_LIBS})
set(Currender_INCLUDE_DIRS ${Currender_INCLUDE_DIRS} ${Ugu_INCLUDE_DIRS})

set_with_parent(Currender_LIBS "${Currender_LIBS}" "Currender_LIBS")
set_with_parent(Currender_INCLUDE_DIRS "${Currender_INCLUDE_DIRS}" "Currender_INCLUDE_DIRS")

message("Currender_LIBS: ${Currender_LIBS}") 
message("Currender_INCLUDE_DIRS: ${Currender_INCLUDE_DIRS}") 

target_include_directories(${Currender_LIB} PUBLIC ${Currender_INCLUDE_DIRS})
set_target_properties(${Currender_LIB} PROPERTIES VERSION ${PROJECT_VERSION})

if (NOT hasParent)
  set(MINIMUM_EXAMPLE_EXE currender_minimum_example)
  add_executable(${MINIMUM_EXAMPLE_EXE}
    minimum_example.cc)
  target_include_directories(${MINIMUM_EXAMPLE_EXE} PRIVATE ${Currender_INCLUDE_DIRS})
  target_link_libraries(${MINIMUM_EXAMPLE_EXE}
    ${Currender_LIBS}
    )

  set(EXAMPLES_EXE currender_examples)
  add_executable(${EXAMPLES_EXE}
    examples.cc)
  target_include_directories(${EXAMPLES_EXE} PRIVATE ${Currender_INCLUDE_DIRS})
  target_link_libraries(${EXAMPLES_EXE}
    ${Currender_LIBS}
    )
endif()

if (WIN32)
  set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT ${EXAMPLES_EXE})
  # suppress C2338 for eigen
  add_definitions(-D_ENABLE_EXTENDED_ALIGNED_STORAGE)
endif()

# make test data directory
if(NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/data)
file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/data)
endif()
if(NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/data/bunny)
file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/data/bunny)
endif()
if(NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/data/minimum_example)
file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/data/minimum_example)
endif()

# test data preparation
if(NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/data/bunny/bunny.obj)

# download test data
if(NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/data/bunny.zip)
file(DOWNLOAD http://www.kunzhou.net/tex-models/bunny.zip ${CMAKE_CURRENT_SOURCE_DIR}/data/bunny.zip)
endif()

# unzip test data
if (NOT hasParent)
  add_custom_target( Currender_UNZip ALL)
  add_custom_command(TARGET Currender_UNZip PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E tar xzf ${CMAKE_CURRENT_SOURCE_DIR}/data/bunny.zip
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/data/bunny
  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/data/bunny.zip
  COMMENT "Unpacking bunny.zip"
  VERBATIM)
endif()

endif()