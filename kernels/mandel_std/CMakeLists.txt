set (target mandelbrot_std)

add_executable(${target}
    main.cpp
)

target_compile_options(${target} PUBLIC "-O3" "-std=c++20" "-march=native" "-ftree-vectorize" "-ffast-math")
target_link_libraries(${target} PUBLIC Boost::boost "-ltbb")
add_test(NAME ${target}_test COMMAND ${target} ${CORES} ${END})