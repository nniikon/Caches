cmake_minimum_required(VERSION 3.14)
project(Caches LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(BUILD_MAIN "Build the test applications" ON)

# Loguru
set(LOGURU_WITH_STREAMS TRUE)
add_subdirectory(external/loguru)

include(CTest)

set(DEBUG_FLAGS
    -D_DEBUG
    -ggdb3
    -std=c++20
    -Wdocumentation -Wdocumentation-unknown-command
    -O0
    -Wall
    -Wextra
    -Wpedantic
    -Weffc++
    -Wmissing-declarations
    -Wcast-align
    -Wcast-qual
    -Wchar-subscripts
    -Wconversion
    -Wctor-dtor-privacy
    -Wempty-body
    -Wfloat-equal
    -Wformat-nonliteral
    -Wformat-security
    -Wformat-signedness
    -Wformat=2
    -Winline
    -Wnon-virtual-dtor
    -Woverloaded-virtual
    -Wpacked
    -Wpointer-arith
    -Winit-self
    -Wredundant-decls
    -Wshadow
    -Wshadow-all
    -Wextra-semi
    -Wheader-hygiene
    -Wsign-conversion
    -Wsign-promo
    -Wstrict-overflow=2
    -Wsuggest-override
    -Wswitch-default
    -Wswitch-enum
    -Wundef
    -Wunreachable-code
    -Wextra-semi
    -Wheader-hygiene
    -Wunused
    -Wvariadic-macros
    -Wno-missing-field-initializers
    -Wfloat-conversion
    -Wdouble-promotion
    -Wcomma
    -Wno-narrowing
    -Wno-old-style-cast
    -Wno-varargs
    -Wstack-protector
    -fcheck-new
    -fsized-deallocation
    -fstack-protector
    -fstrict-overflow
    -fno-omit-frame-pointer
    -fPIE
    -Werror=vla
    -fsanitize=address,alignment,bool,bounds,enum,float-cast-overflow,float-divide-by-zero,integer-divide-by-zero,leak,nonnull-attribute,null,return,returns-nonnull-attribute,shift,signed-integer-overflow,undefined,unreachable,vla-bound,vptr
)

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
  target_compile_options(CachesTests PRIVATE
      $<$<CONFIG:Debug>:${DEBUG_FLAGS}>
  )

  target_link_options(CachesTests PRIVATE
      $<$<CONFIG:Debug>:-fsanitize=address,alignment,bool,bounds,enum,float-cast-overflow,float-divide-by-zero,integer-divide-by-zero,leak,nonnull-attribute,null,return,returns-nonnull-attribute,shift,signed-integer-overflow,undefined,unreachable,vla-bound,vptr -fstack-protector -fPIE -pie>
  )

  if(MSVC)
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
  endif()

  include(GoogleTest)
  gtest_discover_tests(CachesTests)
endif()