import json
import matplotlib.pyplot as plt
import matplotlib.patches as mpatches
import numpy as np

opt_flags = {
    'Tails' : ('Tails',  'royalblue', '++'),
    'Hawaii' : ('Hawaii',  'royalblue', '--'),
    'Bottom-up' : ('Bottom-up', 'pink', 'xx'),
    'Lupe' : ('Lupe',  'firebrick', '\\\\'),
}

cmap1 = plt.get_cmap('cool')
cmap2 = plt.get_cmap('Wistia')
N = len(opt_flags)
colors1 = cmap1(np.linspace(0, 1, N))
colors2 = cmap2(np.linspace(0, 1, N))
for (key, (label, _, marker)), color in zip(opt_flags.items(), colors1):
    opt_flags[key] = (label, color, marker)

width = 0.35
plt.rcParams["font.family"] = "Times New Roman"
plt.rcParams.update({'font.size': 16})
plt.rcParams["font.weight"] = "bold"
plt.rcParams["axes.labelweight"] = "bold"

with open('results.json', 'r') as file:
    data = json.load(file)

ordered_models = ['MLPClassifier', 'LeNet', 'ResNet3', 'MobileNetV2', 'DS-CNN']
ordered_configs = list(opt_flags.keys())

shifter = 1000

fig = plt.figure(figsize=(25, 8))
subfigs = fig.subfigures(nrows=1, ncols=len(ordered_models))

for idx, (model, subfig) in enumerate(zip(ordered_models, subfigs)):
    ax = subfig.subplots()

    config_data = data[model]

    continuous_vals = []
    intermittent_vals = []
    colors = []
    hatches = []
    
    for config in ordered_configs:
        if config in config_data:
            continuous_vals.append(config_data[config].get('continuous', 0) * shifter)
            intermittent_vals.append(config_data[config].get('intermittent', 0) * shifter)
        else:
            continuous_vals.append(0)
            intermittent_vals.append(0)

        colors.append(opt_flags[config][1])
        hatches.append('xx')

    x = np.arange(len(ordered_configs))
    
    ax.bar(x - width/2, continuous_vals, width, color=colors)
    bars = ax.bar(x + width/2, intermittent_vals, width, color=colors)
    for bar, hatch in zip(bars, hatches):
        bar.set_hatch(hatch)

    if model == 'MLPClassifier':
        ax.set_ylabel('Energy (mJ)')
    else:
        yticks = ax.get_yticks()[:-1]
        ax.set_yticks([tick for tick in yticks if tick != 0])

    if model == 'MobileNetV2':
        ax.scatter(1, 7, marker='X', s=300, color='red')

    ax.set_xticklabels([])
    ax.xaxis.set_tick_params(length=0)

    subfig.supxlabel(model, y=0.015, x=0.58, fontweight='bold')

l = []
for _, f in opt_flags.items():
    p = mpatches.Patch(facecolor=f[1], label=f[0])
    l.append(p)
l.append(mpatches.Patch(facecolor='w', label='Continuous', edgecolor='black'))
l.append(mpatches.Patch(facecolor='w', hatch='xx', label='Intermittent', edgecolor='black'))

fig.legend(handles=l, loc='lower center', ncol=6, bbox_to_anchor=(0.5, -0.06))

fig.tight_layout(pad=0.05, rect=[0, 0.05, 1, 0.95])

plt.savefig(f'figures/energy.png', dpi=500, bbox_inches='tight')
