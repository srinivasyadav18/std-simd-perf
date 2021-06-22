#include <cmath>
#include <experimental/simd>

// Actual test function object
struct test_t
{
    template <typename T>
    void operator()(T &i)
    {
        using std::log;
        using std::experimental::log;
        i = log(i);
    }
};

// containes main() and other helpers to execute
#include "../generate_plots.hpp"