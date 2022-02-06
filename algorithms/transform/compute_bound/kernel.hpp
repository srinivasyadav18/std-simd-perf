#include <eve/module/math.hpp>

// Actual test function object
struct test_t
{    
    template <typename T>
    T operator()(T &x, T &y)
    {
        for (int i = 0; i < 100; i++)
            x = 5.0f * eve::sin(x) + 6.0f * eve::cos(y);
        return x;
    }
} test_{};

#define SIMD_TEST_WITH_FLOAT
// #define SIMD_TEST_WITH_DOUBLE