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
            


for dir in os.listdir():
    if os.path.isdir(dir):
        if (dir[0] != '.'):

            # Getting floats data
            pd_path = dir + '/plots/float.csv'
            if (os.path.exists(pd_path)):
                df = pd.read_csv(pd_path)
                

                n_float = df['n'].to_numpy()
                seq_float = df['seq'].to_numpy()
                simd_float = df['simd'].to_numpy()
                lane_size_float = df['lane'].to_numpy()[0]

                speed_up_float = seq_float / simd_float
                max_speed_up_float = np.max(speed_up_float)
                max_index_float = np.argmax(speed_up_float)

                # Getting doubles data
                df = pd.read_csv(dir + '/plots/double.csv')

                n_double = df['n'].to_numpy()
                seq_double = df['seq'].to_numpy()
                simd_double = df['simd'].to_numpy()
                lane_size_double = df['lane'].to_numpy()[0]

                speed_up_double = seq_double / simd_double
                max_speed_up_double = np.max(speed_up_double)
                max_index_double = np.argmax(speed_up_double)

                # set plot height and width
                height =  round(max(max_speed_up_float, max_speed_up_double)) + 1
                width = len(n_float)

                # Assert both float and doubles have same datapoints
                assert(n_float.all() == n_double.all())
                n_start = n_float[0]
                n_end = n_float[-1]

                # Get x_tick_labels and y_tick_labels
                x_tick_labels = []
                y_tick_labels = []

                x_tick_labels = n_float
                step_ = 1
                # if (height < 5) : step_ = 0.5

                if height > 15 :  step_ = 2.5
                if height > 5 and height < 15 :  step_ = 1
                if height < 5  : step_ = 0.5

                y_tick_labels = np.arange(0, height + math.log2(height), step_)

                # Figure 
                fig, ax = plt.subplots()
                fig.set_figheight(8)
                fig.set_figwidth(16)


                # Plotting Actual Data

                # Title
                title = dir + '   ' + util_dict['cpu_name']
                ax.set_title(title, fontsize=20, color="gray", pad=15, weight="heavy")
                ax.set_ylabel("Speed Up vs sequential", color='green', fontsize=20)
                ax.set_xlabel("Elements in 2^", color='green', fontsize=20)


                # float data
                ax.plot(n_float, speed_up_float, linewidth=2.0,
                    linestyle='solid',
                    marker="2",
                    color="orange",
                    zorder=10)

                #double data
                ax.plot(n_double, speed_up_double, linewidth=2.0,
                    linestyle='solid',
                    marker="2",
                    color="red",
                    zorder=10)

                # Annotate
                for line, name in zip(ax.lines, ['float', 'double']):
                    y = line.get_ydata()[-1]
                    ax.annotate(name, xy=(1,y), xytext=(16,0), color=line.get_color(), 
                                xycoords = ax.get_yaxis_transform(), textcoords="offset points",
                                size=14, va="center")

                # Box
                cache_str_ = cache_str + f"\nfloat pack size : {lane_size_float}" + \
                            f"\ndouble pack size : {lane_size_double}"
                ax.text(5, height - step_ / 2, cache_str_, bbox=dict(facecolor='grey', alpha=0.2), fontsize = 13, color='grey')
                
                # Peaks
                ax.text(max_index_float + n_start, max_speed_up_float + 0.2, f"{round(max_speed_up_float, 2)}")
                ax.text(max_index_double + n_start, max_speed_up_double + 0.2, f"{round(max_speed_up_double, 2)}")

                # Cache Lines
                for i in ['1', '2', '3']:
                    float_l = math.log2(util_dict['l' + i]['bytes'] / 4)
                    double_l = math.log2(util_dict['l' + i]['bytes'] / 8)

                    ax.axvline(float_l, c='grey', ls='--')
                    plt.text(float_l - 0.3, height, f'L{i} floats', rotation=90, color='grey', fontsize=15)
                    ax.axvline(double_l, c='grey', ls='--')
                    plt.text(double_l - 0.3, height, f'L{i} doubles', rotation=90, color='grey', fontsize=15)

                # Ticks
                # ax.set_xticklabels(n_float, fontsize=12)
                ax.set_xticks(n_float)
                ax.set_yticks(y_tick_labels)

                #save
                plt.tight_layout()
                plt.savefig(dir + '/plots/plot.png')


