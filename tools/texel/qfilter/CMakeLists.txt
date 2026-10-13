# quiet_eval: Petir's static evaluation and quiescence scores per FEN, for the
# Texel quiet filter. Build it from a separate source tree (a worktree at the
# commit whose evaluation is tuned), never from the shared src/:
#   cmake -S tools/texel/qfilter -B build/texel/qfilter -G "Visual Studio 17 2022" -A x64
#         -DPETIR_SRC=C:/chess/Petir_CPP/build/<tree>/src
#   cmake --build build/texel/qfilter --config Release -- -m
cmake_minimum_required(VERSION 3.20)
project(QuietEval LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(PETIR_SRC "" CACHE PATH "Petir source directory (a tree's src/)")
if(NOT EXISTS "${PETIR_SRC}/ueval.cpp")
    message(FATAL_ERROR "Set -DPETIR_SRC to a Petir src/ directory")
endif()
find_package(Threads REQUIRED)
file(GLOB PETIR_SOURCES "${PETIR_SRC}/*.cpp")
list(FILTER PETIR_SOURCES EXCLUDE REGEX "(main|selftest)\\.cpp$")
add_executable(quiet_eval quiet_eval.cpp ${PETIR_SOURCES})
target_include_directories(quiet_eval PRIVATE "${PETIR_SRC}")
target_link_libraries(quiet_eval PRIVATE Threads::Threads)
set_target_properties(quiet_eval PROPERTIES UNITY_BUILD ON UNITY_BUILD_BATCH_SIZE 40)
# Keep the driver's own names out of the Petir unity file.
set_source_files_properties(quiet_eval.cpp PROPERTIES SKIP_UNITY_BUILD_INCLUSION ON)
if(MSVC)
    target_compile_options(quiet_eval PRIVATE /utf-8 /W3 /permissive- /bigobj $<$<CONFIG:Release>:/GS->)
else()
    target_compile_options(quiet_eval PRIVATE -fwrapv -fno-strict-aliasing)
endif()
