using std::sin;
using std::cos;
using std::experimental::sin;
using std::experimental::cos;

// Actual test function object
struct test_t
{
    template <typename T>
    void operator()(T&& t)
    {
        auto &x = hpx::get<0>(t);
        auto &y = hpx::get<1>(t);
        x = 5 * x + y;
    }
} test_{};

#define SIMD_TEST_WITH_FLOAT
#define SIMD_TEST_WITH_DOUBLE