import os
from PIL import Image
import matplotlib.pyplot as plt
# import subprocess
os.system('find . -name "plots" | sort -n > plots.txt')
f = open('plots.txt')

perf_plots_paths = []
grain_size_plots_paths = []

types = ['int', 'float', 'double']
while True:
    line = f.readline()
    if not line:
        break
    path = line[:-1]
    dirs = os.listdir(path)
    for type_ in types:
        if f'{type_}.png' in dirs:
            perf_plots_paths.append(os.path.join(path, f'{type_}.png'))
        if f'{type_}_grain_size.png' in dirs:
            grain_size_plots_paths.append(
                os.path.join(path, f'{type_}_grain_size.png'))

print(len(perf_plots_paths), len(grain_size_plots_paths))

perfs = perf_plots_paths
grains = grain_size_plots_paths

for chunk, chunk_name in zip([perfs, grains], ['perfs', 'grains']):
    fig, ax = plt.subplots(len(chunk), 1)
    fig.set_figheight(len(chunk) * 8)
    fig.set_figwidth(16)
    for i in range(len(chunk)):
        ax[i].imshow(Image.open(chunk[i]).convert('RGB'))
        ax[i].axis('off')
    plt.tight_layout()
    plt.savefig(f'{chunk_name}.png')
    