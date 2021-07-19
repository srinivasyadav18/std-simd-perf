cmake_minimum_required(VERSION 3.17)
project(hpx_datapar_benchmarks VERSION 1.0.0)

find_package(HPX REQUIRED)

enable_testing()

function(add_simd_test target)
    add_hpx_executable(${target}
    ESSENTIAL
    SOURCES main.cpp
    )

    target_compile_options(${target} PUBLIC "-march=native" "-fno-tree-vectorize")
    target_link_libraries(${target} PUBLIC HPX::hpx)

    set(CORES)
    if(${SIMD_CORES})
        set(CORES "--hpx:threads=${SIMD_CORES}")
        message("Added extra args : ${CORES}")
    endif()

    set(END)
    if(${SIMD_END})
        set(END "--end=${SIMD_END}")
        message("Added extra args : ${END}")
    endif()

    add_test(NAME ${target}_test COMMAND ${target} ${CORES} ${END})
endfunction()

set(subdirs 
    algorithms 
    simd_math_tests
)

foreach(subdir ${subdirs})
    message("Adding subdir ${subdir}")
    add_subdirectory(${subdir})
endforeach(subdir)