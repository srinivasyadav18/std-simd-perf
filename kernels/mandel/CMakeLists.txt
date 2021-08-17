set (target mandelbrot)

add_hpx_executable(${target}
    ESSENTIAL
    SOURCES main.cpp
)

target_compile_options(${target} PUBLIC "-march=native")
target_link_libraries(${target} PUBLIC HPX::hpx)
add_test(NAME ${target}_test COMMAND ${target} ${CORES} ${END})