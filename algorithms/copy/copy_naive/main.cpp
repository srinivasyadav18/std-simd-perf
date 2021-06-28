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

int threads;

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

template <typename Iter>
void copy_algo(Iter first, Iter last, Iter dest)
{
    while (!is_aligned(first) && !is_aligned(dest) && first != last)
    {
        *dest++ = *first++;
    }
    
    typedef typename std::iterator_traits<Iter>::value_type value_type;
    std::experimental::native_simd<value_type> tmp;
    std::size_t size = tmp.size();
    Iter const lastV = last - (size + 1);

    while (first < lastV)
    {
        tmp.copy_from(std::addressof(*first), std::experimental::vector_aligned);
        tmp.copy_to(std::addressof(*dest), std::experimental::vector_aligned);
        std::advance(first, size);
        std::advance(dest, size);
    }

    while (first != last)
    {
        *dest++ = *first++;
    }
}

template <typename ExPolicy, typename T>
auto test(ExPolicy policy, std::size_t n)
{  
    std::vector<T> nums(n), nums2(n);
    for (auto &i : nums)
        i = rand() % 1024;

    auto t1 = std::chrono::high_resolution_clock::now();

        if constexpr(!hpx::is_vectorpack_execution_policy_v<ExPolicy>)
            hpx::copy(policy, nums.begin(), nums.end(), nums2.begin());
        else copy_algo(nums.begin(), nums.end(), nums2.begin());

    auto t2 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> diff = t2 - t1;
    return diff.count();
}


// INCLUDE MAIN FUNCTION
#include "../../main.hpp"