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


template <typename ExPolicy, typename T, typename Gen>
auto test(ExPolicy policy, std::size_t n, Gen gen)
{  
    // HPX COMPUTE VECTOR
    using allocator_type = hpx::compute::host::block_allocator<T>;
    using executor_type = hpx::compute::host::block_executor<>;

    auto numa_domains = hpx::compute::host::numa_domains();
    allocator_type alloc(numa_domains);
    executor_type executor(numa_domains);

    hpx::compute::vector<T, allocator_type> nums(n, 0.0, alloc);
    if constexpr (hpx::is_parallel_execution_policy_v<ExPolicy>){
        hpx::generate(hpx::execution::par.on(executor), nums.begin(), nums.end(), gen);
    }
    else
    {
        hpx::generate(hpx::execution::seq, nums.begin(), nums.end(), gen);
    }

    // STATIC CHUNK SIZE
    hpx::execution::static_chunk_size cs(((int)n)/threads);
    
    auto t1 = std::chrono::high_resolution_clock::now();
        if constexpr (hpx::is_parallel_execution_policy_v<ExPolicy>)
            hpx::for_each(policy.on(executor).with(cs), nums.begin(), nums.end(), test_);
        else
            hpx::for_each(policy, nums.begin(), nums.end(), test_);
    auto t2 = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double> diff = t2 - t1;
    return diff.count();
}


// INCLUDE MAIN FUNCTION
#include "../../../main.hpp"