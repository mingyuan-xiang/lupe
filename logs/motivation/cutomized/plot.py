# reference: https://stackoverflow.com/a/22845857/13813036

import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
import matplotlib.colors as mcolors

FREQ = 32767
plt.rcParams["font.family"] = "Times New Roman"
plt.rcParams.update({'font.size': 14})

def plot_clustered_stacked(dfall, labels=None, title='LeNet5',  H="//", **kwargs):
    n_df = len(dfall)
    n_col = len(dfall[0].columns) 
    n_ind = len(dfall[0].index)
    fig = plt.figure()
    axe = fig.add_subplot(111)

    for df in dfall : # for each data frame
        axe = df.plot(kind="barh",  # Change 'kind' to 'barh' for horizontal bars
                        linewidth=0,
                        stacked=True,
                        ax=axe,
                        legend=False,
                        grid=False,
                      **kwargs)  # make bar plots

    h,l = axe.get_legend_handles_labels() # get the handles we want to modify
    for i in range(0, n_df * n_col, n_col): # len(h) = n_col * n_df
        for j, pa in enumerate(h[i:i+n_col]):
            for rect in pa.patches: # for each index
                rect.set_y(rect.get_y() + 1 / float(n_df + 1) * i / float(n_col)) # Adjust y position
                rect.set_hatch(H * int(i / n_col)) #edited part     
                rect.set_height(1 / float(n_df + 1)) # Adjust height

    axe.set_yticks((np.arange(0, 2 * n_ind, 2) + 1 / float(n_df + 1)) / 2.)
    axe.set_yticklabels(df.index, rotation = 0)  # Use set_yticklabels to set labels for horizontal bars
    axe.set_xlabel('Time/Sec')  # Change from ylabel to xlabel for horizontal bars

    # Add invisible data to add another legend
    n=[]        
    for i in range(n_df):
        n.append(axe.barh(0, 0, color="gray", hatch=H * i))

    l1 = axe.legend(h[:n_col], l[:n_col], loc=[0.6, 0.65])
    if labels is not None:
        l2 = plt.legend(n, labels, loc=[0.7, 0.45]) 
    axe.add_artist(l1)
    fig.tight_layout(pad=0.1)
    return axe

data = np.array([[59268, 2977, 27792, 9776],
                [27994, 305, 17205, 4007],
                [9699, 1408, 6435, 909]]) / FREQ
data_unroll = np.array([[50875, 21, 27814, 8124],
                        [27573, 155, 17206, 3937],
                        [9639, 1399, 6435, 900]]) / FREQ

for i in range(3):
    data[i, 0] -= np.sum(data[i, 1:])
    data_unroll[i, 0] -= np.sum(data_unroll[i, 1:])

df = pd.DataFrame(data,
                    index=['conv3', 'conv2', 'conv1'],
                    columns=['Control', 'Initialization', 'Compute', 'Data Movement'])
df_unroll = pd.DataFrame(data_unroll,
                    index=['conv3', 'conv2', 'conv1'],
                    columns=['Control', 'Initialization', 'Compute', 'Data Movement'])

pastel2 = plt.colormaps['PuOr']
new_colors = pastel2(np.linspace(0.25, 0.75, 256))
new_cmap = mcolors.LinearSegmentedColormap.from_list("PuOr_mod", new_colors)
plot_clustered_stacked([df_unroll, df],["Unrolled", "Original"], cmap=new_cmap)

plt.savefig('lenet_unroll.png', dpi=1000)
