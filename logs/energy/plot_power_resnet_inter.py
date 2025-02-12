import os
import pandas as pd
import matplotlib.pyplot as plt
import numpy as np

prefix = os.path.join('logs', 'power_snapshot')

files = [
    ('inter_tails_resnet.csv', 'Tails'),
    ('inter_lupe_no_opt_resnet.csv', 'Lupe-BT'),
    ('inter_hawaii_resnet.csv', 'Hawaii'),
    ('inter_lupe_resnet.csv', 'Lupe'),
]

cmap1 = plt.get_cmap('cool')
N = len(files)
colors1 = cmap1(np.linspace(0, 1, N))
files = [(*files[i], colors1[i]) for i in range(N)]

width = 0.35
plt.rcParams["font.family"] = "Times New Roman"
plt.rcParams.update({'font.size': 20})
plt.rcParams["font.weight"] = "bold"
plt.rcParams["axes.labelweight"] = "bold"


def get_df(filepath):
    df = pd.read_csv(filepath)

    df['Power:float'] = pd.to_numeric(df['Power:float'], errors='coerce')
    df['Time:ulong'] = pd.to_numeric(df['Time:ulong'], errors='coerce')
    df['Time:ulong'] = (df['Time:ulong'] - df.iloc[0, 0]) / 1000000000

    return df

def disable_tick(t):
    t.label1.set_visible(False)
    t.tick1line.set_markersize(0)
    t.tick2line.set_markersize(0)

fig, axes = plt.subplots(nrows=4, ncols=1, figsize=(20, 5), sharex=True)

for ax, (f, n, c) in zip(axes, files):
    filepath = os.path.join(prefix, f)
    df = get_df(filepath)

    ax.plot(df['Time:ulong'], df['Power:float'], linestyle='-', color=c, label=n, zorder=3)
    ax.set_ylim([0, 9])
    ax.set_yticks([0, 3, 6])
    ax.set_xlim(0, 60)
    ax.grid(axis='both', zorder=0, linestyle='--')

    if n != 'Lupe':
        ax.xaxis.set_tick_params(labelbottom=False)
        ax.xaxis.set_tick_params(length=0)

        yticks = ax.yaxis.get_major_ticks()
        disable_tick(yticks[0])
    else:
        ax.set_xlabel('Time (s)')


fig.legend(
    loc='lower center', ncol=4,
    bbox_to_anchor=(0.51, -0.025),
    edgecolor='black'
)
fig.tight_layout(pad=0.05, rect=[0.015, 0.10, 1, 1])

fig.text(0, 0.6, 'Power (mW)', va='center', rotation='vertical')

plt.subplots_adjust(hspace=0)

plt.savefig(f"figures/power_resnet_inter.png", dpi=500)
# plt.show()
