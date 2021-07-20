# std-simd-perf

### SIMD BENCHMARKS WITH HPX

## Requirements:

* `GCC 11.1` for simd-backend.
* `boost` and `hwloc` for HPX installation.
* `Ninja and CTest` for building and running benchmarks.

## Steps to follow to install and run the benchmarks in Unix-like

### HPX INSTALLATION
* create a workdir `mkdir workdir`
* change current workind director to workdir `cd workdir`
* clone hpx repo `git clone --single-branch -b simd_algorithms_v3 https://github.com/srinivasyadav18/hpx.git`
* create build and install dirs for hpx installation `mkdir hpx_build hpx_install`
* Run cmake for `hpx` build. `cmake -GNinja -S ./hpx -B ./hpx_build -DCMAKE_INSTALL_PREFIX=./hpx_install -DCMAKE_BUILD_TYPE=Release -DHPX_WITH_CXX20=ON -DHPX_WITH_FETCH_ASIO=ON`
* Build and install hpx. `cmake --build ./hpx_build` `cmake --install ./hpx_build`

### RUNNING BENCHMARKS
* clone `std-simd-perf` repo. `git clone --single-branch -b master_v4 https://github.com/srinivasyadav18/std-simd-perf.git`
* create a dir for benchmarks `mkdir benchmarks`
* build the project `HPX_DIR=./hpx_install cmake -GNinja -S ./std-simd-perf -B ./benchmarks -DCMAKE_BUILD_TYPE=Release -DSIMD_CORES=20 -DSIMD_END=23`
* build benchmarks `cmake --build ./benchmarks`
* Run benchmarks `cd benchmarks && ctest`

##### Note
* We can pass additional flags to cmake when building `std-simd-perf`
  * `-DSIMD_END=N` would run the benchmarks on array sizes from 2^5 to 2^N.
  * `-DSIMD_CORES=N` would run the bencmarks on N cores.
 

