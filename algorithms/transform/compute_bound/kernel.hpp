#if defined(HPX_HAVE_EVE)
    #include <eve/eve.hpp>
    #include <eve/module/math.hpp>
#elif defined(HPX_HAVE_STD_EXPERIMENTAL_SIMD)
    #include <experimental/simd>
#endif

// Actual test function object
struct test_t
{   
    template <typename T>
    T operator()(T &x, T &y)
    {
    #if defined(HPX_HAVE_EVE)
        using eve::sin;
        using eve::cos;
    #elif defined(HPX_HAVE_STD_EXPERIMENTAL_SIMD)
        using std::sin;
        using std::cos;
        using std::experimental::sin;
        using std::experimental::cos;
    #endif

        for (int i = 0; i < 100; i++)
            x = 5.0f * sin(x) + 6 * cos(y);
        return x;
    }
} test_{};

#define SIMD_TEST_WITH_FLOAT
// #define SIMD_TEST_WITH_DOUBLE