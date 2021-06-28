using std::sin;
using std::cos;
using std::experimental::sin;
using std::experimental::cos;

// Actual test function object
struct test_t
{    
    template <typename T>
    void operator()(T &x)
    {
        for (int i = 0; i < 100; i++)
            x = 5 * sin(x) + 6 * cos(x);
    }
} test_{};

#define SIMD_TEST_WITH_FLOAT
#define SIMD_TEST_WITH_DOUBLE