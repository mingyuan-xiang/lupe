import matplotlib.pyplot as plt
import matplotlib.patches as mpatches
import matplotlib.ticker as ticker
import json
import numpy as np

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

fig, axs = plt.subplots(1, len(results), figsize=(20, 5))

def disable_tick(t):
    t.label1.set_visible(False)
    t.tick1line.set_markersize(0)
    t.tick2line.set_markersize(0)

total_times = []

def plot(n, r, ax):
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

    ax.yaxis.set_major_locator(ticker.MaxNLocator(integer=True))

    ax.set_xticklabels([])
    ax.xaxis.set_tick_params(length=0)

    if n == 'ResNet3':
        ax.set_ylim([0, 11])
        ax.set_ylabel('Normalized Speedup')
        ax.grid(axis='y', zorder=0)
    elif n == 'DS_CNN':
        ax.set_ylim([0, 30])
        ax.yaxis.tick_right()
        ax.yaxis.set_label_position("right")
        ax.grid(axis='y', which='major', zorder=0)
        ax.set_facecolor('gainsboro')
    else:
        ax.set_ylim([0, 11])
        ax.set_yticklabels([])
        ax.yaxis.set_tick_params(length=0)
        ax.grid(axis='y', zorder=0)

    if n == 'DS_CNN':
        n = 'DS-CNN (Right Axis)'
    ax.text(0.5, -0.015, n, transform=ax.transAxes, 
        fontsize=16, verticalalignment='top', horizontalalignment='center')

cnt = 0

model_order = ['ResNet3', 'MobileNetV2', 'LeNet', 'MLPClassifier', 'DS_CNN']

results = {key: results[key] for key in model_order}

for name, m in results.items():
    plot(name, m, axs[cnt])
    cnt += 1

l = []
for _, f in opt_flags.items():
    p = mpatches.Patch(facecolor=f[1], hatch=f[2],label=f[0])
    l.append(p)

fig.legend(handles=l, loc='lower center', ncol=7, fontsize=16, bbox_to_anchor=(0.5, -0.018), edgecolor='black')

plt.subplots_adjust(wspace=0)

fig.tight_layout(pad=0.05, rect=[0, 0.085, 1, 1])

plt.savefig(f'figures/opt_perf_continous.png', dpi=500)

print(total_times)
for i in range(len(total_times[0])):
    s = 0
    for l in total_times:
        s += l[i]
    print(round(s / len(total_times), 2))