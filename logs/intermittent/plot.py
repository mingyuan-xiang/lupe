import matplotlib.pyplot as plt
import matplotlib.patches as mpatches
import matplotlib.ticker as ticker
import json
import numpy as np

opt_flags = {
    'no_opt' : ('Bottom-up', 'pink', 'xx'),
    'hawaii' : ('HAWAII',  'royalblue', '--'),
    'sonic' : ('TAILS',  'royalblue', '++'),
    'dma_lea_opt_adaptive_buffer_mem_acc' : ('Lupe',  'firebrick', '\\\\'),
}

restart_map = {
    '(0, 0)' : 'Continuous Power',
    '(164, 328)' : 'Recharge Frequency 5 - 10 ms',
    '(1311, 1638)' : 'Recharge Frequency 40 - 50 ms',
    '(2949, 3276)' : 'Recharge Frequency 90 - 100 ms',
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

models = ['MLPClassifier', 'LeNet', 'ResNet3', 'MobileNetV2', 'DS_CNN']
data_ranges = ['(0, 0)', '(164, 328)', '(1311, 1638)', '(2949, 3276)']

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
            ratio = (intermittent_time - continuous_time) / intermittent_time
            data_dict[model][data_range][config] = {
                'intermittent_time': intermittent_time,
                'continuous_time': continuous_time,
                'intermittent_time_bar_value': intermittent_time - continuous_time,
                'ratio': ratio
            }

fig = plt.figure(figsize=(20, 25))

subfigs = fig.subfigures(nrows=5, ncols=1)

for i, (subfig, model) in enumerate(zip(subfigs, models)):
    axs = subfig.subplots(nrows=1, ncols=4, gridspec_kw={'wspace': 0})

    for j, (ax, data_range) in enumerate(zip(axs, data_ranges)):
        configs = data_dict.get(model, {}).get(data_range, {})

        intermittent_time_bar_values = []
        continuous_times = []
        ratios = []
        for config in config_list:
            config_values = configs.get(config, {})
            intermittent_time_bar_value = config_values.get('intermittent_time_bar_value', None)
            continuous_time = config_values.get('continuous_time', None)
            ratio = config_values.get('ratio', None)

            intermittent_time_bar_values.append(intermittent_time_bar_value)
            continuous_times.append(continuous_time)
            ratios.append(ratio)

        x_indices = np.arange(len(config_list))

        intermittent_bar_values = [0 if v is None else v for v in intermittent_time_bar_values]
        continuous_bar_values = [0 if v is None else v for v in continuous_times]
        total_bar_values = [i + c for i, c in zip(intermittent_bar_values, continuous_bar_values)]

        ax.bar(x_indices, continuous_bar_values, label='Continuous Time', color='skyblue')
        ax.bar(x_indices, intermittent_bar_values, bottom=continuous_bar_values, label='Intermittent Overhead', color='orange')
        if i == 0:
            ax.set_title(restart_map[data_range])

        ax2 = ax.twinx()
        ratio_values = [np.nan if v is None else v for v in ratios]
        ax2.plot(x_indices, ratio_values, marker='o', color='red', label='Ratio')

        if data_range != '(0, 0)':
            ax.set_yticklabels([])
            ax.yaxis.set_tick_params(length=0)
        else:
            ax.set_ylabel('Time (s)')

        if data_range != '(2949, 3276)':
            ax2.set_yticklabels([])
            ax2.yaxis.set_tick_params(length=0)
        else:
            ax2.set_ylabel('Intermittent Overhead (%)')

    subfig.supxlabel(model, fontsize=14, y=0.02)

# plt.show()
plt.savefig(f'figures/opt_perf_intermittent.png', dpi=500)
