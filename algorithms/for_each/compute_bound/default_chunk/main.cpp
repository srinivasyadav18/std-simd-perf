#include <hpx/config.hpp>
#include <hpx/hpx.hpp>
#include <hpx/hpx_init.hpp>
#include <hpx/include/datapar.hpp>

#include <string>
#include <type_traits>
#include <vector>
#include <fstream>

std::size_t threads;

// INCLUDE COMPUTE KERNEL
//----------------------------------
#include "../kernel.hpp"
//----------------------------------


template <typename ExPolicy, typename T, typename Gen>
auto test(ExPolicy policy, std::size_t n, Gen gen)
{
    std::vector<T> nums(n);

    if constexpr (hpx::is_parallel_execution_policy_v<ExPolicy>){
        hpx::generate(hpx::execution::par, nums.begin(), nums.end(), gen);
    }
    else
    {
        hpx::generate(hpx::execution::seq, nums.begin(), nums.end(), gen);
    }

    auto t1 = std::chrono::high_resolution_clock::now();
        hpx::for_each(policy, nums.begin(), nums.end(), test_); 
    auto t2 = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double> diff = t2 - t1;
    return diff.count();
}


// INCLUDE MAIN FUNCTION
#include "../../../main.hpp"