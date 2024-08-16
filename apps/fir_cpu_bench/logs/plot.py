import re
from math import log2

import matplotlib.pyplot as plt
import seaborn as sns
import pandas as pd
import numpy as np

freq = 32767

def parse_data(data):
    pattern = r"total cycles \((?P<frequency>\d+ Hz)\), repeat for (?P<repeat>\d+) times: (?P<time>\d+) \(kernel size: (?P<kernel_size>\d+), in_channels: (?P<in_channels>\d+), out_channels: (?P<out_channels>\d+), input size: (?P<input_size>\d+)\)"
    matches = re.findall(pattern, data, flags=re.DOTALL)
    results = []
    for _, repeat, cycles, kernel_size, in_channels, out_channels, input_size in matches:
        k = int(kernel_size)
        i_ch = int(in_channels)
        o_ch = int(out_channels)
        i_s = int(input_size)
        o_s = i_s - k + 1

        results.append({
            'time': int(cycles) / (int(repeat) * freq),
            'flops': k * k * i_ch * o_s * o_s * o_ch / (10**6),
        })

    return results

with open(f'fir_conv.log') as f:
    fir_data = f.read()
    fir_data = parse_data(fir_data)
fir_df = pd.DataFrame(fir_data)

with open(f'cpu_conv.log') as f:
    cpu_data = f.read()
    cpu_data = parse_data(cpu_data)
cpu_df = pd.DataFrame(cpu_data)

plt.rcParams["font.family"] = "Times New Roman"
plt.rcParams.update({'font.size': 16})
plt.rcParams["font.weight"] = "bold"
plt.rcParams["axes.labelweight"] = "bold"

fig = plt.figure(figsize=(8, 3))
fig.add_subplot(111)

plt.scatter(fir_df['flops'], fir_df['time'], s=15, label='LEA (FIR)')
plt.scatter(cpu_df['flops'], cpu_df['time'], s=15, label='CPU')

plt.xlabel('MegaFLOPs')
plt.ylabel('Time (s)')

plt.legend(markerscale=2)

fig.tight_layout(pad=0.1)

# plt.show()
plt.savefig(f'flops_time.png', dpi=500)
