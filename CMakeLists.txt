cmake_minimum_required(VERSION 3.10)

project(tokenizer CXX)
include(CMakePackageConfigHelpers)
include(CTest)
include(GNUInstallDirs)

# ABI version
# http://tldp.org/HOWTO/Program-Library-HOWTO/shared-libraries.html
set(SONAME 10)

add_subdirectory(re2)

add_library(tokenizer tokenizer/bpe.cc tokenizer/bpe.h ${RE2_SOURCES})
target_compile_features(tokenizer PUBLIC cxx_std_11)
target_include_directories(tokenizer PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/re2)
set_target_properties(tokenizer PROPERTIES SOVERSION ${SONAME} VERSION ${SONAME}.0.0)

if(APPLE)
  set_target_properties(tokenizer PROPERTIES
    FRAMEWORK TRUE
    FRAMEWORK_VERSION A
    MACOSX_FRAMEWORK_IDENTIFIER wang.yi.tokenizer
    PUBLIC_HEADER "tokenizer/bpe.h"
  )
endif()

add_executable(bpe_test tokenizer/bpe_test.cc)
set_target_properties(bpe_test
  PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/bin"
)
target_link_libraries(bpe_test PRIVATE tokenizer re2::re2)
