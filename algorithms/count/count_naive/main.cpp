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

template <typename Iter>
bool is_aligned(Iter const& it)
{
    typedef typename std::iterator_traits<Iter>::value_type value_type;
    return (reinterpret_cast<std::uintptr_t>(std::addressof(*it)) &
                (std::experimental::memory_alignment_v<std::experimental::native_simd<value_type>> - 1)) == 0;
}

template <typename Iter, typename T>
std::size_t count_algo(Iter first, Iter last, T const val)
{
    std::size_t ret = 0;

    while (!is_aligned(first) && first != last)
    {
        if (*first++ == val)
            ++ret;
    }
    
    typedef typename std::iterator_traits<Iter>::value_type value_type;
    std::experimental::native_simd<value_type> tmp;
    std::size_t size = tmp.size();
    Iter const lastV = last - (size + 1);

    while (first < lastV)
    {
        tmp.copy_from(std::addressof(*first), std::experimental::vector_aligned);
        ret += std::experimental::popcount(tmp == val);
        std::advance(first, size);
    }

    while (first != last)
    {
        if (*first++ == val)
            ++ret;
    }
    return ret;
}

template <typename ExPolicy, typename T, typename Gen>
auto test(ExPolicy policy, std::size_t n, Gen gen)
{ 
    using executor_type = hpx::compute::host::block_executor<>;
    using allocator_type = hpx::compute::host::block_allocator<T>;

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

    auto t1 = std::chrono::high_resolution_clock::now();
        int ret = 0;
        if constexpr(!hpx::is_vectorpack_execution_policy_v<ExPolicy>)
            ret = hpx::count(policy, nums.begin(), nums.end(), 42);
        else ret = count_algo(nums.begin(), nums.end(),  42);
    std::cout << ret << "\n";
    auto t2 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> diff = t2 - t1;
    return diff.count();
}


// INCLUDE MAIN FUNCTION
#include "../../main.hpp"
