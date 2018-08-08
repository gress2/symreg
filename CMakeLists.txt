cmake_minimum_required (VERSION 3.5.1)
project (symreg VERSION 0.1 LANGUAGES CXX)

set (CMAKE_CXX_STANDARD 17)

set (CMAKE_CXX_FLAGS "-ferror-limit=1 -Wfatal-errors -Wall -O3")
set (CMAKE_CXX_FLAGS_DEBUG "-ferror-limits=1 -Wfatal-errors -Wall")
set (CMAKE_CXX_FLAGS_RELEASE "-ferror-limit=1 -Wfatal-errors -O3")

set (CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/lib")
set (CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/lib")
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/bin")
set (CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG "${CMAKE_CURRENT_BINARY_DIR}/Debug/lib")
set (CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG "${CMAKE_CURRENT_BINARY_DIR}/Debug/lib")
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_CURRENT_BINARY_DIR}/Debug/bin")
set (CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE "${CMAKE_CURRENT_BINARY_DIR}/Release/lib")
set (CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE "${CMAKE_CURRENT_BINARY_DIR}/Release/lib")
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_CURRENT_BINARY_DIR}/Release/bin")
set (TEST_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_BUILD_TYPE}/bin")

set (include_dir "${CMAKE_CURRENT_SOURCE_DIR}/include")
set (dlib_include_dir "${CMAKE_CURRENT_SOURCE_DIR}/deps/dlib")
include_directories (${include_dir} ${dlib_include_dir})

add_subdirectory("./deps/dlib")

set (symreg_tests OFF)
if (MAKE_TESTS)
  set (symreg_tests ON)
  set (MAKE_TESTS OFF CACHE BOOL "MAKE_TESTS" FORCE)
endif()

add_subdirectory("./deps/Brick")
link_directories(${ANTLR4CPP_LIBS})

option (MAKE_TESTS "MAKE_TESTS" OFF)

if (symreg_tests)
  enable_testing()
  add_subdirectory("./deps/googletest")
  add_subdirectory("./test")
endif()

add_subdirectory("./src")
