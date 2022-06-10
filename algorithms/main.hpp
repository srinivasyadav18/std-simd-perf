#include <hpx/config.hpp>
#include <hpx/hpx.hpp>
#include <hpx/hpx_init.hpp>
#include <hpx/include/datapar.hpp>

#include <string>
#include <type_traits>
#include <vector>
#include <fstream>
#include <cmath>
#include <random>
#include <filesystem>

template <typename ExPolicy, typename T, typename Gen>
auto test3(ExPolicy policy, std::size_t iterations, std::size_t n, Gen gen)
{
    double avg_time = 0.0;
    for (std::size_t i = 0; i < iterations; i++)
    {
        avg_time += test<ExPolicy, T, Gen>(policy, n, Gen{});
    }
    avg_time /= (double) iterations;
    return avg_time;
}

template <typename T, typename Gen>
void test4(std::string type, 
        std::size_t start, std::size_t end,
        std::size_t iterations, Gen gen)
{
    std::string file_name = std::string("plots/") +
                            type + 
                            std::string(".csv");
    std::ofstream fout(file_name.c_str());

    using V = typename hpx::parallel::traits::vector_pack_type<T>::type;
    static constexpr size_t lane = hpx::parallel::traits::vector_pack_size<V>::value;

    auto& seq_pol = hpx::execution::seq;
    auto& simd_pol = hpx::execution::simd;
    auto& par_pol = hpx::execution::par;
    auto& par_simd_pol = hpx::execution::par_simd;

    fout << "n,lane,threads,seq,simd,par,par_simd\n";
    for (std::size_t i = start; i <= end; i++)
    {
        double offset = 0.0;
        for (std::size_t chunk = std::pow(2, i); chunk < std::pow(2, i+1); chunk += (chunk/4))
        {
            for (std::size_t iter = 0; iter < iterations; iter++)
            {
                fout << double(double(i) + offset)
                    << ","
                    << lane
                    << ","
                    << threads
                    << ","
                    << test<decltype(seq_pol), T, Gen>(
                        seq_pol, chunk, gen) 
                    << ","
                    << test<decltype(simd_pol), T, Gen>(
                        simd_pol, chunk, gen)
                    << ","
                    << test<decltype(par_pol), T, Gen>(
                        par_pol, chunk, gen) 
                    << ","
                    << test<decltype(par_simd_pol), T, Gen>(
                        par_simd_pol, chunk, gen) 
                    << "\n";
                fout.flush();
            }
            offset += 0.25;
        }
    }
    fout.close();
}

struct gen_int_t{
    std::mt19937 mersenne_engine {42};
    std::uniform_int_distribution<int> dist_int {1, 1024};
    auto operator()()
    {
        return dist_int(mersenne_engine);
    }
} gen_int{};

struct gen_float_t{
    std::mt19937 mersenne_engine {42};
    std::uniform_real_distribution<float> dist_float {1, 1024};
    auto operator()()
    {
        return dist_float(mersenne_engine);
   }
} gen_float{};

struct gen_double_t{
    std::mt19937 mersenne_engine {42};
    std::uniform_real_distribution<float> dist_double {1, 1024};
    auto operator()()
    {
        return dist_double(mersenne_engine);
    }
} gen_double{};

int hpx_main(hpx::program_options::variables_map& vm)
{
    std::filesystem::create_directory("plots");
    threads = hpx::get_os_thread_count();
    std::uint64_t iterations = vm["iterations"].as<std::uint64_t>();
    std::uint64_t start = vm["start"].as<std::uint64_t>();
    std::uint64_t end = vm["end"].as<std::uint64_t>();

    #if defined (SIMD_TEST_WITH_INT)
        test4<int, gen_int_t>("int", start, end, iterations, gen_int);
    #endif

    #if defined (SIMD_TEST_WITH_FLOAT)
        test4<float, gen_float_t>("float", start, end, iterations, gen_float);
    #endif

    #if defined (SIMD_TEST_WITH_DOUBLE)
        test4<double, gen_double_t>("double", start, end, iterations, gen_double);
    #endif

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
        ("end", po::value<std::uint64_t>()->default_value(22),
         "end of number of elements in 2^x")
    ;

    // Initialize and run HPX
    hpx::init_params init_args;
    init_args.desc_cmdline = desc_commandline;

    return hpx::init(argc, argv, init_args);    
}
