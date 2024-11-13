import matplotlib.pyplot as plt
import matplotlib.patches as mpatches
import matplotlib.ticker as ticker
import json
import numpy as np

opt_flags = {
    'Tails' : ('Tails',  'royalblue', '++'),
    'no_opt' : ('Bottom-up', 'pink', 'xx'),
    'Hawaii' : ('Hawaii',  'royalblue', '--'),
    'dma_lea_opt_adaptive_buffer_mem_acc' : ('Lupe',  'firebrick', '\\\\'),
}

restart_map = {
    '(0, 0)' : 'No Reboot',
    '(2500, 5000)' : '5 - 10 ms',
    '(20000, 25000)' : '40 - 50 ms',
    '(45000, 50000)' : '90 - 100 ms',
}

y_limits = {
    'MLPClassifier' : ((0, 0.5), np.arange(0, 0.5, 0.1)),
    'LeNet' : ((0, 6), np.arange(0, 6, 1)),
    'ResNet3' : ((0, 60), np.arange(0, 60, 8)),
    'MobileNetV2' : ((0, 35), np.arange(0, 35, 5)),
    'DS_CNN' : ((0, 95), np.arange(0, 100, 10)),
}

cmap = plt.get_cmap('cool')
N = len(opt_flags)
colors = cmap(np.linspace(0, 1, N))
for (key, (label, _, marker)), color in zip(opt_flags.items(), colors):
    opt_flags[key] = (label, color, marker)

results_json = 'results.json'
with open(results_json, 'r') as f:
    results = json.load(f)

width = 0.5
plt.rcParams["font.family"] = "Times New Roman"
plt.rcParams.update({'font.size': 16})
plt.rcParams["font.weight"] = "bold"
plt.rcParams["axes.labelweight"] = "bold"

models = ['ResNet3', 'DS_CNN', 'MobileNetV2', 'DS_CNN', 'LeNet', 'MLPClassifier']
data_ranges = ['(2500, 5000)', '(20000, 25000)', '(45000, 50000)', '(0, 0)']

reordered_results = {}
for r in data_ranges:
    reordered_results[r] = {}
    for m in models:
        reordered_results[r][m] = results[r][m]

data_dict = {}

config_list = list(opt_flags.keys())

for data_range, models in reordered_results.items():
    for model, entries in models.items():
        if model not in data_dict:
            data_dict[model] = {}
        if data_range not in data_dict[model]:
            data_dict[model][data_range] = {}
        for entry in entries:
            config = entry['config']
            intermittent_time = entry['intermittent_time']
            continuous_time = entry['continuous_time']
            ratio = 100.0 * (intermittent_time - continuous_time) / intermittent_time
            data_dict[model][data_range][config] = {
                'intermittent_time': intermittent_time,
                'continuous_time': continuous_time,
                'intermittent_time_bar_value': intermittent_time - continuous_time,
                'ratio': ratio
            }

fig = plt.figure(figsize=(10, 15), constrained_layout=True)

subfigs = fig.subfigures(nrows=5, ncols=1)

speedup = {}
overhead = {}
for d in data_ranges:
    speedup[d] = []
    overhead[d] = []

for i, (subfig, model) in enumerate(zip(subfigs, models)):
    axs = subfig.subplots(nrows=1, ncols=4, gridspec_kw={'wspace': 0})

    for j, (ax, data_range) in enumerate(zip(axs, data_ranges)):
        configs = data_dict.get(model, {}).get(data_range, {})

        intermittent_time_bar_values = []
        continuous_times = []
        colors = []
        hatches = []
        for config in config_list:
            config_values = configs.get(config, {})
            intermittent_time_bar_value = config_values.get('intermittent_time_bar_value', None)
            continuous_time = config_values.get('continuous_time', None)

            intermittent_time_bar_values.append(intermittent_time_bar_value)
            continuous_times.append(continuous_time)

            colors.append(opt_flags[config][1])
            hatches.append(opt_flags[config][2])

        x_indices = np.arange(len(config_list))

        intermittent_bar_values = [0 if v is None else v for v in intermittent_time_bar_values]
        continuous_bar_values = [0 if v is None else v for v in continuous_times]
        total_bar_values = [i + c for i, c in zip(intermittent_bar_values, continuous_bar_values)]

        speedup[data_range].append(total_bar_values)
        overhead[data_range].append(intermittent_bar_values)

        bars = ax.bar(x_indices, continuous_bar_values, width, label=config_list, color=colors, zorder=3)
        for bar, hatch in zip(bars, hatches):
            bar.set_hatch(hatch)
        ax.bar(x_indices, intermittent_bar_values, width, bottom=continuous_bar_values, label='Intermittent Overhead', color='darkgrey', zorder=3)

        if model == 'MobileNetV2':
            ax.scatter(2, 1.75, marker='X', s=150, color='red')

        if i == 0:
            ax.set_title(restart_map[data_range], fontweight='bold')

        if data_range != data_ranges[0]:
            ax.set_yticklabels([])
            ax.yaxis.set_tick_params(length=0)
        else:
            ax.set_ylabel('Latency Per Inference (s)')

        ax.grid(axis='y', color='grey', zorder=0)

        ax.set_ylim(*y_limits[model][0])
        ax.set_yticks(y_limits[model][1])
        ax.set_xticklabels([])
        ax.xaxis.set_tick_params(length=0)

    if model == 'DS_CNN':
        subfig.supxlabel('DS-CNN', y=-0.05, x=0.65, fontweight='bold')
    else:
        subfig.supxlabel(model, y=-0.05, x=0.65, fontweight='bold')

l = []
for _, f in opt_flags.items():
    n = f[0]
    if n == 'Bottom-up':
        n = 'Lupe-BT'
    p = mpatches.Patch(facecolor=f[1], hatch=f[2],label=n)
    l.append(p)

l.append(mpatches.Patch(facecolor='darkgrey',label='Intermittent Support Overhead'))

fig.legend(handles=l, loc='lower center', ncol=3, fontsize=14, bbox_to_anchor=(0.615, -0.057), edgecolor='black')

fig.tight_layout(pad=0.05, rect=[0, 0.05, 1, 0.95])

# plt.show()
plt.savefig(f'figures/opt_perf_intermittent.png', dpi=500, bbox_inches='tight')

print('lupe speedup')
for i in range(3):
    s = 0
    cnt = 0
    for _, d in speedup.items():
        for l in d:
            s += l[i] / l[3]
            if l[i] != 0:
                cnt += 1
    print(round(s / cnt, 2))

print('lupe overhead')
for i in range(3):
    s = 0
    cnt = 0
    for _, d in overhead.items():
        for l in d:
            if l[i] != 0:
                s += (1 - l[3] / l[i])
                cnt += 1
    print(round(100.0 * s / cnt, 2))

print('polaris overhead (tails)')
s = 0
cnt = 0
for _, d in overhead.items():
    for l in d:
        if l[0] != 0:
            s += (1 - l[1] / l[0])
            cnt += 1
print(round(100.0 * s / cnt, 2))
