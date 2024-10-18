import json
import numpy as np
import matplotlib.pyplot as plt
import matplotlib.ticker as ticker
import matplotlib.patches as mpatches

opt_flags = {
    'no_opt' : ('No Optimization', 'pink', 'xx'),
    'dma' : ('DMA Optimization',  'deepskyblue', '||'),
    'dma_lea_opt' : ('LEA Optimization',  'mediumspringgreen', '--'),
    'dma_lea_opt_adaptive_buffer_mem' : ('Adaptive Data Movement',  'royalblue', '\\\\'),
    'dma_lea_opt_adaptive_buffer_mem_batched' : ('Batched Acceleration',  'royalblue', '++'),
    'dma_lea_opt_adaptive_buffer_mem_acc' : ('Adaptive Generation',  'firebrick', '//'),
}

cmap = plt.get_cmap('cool')
N = len(opt_flags)
colors = cmap(np.linspace(0, 1, N))
for (key, (label, _, marker)), color in zip(opt_flags.items(), colors):
    opt_flags[key] = (label, color, marker)

results_json = 'results.json'
with open(results_json, 'r') as f:
    results = json.load(f)

bar_width = 0.5
plt.rcParams["font.family"] = "Times New Roman"
plt.rcParams.update({'font.size': 16})
plt.rcParams["font.weight"] = "bold"
plt.rcParams["axes.labelweight"] = "bold"

def get_results(r, flag):
    for d in r:
        if d['optimization_flags'] == flag:
            return d
    return None

def get_min_time(times):
    min_time = float("inf")
    for t in times:
        if min_time > t and t > 0:
            min_time = t
    return min_time

fig = plt.figure(figsize=(25, 8))
model_order = ['ResNet3', 'MobileNetV2', 'LeNet', 'MLPClassifier', 'DS_CNN']
subfigs = fig.subfigures(nrows=1, ncols=len(model_order))

total_times = []

for idx, (model, subfig) in enumerate(zip(model_order, subfigs)):
    ax = subfig.subplots()
    r = results[model]
    
    times = []
    nums = list(range(len(opt_flags)))
    colors = []
    hatches = []
    for flag, setting in opt_flags.items():
        d = get_results(r, flag)
        if d is not None:
            times.append(d['total_cycles'] / d['total_images'])
        else:
            times.append(0)
        colors.append(setting[1])
        hatches.append(setting[2])

    prev = times[0]
    sp = [prev / times[-1]]
    for t in times[1:]:
        sp.append(prev / t)
        prev = t
    total_times.append(sp)

    min_time = get_min_time(times)
    times = [t / min_time for t in times]

    bars = ax.bar(nums, times, bar_width, color=colors, zorder=3)

    for bar, hatch in zip(bars, hatches):
        bar.set_hatch(hatch)

    ax.set_xticklabels([])
    ax.xaxis.set_tick_params(length=0)

    # if model == 'ResNet3':
    #     ax.set_ylim([0, 11])
    #     ax.set_ylabel('Normalized Speedup')
    #     ax.grid(axis='y', color='grey', zorder=0)
    # elif model == 'DS_CNN':
    #     ax.set_ylim([0, 30])
    #     ax.yaxis.tick_right()
    #     ax.yaxis.set_label_position("right")
    #     ax.grid(axis='y', color='grey', which='major', zorder=0)
    #     ax.set_facecolor('silver')
    # else:
    #     ax.set_ylim([0, 11])
    #     ax.set_yticklabels([])
    #     ax.grid(axis='y', color='grey', zorder=0)

    if model == 'DS_CNN':
        model = 'DS-CNN (Right Axis)'

    subfig.supxlabel(model, y=0.1, x=0.58, fontweight='bold')

l = []
for _, f in opt_flags.items():
    p = mpatches.Patch(facecolor=f[1], hatch=f[2], label=f[0])
    l.append(p)

fig.legend(handles=l, loc='lower center', ncol=7, fontsize=16, bbox_to_anchor=(0.5, -0.02), edgecolor='black')

fig.tight_layout(pad=0.05, rect=[0, 0.05, 1, 0.95])

plt.savefig(f'figures/opt_perf_continous.png', dpi=500)

print(total_times)
for i in range(len(total_times[0])):
    s = 0
    for l in total_times:
        s += l[i]
    print(round(s / len(total_times), 2))
