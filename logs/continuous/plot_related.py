import json
import matplotlib.pyplot as plt
import matplotlib.patches as mpatches
import matplotlib.ticker as ticker
import numpy as np

opt_flags = {
    'Tails' : ('Tails',  'royalblue', '++'),
    'Hawaii' : ('Hawaii',  'royalblue', '--'),
    'Bottom-up' : ('no_opt', 'pink', 'xx'),
    'Lupe' : ('dma_lea_opt_adaptive_buffer_mem_acc',  'firebrick', '\\\\'),
}

cmap1 = plt.get_cmap('cool')
cmap2 = plt.get_cmap('Wistia')
N = len(opt_flags)
colors1 = cmap1(np.linspace(0, 1, N))
colors2 = cmap2(np.linspace(0, 1, N))
for (key, (label, _, marker)), color in zip(opt_flags.items(), colors1):
    opt_flags[key] = (label, color, marker)

width = 0.5
plt.rcParams["font.family"] = "Times New Roman"
plt.rcParams.update({'font.size': 16})
plt.rcParams["font.weight"] = "bold"
plt.rcParams["axes.labelweight"] = "bold"

with open('../intermittent/results.json', 'r') as file:
    results = json.load(file)['(0, 0)']

ordered_models = ['MLPClassifier', 'LeNet', 'ResNet3', 'MobileNetV2', 'DS-CNN']
ordered_configs = list(opt_flags.keys())

data = {}
for m in ordered_models:
    if m == 'DS-CNN':
        model = 'DS_CNN'
    else:
        model = m
    data[m] = {}
    for d in results[model]:
        for f in opt_flags:
            if opt_flags[f][0] == d['config']:
                config = f
                break
        data[m][config] = d['continuous_time']

fig = plt.figure(figsize=(25, 8))
subfigs = fig.subfigures(nrows=1, ncols=len(ordered_models))

for idx, (model, subfig) in enumerate(zip(ordered_models, subfigs)):
    ax = subfig.subplots()

    config_data = data[model]

    continuous_vals = []
    colors = []
    hatches = []

    for config in ordered_configs:
        if config in config_data:
            continuous_vals.append(config_data[config])
        else:
            continuous_vals.append(0)

        colors.append(opt_flags[config][1])
        hatches.append(opt_flags[config][2])

    x = np.arange(len(ordered_configs))
    
    bars = ax.bar(x, continuous_vals, width, color=colors)
    for bar, hatch in zip(bars, hatches):
        bar.set_hatch(hatch)

    if model == 'MLPClassifier':
        ax.set_ylabel('Time (s)')
    else:
        ax.yaxis.set_major_locator(ticker.MaxNLocator(integer=True))
        yticks = ax.get_yticks()[:-1]
        ax.set_yticks([tick for tick in yticks if tick != 0])

    if model == 'MobileNetV2':
        ax.scatter(1, 0.3, marker='X', s=300, color='red')

    ax.set_xticklabels([])
    ax.xaxis.set_tick_params(length=0)

    subfig.supxlabel(model, y=0.015, x=0.58, fontweight='bold')

l = []
for n, f in opt_flags.items():
    p = mpatches.Patch(facecolor=f[1], hatch=f[2], label=n)
    l.append(p)

fig.legend(handles=l, loc='lower center', ncol=6, bbox_to_anchor=(0.5, -0.06))

fig.tight_layout(pad=0.05, rect=[0, 0.05, 1, 0.95])

plt.savefig(f'figures/continuous_related.png', dpi=500, bbox_inches='tight')