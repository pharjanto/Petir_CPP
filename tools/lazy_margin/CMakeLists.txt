# Lazy-eval statistics in search: the new tree's engine with a second ueval.cpp
# compared call by call at the lazy checkpoint, and the replay analyzer. See README.md.
#
#   cmake -S tools/lazy_margin -B <build dir> -G "Visual Studio 17 2022" -A x64
#         -DLAZY_TREES=<OUT of prepare_trees.py> [-DLAZY_SAME_MATERIAL=ON]
#   cmake --build <build dir> --config Release
cmake_minimum_required(VERSION 3.20)
project(PetirLazyMargin LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(LAZY_TREES "" CACHE PATH "OUT directory of prepare_trees.py (holds new/src and second/src)")
option(LAZY_SAME_MATERIAL "The second eval uses the new tree's piece values (no material recompute)" OFF)

add_executable(analyze analyze.cpp)
target_compile_options(analyze PRIVATE /utf-8 /W3 /permissive-)

if(NOT EXISTS "${LAZY_TREES}/new/src/ueval.cpp" OR NOT EXISTS "${LAZY_TREES}/second/src/ueval.cpp")
    message(STATUS "LAZY_TREES not set: building analyze only.")
    return()
endif()
set(NEW_SRC "${LAZY_TREES}/new/src")
set(SECOND_SRC "${LAZY_TREES}/second/src")
file(GLOB NEW_SOURCES ${NEW_SRC}/*.cpp)
# lazystat: the whole engine (UCI). Search sees exactly the new eval; every
# null-window call that reaches the checkpoint is also run through the second eval.
add_executable(lazystat lazy_harness.cpp ${NEW_SOURCES} ${SECOND_SRC}/ueval.cpp)
set_source_files_properties(${NEW_SRC}/ueval.cpp PROPERTIES COMPILE_DEFINITIONS "evaluate=evaluateCandidate")
set_source_files_properties(${SECOND_SRC}/ueval.cpp PROPERTIES
    COMPILE_DEFINITIONS "petir_ueval=petir_ueval_old" INCLUDE_DIRECTORIES ${SECOND_SRC})
target_include_directories(lazystat PRIVATE ${NEW_SRC} ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_options(lazystat PRIVATE /utf-8 /W1 /permissive- /bigobj /GS-)
if(LAZY_SAME_MATERIAL)
    target_compile_definitions(lazystat PRIVATE LAZY_SAME_MATERIAL)
endif()
