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

def get_detail_csv(df, policies):
    df2 = pd.DataFrame()
    df2['n_in_2_pow'] = df['n']
    df2['n'] = np.power(2, df['n'].to_numpy())

    for policy in policies:
        df2[f'{policy}_time'] = df[f'{policy}']
        df2[f'{policy}_grain_size'] = df[f'{policy}'].to_numpy() / df2['n'].to_numpy()
        df2[f'{policy}_speed_up'] = df[f'seq'].to_numpy() / df[f'{policy}']
    return df2

def plot_grain_size(policies):
    fig, axs = plt.subplots(1, 1)
    fig.set_figheight(8)
    fig.set_figwidth(16)

    n = df['n'].to_numpy()
    seq = df2['seq_grain_size'].to_numpy()

    width = 0
    height = 0
    peaks = {}
    speed_ups = {}
    for policiy_idx, policy in enumerate(policies):
        speed_up = np.log10(df2[f'{policy}_grain_size'].to_numpy())
        speed_ups[policy] = speed_up
    
    height = round(height) + 1
    width = len(n)
    start = n[0]
    end = n[-1]

    x_tick_labels = []
    y_tick_labels = []  

    for i in range(1, 13):
        y_tick_labels.append(-i)
    x_tick_labels = n

    title_ = dir.replace('.', ' ').replace('/', ' ').strip()
    title = f'{title_} with {type_}s  ({cpu_familiy})'
    axs.set_title(title, fontsize=20, color="gray", pad=15, weight="heavy")
    axs.set_ylabel("Grain size in 10^", color='green', fontsize=20)
    axs.set_xlabel("Elements in 2^", color='green', fontsize=20)

    for policiy_idx, policy in enumerate(policies):
        axs.plot(n, speed_ups[policy], linewidth=2.0,
            linestyle='solid',
            marker="2",
            color=policy_colors[policy],
            zorder=10)

    for line, name in zip(axs.lines, policies):
        y = line.get_ydata()[-1]
        axs.annotate(name, xy=(1,y), xytext=(16,0), color=line.get_color(), 
        xycoords = axs.get_yaxis_transform(), textcoords="offset points",
        size=14, va="center")
    
    axs.set_xticks(n)
    axs.set_yticks(y_tick_labels)
    
    plt.tight_layout()
    plt.savefig(os.path.join(plots_path, f'{type_}_grain_size.png'))

policies = ['seq', 'simd', 'par', 'simdpar']
policy_colors = {
                'seq' : 'grey',
                'simd' :  'orange', 
                'par' : 'blue',
                'simdpar' : 'red'
                }
types = ['int', 'float', 'double']
types_sizes = {
                'int' : 4,
                'float' : 4,
                'double' : 8
              }

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

cache_str = ""
for i in range(1, 4):
    if f'l{i}' in util_dict:
        cache_str += f"L{i} Cache : {util_dict[f'l{i}']['str']}\n"

for path, dirs, files in os.walk('.'):
    if 'plots' in dirs:
        plots_path = os.path.join(path, 'plots')
        dir = os.path.dirname(plots_path)
        policies_ = policies
        print(dir)
        if dir.find('naive') != -1:
            policies_ = policies[:2]
        # print(plots_path, dir)
        for file_ in os.listdir(plots_path):
            for type_ in types:
                if file_ == f'{type_}.csv':
                    df = pd.read_csv(os.path.join(plots_path, file_))
                    df2 = get_detail_csv(df, policies_)
                    df2.to_csv(os.path.join(plots_path, f'{type_}_detail.csv'))

                    fig, axs = plt.subplots(1, 1)
                    fig.set_figheight(8)
                    fig.set_figwidth(16)

                    n = df['n'].to_numpy()
                    lane_size = df['lane'].to_numpy()[0]
                    threads = df['threads'].to_numpy()[0]
                    seq = df['seq'].to_numpy()

                    width = 0
                    height = 0
                    peaks = {}
                    speed_ups = {}
                    for policiy_idx, policy in enumerate(policies_):
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

                    x_tick_labels = []
                    y_tick_labels = []  

                    x_tick_labels = n
                    step_ = 0.1 * height

                    y_tick_labels = np.arange(0, height + math.log(height) * step_, step_)

                    # Title
                    # print(f'dir = {dir}')
                    title_ = dir.replace('.', ' ').replace('/', ' ').strip()
                    title = f'{title_} with {type_}s  ({cpu_familiy})'
                    axs.set_title(title, fontsize=20, color="gray", pad=15, weight="heavy")
                    axs.set_ylabel("Speed Up vs sequential", color='green', fontsize=20)
                    axs.set_xlabel("Elements in 2^", color='green', fontsize=20)

                    # Plot speed ups
                    for policiy_idx, policy in enumerate(policies_):
                        axs.plot(n, speed_ups[policy], linewidth=2.0,
                            linestyle='solid',
                            marker="2",
                            color=policy_colors[policy],
                            zorder=10)

                        # Peaks
                        if policy != 'seq':
                            axs.text(peaks[policy]['peak_idx'] + start, peaks[policy]['peak'] + 0.2,
                                            f"{round(peaks[policy]['peak'], 2)}",
                                            fontsize=12,
                                            weight="medium")
                    
                    # Box
                    cache_str_ = cache_str + f"{type_} pack size : {lane_size}\nthreads : {threads}"
                    axs.text(start, height - math.log2(height), cache_str_, bbox=dict(facecolor='grey', alpha=0.2), fontsize = 13, color="grey")
                    
                    # Cache lines
                    for i in ['1', '2', '3']:
                        if f'l{i}' in util_dict:
                            float_l = math.log2(util_dict['l' + i]['bytes'] / types_sizes[type_])

                            axs.axvline(float_l, c='grey', ls='--')
                            axs.text(float_l - 0.3, height - math.log2(height), f'L{i} {type_}s', rotation=90, color='grey', fontsize=12)

                    # Offset points annotations
                    for line, name in zip(axs.lines, policies_):
                        y = line.get_ydata()[-1]
                        axs.annotate(name, xy=(1,y), xytext=(16,0), color=line.get_color(), 
                        xycoords = axs.get_yaxis_transform(), textcoords="offset points",
                        size=14, va="center")
                    
                    axs.set_xticks(n)
                    axs.set_yticks(y_tick_labels)
                    
                    plt.tight_layout()
                    plt.savefig(os.path.join(plots_path, f'{type_}.png'))
                    plot_grain_size(policies_)

