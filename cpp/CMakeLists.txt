cmake_minimum_required(VERSION 3.16)
project(Benchmark-v700 LANGUAGES CXX)

##### tunable build parameters #####

option(SANITIZE        "enable sanitizers
    (compile and link with address and UB sanitizers)"      OFF)
option(HARDENING       "enable stdlib hardening
    (use stdlib hard debug asserts)"                        OFF)
option(PAPI            "enable PAPI"                         ON)
option(LIBNUMA         "enable libnuma"                      ON)
option(OPENMP          "enable OpenMP"                       ON)
option(OPTIMIZE        "enable compiler optimizations (O3)"  ON)
option(TIMELINES       "enable measure timelines
    (needed for tracing tracing)"                           OFF)
option(SKIP_VALIDATION "enable validation skipping
    (validate data structure or not)"                       OFF)
option(TESTING         "enable utility code testing
    (compile target for testing workloads code)"            OFF)
option(MAX_THREADS     "declare max threads
    (should be power of 2)"                               "512")
option(CPU_FREQ        "declare freq for server clock"    "2.1")
option(KEY_TOTAL_STAT  "trace depth & search total stats"    ON)


##### compiler configuration #####

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "")

if(CMAKE_EXPORT_COMPILE_COMMANDS)
    set(CMAKE_CXX_STANDARD_INCLUDE_DIRECTORIES
      ${CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES})
endif()

set(compilerOptions "")
set(compilerDefinitions "")
set(linkerOptions "")

list(APPEND compilerOptions -mrtm)
list(APPEND compilerOptions -Wno-volatile)
list(APPEND linkerOptions -ldl)

set(isGCC OFF)
set(isClang OFF)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(isGCC ON)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(isClang ON)
endif()

if(SANITIZE)
    list(APPEND compilerOptions -fsanitize=undefined,address)
    list(APPEND linkerOptions -fsanitize=undefined,address -static-libasan)
    list(APPEND compilerOptions
    -fno-sanitize-recover=all
    -fno-optimize-sibling-calls
    -fno-omit-frame-pointer
  )
endif()

option(USE_TIMELINES "USE_TIMELINES" OFF)
if (USE_TIMELINES)
    add_definitions("-DMEASURE_TIMELINE_STATS")
endif ()

option(NO_OPTIMIZE "NO_OPTIMIZE" OFF)
if (NO_OPTIMIZE)
    add_definitions("-O0" "-fno-inline-functions" "-fno-inline")
else ()
    add_definitions("-O3")
endif ()

option(SKIP_VALIDATION "SKIP_VALIDATION" OFF)
if (SKIP_VALIDATION)
    add_definitions("-DSKIP_VALIDATION")
endif ()

# TODO support libc++
if(HARDENING)
    list(APPEND compilerDefinitions _GLIBCXX_DEBUG)
endif()

include_directories(.)
include_directories(microbench)
include_directories(common)
include_directories(common/atomic_ops)
include_directories(common/atomic_ops/atomic_ops)
include_directories(common/dcss)
include_directories(common/descriptors)
include_directories(common/kcas)
include_directories(common/papi)
include_directories(common/recordmgr)
include_directories(common/rlu)
include_directories(common/rq)
include_directories(common/rq/snapcollector)
include_directories(common/urcu)
include_directories(ds/verlib_structs/include)

set(libs "")

if(OPTIMIZE)
    add_compile_definitions(DCMAKE_BUILD_TYPE=Release)
else()
    add_compile_definitions(DCMAKE_BUILD_TYPE=Debug)
endif()

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads)
list(APPEND libs Threads::Threads)

if(PAPI)
    find_library(papi papi REQUIRED)
    list(APPEND libs ${papi})
endif()

if(LIBNUMA)
    find_library(libnuma numa REQUIRED)
    list(APPEND libs ${libnuma})
endif()

if(OPENMP)
    find_package(OpenMP REQUIRED)
    list(APPEND libs OpenMP::OpenMP_CXX)
endif()

if(TIMELINES)
    add_definitions("-DMEASURE_TIMELINE_STATS")
endif()

if(SKIP_VALIDATION)
    add_definitions("-DSKIP_VALIDATION")
endif()

add_definitions("-DMAX_THREADS_POW2=${MAX_THREADS}")
add_definitions("-DCPU_FREQ_GHZ=${CPU_FREQ}")

if (KEY_TOTAL_STAT)
    add_definitions(
        "-DKEY_DEPTH_TOTAL_STAT"
        "-DKEY_SEARCH_TOTAL_STAT"
    )
endif()


add_definitions(
  "-DMEMORY_STATS=if\(1\)"    # purely hardcoded for trbot's code
  "-DMEMORY_STATS2=if\(0\)"   # purelu hardcoded for thbot's code
)

add_compile_options(${compilerOptions})
add_compile_definitions(${compilerDefinitions})
add_link_options(${linkerOptions})

##### configure data structures & other targets #####

set(DSDir "ds")

file(GLOB adapters ${DSDir}/*/adapter.h)

set(dataStructures "")

foreach(adapter ${adapters})
    get_filename_component(dirPath ${adapter} DIRECTORY)
    get_filename_component(dataStructure ${dirPath} NAME)
    list(APPEND dataStructures ${dataStructure})
endforeach()

list(JOIN dataStructures "\n* " dataStructuresPP)
message(STATUS "Found data structures: \n* ${dataStructuresPP}")

set(reclaimers "")
list(APPEND reclaimers "debra")

foreach(dataStructure ${dataStructures})
    foreach(reclaimer ${reclaimers})
        set(target "${dataStructure}.${reclaimer}")
        add_executable(${target} microbench/main.cpp)
        target_include_directories(${target} PUBLIC ${DSDir}/${dataStructure})
        target_link_libraries(${target} PUBLIC ${libs})
        if(${dataStructure} MATCHES "wang_openbwtree")
            target_sources(${target} PUBLIC ${DSDir}/${dataStructure}/bwtree.cpp)
            target_link_options(${target} PUBLIC -latomic)
        endif()
        if(${dataStructure} MATCHES "verlib.*")
            target_include_directories(${target} PUBLIC parlay)
        endif()
    endforeach()
endforeach()

if(TESTING)
    find_package(GTest REQUIRED)
    file(GLOB testSources tests/*.cpp)
    add_executable(tests ${testSources})
    target_link_libraries(tests GTest::gtest GTest::gtest_main)
endif()
