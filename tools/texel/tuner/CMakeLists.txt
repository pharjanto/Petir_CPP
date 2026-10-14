# eval_trace: how Petir's evaluation depends on each tunable weight, for the
# Texel tuner; texel_fit fits the weights to its output. eval_trace compiles
# the whole engine with PETIR_EVAL_TUNING, which turns the evaluation's scalar
# weights into variables and makes the pawn caches recompute every entry; the
# engine build is not affected. Build it from a
# separate source tree (a worktree at the commit whose evaluation is tuned),
# never from the shared src/:
#   cmake -S tools/texel/tuner -B build/texel/tuner -G "Visual Studio 17 2022" -A x64
#         -DPETIR_SRC=C:/chess/Petir_CPP/build/<tree>/src
#   cmake --build build/texel/tuner --config Release -- -m
cmake_minimum_required(VERSION 3.20)
project(EvalTrace LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(PETIR_SRC "" CACHE PATH "Petir source directory (a tree's src/)")
if(NOT EXISTS "${PETIR_SRC}/ueval.cpp")
    message(FATAL_ERROR "Set -DPETIR_SRC to a Petir src/ directory")
endif()
find_package(Threads REQUIRED)
file(GLOB PETIR_SOURCES "${PETIR_SRC}/*.cpp")
list(FILTER PETIR_SOURCES EXCLUDE REGEX "(main|selftest)\\.cpp$")
add_executable(eval_trace eval_trace.cpp weights.cpp ${PETIR_SOURCES})
target_include_directories(eval_trace PRIVATE "${PETIR_SRC}")
target_compile_definitions(eval_trace PRIVATE PETIR_EVAL_TUNING)
target_link_libraries(eval_trace PRIVATE Threads::Threads)
set_target_properties(eval_trace PROPERTIES UNITY_BUILD ON UNITY_BUILD_BATCH_SIZE 40)
# Keep the tool's own names out of the Petir unity file.
set_source_files_properties(eval_trace.cpp weights.cpp PROPERTIES SKIP_UNITY_BUILD_INCLUSION ON)
if(MSVC)
    target_compile_options(eval_trace PRIVATE /utf-8 /W3 /permissive- /bigobj $<$<CONFIG:Release>:/GS->)
else()
    target_compile_options(eval_trace PRIVATE -fwrapv -fno-strict-aliasing)
endif()

# texel_fit: fits the weights to a traced, labelled set (see fit.cpp). It reads
# only the trace files, so it does not include the engine.
add_executable(texel_fit fit.cpp)
target_link_libraries(texel_fit PRIVATE Threads::Threads)
if(MSVC)
    target_compile_options(texel_fit PRIVATE /utf-8 /W3 /permissive- $<$<CONFIG:Release>:/GS->)
endif()
