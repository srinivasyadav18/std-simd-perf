#include <hpx/config.hpp>
#include <hpx/hpx.hpp>
#include <hpx/hpx_init.hpp>
#include <hpx/include/datapar.hpp>
#include <hpx/include/compute.hpp>
#include <boost/range/irange.hpp>

#include <fstream>
#include <cstdio>
#include <cmath>
#include <iostream>
#include <vector>
#include <algorithm>
#include <numeric>
#include <utility>
#include <experimental/simd>
#include <chrono>
#include <execution>

std::size_t threads;

template <typename vector>
void write_ppm(const char* fname, vector &image,
            unsigned int width, unsigned int height)
{
    using namespace std;
    ofstream ofs(fname, ios_base::out | ios_base::binary);
    ofs << "P6" << endl << width << ' ' << height << endl << "255" << endl;
    for (unsigned int i = 0; i < width * height; i++)
    {
        char pix = image[i];
        ofs << (pix) << (pix) << (pix);
        // red, green, blue
    }
    ofs.close();
}

////////////////////////////////////////////////////////////////////
template <typename T, typename Enable=void>
struct get_mask_type
{
    using type = bool;
};

template <typename T>
struct get_mask_type<T, typename std::enable_if_t<std::experimental::is_simd_v<T>>>
{
    using type = std::experimental::simd_mask<typename T::value_type,
                                            typename T::abi_type>;
};

////////////////////////////////////////////////////////////////////
template <typename ExPolicy, typename Enable=void>
struct get_base_policy
{
    static constexpr auto value = hpx::execution::seq;
};

template <typename ExPolicy>
struct get_base_policy<ExPolicy, 
            typename std::enable_if_t<
                hpx::is_parallel_execution_policy<ExPolicy>::value>>
{
    static constexpr auto value = hpx::execution::par;
};

template <>
struct get_base_policy<decltype(std::execution::unseq)>
{
    static constexpr auto value = std::execution::seq;
};

template <>
struct get_base_policy<decltype(std::execution::par_unseq)>
{
    static constexpr auto value = std::execution::par_unseq;
};

////////////////////////////////////////////////////////////////////
template <typename ExPolicy, typename Enable=void>
struct get_simd_policy
{
    static constexpr auto value = hpx::execution::seq;
};

template <typename ExPolicy>
struct get_simd_policy<ExPolicy, 
            typename std::enable_if_t<
                hpx::is_vectorpack_execution_policy<ExPolicy>::value>>
{
    static constexpr auto value = hpx::execution::simd;
};

template <>
struct get_simd_policy<decltype(std::execution::unseq)>
{
    static constexpr auto value = std::execution::unseq;
};

template <>
struct get_simd_policy<decltype(std::execution::par_unseq)>
{
    static constexpr auto value = std::execution::seq;
};

////////////////////////////////////////////////////////////////////
inline constexpr bool all_of_simd(bool msk)
{
    return msk;
}

template <typename Mask>
inline constexpr std::enable_if_t<std::experimental::is_simd_mask_v<Mask>, bool>
    all_of_simd(Mask const& msk)
{
    return std::experimental::all_of(msk);
}

////////////////////////////////////////////////////////////////////
template <typename T>
inline constexpr void mask_assign(bool msk, T &v, T val)
{
    if (msk) v = val;
}

template <typename Mask, typename Vector, typename T>
inline constexpr std::enable_if_t<std::experimental::is_simd_mask_v<Mask>>
mask_assign(const Mask& msk, Vector& v, T val)
{
    where(msk, v) = val;
}

template <typename ExPolicy>
auto mandel(ExPolicy policy, const std::string& fname,
            float x_min, float x_max, 
            float y_min, float y_max,
            unsigned int width, unsigned int height,
            float iterations)
{
    auto policy1 = get_base_policy<ExPolicy>::value;
    auto policy2 = get_simd_policy<ExPolicy>::value;

    using Policy1 = std::decay_t<decltype(policy1)>;
    using Policy2 = std::decay_t<decltype(policy2)>;

    using executor_type = hpx::compute::host::block_executor<>;
    using allocator_type = hpx::compute::host::block_allocator<float>;

    auto numa_domains = hpx::compute::host::numa_domains();
    allocator_type alloc(numa_domains);
    executor_type executor(numa_domains);

    float dx = (x_max - x_min) / width;
    float dy = (y_max - y_min) / height;
    std::size_t n = width * height;
    hpx::compute::vector<float, allocator_type> image(n, static_cast<float>(0), alloc);

    hpx::for_loop(hpx::execution::par.on(executor), 0, height, [&image, width, height](int i)
    {
        for (int j = 0; j < width; j++)
        {
            image[width*i + j] = j;
        }
    });

    auto t1 = std::chrono::high_resolution_clock::now();
        hpx::for_loop(policy1, 0, height, [=, &image](int i)
        {
            auto width_begin = image.begin() + width*i;
            hpx::for_each(policy2, 
                width_begin, width_begin + width, [=, &image](auto &j)
            {
                using Vector = std::decay_t<decltype(j)>;
                using Mask = get_mask_type<Vector>::type;
                
                const Vector x = x_min + (j) * dx;
                const Vector y = y_min + (i) * dy;
                const Vector four(4), two(2), iters(iterations), rgb(255);

                Vector count = 0;
                Mask msk(0);

                Vector zr = x;
                Vector zi = y;
                for (float k = 0; k < iterations; k++)
                {
                    Vector r2 = zr * zr;
                    Vector i2 = zi * zi;
                    Mask curr_msk = (r2 + i2) > four;

                    if (all_of_simd(curr_msk))
                    {
                        mask_assign(msk ^ curr_msk, count, k);
                        break;
                    }
                    mask_assign(msk ^ curr_msk, count, k);
                    msk = curr_msk;
                    zi = two * zr * zi + y;
                    zr = r2 - i2 + x;
                }
                count /= iters;
                count *= rgb;
                j = count;
            });
        });
    auto t2 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> diff = t2 - t1;

    std::string fname_ = fname;
    static int i = 0;
    i++;
    fname_ += std::string("_") + std::to_string(i) + std::string(".ppm"); 
    write_ppm(fname_.c_str(), image, width, height);
    return diff.count();
}

