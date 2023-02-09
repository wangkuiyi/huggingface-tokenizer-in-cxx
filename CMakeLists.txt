cmake_minimum_required(VERSION 3.10)

project(tokenizer CXX)
include(CMakePackageConfigHelpers)
include(CTest)
include(GNUInstallDirs)

# ABI version
# http://tldp.org/HOWTO/Program-Library-HOWTO/shared-libraries.html
set(SONAME 10)

add_subdirectory(re2)

add_library(bpe src/bpe.cc src/bpe.h)
target_compile_features(bpe PUBLIC cxx_std_11)
target_include_directories(bpe PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/re2)
set_target_properties(bpe PROPERTIES SOVERSION ${SONAME} VERSION ${SONAME}.0.0)
add_dependencies(bpe re2)


