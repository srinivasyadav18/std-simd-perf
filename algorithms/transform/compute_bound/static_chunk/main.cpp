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

// INCLUDE COMPUTE KERNEL
//----------------------------------
#include "../kernel.hpp"
//----------------------------------


template <typename ExPolicy, typename T>
auto test(ExPolicy policy, std::size_t n)
{  
    std::vector<T> nums(n), nums2(n), nums3(n);
    for (auto &i : nums) i = rand() % 1024;
    for (auto &i : nums2) i = rand() % 1024;

    hpx::execution::static_chunk_size cs(((int)n)/threads);
    auto t1 = std::chrono::high_resolution_clock::now();
        if constexpr(hpx::is_parallel_execution_policy_v<ExPolicy>)
           hpx::transform(policy.with(cs), nums.begin(), nums.end(),
                nums2.begin(), 
                nums3.begin(),
                test_);
        else
           hpx::transform(policy, nums.begin(), nums.end(),
                nums2.begin(), 
                nums3.begin(),
                test_);
    auto t2 = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double> diff = t2 - t1;
    return diff.count();
}


// INCLUDE MAIN FUNCTION
#include "../../../main.hpp"