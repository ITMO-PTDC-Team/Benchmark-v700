add_library(microbench INTERFACE)

target_include_directories(microbench INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/workloads
)

add_library(microbench::microbench ALIAS microbench)
