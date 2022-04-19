set (target mandelbrot)

set (policies seq simd par par_simd)
# set (policies seq)


foreach(policy ${policies})
    set(exe ${target}_${policy})
    add_hpx_executable(${exe}
        ESSENTIAL
        SOURCES ${policy}.cpp
    )
    target_compile_options(${exe} PUBLIC "-march=native" "-g")
    target_link_libraries(${exe} PUBLIC HPX::hpx)
    add_test(NAME ${exe}_test COMMAND ${exe} ${CORES} ${END})
endforeach()

