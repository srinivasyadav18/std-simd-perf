#include <hpx/config.hpp>
#include <hpx/hpx.hpp>
#include <hpx/hpx_init.hpp>
#include <hpx/include/datapar.hpp>
#include <hpx/modules/testing.hpp>
#include <hpx/modules/timing.hpp>
#include <hpx/parallel/util/loop.hpp>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <numeric>
#include <sstream>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>
#include <fstream>

#include <cmath>
#include <experimental/simd>

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



template <typename ExPolicy, typename T>
auto test(ExPolicy policy, std::size_t n)
{  
    std::vector<T> nums(n), nums2(n);
    for (auto &i : nums)
        i = rand() % 1024;

    auto t1 = std::chrono::high_resolution_clock::now();

        if constexpr(!hpx::is_vectorpack_execution_policy_v<ExPolicy>)
            hpx::count(policy, nums.begin(), nums.end(), 42);
        else count_algo(nums.begin(), nums.end(),  42);

    auto t2 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> diff = t2 - t1;
    return diff.count();
}

template <typename ExPolicy, typename T>
auto test(ExPolicy policy, std::size_t iterations, std::size_t n)
{
    double avg_time = 0.0;
    for (std::size_t i = 0; i < iterations; i++)
    {
        avg_time += test<ExPolicy, T>(policy, n);
    }
    avg_time /= (double) iterations;
    return avg_time;
}

template <typename T>
void test(std::string type, 
        std::size_t start, std::size_t end,
        std::size_t iterations)
{
    std::string file_name = std::string("plots/") +
                            type + 
                            std::string(".csv");
    std::ofstream fout(file_name.c_str());

    static constexpr size_t lane = std::experimental::native_simd<T>::size();
    size_t threads = hpx::get_os_thread_count();
    fout << "n,lane,threads,seq,simd,par,simdpar\n";
    for (std::size_t i = start; i <= end; i++)
    {
        fout << i 
            << ","
            << lane
            << ","
            << threads
            << ","
            << test<hpx::execution::sequenced_policy, T>(
            hpx::execution::seq, iterations, std::pow(2, i)) 
            << ","
            << test<hpx::execution::simd_policy, T>(
            hpx::execution::simd, iterations, std::pow(2, i))
            << ","
            << test<hpx::execution::parallel_policy, T>(
            hpx::execution::par, iterations, std::pow(2, i)) 
            << ","
            << test<hpx::execution::simdpar_policy, T>(
            hpx::execution::simdpar, iterations, std::pow(2, i)) 
            << "\n";
    }
    fout.close();
}

int hpx_main(hpx::program_options::variables_map& vm)
{
    system("rm -rf plots && mkdir -p plots");
    std::uint64_t const iterations = vm["iterations"].as<std::uint64_t>();
    std::uint64_t const start = vm["start"].as<std::uint64_t>();
    std::uint64_t const end = vm["end"].as<std::uint64_t>();

    test<int>("int", start, end, iterations);
    test<float>("float", start, end, iterations);
    test<double>("double", start, end, iterations);

    return hpx::finalize();
}

///////////////////////////////////////////////////////////////////////////////
int main(int argc, char *argv[]) 
{ 
    namespace po = hpx::program_options;

    po::options_description desc_commandline;
    desc_commandline.add_options()
        ("iterations", po::value<std::uint64_t>()->default_value(5),
         "number of repititions")
        ("start", po::value<std::uint64_t>()->default_value(5),
         "start of number of elements in 2^x")
        ("end", po::value<std::uint64_t>()->default_value(25),
         "end of number of elements in 2^x")
    ;

    // Initialize and run HPX
    hpx::init_params init_args;
    init_args.desc_cmdline = desc_commandline;

    return hpx::init(argc, argv, init_args);    
}