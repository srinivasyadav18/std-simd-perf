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

// INCLUDE COMPUTE KERNEL
//----------------------------------
#include "../kernel.hpp"
//----------------------------------


template <typename ExPolicy, typename T>
auto test(ExPolicy policy, std::size_t n)
{   
    std::vector<T> a(n), b(n);
    for (auto &i : a)
        i = rand() % 1024;
    for (auto &i : b)
        i = rand() % 1024;
    
    
    auto begin_ = hpx::util::make_zip_iterator(
        std::begin(a), std::begin(b));
    auto end_ = hpx::util::make_zip_iterator(
        std::end(a), std::end(b));
    
    auto t1 = std::chrono::high_resolution_clock::now();
        hpx::for_each(policy, begin_, end_, test_); 
    auto t2 = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double> diff = t2 - t1;
    return diff.count();
}


// INCLUDE MAIN FUNCTION
#include "../../../main.hpp"