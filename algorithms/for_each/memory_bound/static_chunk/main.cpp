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

std::size_t threads;

// INCLUDE COMPUTE KERNEL
//----------------------------------
#include "../kernel.hpp"
//----------------------------------


template <typename ExPolicy, typename T, typename Gen>
auto test(ExPolicy policy, std::size_t n, Gen gen)
{  
    std::vector<T> a(n), b(n);
    if constexpr (hpx::is_parallel_execution_policy_v<ExPolicy>){
        hpx::generate(hpx::execution::par, a.begin(), a.end(), gen);
    }
    else
    {
        hpx::generate(hpx::execution::seq, a.begin(), a.end(), gen);
    }
    if constexpr (hpx::is_parallel_execution_policy_v<ExPolicy>){
        hpx::generate(hpx::execution::par, b.begin(), b.end(), gen);
    }
    else
    {
        hpx::generate(hpx::execution::seq, b.begin(), b.end(), gen);
    }
    
    
    auto begin_ = hpx::util::make_zip_iterator(
        std::begin(a), std::begin(b));
    auto end_ = hpx::util::make_zip_iterator(
        std::end(a), std::end(b));
    
    hpx::execution::static_chunk_size cs(n/threads);;
    auto t1 = std::chrono::high_resolution_clock::now();
        if constexpr(hpx::is_parallel_execution_policy_v<ExPolicy>)
            hpx::for_each(policy.with(cs), begin_, end_, test_);
        else
            hpx::for_each(policy, begin_, end_, test_); 
    auto t2 = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double> diff = t2 - t1;
    return diff.count();
}


// INCLUDE MAIN FUNCTION
#include "../../../main.hpp"