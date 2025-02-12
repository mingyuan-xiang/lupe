import json
import matplotlib.pyplot as plt
import matplotlib.patches as mpatches
import numpy as np

opt_flags = {
    'Tails' : ('Tails',  'royalblue', '++'),
    'Bottom-up' : ('Bottom-up', 'pink', 'xx'),
    'Hawaii' : ('Hawaii',  'royalblue', '--'),
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
plt.rcParams.update({'font.size': 20})
plt.rcParams["font.weight"] = "bold"
plt.rcParams["axes.labelweight"] = "bold"

with open('results.json', 'r') as file:
    data = json.load(file)

ordered_models = ['ResNet3', 'DS-CNN', 'MobileNetV2', 'LeNet', 'MLPClassifier']
ordered_configs = list(opt_flags.keys())

shifter = 0.001

fig = plt.figure(figsize=(25, 6))
subfigs = fig.subfigures(nrows=1, ncols=len(ordered_models), wspace=0.25)

cont = []
inter = []

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

    lupe = continuous_vals[-1]
    cont.append([v / lupe for v in continuous_vals[:-1]])
    lupe = intermittent_vals[-1]
    inter.append([v / lupe for v in intermittent_vals[:-1]])
    
    ax.bar(x - width/2, continuous_vals, width, color=colors, zorder=3)
    bars = ax.bar(x + width/2, intermittent_vals, width, color=colors, zorder=3)
    for bar, hatch in zip(bars, hatches):
        bar.set_hatch(hatch)

    ax.set_ylabel('Energy Per Inference (mJ)', fontsize=22)

    if model == 'MobileNetV2':
        ax.scatter(2, 6, marker='X', s=300, color='red')

    ax.set_xticklabels([])
    ax.xaxis.set_tick_params(length=0)

    ax.grid(axis='y', color='grey', zorder=0)

    subfig.supxlabel(model, y=0.1, x=0.58, fontweight='bold')

l = []
for n, f in opt_flags.items():
    if n == 'Bottom-up':
        n = 'Lupe-BT'
    p = mpatches.Patch(facecolor=f[1], label=n)
    l.append(p)
l.append(mpatches.Patch(facecolor='w', label='Continuous', edgecolor='black'))
l.append(mpatches.Patch(facecolor='w', hatch='xx', label='Intermittent', edgecolor='black'))

fig.legend(handles=l, loc='lower center', ncol=6, fontsize=22, bbox_to_anchor=(0.5, -0.015), edgecolor='black')

fig.tight_layout(pad=0.05, rect=[0, 0.03, 1, 0.95])

plt.savefig(f'figures/energy.png', dpi=500, bbox_inches='tight')

print('continuous')
for i in range(len(cont[0])):
    s = 0
    cnt = 0
    for l in cont:
        if l[i] != 0:
            s += (1 - 1.0 / l[i])
            cnt += 1
    print(round(100.0 * s / cnt, 2))

print('intermittent')
for i in range(len(inter[0])):
    s = 0
    cnt = 0
    for l in inter:
        if l[i] != 0:
            s += (1 - 1.0 / l[i])
            cnt += 1
    print(round(100.0 * s / cnt, 2))
