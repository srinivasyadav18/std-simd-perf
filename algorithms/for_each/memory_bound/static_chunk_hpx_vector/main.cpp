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
    using allocator_type = hpx::compute::host::block_allocator<T>;
    using executor_type = hpx::compute::host::block_executor<>;

    auto numa_domains = hpx::compute::host::numa_domains();
    allocator_type alloc(numa_domains);
    executor_type executor(numa_domains);

    hpx::compute::vector<T, allocator_type> a(n, 0.0, alloc), b(n, 0.0, alloc);
    if constexpr (hpx::is_parallel_execution_policy_v<ExPolicy>){
        hpx::generate(hpx::execution::par.on(executor), a.begin(), a.end(), gen);
    }
    else
    {
        hpx::generate(hpx::execution::seq, a.begin(), a.end(), gen);
    }
    if constexpr (hpx::is_parallel_execution_policy_v<ExPolicy>){
        hpx::generate(hpx::execution::par.on(executor), b.begin(), b.end(), gen);
    }
    else
    {
        hpx::generate(hpx::execution::seq, b.begin(), b.end(), gen);
    }
    
    auto begin_ = hpx::util::make_zip_iterator(
        std::begin(a), std::begin(b));
    auto end_ = hpx::util::make_zip_iterator(
        std::end(a), std::end(b));
    
    // STATIC CHUNKING
    hpx::execution::static_chunk_size cs(((int)n)/threads);

    auto t1 = std::chrono::high_resolution_clock::now();
        if constexpr (hpx::is_parallel_execution_policy_v<ExPolicy>)
            hpx::for_each(policy.on(executor).with(cs), begin_, end_, test_); 
        else
            hpx::for_each(policy, begin_, end_, test_);
    auto t2 = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double> diff = t2 - t1;
    return diff.count();
}


// INCLUDE MAIN FUNCTION
#include "../../../main.hpp"