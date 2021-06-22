import os
import cpuinfo
from sys import base_exec_prefix
from matplotlib import pyplot as plt
from matplotlib import ticker
from fractions import Fraction
import numpy as np
import math
import bitmath
import cpuinfo
import matplotlib.pyplot as plt
import matplotlib.patches as patches
from matplotlib.ticker import ScalarFormatter

def parse_cache(cache):
    ret = {}
    if isinstance(cache, int):
        ret['bytes'] = cache
        ret['str'] = str(bitmath.best_prefix(cache))
    else :
        ret['bytes'] = int(bitmath.parse_string(cache).bytes)
        ret['str'] = cache
    return ret

cpu_familiy = ""
try:
    cpu_familiy = os.environ['CPU_FAMILIY']
except:
    pass

util_dict = {}
cpu_dict = cpuinfo.get_cpu_info()

if 'brand_raw' in cpu_dict : 
    util_dict['cpu_name'] = cpu_dict['brand_raw']
else : util_dict['cpu_name'] = ""

if cpu_familiy != "":
        util_dict['cpu_name'] += f"   ({cpu_familiy})"

if 'l1_data_cache_size' in cpu_dict:
    util_dict['l1'] = parse_cache(cpu_dict['l1_data_cache_size'])

if 'l2_cache_size' in cpu_dict:
    util_dict['l2'] = parse_cache(cpu_dict['l2_cache_size'])

if 'l3_cache_size' in cpu_dict:
    util_dict['l3'] = parse_cache(cpu_dict['l3_cache_size'])

util_dict['dram'] = {'str' : '', 'bytes' : ''}

cpu_dict = cpuinfo.get_cpu_info()
cpu_freq = cpu_dict['hz_advertised'][0]
cpu_count = cpu_dict['count']

fig, axs = plt.subplots(3, 1)
fig.set_figheight(24)
fig.set_figwidth(16)



colors = {'dram' : 'blue', 'l1' : 'orange', 'l2' : 'red'}
bw_indexes = {'l1' : 0, 'l2' : 1, 'dram' : 2}

for bw_idx, bw in enumerate(['l1', 'l2', 'dram']):
# for bw in ['dram']:
    axs[bw_idx].set_ylabel("Performance in GFLOPS", color='green', fontsize=15)
    axs[bw_idx].set_xlabel("AI in GFLOPS", color='green', fontsize=15)
    title = f'{bw.upper()} ROOFLINE {cpu_dict["brand_raw"]}'
    axs[bw_idx].set_title(title, fontsize=20, color="gray", pad=15, weight="heavy")

    bw_file = open(f'bw/{bw}.txt')
    seq_bw = float(bw_file.readline().split()[1]) / 1024
    par_bw = float(bw_file.readline().split()[1]) / 1024

    # print(f'{bw.capitalize()} SEQ BANDWIDTH {seq_bw} GB/s')
    # print(f'{bw.capitalize()} PAR BANDWIDTH : {par_bw} GB/s')


    # PEAK = CPU FREQ * Vector lane size * MUL/ADD
    p_peak_seq = (cpu_freq / 1e9) * 8 * 2
    p_peak_par = p_peak_seq * cpu_count
    # print(f'P {bw} Peak seq : {p_peak_seq}, GFLOPS')
    # print(f'P {bw} Peak par : {p_peak_par}, GFLOPS')

    x = np.linspace(0, 2**12, 40000)
    y1 = seq_bw * x
    y2 = par_bw * x


    Y1 = []
    for i in y1:
        Y1.append(min(i, p_peak_seq))

    Y2 = []
    for i in y2:
        Y2.append(min(i, p_peak_par))


    axs[bw_idx].loglog(x, Y1, base = 2, linewidth=1.0,
        linestyle='--',
        color='orange',
        )

    axs[bw_idx].loglog(x, Y2, base = 2, linewidth=1.0,
        linestyle='--',
        color='red',
        )

    for axis in [axs[bw_idx].xaxis, axs[bw_idx].yaxis]:
        axis.set_major_formatter(ticker.StrMethodFormatter("{x:.3f}"))
        axs[bw_idx].locator_params('x', numticks=20)
        axs[bw_idx].locator_params('y', numticks=20)
        axs[bw_idx].plot(4096, 4096, 'wo')

    axs[bw_idx].text(1024, p_peak_seq + math.log2(p_peak_seq), 
                        f'SEQ {bw.upper()} BW : {round(seq_bw)} GBPS',
                        color='grey')
    axs[bw_idx].text(1024, p_peak_par + cpu_count/2 * math.log2(p_peak_par),
                        f'PAR {bw.upper()} BW : {round(par_bw)} GBPS',
                        color='grey')


for dir, c in {'foreach_simple' : 'green', 'foreach_complex' : 'blue'}.items():
    file_path = f'{dir}/plots/plt.txt'
    if os.path.exists(file_path):
        file_dict = {}
        f = open(file_path)
        simd_lane = int(f.readline()[:-1].split('=')[1])
        count = int(f.readline()[:-1].split('=')[1])
        ai = float(f.readline()[:-1].split('=')[1])
        # print(simd_lane, count, ai)

        while True:
            line = f.readline()
            if not line:
                break
            # print(line[:-1].split('='))
            k, v = line[:-1].split('=')
            v = float(v)
            # print(k, v)
            bw, pol = k.split('_')
            axs[bw_indexes[bw]].plot(ai, v, 'o', color=c)
            axs[bw_indexes[bw]].text(ai +  0.05 + math.log(ai)/2, v, f'{round(v, 1)}\n{pol}', verticalalignment='center')
            axs[bw_indexes[bw]].axvline(ai, color='grey', ls='--')
            axs[bw_indexes[bw]].text(ai, 1024, f'{dir}', rotation=90, color='grey', fontsize=12, weight="light")

            c_str = ''
            if (bw != 'dram'):
                c_str = f"{bw} size {util_dict[bw]['str']}\n"
            cache_str_ =  c_str + f"float pack size : {simd_lane}"
            axs[bw_indexes[bw]].text(0.125, 512, cache_str_, bbox=dict(facecolor='grey', alpha=0.3), fontsize = 12)

plt.tight_layout()
plt.savefig('plots.png')