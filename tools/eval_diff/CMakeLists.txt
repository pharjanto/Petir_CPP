# Differential tests for exact-score evaluation changes: a candidate src/ueval.cpp
# against a baseline one. See README.md.
#
#   cmake -S tools/eval_diff -B <build dir> -G "Visual Studio 17 2022" -A x64
#         -DCANDIDATE_SRC=<tree>/src -DBASELINE_SRC=<other tree>/src [-DEVAL_DIFF_CHECK_BOUNDS=ON]
#   cmake --build <build dir> --config Release
cmake_minimum_required(VERSION 3.20)
project(PetirEvalDiff LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CANDIDATE_SRC "" CACHE PATH "src directory of the tree with the changed ueval.cpp")
set(BASELINE_SRC "" CACHE PATH "src directory of the reference tree (for example a git worktree at HEAD)")
option(EVAL_DIFF_CHECK_BOUNDS "Check the original Pascal array bounds" OFF)
if(NOT EXISTS "${CANDIDATE_SRC}/ueval.cpp" OR NOT EXISTS "${BASELINE_SRC}/ueval.cpp")
    message(FATAL_ERROR "Set CANDIDATE_SRC and BASELINE_SRC to two src directories.")
endif()
file(GLOB CANDIDATE_SOURCES ${CANDIDATE_SRC}/*.cpp)
set(LIBRARY_SOURCES ${CANDIDATE_SOURCES})
list(FILTER LIBRARY_SOURCES EXCLUDE REGEX "(main|selftest)\\.cpp$")

# evaldiff: the whole engine (UCI) with the candidate's sources. Every evaluate()
# call from search runs both evaluations and aborts on the first difference.
add_executable(evaldiff search_harness.cpp ${CANDIDATE_SOURCES} ${BASELINE_SRC}/ueval.cpp)
# evalrandom: random games, random endgames, random pawn structures and the boards
# after every pseudo-legal capture, including king captures.
add_executable(evalrandom random_harness.cpp ${LIBRARY_SOURCES} ${BASELINE_SRC}/ueval.cpp)
get_filename_component(BOOKS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../books" ABSOLUTE)
target_compile_definitions(evalrandom PRIVATE EVAL_DIFF_BOOKS="${BOOKS_DIR}")

# The candidate's evaluate() is renamed so that each harness can define
# petir_ueval::evaluate in front of it. The baseline's ueval.cpp is compiled into
# namespace petir_ueval_old, with its own headers.
set_source_files_properties(${CANDIDATE_SRC}/ueval.cpp PROPERTIES COMPILE_DEFINITIONS "evaluate=evaluateCandidate")
set_source_files_properties(${BASELINE_SRC}/ueval.cpp PROPERTIES
    COMPILE_DEFINITIONS "petir_ueval=petir_ueval_old" INCLUDE_DIRECTORIES ${BASELINE_SRC})
foreach(target evaldiff evalrandom)
    target_include_directories(${target} PRIVATE ${CANDIDATE_SRC})
    target_compile_options(${target} PRIVATE /utf-8 /W1 /permissive- /bigobj)
    if(EVAL_DIFF_CHECK_BOUNDS)
        target_compile_definitions(${target} PRIVATE PETIR_CHECK_BOUNDS)
    endif()
endforeach()
