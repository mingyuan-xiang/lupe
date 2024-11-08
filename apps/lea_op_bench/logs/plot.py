import re
from collections import defaultdict
import matplotlib.pyplot as plt
import matplotlib.ticker as ticker
import numpy as np
from sklearn.linear_model import LinearRegression

def parse_log(log):
    benchmarks = defaultdict(list)
    current_benchmark = None

    for line in log.splitlines():
        if "benchmark" in line:
            current_benchmark = re.search(r'benchmark (.*) ', line).group(1).strip()
        elif "size of" in line:
            size = int(re.search(r'size of (\d+)', line).group(1))
            benchmarks[current_benchmark].append({'size': size})
        elif "invocation time" in line:
            invocation_time = int(re.search(r'invocation time: (\d+)', line).group(1))
            benchmarks[current_benchmark][-1]['core time'] = invocation_time

            other_time = int(re.search(r'other time: (\d+)', line).group(1))
            benchmarks[current_benchmark][-1]['set-up time'] = other_time
        elif "DMA time" in line:
            dma_time = int(re.search(r'DMA time: (\d+)', line).group(1))
            benchmarks[current_benchmark][-1]['core time'] = dma_time
        elif "total time" in line:
            total_time = int(re.search(r'total time: (\d+)', line).group(1))
            benchmarks[current_benchmark][-1]['total time'] = total_time

    for d in benchmarks['DMA']:
        d['set-up time'] = d['total time'] - d['core time']

    for v in benchmarks:
        for d in benchmarks[v]:
            d['total time'] = d['set-up time'] + d['core time']

    return benchmarks

with open('log', 'r') as file:
    log = file.read()

data = parse_log(log)

log_data = {}
for d in data:
    if d in ('msp_fir_q15', 'msp_add_q15'):
        log_data[d] = data[d]

bar_width = 2.5
plt.rcParams["font.family"] = "Times New Roman"
plt.rcParams.update({'font.size': 14})
plt.rcParams["font.weight"] = "bold"
plt.rcParams["axes.labelweight"] = "bold"

n_subplots = len(log_data)

fig, axes = plt.subplots(n_subplots, 1, figsize=(8, n_subplots * 1.2))

def disable_tick(t):
    t.label1.set_visible(False)
    t.tick1line.set_markersize(0)
    t.tick2line.set_markersize(0)

for i, (key, values) in enumerate(log_data.items()):
    sizes = [entry['size'] for entry in values]
    core_times = [entry['core time'] for entry in values]
    setup_times = [entry['set-up time'] for entry in values]
    total_times = [entry['total time'] for entry in values]

    norm_factor = total_times[0]

    norm_core_times = [core_time / norm_factor for core_time in core_times]
    norm_setup_times = [setup_time / norm_factor for setup_time in setup_times]

    # Use linear regression to get the intercept
    y = np.array(norm_core_times)
    x = np.array(sizes).reshape(-1, 1)
    model = LinearRegression()
    model.fit(x, y)
    intercept = model.intercept_

    constant = [intercept] * len(sizes)
    real_norm_core_times = [t - intercept for t in norm_core_times]

    st_time = None
    core_label = None
    total_label = None
    if key == 'msp_fir_q15':
        st_time = 'Invocation Time'
        core_label = 'Computation Time'
        total_label = 'SRAM Preparation Time'

    bars1 = axes[i].bar(sizes, real_norm_core_times, bar_width, label=core_label, color='royalblue', hatch='oo')
    bars2 = axes[i].bar(sizes, constant, bar_width, label=st_time, bottom=real_norm_core_times, color='firebrick', hatch='++')
    bars3 = axes[i].bar(sizes, norm_setup_times, bar_width, bottom=norm_core_times, label=total_label, color='pink')

    axes[i].yaxis.set_major_formatter(ticker.FormatStrFormatter('%.1f'))
    axes[i].yaxis.set_major_locator(ticker.MaxNLocator(integer=True))

    norm_total_times = [total_time / norm_factor for total_time in total_times]
    max_time = max(norm_total_times)
    axes[i].set_ylim([0, max_time + 0.5])

    # if key == 'msp_fill_q15':
    #     yticks = axes[i].yaxis.get_major_ticks()
    #     disable_tick(yticks[2])

    # if key == 'msp_mac_q15':
    #     yticks = axes[i].yaxis.get_major_ticks()
    #     disable_tick(yticks[1])

    if key == 'msp_add_q15':
        yticks = axes[i].yaxis.get_major_ticks()
        disable_tick(yticks[1])

    if key == 'msp_fir_q15':
        yticks = axes[i].yaxis.get_major_ticks()
        disable_tick(yticks[1])
        disable_tick(yticks[3])
        disable_tick(yticks[5])

    # if key == 'msp_offset_q15':
    #     yticks = axes[i].yaxis.get_major_ticks()
    #     disable_tick(yticks[2])

    # if key == 'msp_mpy_q15':
    #     yticks = axes[i].yaxis.get_major_ticks()
    #     disable_tick(yticks[1])

    if key != 'msp_fir_q15':
        axes[i].set_xticklabels([])
        axes[i].xaxis.set_tick_params(length=0)

        yticks = axes[i].yaxis.get_major_ticks()
        disable_tick(yticks[0])
        lines = axes[i].axvline(x=52, color='orange', linestyle='--')
    else:
        axes[i].set_xlabel('Input Size')

        x_sizes = []
        for s in range(len(sizes)):
            if s % 2 == 0:
                x_sizes.append(sizes[s])
        x_sizes.append(52)

        axes[i].set_xticks(x_sizes)
        axes[i].set_xticklabels(x_sizes, fontsize=12)

        yticks = axes[i].yaxis.get_major_ticks()
        disable_tick(yticks[3])
        disable_tick(yticks[1])

        current_ticks = axes[i].get_xticks()
        new_ticks = list(current_ticks) + [52]  # Add a new tick at 2.5
        axes[i].set_xticks(new_ticks)
        lines = axes[i].axvline(x=52, color='orange', linestyle='--', label='Max Input Size')

    axes[i].text(0.5, 0.85, key, ha='center', va='center', transform=axes[i].transAxes, 
        fontsize=12, bbox=dict(facecolor='white', edgecolor='gainsboro', boxstyle='round,pad=0.1'))

    axes[i].tick_params(axis='y', labelsize=12)

fig.legend(
    handles=[lines, bars3, bars2, bars1],
    loc='lower center', ncol=4,
    fontsize=11, bbox_to_anchor=(0.50, -0.03),
    edgecolor='black'
)
fig.tight_layout(pad=0.05, rect=[0.03, 0.12, 1, 1])

fig.text(0.0, 0.65, 'Normalized Time', va='center', rotation='vertical', fontsize=12)

plt.subplots_adjust(hspace=0)

plt.savefig(f"lea_perf.png", dpi=500)
