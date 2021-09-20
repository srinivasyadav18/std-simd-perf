# std-simd-perf

### SIMD BENCHMARKS WITH HPX
This repository contains benchmarks for various artficial codes (std template library algorithms), real world kernels, mathematical functions (c++ cmath header).
 
 There are multiple branches each running on different architecure.
 
 ##### Some of the branches are : 
 
 [skylake_v3](https://github.com/srinivasyadav18/std-simd-perf/tree/skylake_v3): Intel(R) Xeon(R) Gold 6148 CPU @ 2.40GHz
 
 [amd_eypc_v3](https://github.com/srinivasyadav18/std-simd-perf/tree/amd_epyc_v3): AMD EPYC 7002 @ 3.3GHz 
 
 [cascade_lake_v3](https://github.com/srinivasyadav18/std-simd-perf/tree/cascade_lake_v3): Intel(R) Xeon(R) Silver 4210R CPU @ 2.40GHz
 
## Requirements:

* `GCC 11.1` for simd-backend.
* `boost` and `hwloc` for HPX installation.
* `Ninja and CTest` for building and running benchmarks.

## HOW TO INSTALL?? (on unix-like)

#### HPX INSTALLATION
* create a workdir `mkdir workdir`
* change current workind director to workdir `cd workdir`
* clone hpx repo `git clone --single-branch -b simd_algorithms_v3 https://github.com/srinivasyadav18/hpx.git`
* create build and install dirs for hpx installation `mkdir hpx_build hpx_install`
* Run cmake for `hpx` build. `cmake -GNinja -S ./hpx -B ./hpx_build -DCMAKE_INSTALL_PREFIX=./hpx_install -DCMAKE_BUILD_TYPE=Release -DHPX_WITH_CXX20=ON -DHPX_WITH_FETCH_ASIO=ON`
* Build and install hpx. `cmake --build ./hpx_build` `cmake --install ./hpx_build`

#### RUNNING BENCHMARKS
* clone `std-simd-perf` repo. `git clone --single-branch -b master_v6 https://github.com/srinivasyadav18/std-simd-perf.git`
* create a dir for benchmarks `mkdir benchmarks`
* build the project `HPX_DIR=./hpx_install cmake -GNinja -S ./std-simd-perf -B ./benchmarks -DCMAKE_BUILD_TYPE=Release -DSIMD_CORES=20 -DSIMD_END=23`
* build benchmarks `cmake --build ./benchmarks`
* Run benchmarks `cd benchmarks && ctest`

##### Note
* We can pass additional flags to cmake when building `std-simd-perf`
  * `-DSIMD_END=N` would run the benchmarks on array sizes from 2^5 to 2^N.
  * `-DSIMD_CORES=N` would run the bencmarks on N cores.
 

