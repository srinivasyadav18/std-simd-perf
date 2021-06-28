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

int threads;


// INCLUDE COMPUTE KERNEL
//----------------------------------
#include "../kernel.hpp"
//----------------------------------


template <typename ExPolicy, typename T>
auto test(ExPolicy policy, std::size_t n)
{  
    using allocator_type = hpx::compute::host::block_allocator<T>;
    using executor_type = hpx::compute::host::block_executor<>;

    auto numa_domains = hpx::compute::host::numa_domains();
    allocator_type alloc(numa_domains);
    executor_type executor(numa_domains);

    hpx::compute::vector<T, allocator_type> nums(n, 0.0, alloc),
                                            nums2(n, 0.0, alloc),
                                            nums3(n, 0.0, alloc);
    for (auto &i : nums)
        i = rand() % 1024;
    
    for (auto &i : nums2)
        i = rand() % 1024;

    auto t1 = std::chrono::high_resolution_clock::now();
        if constexpr (hpx::is_parallel_execution_policy_v<ExPolicy>)
        {
            hpx::transform(policy.on(executor), nums.begin(), nums.end(),
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