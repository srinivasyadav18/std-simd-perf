#include <cmath>
#include <experimental/simd>

// Actual test function object
struct test_t
{
    template <typename T>
    void operator()(T &i)
    {
        using std::exp;
        using std::experimental::exp;
        i = exp(i);
    }
};

// containes main() and other helpers to execute
#include "../generate_plots.hpp"