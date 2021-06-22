from PIL import Image
import os
import matplotlib.pyplot as plt

img_arr = []

height = 0
for dir in ['simd_math_tests', 'algorithms', 'roofline']:
    path = dir + '/plots.png'
    img_arr.append(Image.open(path))

for i in range(len(img_arr)):
    img_arr[i] = img_arr[i].convert('RGB')
    height += (img_arr[i].size)[1] // 100 


fig, axs = plt.subplots(len(img_arr), 1)

fig.set_figheight(height)
fig.set_figwidth(16)
for i in range(len(img_arr)):
    axs[i].imshow(img_arr[i])
    axs[i].axis('off')

plt.tight_layout()
plt.savefig('plots.png')