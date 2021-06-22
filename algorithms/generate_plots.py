import os
import bitmath
import cpuinfo
import platform
import math
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


cache_str = f"L1 Cache : {util_dict['l1']['str']}\n" + \
            f"L2 Cache : {util_dict['l2']['str']}\n" + \
            f"L3 Cache : {util_dict['l3']['str']}\n"

policies = ['simd', 'par', 'simdpar']
colors = ['orange', 'blue', 'red']
sizeof = {'int' : 4, 'float' : 4, 'double' : 8}

for dir in os.listdir():
# for dir in ['fill']:
    fig, axs = plt.subplots(3, 1)
    fig.set_figheight(24)
    fig.set_figwidth(16)
    if os.path.isdir(dir):
        if (dir[0] != '.'):
            for type_idx, Type in enumerate(['int', 'float', 'double']):
                csv_path = f'{dir}/plots/{Type}.csv'
                # print(Idx, Type, os.path.exists(csv_path))
                if (os.path.exists(csv_path)):
                    df = pd.read_csv(csv_path)
                    
                    n = df['n'].to_numpy()
                    lane_size = df['lane'].to_numpy()[0]
                    threads = df['threads'].to_numpy()[0]
                    seq = df['seq'].to_numpy()

                    width = 0
                    height = 0
                    peaks = {}
                    speed_ups = {}
                    for policiy_idx, policy in enumerate(policies):
                        speed_up = seq / df[policy].to_numpy()
                        speed_ups[policy] = speed_up
                        peak = speed_up.max()
                        peak_idx = speed_up.argmax()
                        peaks[policy] = {'peak' : peak, 'peak_idx' : peak_idx}
                        height = max(height, peak)
                    
                    height = round(height) + 1
                    width = len(n)
                    start = n[0]
                    end = n[-1]

                    for policiy_idx, policy in enumerate(policies):
                        x_tick_labels = []
                        y_tick_labels = []  

                        x_tick_labels = n
                        # step_ = 1

                        # # if height > 15 :  step_ = 2.5
                        # # if height > 5 and height < 15 :  step_ = 1
                        # # if height < 5  : step_ = 0.5
                        step_ = 0.1 * height

                        y_tick_labels = np.arange(0, height + math.log(height) * step_, step_)

                        # Title
                        title = dir + '  ' +  Type + '   ' + util_dict['cpu_name']
                        axs[type_idx].set_title(title, fontsize=20, color="gray", pad=15, weight="heavy")
                        axs[type_idx].set_ylabel("Speed Up vs sequential", color='green', fontsize=20)
                        axs[type_idx].set_xlabel("Elements in 2^", color='green', fontsize=20)

                        axs[type_idx].plot(n, speed_ups[policy], linewidth=2.0,
                            linestyle='solid',
                            marker="2",
                            color=colors[policiy_idx],
                            zorder=10)
                    
                        # Box
                        cache_str_ = cache_str + f"{Type} pack size : {lane_size}\nthreads : {threads}"
                        axs[type_idx].text(start, height - math.log2(height), cache_str_, bbox=dict(facecolor='grey', alpha=0.2), fontsize = 13, color="grey")

                        # Peaks
                        axs[type_idx].text(peaks[policy]['peak_idx'] + start, peaks[policy]['peak'] + 0.2,
                                         f"{round(peaks[policy]['peak'], 2)}",
                                         fontsize=12,
                                          weight="medium")
            
                    for i in ['1', '2', '3']:
                        float_l = math.log2(util_dict['l' + i]['bytes'] / sizeof[Type])

                        axs[type_idx].axvline(float_l, c='grey', ls='--')
                        axs[type_idx].text(float_l - 0.3, height - math.log2(height), f'L{i} {Type}s', rotation=90, color='grey', fontsize=12)

                    for line, name in zip(axs[type_idx].lines, policies):
                        y = line.get_ydata()[-1]
                        axs[type_idx].annotate(name, xy=(1,y), xytext=(16,0), color=line.get_color(), 
                        xycoords = axs[type_idx].get_yaxis_transform(), textcoords="offset points",
                        size=14, va="center")
                    
                    axs[type_idx].set_xticks(n)
                    axs[type_idx].set_yticks(y_tick_labels)


                    plt.tight_layout()
                    plt.savefig(f'{dir}/plots/plot.png')
