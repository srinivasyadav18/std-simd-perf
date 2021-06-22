#include <cmath>
#include <experimental/simd>

// Actual test function object
struct test_t
{
    template <typename T>
    void operator()(T &i)
    {
        using std::cos;
        using std::experimental::cos;
        i = cos(i);
    }
};

// containes main() and other helpers to execute
#include "../generate_plots.hpp"