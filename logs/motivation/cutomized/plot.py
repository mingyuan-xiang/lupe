# reference: https://stackoverflow.com/a/22845857/13813036

import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
import matplotlib.colors as mcolors

FREQ = 32767

time = np.array(
    [[59268, 45729, 14107, 13248],
    [27994, 27719, 31386, 35716],
    [9699, 10863, 20463, 22274]]
) / FREQ

time_df = pd.DataFrame(time,
    index=['conv3', 'conv2', 'conv1'],
    columns=[
        'FIR', 'FIR + Loop Unrolling',
        'MAC', 'MAC + Loop Unrolling']
)

plt.rcParams["font.family"] = "Times New Roman"
plt.rcParams.update({'font.size': 14})

fig = plt.figure()
ax = fig.add_subplot(111)

maps = plt.colormaps['Set2']
new_colors = maps(np.linspace(0.25, 0.75, 256))
new_cmap = mcolors.LinearSegmentedColormap.from_list("Set2_mod", new_colors)

ax = time_df.plot(
    kind="barh", linewidth=0, ax=ax, legend=False, grid=False,
    width=0.65, cmap=new_cmap
)

ax.set_xlabel('Time/Sec')
ax.legend()

fig.tight_layout(pad=0.1)
plt.savefig('lenet_unroll.png', dpi=1000)
# plt.show()
