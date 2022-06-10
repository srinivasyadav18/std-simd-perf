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

#include <eve/memory/aligned_allocator.hpp>

std::size_t threads;

// INCLUDE COMPUTE KERNEL
//----------------------------------
#include "../kernel.hpp"
//----------------------------------


template <typename ExPolicy, typename T, typename Gen>
auto test(ExPolicy policy, std::size_t n, Gen gen)
{  
    using allocator_type = eve::aligned_allocator<T>;

    std::vector<T, allocator_type> nums(n, allocator_type{});

    if constexpr (hpx::is_parallel_execution_policy_v<ExPolicy>){
        hpx::generate(hpx::execution::par, nums.begin(), nums.end(), gen);
    }
    else
    {
        hpx::generate(hpx::execution::seq, nums.begin(), nums.end(), gen);
    }
    
    hpx::execution::static_chunk_size cs(n/threads);;
    auto t1 = std::chrono::high_resolution_clock::now();
        if constexpr(hpx::is_parallel_execution_policy_v<ExPolicy>)
            hpx::for_each(policy.with(cs), nums.begin(), nums.end(), test_);
        else
            hpx::for_each(policy, nums.begin(), nums.end(), test_); 
    auto t2 = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double> diff = t2 - t1;
    return diff.count();
}


// INCLUDE MAIN FUNCTION
#include "../../../main.hpp"