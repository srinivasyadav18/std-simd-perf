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
#include <random>

#include <eve/eve.hpp>

std::size_t threads;

#define SIMD_TEST_WITH_INT
#define SIMD_TEST_WITH_FLOAT
#define SIMD_TEST_WITH_DOUBLE

struct foo
{
    template <typename T>
    T operator()()
    {
        return T(10);
    }
};

template <typename ExPolicy, typename T, typename Gen>
auto test(ExPolicy policy, std::size_t n, Gen gen)
{  
    using executor_type = hpx::compute::host::block_executor<>;
    using allocator_type = hpx::compute::host::block_allocator<T>;

    auto numa_domains = hpx::compute::host::numa_domains();
    allocator_type alloc(numa_domains);
    executor_type executor(numa_domains);

    hpx::compute::vector<T, allocator_type> nums(n, 0.0, alloc);
    
    foo obj;
    auto t1 = std::chrono::high_resolution_clock::now();
        if constexpr (hpx::is_parallel_execution_policy_v<ExPolicy>
            && hpx::is_vectorpack_execution_policy_v<ExPolicy>){
            hpx::generate(policy.on(executor), 
                    nums.begin(), nums.end(), obj);
        }
        else if constexpr (hpx::is_parallel_execution_policy_v<ExPolicy>
            && !hpx::is_vectorpack_execution_policy_v<ExPolicy>){
            hpx::generate(policy.on(executor), 
                    nums.begin(), nums.end(), [](){return 10;});
        }
        else if constexpr (!hpx::is_parallel_execution_policy_v<ExPolicy>
            && hpx::is_vectorpack_execution_policy_v<ExPolicy>){
            hpx::generate(policy.on(executor), 
                    nums.begin(), nums.end(), obj);
        }
        else
        {
            hpx::generate(policy, 
                    nums.begin(), nums.end(), [](){return 10;});
        }
    auto t2 = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double> diff = t2 - t1;
    return diff.count();
}


// INCLUDE MAIN FUNCTION
#include "../../main.hpp"