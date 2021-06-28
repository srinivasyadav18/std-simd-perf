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

template <typename ExPolicy, typename T>
auto test(ExPolicy policy, std::size_t n)
{  
    std::vector<T> nums(n);
    for (auto &i : nums)
        i = rand() % 1024;
    
    auto t1 = std::chrono::high_resolution_clock::now();
        int count_ = hpx::count_if(policy,
                        nums.begin(), nums.end(),
                        [](auto const& x) { return x < 42; }
                        ); 
    auto t2 = std::chrono::high_resolution_clock::now();
    
    std::cout << count_ << "\n";

    std::chrono::duration<double> diff = t2 - t1;
    return diff.count();
}


// INCLUDE MAIN FUNCTION
#include "../../main.hpp"