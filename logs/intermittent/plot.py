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

model_list = ['MLPClassifier', 'LeNet', 'ResNet3', 'MobileNetV2', 'DS_CNN']

for range_key, models in results.items():
    for model_name in model_list:
        fig, ax = plt.subplots(figsize=(6, 5))

        model_data = models[model_name]

        configs = []
        intermittent_times = []
        continuous_times = []
        overhead_times = []
        restarts = []
        colors = [opt_flags[k][1] for k in opt_flags]
        hatches = [opt_flags[k][2] for k in opt_flags]

        intermittent_times = {config: 0 for config in opt_flags}
        continuous_times = {config: 0 for config in opt_flags}
        overhead_times = {config: 0 for config in opt_flags}
        restarts = {config: 0 for config in opt_flags}

        for entry in model_data:
            config = entry['config']
            configs.append(opt_flags[entry['config']][0])
            intermittent_time = entry['intermittent_time']
            continuous_time = entry['continuous_time']
            overhead_time = intermittent_time - continuous_time
            restart = entry['restart']

            intermittent_times[config] = intermittent_time
            continuous_times[config] = continuous_time
            overhead_times[config] = overhead_time
            restarts[config] = restart

        cont_times = [continuous_times[config] for config in opt_flags]
        over_times = [overhead_times[config] for config in opt_flags]
        restarts_list = [restarts[config] for config in opt_flags]

        x = np.arange(len(opt_flags))

        ax.yaxis.set_major_formatter(ticker.FormatStrFormatter('%.1f'))

        if model_name == 'MLPClassifier':
            ax.set_ylim(0, 0.2)
            ax.yaxis.set_major_locator(ticker.MultipleLocator(0.1))

        bars = ax.bar(x, cont_times, width, color=colors)
        for bar, hatch in zip(bars, hatches):
            bar.set_hatch(hatch)

        ax.bar(x, over_times, width, bottom=cont_times, color='darkgray')

        if range_key != '(0, 0)':
            ax2 = ax.twinx()
            ax2.yaxis.set_major_formatter(ticker.FormatStrFormatter('%.1f'))
            ax2.plot(x, restarts_list, color='red', marker='o', linestyle='-', label='Restart')
            ax2.set_ylabel('Recharge Times')

        x_labels = [opt_flags[k][0] for k in opt_flags]

        # Set labels and titles
        ax.set_xticks(x)
        ax.set_xticklabels(x_labels)
        ax.set_ylabel('Time (s)')
        ax.set_title(model_name)

        fig.tight_layout(pad=0.05, rect=[0, 0, 1, 1])

        plt.savefig(f'figures/{restart_map[range_key]}_{model_name}.png', dpi=500)
