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
#include <thread>

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

template <typename ExPolicy>
auto mandel(ExPolicy policy, const std::string& fname,
            float x_min, float x_max, 
            float y_min, float y_max,
            unsigned int width, unsigned int height,
            float iterations)
{
    float dx = (x_max - x_min) / width;
    float dy = (y_max - y_min) / height;
    std::size_t n = width * height;

    std::vector<float> image(n, static_cast<float>(0));

    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            image[width*i + j] = j;
        }
    }
    auto rng = boost::irange(0, (int)height);
    auto t1 = std::chrono::high_resolution_clock::now();
        std::for_each(policy, rng.begin(), rng.end(), [=, &image](int i)
        {
            auto width_begin = image.begin() + width*i;
            auto width_end = width_begin + width;
            std::for_each(std::execution::unseq, 
                width_begin, width_end, 
                [=, &image](auto &j)
            {
                const float x = x_min + (j) * dx;
                const float y = y_min + (i) * dy;
                const float four(4), two(2), iters(iterations), rgb(255);

                float count = 0;
                bool msk(0);

                float zr = x;
                float zi = y;
                for (float k = 0; k < iterations; k++)
                {
                    float r2 = zr * zr;
                    float i2 = zi * zi;
                    if ((r2 + i2) > four)
                    {
                        count = k;
                        break;
                    }
                    count = k;
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

int main()
{
    system("rm -rf plots && mkdir plots");
    std::string file_name = std::string("plots/float.csv");
    std::ofstream fout(file_name.c_str());

    threads = std::thread::hardware_concurrency();
    std::cout << "Threads : " << threads << std::endl;
    std::size_t lane = std::experimental::native_simd<float>::size();

    unsigned int width = 4096;
    unsigned int height = 4096;
    std::size_t n = width * height;

    float x_min = -2.0;
    float x_max = 1.0;
    float y_min = -1.0;
    float y_max = 1.0;

    double avg_unseq = 0.0;
    double avg_par_unseq = 0.0;
    double iters = 10;
    for (int i = 0; i < iters; i++)
    {
        auto t1 = mandel(std::execution::seq, "unseq", x_min, x_max, y_min, y_max, width, height, 2000);
        auto t2 = mandel(std::execution::par, "par_unseq", x_min, x_max, y_min, y_max, width, height, 2000);
        avg_unseq += t1;
        avg_par_unseq += t2;
    }

    avg_unseq /= iters;
    avg_par_unseq /= iters;

    std::cout << "unseq time : " << avg_unseq << std::endl;
    std::cout << "par_seq time : " << avg_par_unseq << std::endl;
    fout << "n,lane,threads,unseq,par_unseq\n";
    fout << n << "," 
        << lane << "," 
        << threads << "," 
        << avg_unseq << ","
        << avg_par_unseq << "\n";
    fout.close();
    return 0;
}