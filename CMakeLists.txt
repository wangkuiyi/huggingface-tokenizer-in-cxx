cmake_minimum_required(VERSION 3.10)

project(tokenizer CXX)
include(CMakePackageConfigHelpers)
include(CTest)
include(GNUInstallDirs)

# ABI version
# http://tldp.org/HOWTO/Program-Library-HOWTO/shared-libraries.html
set(SONAME 10)

# Normally, we should call add_subdirectory with the RE2 git submodule
# to build libre2.a, and then make it a dependency of libbpe.a.
# However, we'd prefer a single static library of both bpe and re2
# code over two libraries, so can easily put it in an Apple Framework.
set(RE2_SOURCES
    re2/re2/bitmap256.cc
    re2/re2/bitstate.cc
    re2/re2/compile.cc
    re2/re2/dfa.cc
    re2/re2/filtered_re2.cc
    re2/re2/mimics_pcre.cc
    re2/re2/nfa.cc
    re2/re2/onepass.cc
    re2/re2/parse.cc
    re2/re2/perl_groups.cc
    re2/re2/prefilter.cc
    re2/re2/prefilter_tree.cc
    re2/re2/prog.cc
    re2/re2/re2.cc
    re2/re2/regexp.cc
    re2/re2/set.cc
    re2/re2/simplify.cc
    re2/re2/stringpiece.cc
    re2/re2/tostring.cc
    re2/re2/unicode_casefold.cc
    re2/re2/unicode_groups.cc
    re2/util/rune.cc
    re2/util/strutil.cc
    )

add_library(bpe src/bpe.cc src/bpe.h ${RE2_SOURCES})
target_compile_features(bpe PUBLIC cxx_std_11)
target_include_directories(bpe PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/re2)
set_target_properties(bpe PROPERTIES SOVERSION ${SONAME} VERSION ${SONAME}.0.0)

if(APPLE)
  set_target_properties(bpe PROPERTIES
    FRAMEWORK TRUE
    FRAMEWORK_VERSION A
    MACOSX_FRAMEWORK_IDENTIFIER dev.iree.iree
    VERSION 16.4.0
    SOVERSION 1.0.0
    XCODE_ATTRIBUTE_CODE_SIGNING_REQUIRED OFF
    PUBLIC_HEADER "src/bpe.h"
  )
endif()

add_executable(bpe_test src/bpe_test.cc)
set_target_properties(bpe_test
  PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/bin"
)
target_link_libraries(bpe_test PRIVATE bpe)
