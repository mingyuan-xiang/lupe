# reference: https://stackoverflow.com/a/22845857/13813036

import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
import matplotlib.colors as mcolors

FREQ = 32767

# time = np.array(
#     [[6853, 13421, 7882, 8096, 32737, 27353],
#     [15823, 32684, 17618, 21963, 15751, 16387],
#     [5448, 18020, 11549, 13722, 5435, 6337]]
# ).astype('float64')

# no loop unroll
time = np.array(
    [[6853, 7882, 32737],
    [15823, 17618, 15751],
    [5448, 11549, 5435]]
).astype('float64')

lupe = time[:, 0].reshape(-1, 1) 
time = time / lupe


time_df = pd.DataFrame(time,
    index=['conv3', 'conv2', 'conv1'],
    # columns=[
    #     'Lupe', 'MAC + DMA (Static)', 'MAC + DMA', 'MAC + Loop Copy',
    #     'FIR + DMA', 'FIR + Loop Copy']
    columns=[
        'Top-down', 'MAC', 'FIR']
)

plt.rcParams["font.family"] = "Times New Roman"
plt.rcParams.update({'font.size': 14})
plt.rcParams["font.weight"] = "bold"
plt.rcParams["axes.labelweight"] = "bold"

fig = plt.figure()
ax = fig.add_subplot(111)

maps = plt.colormaps['Set2']
new_colors = maps(np.linspace(0.125, 0.825, 256))
new_cmap = mcolors.LinearSegmentedColormap.from_list("Set2_mod", new_colors)

ax = time_df.plot(
    kind="barh", linewidth=0, ax=ax, legend=False, grid=False,
    width=0.7, cmap=new_cmap
)

bars = ax.patches
# patterns = [ "/" , "\\" , "|" , "-" , "+" , "x", "o", "O", ".", "*" ]
patterns = ["oo", "++", "\\\\"]

hatches = []
for p in patterns:
    for _ in range(len(time)):
        hatches.append(p)

for bar, hatch in zip(bars, hatches):
    bar.set_hatch(hatch)

ax.set_xlabel('Normalized to Inference Time of Lupe')
handles, labels = ax.get_legend_handles_labels()
ax.legend(handles[::-1], labels[::-1], loc='upper right')

fig.tight_layout(pad=0.1)
plt.savefig('lenet_unroll.png', dpi=500)
# plt.show()
