#include <hpx/config.hpp>
#include <hpx/hpx.hpp>
#include <hpx/hpx_init.hpp>
#include <hpx/include/datapar.hpp>
#include <hpx/include/compute.hpp>

#include <string>
#include <type_traits>
#include <vector>
#include <fstream>
#include <cmath>
#include <experimental/simd>
#include <random>

int threads;

#define SIMD_TEST_WITH_INT
#define SIMD_TEST_WITH_FLOAT
#define SIMD_TEST_WITH_DOUBLE

template <typename ExPolicy, typename T, typename Gen>
auto test(ExPolicy policy, std::size_t n, Gen gen)
{  
    using executor_type = hpx::compute::host::block_executor<>;
    using allocator_type = hpx::compute::host::block_allocator<T>;

    auto numa_domains = hpx::compute::host::numa_domains();
    allocator_type alloc(numa_domains);
    executor_type executor(numa_domains);

    hpx::compute::vector<T, allocator_type> nums(n, 0.0, alloc), 
                                            nums2(n, 0.0, alloc);
    if constexpr (hpx::is_parallel_execution_policy_v<ExPolicy>){
        hpx::generate(hpx::execution::par.on(executor), nums.begin(), nums.end(), gen);
    }
    else
    {
        hpx::generate(hpx::execution::seq, nums.begin(), nums.end(), gen);
    }
    
    auto t1 = std::chrono::high_resolution_clock::now();
        if constexpr (hpx::is_parallel_execution_policy_v<ExPolicy>){
            hpx::copy_n(policy.on(executor), nums.begin(), nums.size(), nums2.begin());
        }
        else
        {
            hpx::copy_n(policy, nums.begin(), nums.size(), nums2.begin());
        }
    auto t2 = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double> diff = t2 - t1;
    return diff.count();
}


// INCLUDE MAIN FUNCTION
#include "../../main.hpp"