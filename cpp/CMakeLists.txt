cmake_minimum_required(VERSION 3.16)
project(Benchmark-v700)

set(CMAKE_CXX_STANDARD 20)
set(BENCH_DIR "microbench")
set(DS_DIR "ds")
set(BUILD_DIR "bin/")
set(RECLAIM ".debra")

#set(SOURCE_EXE main.cpp)
#set(CMAKE_CXX_FLAGS "-lpthread -ldl -mrtm")
list(APPEND CMAKE_CXX_FLAGS_LIST "-lpthread -ldl -mrtm ")
list(APPEND CXX_LIBS_LIST "pthread" "dl")

### if libpapi is installed but you do not want to use it, invoke make with extra argument "has_libpapi=0"
execute_process(COMMAND "./${BENCH_DIR}/_check_lib.sh papi" OUTPUT_VARIABLE has_libpapi)
if (NOT ${has_libpapi} MATCHES "0")
    add_definitions("-DUSE_PAPI")
    list(APPEND CXX_LIBS_LIST "papi")
endif ()

### if libnuma is installed but you do not want to use it, invoke make with extra argument "has_libnuma=0"
execute_process(COMMAND "./${BENCH_DIR}/_check_lib.sh numa" OUTPUT_VARIABLE has_libnuma)
if (NOT ${has_libnuma} MATCHES "0")
    add_definitions("-DUSE_LIBNUMA")
    list(APPEND CXX_LIBS_LIST "numa")
endif ()

option(USE_ASAN "USE_ASAN" OFF)
if (USE_ASAN)
    add_definitions("-fsanitize=address" "-static-libasan")
endif ()

option(USE_ASSERTS "USE_ASSERTS" ON)
if (USE_ASSERTS)
    add_definitions("-DNDEBUG")
endif ()

option(USE_FOPENMP "USE_FOPENMP" ON)
if (USE_FOPENMP)
    find_package(OpenMP)
    list(APPEND CXX_LIBS_LIST "${OpenMP_CXX_FLAGS}")
endif ()

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

add_definitions("-DMAX_THREADS_POW2=512"
        "-DCPU_FREQ_GHZ=2.1"
        "-DMEMORY_STATS=if\(1\)"
        "-DMEMORY_STATS2=if\(0\)"
        "-std=c++20 -mcx16 -gdwarf"
        "-fno-omit-frame-pointer"
        "-DDEBRA_ORIGINAL_FREE"
        "-DKEY_DEPTH_TOTAL_STAT"
        "-DKEY_SEARCH_TOTAL_STAT")

set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS_LIST})
message(CMAKE_CXX_FLAGS=${CMAKE_CXX_FLAGS})

include_directories(.)
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


include_directories(${BENCH_DIR})

find_package(Threads REQUIRED)

set(SOURCE_EXE ${BENCH_DIR}/main.cpp)

#include_directories(dragonfly_table)
#add_subdirectory(dragonfly_table)

file(GLOB ADAPTERS ${DS_DIR}/*/adapter.h)
string(REGEX MATCHALL "\/[^\/]+\/adapter.h" RAW_DATA_STRUCTURES ${ADAPTERS})

foreach (DATA_STRUCTURE ${RAW_DATA_STRUCTURES})
    #    message(ADAPTER="${ADAPTER}")
    string(REPLACE "\/adapter.h" "" DS ${DATA_STRUCTURE})
    string(REPLACE "\/" "" DS ${DS})
    list(APPEND DATA_STRUCTURES ${DS})
endforeach ()

include_directories(ds/verlib_structs/include)

foreach (DS ${DATA_STRUCTURES})

    message(DS_DIR= "${DS_DIR}/${DS}")

#    include_directories(${DS_DIR}/${DS})

#    set(DS_RUN ${BUILD_DIR}${DS}${RECLAIM})
    set(DS_RUN ${DS}${RECLAIM})

    add_executable(${DS_RUN} ${SOURCE_EXE})

    target_include_directories(${DS_RUN} PUBLIC ${DS_DIR}/${DS})

    target_link_libraries(${DS_RUN} PUBLIC ${CXX_LIBS_LIST})

    if (${DS} MATCHES "dragfly.*")
        add_subdirectory(dragonfly_table)

        message(DS="${DS} || dragfly type")

        target_include_directories(${DS_RUN} PUBLIC dragonfly_table)

#        include_directories(dragonfly_table)
#        include_directories(dragonfly_table/src)

        target_link_libraries(${DS_RUN} PUBLIC dragonfly_static)
    endif ()

    if (${DS} MATCHES "verlib.*")

        message(DS="${DS} || verlib type")

        target_include_directories(${DS_RUN} PUBLIC parlay)
    endif ()
endforeach ()

#message(ADAPTERS="${ADAPTERS}")
#message(DATA_STRUCTURES="${DATA_STRUCTURES}")
#message(DATA_STRUCTURES_NEW="${DATA_STRUCTURES}")
