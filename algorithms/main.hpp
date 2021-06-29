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

    auto& seq_pol = hpx::execution::seq;
    auto& simd_pol = hpx::execution::simd;
    auto& par_pol = hpx::execution::par;
    auto& simdpar_pol = hpx::execution::simdpar;

    std::size_t buffer = 0;
    fout << "n,lane,threads,seq,simd,par,simdpar\n";
    for (std::size_t i = start; i <= end; i++)
    {
        fout << i 
            << ","
            << lane
            << ","
            << threads
            << ","
            << test<decltype(seq_pol), T>(
                seq_pol, iterations, std::pow(2, i)) 
            << ","
            << test<decltype(simd_pol), T>(
                simd_pol, iterations, std::pow(2, i))
            << ","
            << test<decltype(par_pol), T>(
                par_pol, iterations * 2, std::pow(2, i)) 
            << ","
            << test<decltype(simdpar_pol), T>(
                simdpar_pol, iterations * 2, std::pow(2, i)) 
            << "\n";
        buffer++;
        if (buffer % 5 == 0) 
        {
            buffer = 0;
            iterations /= 2;
        }
        fout.flush();
    }
    fout.close();
}

int hpx_main(hpx::program_options::variables_map& vm)
{
    system("rm -rf plots && mkdir -p plots");
    threads = hpx::get_os_thread_count();
    std::uint64_t const iterations = vm["iterations"].as<std::uint64_t>();
    std::uint64_t const start = vm["start"].as<std::uint64_t>();
    std::uint64_t const end = vm["end"].as<std::uint64_t>();

    #if defined (SIMD_TEST_WITH_INT)
        test<int>("int", start, end, iterations);
    #endif


    #if defined (SIMD_TEST_WITH_FLOAT)
        test<float>("float", start, end, iterations);
    #endif

    #if defined (SIMD_TEST_WITH_DOUBLE)
        test<double>("double", start, end, iterations);
    #endif

    return hpx::finalize();
}

///////////////////////////////////////////////////////////////////////////////
int main(int argc, char *argv[]) 
{ 
    namespace po = hpx::program_options;

    po::options_description desc_commandline;
    desc_commandline.add_options()
        ("iterations", po::value<std::uint64_t>()->default_value(50),
         "number of repititions")
        ("start", po::value<std::uint64_t>()->default_value(5),
         "start of number of elements in 2^x")
        ("end", po::value<std::uint64_t>()->default_value(23),
         "end of number of elements in 2^x")
    ;

    // Initialize and run HPX
    hpx::init_params init_args;
    init_args.desc_cmdline = desc_commandline;

    return hpx::init(argc, argv, init_args);    
}
