import os
import bitmath
import cpuinfo
import platform
import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
import matplotlib.patches as patches

def parse_cache(cache):
    ret = {}
    if isinstance(cache, int):
        ret['bytes'] = cache
        ret['str'] = str(bitmath.best_prefix(cache))
    else :
        ret['bytes'] = int(bitmath.parse_string(cache).bytes)
        ret['str'] = cache
    return ret

util_dict = {}
cpu_dict = cpuinfo.get_cpu_info()

if 'brand_raw' in cpu_dict : 
    util_dict['cpu_name'] = cpu_dict['brand_raw']
else : cpu_dict['cpu_name'] = ""

if 'l1_data_cache_size' in cpu_dict:
    util_dict['l1'] = parse_cache(cpu_dict['l1_data_cache_size'])

if 'l2_cache_size' in cpu_dict:
    util_dict['l2'] = parse_cache(cpu_dict['l2_cache_size'])

if 'l3_cache_size' in cpu_dict:
    util_dict['l3'] = parse_cache(cpu_dict['l3_cache_size'])

if 'count' in cpu_dict:
    util_dict['count'] = cpu_dict['count']

l1_seq = f'$CC -O3 -ffreestanding -fopenmp -mcmodel=medium -DSTREAM_ARRAY_SIZE={util_dict["l1"]["bytes"]//8} -DNTIMES=200 stream.c -o l1_seq\n'
l1_par = f'$CC -O3 -ffreestanding -fopenmp -mcmodel=medium -DSTREAM_ARRAY_SIZE={(util_dict["l1"]["bytes"]//8)*util_dict["count"]} -DNTIMES=150 stream.c -o l1_par\n'

l2_seq = f'$CC -O3 -ffreestanding -fopenmp -mcmodel=medium -DSTREAM_ARRAY_SIZE={util_dict["l2"]["bytes"]//8} -DNTIMES=100 stream.c -o l2_seq\n'
l2_par = f'$CC -O3 -ffreestanding -fopenmp -mcmodel=medium -DSTREAM_ARRAY_SIZE={(util_dict["l2"]["bytes"]//8)*util_dict["count"]} -DNTIMES=50 stream.c -o l2_par\n'

dram_seq = f'$CC -O3 -ffreestanding -fopenmp -mcmodel=medium -DSTREAM_ARRAY_SIZE={(util_dict["l3"]["bytes"]//8) * 4} -DNTIMES=25 stream.c -o dram_seq\n'
dram_par = f'$CC -O3 -ffreestanding -fopenmp -mcmodel=medium -DSTREAM_ARRAY_SIZE={(util_dict["l3"]["bytes"]//8)* 4 * util_dict["count"]} -DNTIMES=25 stream.c -o dram_par\n'

f = open('generate_bw.sh', 'w')

f.write(l1_seq)
f.write(l1_par)
f.write(l2_seq)
f.write(l2_par)
f.write(dram_seq)
f.write(dram_par)


pols = ['l1', 'l2', 'dram']
for pol in pols:
    f.write(f'hwloc-bind core:0.PU:0 ./{pol}_seq | grep Triad: > {pol}.txt\n')

for pol in pols:
    f.write(f'./{pol}_par | grep Triad: >> {pol}.txt\n')

for pol in pols:
    f.write(f'rm -rf {pol}_seq {pol}_par\n')

for pol in pols[:2]:
    f.write(f'export SIMD_{pol.upper()}_SIZE={util_dict[pol]["bytes"]}\n')

f.close()