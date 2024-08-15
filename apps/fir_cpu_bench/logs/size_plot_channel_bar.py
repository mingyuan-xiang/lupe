import re
from math import log2

import matplotlib.pyplot as plt
import seaborn as sns
import pandas as pd
import numpy as np

freq = 32767
kernel_size = 3

def parse_data(data):
    pattern = r"total cycles \((?P<frequency>\d+ Hz)\), repeat for (?P<repeat>\d+) times: (?P<time>\d+) \(kernel size: (?P<kernel_size>\d+), in_channels: (?P<in_channels>\d+), out_channels: (?P<out_channels>\d+), input size: (?P<input_size>\d+)\)"
    matches = re.findall(pattern, data, flags=re.DOTALL)
    results = []
    for _, repeat, cycles, kernel_size, in_channels, out_channels, input_size in matches:
        results.append({
            'time': log2(int(cycles) / (int(repeat) * freq)),
            'kernel_size': int(kernel_size),
            # 'in_channels': int(in_channels),
            # 'out_channels': int(out_channels),
            'channels': int(out_channels) + int(in_channels),
            'input_size': int(input_size),
        })

    return results

with open(f'mac_conv_{kernel_size}.log') as f:
    data = f.read()
    mac_data = parse_data(data)

with open(f'fir_conv_{kernel_size}.log') as f:
    data = f.read()
    fir_data = parse_data(data)

plt.rcParams["font.family"] = "Times New Roman"
plt.rcParams.update({'font.size': 14})
plt.rcParams["font.weight"] = "bold"
plt.rcParams["axes.labelweight"] = "bold"

mac_df = pd.DataFrame(mac_data)
fir_df = pd.DataFrame(fir_data)
tmp_df = pd.merge(fir_df, mac_df, on=['kernel_size', 'channels', 'input_size'], suffixes=('_fir', '_mac'))
df = tmp_df.copy()
df['time'] = tmp_df['time_fir'] - tmp_df['time_mac']
df.drop(columns=['time_fir', 'time_mac'], inplace=True)

fig = plt.figure()
fig.add_subplot(111)

scatter = plt.scatter(df['input_size'], df['time'], c=df['channels'], cmap='cool', s=10)
plt.colorbar(scatter, label='Input Channels + Output Channels')

plt.axhline(y=0, color='red', linestyle='-')

plt.xlabel('Input Size')
plt.ylabel('(log2) Time Differences')
plt.title(f'Convolution benchmark (Kernel Size: {kernel_size})')

fig.tight_layout(pad=0.1)
plt.savefig(f'figures/fir_vs_mac_{kernel_size}_channel_bar.png', dpi=500)


