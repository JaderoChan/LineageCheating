set(OnnxRuntime_INCLUDE_DIRS "${CMAKE_CURRENT_LIST_DIR}/include")
set(OnnxRuntime_LIB_DIR "${CMAKE_CURRENT_LIST_DIR}/lib")

include_directories(${OnnxRuntime_INCLUDE_DIRS})
target_link_directories(${PROJECT_NAME} PRIVATE ${OnnxRuntime_LIB_DIR})

set(_ORT_LIB_NAMES
    onnxruntime_session
    onnxruntime_optimizer
    onnxruntime_providers
    onnxruntime_framework
    onnxruntime_graph
    onnxruntime_util
    onnxruntime_mlas
    onnxruntime_common
    onnxruntime_flatbuffers
    onnx
    onnx_proto
    libprotobuf-lite
    re2
    flatbuffers
    absl_base
    absl_throw_delegate
    absl_raw_hash_set
    absl_hash
    absl_city
    absl_low_level_hash
    absl_raw_logging_internal
    cpuinfo
    clog
    absl_cord
    absl_cordz_info
    absl_cord_internal
    absl_cordz_functions
    absl_cordz_handle
    absl_hashtablez_sampler
    absl_exponential_biased
    absl_synchronization
    absl_graphcycles_internal
    absl_stacktrace
    absl_symbolize
    absl_malloc_internal
    absl_debugging_internal
    absl_demangle_internal
    absl_time
    absl_civil_time
    absl_time_zone
    absl_bad_optional_access
    absl_strings
    absl_strings_internal
    absl_bad_variant_access
    absl_spinlock_wait
    absl_int128
    absl_log_severity
)

set(OnnxRuntime_LIBS "")
foreach(_lib ${_ORT_LIB_NAMES})
    set(_lib_path "${OnnxRuntime_LIB_DIR}/${_lib}.lib")
    if(EXISTS "${_lib_path}")
        list(APPEND OnnxRuntime_LIBS "${_lib_path}")
    else()
        message(WARNING "OnnxRuntime: library not found: ${_lib_path}")
    endif()
endforeach()

if(NOT OnnxRuntime_LIBS)
    message(FATAL_ERROR "No OnnxRuntime libraries found in ${OnnxRuntime_LIB_DIR}")
endif()
