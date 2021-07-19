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

std::size_t threads;

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

    T val = 42;
    hpx::compute::vector<T, allocator_type> nums(n, 0.0, alloc);
    if constexpr (hpx::is_parallel_execution_policy_v<ExPolicy>){
        hpx::fill(hpx::execution::par.on(executor), nums.begin(), nums.end(), val);
    }
    else
    {
        hpx::fill(hpx::execution::seq, nums.begin(), nums.end(), val);
    }
    
    int ans = 0;
    auto t1 = std::chrono::high_resolution_clock::now();
        if constexpr (hpx::is_parallel_execution_policy_v<ExPolicy>){
            ans = hpx::all_of(policy.on(executor), 
                                nums.begin(), nums.end(), 
                                [val](auto const& x) { return x != val; });
        }
        else
        {
            ans = hpx::all_of(policy, 
                                nums.begin(), nums.end(), 
                                [val](auto const& x) { return x != val; });
        }
    auto t2 = std::chrono::high_resolution_clock::now();

    std::cout << ans;
    std::chrono::duration<double> diff = t2 - t1;
    return diff.count();
}


// INCLUDE MAIN FUNCTION
#include "../../main.hpp"