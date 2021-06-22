from PIL import Image
import os
import matplotlib.pyplot as plt

img_arr = []
dirs = os.listdir()
dirs.sort()
for dir in dirs:
    if os.path.isdir(dir):
        if (dir[0] != '.'):
            path = dir + '/plots/plot.png'
            img_arr.append(Image.open(path))

for i in range(len(img_arr)):
    img_arr[i] = img_arr[i].convert('RGB')

im1 = img_arr[0]

fig, axs = plt.subplots(len(img_arr), 1)

fig.set_figheight(24 * len(img_arr))
fig.set_figwidth(16)
for i in range(len(img_arr)):
    axs[i].imshow(img_arr[i])
    axs[i].axis('off')

plt.tight_layout()
plt.savefig('plots.png')
