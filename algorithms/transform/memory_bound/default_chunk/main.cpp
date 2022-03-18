#include <hpx/config.hpp>
#include <hpx/hpx.hpp>
#include <hpx/hpx_init.hpp>
#include <hpx/include/datapar.hpp>
#include <hpx/include/compute.hpp>

#include <string>
#include <type_traits>
#include <vector>
#include <fstream>

std::size_t threads;
#define SIMD_TEST_WITH_FLOAT
// #define SIMD_TEST_WITH_DOUBLE

struct test_t
{    
    template <typename T>
    T operator()(T &x, T &y)
    {
        return 5.0f * x + y;
    }
} test_{};

template <typename ExPolicy, typename T, typename Gen>
auto test(ExPolicy policy, std::size_t n, Gen gen)
{  
    std::vector<T> nums(n, 0.0), nums2(n, 0.0), nums3(n, 0.0);

    if constexpr (hpx::is_parallel_execution_policy_v<ExPolicy>){
        hpx::generate(hpx::execution::par, nums.begin(), nums.end(), gen);
        hpx::generate(hpx::execution::par, nums2.begin(), nums2.end(), gen);
    }
    else
    {
        hpx::generate(hpx::execution::seq, nums.begin(), nums.end(), gen);
        hpx::generate(hpx::execution::seq, nums2.begin(), nums2.end(), gen);
    }

    auto t1 = std::chrono::high_resolution_clock::now();
        if constexpr (hpx::is_parallel_execution_policy_v<ExPolicy>)
        {
            hpx::transform(policy, nums.begin(), nums.end(),
                nums2.begin(), 
                nums3.begin(),
                test_);
        }
        else
        {
            hpx::transform(policy, nums.begin(), nums.end(),
                nums2.begin(), 
                nums3.begin(),
                test_);
        }
    auto t2 = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double> diff = t2 - t1;
    return diff.count();
}


// INCLUDE MAIN FUNCTION
#include "../../../main.hpp"