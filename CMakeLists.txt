cmake_minimum_required(VERSION 3.20)

project(maxcore VERSION 2.5.0 LANGUAGES C CXX)

option(MAXCORE_STRICT_FP "Enable strict floating-point determinism flags" ON)
option(MAXCORE_ENABLE_WORLD_BANK "Build WorldBank research pipeline executables" ON)
option(MAXCORE_USE_CURL "Enable libcurl for WorldBank pipeline (worldbank_pipeline)" OFF)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# =========================
# Helpers
# =========================
function(maxcore_apply_warnings tgt)
  if (MSVC)
    target_compile_options(${tgt} PRIVATE /W4)
  else()
    target_compile_options(${tgt} PRIVATE
      -Wall -Wextra -Wpedantic -Wconversion -Wshadow -Wfloat-equal
    )
  endif()
endfunction()

function(maxcore_apply_strict_fp tgt)
  if (MAXCORE_STRICT_FP)
    if (MSVC)
      target_compile_options(${tgt} PRIVATE /fp:strict)
    else()
      target_compile_options(${tgt} PRIVATE
        -fno-fast-math
        -ffp-contract=off
        -fno-unsafe-math-optimizations
        -fno-associative-math
      )
    endif()
  endif()
endfunction()

# =========================
# Library: MAX-Core + Derived
# =========================
add_library(maxcore STATIC
  src/maxcore/maxcore.cpp
  src/maxcore/derived.cpp
)

target_include_directories(maxcore
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

maxcore_apply_warnings(maxcore)
maxcore_apply_strict_fp(maxcore)

# =========================
# C API (shared library)
# =========================
add_library(maxcore_capi SHARED
  src/maxcore/c_api.cpp
)

target_link_libraries(maxcore_capi PRIVATE maxcore)

target_include_directories(maxcore_capi
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_definitions(maxcore_capi PRIVATE MAXCORE_CAPI_BUILD=1)

maxcore_apply_warnings(maxcore_capi)
maxcore_apply_strict_fp(maxcore_capi)

# =========================
# Examples (C++ only)
# =========================
if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/examples/pipeline_cpp.cpp")
  add_executable(example_pipeline_cpp examples/pipeline_cpp.cpp)
  target_link_libraries(example_pipeline_cpp PRIVATE maxcore)
  target_include_directories(example_pipeline_cpp PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
  maxcore_apply_warnings(example_pipeline_cpp)
  maxcore_apply_strict_fp(example_pipeline_cpp)
endif()

# =========================
# WorldBank Research Pipeline (optional)
# =========================
if (MAXCORE_ENABLE_WORLD_BANK)

  # heatmap_exporter uses Windows WIC (windows.h / wincodec.h)
  if (WIN32 AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/examples/heatmap_exporter.cpp")
    add_executable(heatmap_exporter examples/heatmap_exporter.cpp)
    target_link_libraries(heatmap_exporter PRIVATE maxcore)
    target_include_directories(heatmap_exporter PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

    # WIC / COM libs (safe for MSVC + MinGW)
    target_link_libraries(heatmap_exporter PRIVATE windowscodecs ole32)

    maxcore_apply_warnings(heatmap_exporter)
    maxcore_apply_strict_fp(heatmap_exporter)
  endif()

  # worldbank_pipeline requires CURL
  if (MAXCORE_USE_CURL AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/examples/worldbank_pipeline.cpp")
    find_package(CURL REQUIRED)

    add_executable(worldbank_pipeline examples/worldbank_pipeline.cpp)
    target_link_libraries(worldbank_pipeline PRIVATE maxcore CURL::libcurl)
    target_include_directories(worldbank_pipeline PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
    maxcore_apply_warnings(worldbank_pipeline)
    maxcore_apply_strict_fp(worldbank_pipeline)
  endif()

endif()

# =========================
# Tests (single executable, only if tests exist)
# =========================
include(CTest)

set(MAXCORE_TEST_INIT "${CMAKE_CURRENT_SOURCE_DIR}/tests/test_init.cpp")
if (BUILD_TESTING AND EXISTS "${MAXCORE_TEST_INIT}")

  add_executable(test_init tests/test_init.cpp)
  target_link_libraries(test_init PRIVATE maxcore)
  add_test(NAME test_init COMMAND test_init)

  add_executable(test_validation tests/test_validation.cpp)
  target_link_libraries(test_validation PRIVATE maxcore)
  add_test(NAME test_validation COMMAND test_validation)

  add_executable(test_collapse tests/test_collapse.cpp)
  target_link_libraries(test_collapse PRIVATE maxcore)
  add_test(NAME test_collapse COMMAND test_collapse)

  add_executable(test_canonical tests/test_canonical.cpp)
  target_link_libraries(test_canonical PRIVATE maxcore)
  add_test(NAME test_canonical COMMAND test_canonical)

  add_executable(test_norm_guard tests/test_norm_guard.cpp)
  target_link_libraries(test_norm_guard PRIVATE maxcore)
  add_test(NAME test_norm_guard COMMAND test_norm_guard)

  add_executable(test_dt_stability tests/test_dt_stability.cpp)
  target_link_libraries(test_dt_stability PRIVATE maxcore)
  add_test(NAME test_dt_stability COMMAND test_dt_stability)

  add_executable(test_determinism tests/test_determinism.cpp)
  target_link_libraries(test_determinism PRIVATE maxcore)
  add_test(NAME test_determinism COMMAND test_determinism)

  add_executable(test_derived_consistency tests/test_derived_consistency.cpp)
  target_link_libraries(test_derived_consistency PRIVATE maxcore)
  add_test(NAME test_derived_consistency COMMAND test_derived_consistency)

  add_executable(test_terminal_shortcircuit tests/test_terminal_shortcircuit.cpp)
  target_link_libraries(test_terminal_shortcircuit PRIVATE maxcore)
  add_test(NAME test_terminal_shortcircuit COMMAND test_terminal_shortcircuit)

  add_executable(test_c_api_parity tests/test_c_api_parity.cpp)
  target_link_libraries(test_c_api_parity PRIVATE maxcore maxcore_capi)
  add_test(NAME test_c_api_parity COMMAND test_c_api_parity)

  add_executable(test_c_api_error_channel tests/test_c_api_error_channel.cpp)
  target_link_libraries(test_c_api_error_channel PRIVATE maxcore_capi)
  add_test(NAME test_c_api_error_channel COMMAND test_c_api_error_channel)

  add_executable(test_params_reject tests/test_params_reject.cpp)
  target_link_libraries(test_params_reject PRIVATE maxcore)
  add_test(NAME test_params_reject COMMAND test_params_reject)

  add_executable(test_initial_terminal tests/test_initial_terminal.cpp)
  target_link_libraries(test_initial_terminal PRIVATE maxcore)
  add_test(NAME test_initial_terminal COMMAND test_initial_terminal)

  add_executable(test_clamp_invariants tests/test_clamp_invariants.cpp)
  target_link_libraries(test_clamp_invariants PRIVATE maxcore)
  add_test(NAME test_clamp_invariants COMMAND test_clamp_invariants)

  add_executable(test_long_run_finite tests/test_long_run_finite.cpp)
  target_link_libraries(test_long_run_finite PRIVATE maxcore)
  add_test(NAME test_long_run_finite COMMAND test_long_run_finite)

endif()
