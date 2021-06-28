#include <hpx/config.hpp>
#include <hpx/hpx.hpp>
#include <hpx/hpx_init.hpp>
#include <hpx/include/datapar.hpp>

#include <string>
#include <type_traits>
#include <vector>
#include <fstream>
#include <cmath>
#include <experimental/simd>

int threads;

using std::sin;
using std::cos;
using std::experimental::sin;
using std::experimental::cos;

// Actual test function object
struct test_t
{    
    template <typename T>
    T operator()(T &x)
    {
        return x + 42;
    }
} test_{};

#define SIMD_TEST_WITH_FLOAT
#define SIMD_TEST_WITH_DOUBLE


template <typename ExPolicy, typename T>
auto test(ExPolicy policy, std::size_t n)
{  
    std::vector<T> nums(n), nums2(n);
    for (auto &i : nums)
        i = rand() % 1024;
    
    auto t1 = std::chrono::high_resolution_clock::now();
        hpx::transform(policy,
            nums.begin(), nums.end(), nums2.begin(),
            test_); 
    auto t2 = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double> diff = t2 - t1;
    return diff.count();
}


// INCLUDE MAIN FUNCTION
#include "../../main.hpp"