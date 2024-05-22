# reference: https://stackoverflow.com/a/22845857/13813036

import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
import matplotlib.colors as mcolors

FREQ = 32767

time = np.array(
    [[11473, 29827, 13169, 13248, 55203, 45729],
    [26935, 54271, 29228, 35716, 26844, 27719],
    [9488, 22576, 19064, 22274, 9508, 10863]]
) / FREQ

time_df = pd.DataFrame(time,
    index=['conv3', 'conv2', 'conv1'],
    columns=[
        'Lupe', 'MAC + DMA (Static)', 'MAC + DMA', 'MAC + Loop Copy', 'FIR + DMA', 'FIR + Loop Copy']
)

plt.rcParams["font.family"] = "Times New Roman"
plt.rcParams.update({'font.size': 14})

fig = plt.figure()
ax = fig.add_subplot(111)

maps = plt.colormaps['Set2']
new_colors = maps(np.linspace(0.125, 0.825, 256))
new_cmap = mcolors.LinearSegmentedColormap.from_list("Set2_mod", new_colors)

ax = time_df.plot(
    kind="barh", linewidth=0, ax=ax, legend=False, grid=False,
    width=0.7, cmap=new_cmap
)

ax.set_xlabel('Time/Sec')
handles, labels = ax.get_legend_handles_labels()
ax.legend(handles[::-1], labels[::-1])

fig.tight_layout(pad=0.1)
plt.savefig('lenet_unroll.png', dpi=2000)
# plt.show()
