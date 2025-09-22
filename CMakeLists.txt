cmake_minimum_required(VERSION 3.14)
project(Caches LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(BUILD_MAIN "Build the main applications" ON)

# Loguru
set(LOGURU_WITH_STREAMS TRUE)
add_subdirectory(external/loguru)

# Standard testing toggle & plumbing
include(CTest)

# ---- Main ----
if(BUILD_MAIN)
  add_executable(CachesPerfect
    ${PROJECT_SOURCE_DIR}/source/main_perfect.cpp
  )
  target_link_libraries(CachesPerfect PRIVATE loguru)
  target_include_directories(CachesPerfect PUBLIC
    ${PROJECT_SOURCE_DIR}/include
  )
  target_include_directories(CachesPerfect PRIVATE
    ${PROJECT_SOURCE_DIR}/external/loguru
  )

  add_executable(Caches2Q
    ${PROJECT_SOURCE_DIR}/source/main_2q.cpp
  )
  target_link_libraries(Caches2Q PRIVATE loguru)
  target_include_directories(Caches2Q PUBLIC
    ${PROJECT_SOURCE_DIR}/include
  )
  target_include_directories(Caches2Q PRIVATE
    ${PROJECT_SOURCE_DIR}/external/loguru
  )
endif()

# ---- Tests ----
if(BUILD_TESTING)
  include(FetchContent)
  FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
  )
  FetchContent_MakeAvailable(googletest)

  add_executable(CachesTests
    ${PROJECT_SOURCE_DIR}/source/test.cpp
  )
  target_link_libraries(CachesTests PRIVATE loguru GTest::gtest_main)
  target_include_directories(CachesTests PUBLIC
    ${PROJECT_SOURCE_DIR}/include
  )
  target_include_directories(CachesTests PRIVATE
    ${PROJECT_SOURCE_DIR}/external/loguru
  )

  if(MSVC)
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
  endif()

  include(GoogleTest)
  gtest_discover_tests(CachesTests)
endif()