int hpx_main()
{
    std::filesystem::create_directory("plots");
    std::string file_name = std::string("plots/float.csv");
    std::ofstream fout(file_name.c_str());

    threads = hpx::get_os_thread_count();
    std::cout << "Threads : " << threads << std::endl;
    std::size_t lane = std::experimental::native_simd<float>::size();

    unsigned int width = 4096;
    unsigned int height = 4096;
    std::size_t n = width * height;

    float x_min = -2.0;
    float x_max = 1.0;
    float y_min = -1.0;
    float y_max = 1.0;

    double avg_seq = 0.0, avg_simd = 0.0, avg_par = 0.0, avg_simdpar = 0.0, avg_unseq = 0.0, avg_par_unseq = 0.0;
    double iters = 2;
    for (int i = 0; i < iters; i++)
    {
        auto t1 = mandel(hpx::execution::seq, "sequential", x_min, x_max, y_min, y_max, width, height, 2000);
        auto t2 = mandel(hpx::execution::simd, "simd", x_min, x_max, y_min, y_max, width, height, 2000);
        auto t3 = mandel(hpx::execution::par, "par", x_min, x_max, y_min, y_max, width, height, 2000);
        auto t4 = mandel(hpx::execution::simdpar, "par_simd", x_min, x_max, y_min, y_max, width, height, 2000);
        auto t5 = mandel(std::execution::unseq, "unseq", x_min, x_max, y_min, y_max, width, height, 2000);
        auto t6 = mandel(std::execution::par_unseq, "par_unseq", x_min, x_max, y_min, y_max, width, height, 2000);
        avg_seq += t1;
        avg_simd += t2;
        avg_par += t3;
        avg_simdpar += t4;
        avg_unseq += t5;
        avg_par_unseq += t6;
    }

    avg_seq /= iters;
    avg_simd /= iters;
    avg_par /= iters;
    avg_simdpar /= iters;
    avg_unseq /= iters;
    avg_par_unseq /= iters;

    std::cout << "seq time : " << avg_seq << std::endl;
    std::cout << "simd time : " << avg_simd << std::endl;
    std::cout << "unseq time : " << avg_unseq << std::endl;
    std::cout << "par time : " << avg_par << std::endl;
    std::cout << "simdpar time : " << avg_simdpar << std::endl;
    std::cout << "par_unseq time : " << avg_par_unseq << std::endl;
    std::cout << "==============================\n";
    std::cout << "Threads : " << hpx::get_os_thread_count() << std::endl;
    std::cout << "Vector Pack Size : " << lane << std::endl;
    std::cout << "avg SIMD Speed Up : " << avg_seq/avg_simd << std::endl;
    std::cout << "avg PAR Speed Up : " << avg_seq/avg_par << std::endl;
    std::cout << "avg SIMDPAR Speed Up : " << avg_seq/avg_simdpar << std::endl;
    std::cout << "avg UNSEQ Speed Up : " << avg_seq/avg_unseq << std::endl;
    std::cout << "avg PAR_UNSEQ Speed Up : " << avg_seq/avg_par_unseq << std::endl;

    fout << "n,lane,threads,seq,simd,par,simdpar,unseq,par_unseq\n";
    fout << n << "," 
        << lane << "," 
        << threads << "," 
        << avg_seq << ","
        << avg_simd << ","
        << avg_par << ","
        << avg_simdpar << ","
        << avg_unseq << ","
        << avg_par_unseq << "\n";
    fout.close();
    return hpx::finalize();
}

int main(int argc, char* argv[])
{
    return hpx::init(argc, argv);
}