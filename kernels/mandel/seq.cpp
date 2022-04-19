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
#include <chrono>
#include <execution>

std::size_t threads;

auto mandel(float x_min, float x_max, 
            float y_min, float y_max,
            unsigned int width, unsigned int height,
            float iterations){
    using executor_type = hpx::compute::host::block_executor<>;
    using allocator_type = hpx::compute::host::block_allocator<float>;

    auto numa_domains = hpx::compute::host::numa_domains();
    numa_domains.resize(1);

    allocator_type alloc(numa_domains);
    executor_type executor(numa_domains);

    float dx = (x_max - x_min) / width;
    float dy = (y_max - y_min) / height;
    std::size_t n = width * height;
    hpx::compute::vector<float, allocator_type> image(n, static_cast<float>(0), alloc);

    hpx::experimental::for_loop(hpx::execution::seq,
        0, height, [&image, width, height](int i)
    {
        for (int j = 0; j < width; j++)
        {
            image[width*i + j] = j;
        }
    });

    auto t1 = std::chrono::high_resolution_clock::now();
        hpx::experimental::for_loop(hpx::execution::seq, 0, height, [=, &image](int i)
        {
            auto width_begin = image.begin() + width*i;
            hpx::for_each(hpx::execution::seq, 
                width_begin, width_begin + width, [=, &image](auto &j)
            {
                using namespace hpx::parallel::traits;
                using Vector = std::decay_t<decltype(j)>;
                using Mask = vector_pack_mask_type<Vector>::type;
                
                const Vector x = x_min + Vector(j) * dx;
                const Vector y = y_min + Vector(i) * dy;
                const Vector four(4), two(2), iters(iterations), rgb(255);

                Vector count (0);
                Mask msk(0);

                Vector zr = x;
                Vector zi = y;
                for (float k = 0; k < iterations; k++)
                {
                    Vector r2 = zr * zr;
                    Vector i2 = zi * zi;
                    Mask curr_msk = (r2 + i2) > four;

                    if (hpx::parallel::traits::all_of(curr_msk))
                    {
                        mask_assign(msk ^ curr_msk, count, Vector(k));
                        break;
                    }
                    mask_assign(msk ^ curr_msk, count, Vector(k));
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
    return diff.count();
}

int hpx_main()
{
    std::string file_name = std::string("seq.csv");
    std::ofstream fout(file_name.c_str());

    threads = hpx::get_os_thread_count();
    using namespace hpx::parallel::traits;
    std::size_t lane = vector_pack_size<vector_pack_type<float>::type>::value;
    std::cout << "Threads : " << hpx::get_os_thread_count() << std::endl;
    std::cout << "Vector Pack Size : " << lane << std::endl;

    unsigned int width = 4096;
    unsigned int height = 4096;
    std::size_t n = width * height;

    float x_min = -2.0;
    float x_max = 1.0;
    float y_min = -1.0;
    float y_max = 1.0;

    double avg_time = 0.0;
    double iters = 3;
    for (int i = 0; i < iters; i++)
    {
        auto t1 = mandel(x_min, x_max, y_min, y_max, width, height, 2000);
        avg_time += t1;
    }

    avg_time /= iters;

    fout << "n,lane,threads,avg_time\n";
    fout << n << "," 
        << lane << "," 
        << threads << "," 
        << avg_time <<"\n";
    fout.close();
    return hpx::finalize();
}

int main(int argc, char* argv[])
{
    return hpx::init(argc, argv);
}