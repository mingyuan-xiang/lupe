import re
from math import log2

import matplotlib.pyplot as plt
import seaborn as sns
import pandas as pd
import numpy as np
import matplotlib.ticker as ticker

freq = 32767
flops_threshold = 1

model_flops = {
    'LeNet' : (0.417, 25),
    'ResNet3' : (5.256, 25),
    'MobileNetV2' : (1.115, 25),
    'DS-CNN' : (3.049, 25),
    'MLPClassifier' : (0.165, 25),
}

def parse_data(data, base_results):
    pattern = r"total cycles \((?P<frequency>\d+ Hz)\), repeat for (?P<repeat>\d+) times: (?P<time>\d+) \(kernel size: (?P<kernel_size>\d+), in_channels: (?P<in_channels>\d+), out_channels: (?P<out_channels>\d+), input size: (?P<input_size>\d+)\)"
    matches = re.findall(pattern, data, flags=re.DOTALL)
    results = []
    for _, repeat, cycles, kernel_size, in_channels, out_channels, input_size in matches:
        k = int(kernel_size)
        i_ch = int(in_channels)
        o_ch = int(out_channels)
        i_s = int(input_size)
        o_s = i_s - k + 1

        time = float(cycles) / (int(repeat) * freq)
        flops = float(k * k * i_ch * o_s * o_s * o_ch) / (10**6)

        if flops > flops_threshold:
            continue

        if base_results is not None:
            for i in base_results:
                if i['shape'] == (k, o_s, i_ch, o_ch):
                    time /= i['time']
                    break

        results.append({
            'time': time,
            'flops': flops,
            'shape': (k, o_s, i_ch, o_ch),
        })

    return results

with open(f'mac_conv.log') as f:
    mac_data = f.read()
    mac_data = parse_data(mac_data, None)
mac_df = pd.DataFrame(mac_data)

with open(f'cpu_conv.log') as f:
    cpu_data = f.read()
    cpu_data = parse_data(cpu_data, mac_data)
cpu_df = pd.DataFrame(cpu_data)

with open(f'fir_conv.log') as f:
    fir_data = f.read()
    fir_data = parse_data(fir_data, mac_data)
fir_df = pd.DataFrame(fir_data)

mac_df['time'] = 1.0

plt.rcParams["font.family"] = "Times New Roman"
plt.rcParams.update({'font.size': 16})
plt.rcParams["font.weight"] = "bold"
plt.rcParams["axes.labelweight"] = "bold"

fig = plt.figure(figsize=(8, 3))
fig.add_subplot(111)

plt.scatter(cpu_df['flops'], cpu_df['time'], s=15, alpha=1, color='orange', marker='^', label='CPU')
plt.scatter(mac_df['flops'], mac_df['time'], s=15, alpha=1, color='mediumseagreen', marker='x', label='MAC (LEA)')
plt.scatter(fir_df['flops'], fir_df['time'], s=15, alpha=1, color='crimson', marker='o', label='FIR (LEA)')

# for m, f in model_flops.items():
#     plt.axvline(x=f[0], color='black', linestyle='--')
#     plt.text(f[0], f[1], m, rotation=90, verticalalignment='bottom', horizontalalignment='right')

plt.xlabel('MegaFLOPs')
plt.ylabel('Normalized to MAC')

plt.legend(fontsize=12, markerscale=2, ncol=3, loc='upper center', bbox_to_anchor=(0.5, 1.19), framealpha=1)
plt.gca().yaxis.set_major_formatter(ticker.FormatStrFormatter('%.1f'))

fig.tight_layout(pad=0.05)

# plt.show()
plt.savefig(f'flops_time.png', dpi=500)
