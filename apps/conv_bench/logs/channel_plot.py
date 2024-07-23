import re
from math import log2

import matplotlib.pyplot as plt
import seaborn as sns
import pandas as pd
import numpy as np

freq = 32767

io = 'in'
op = 'fir'
kernel_size = 5

def parse_data(data):
    pattern = r"total cycles \((?P<frequency>\d+ Hz)\), repeat for (?P<repeat>\d+) times: (?P<time>\d+) \(kernel size: (?P<kernel_size>\d+), in_channels: (?P<in_channels>\d+), out_channels: (?P<out_channels>\d+), input size: (?P<input_size>\d+)\)"
    matches = re.findall(pattern, data, flags=re.DOTALL)
    results = []
    for _, repeat, cycles, kernel_size, in_channels, out_channels, input_size in matches:
        if io == 'in':
            ch = in_channels
        else:
            ch = out_channels
        results.append({
            'time': int(cycles) / (int(repeat) * freq),
            f'{io}_channels': int(ch),
            'input_size': int(input_size),
        })

    return results

with open(f'{op}_conv_{kernel_size}.log') as f:
    data = f.read()
    data = parse_data(data)

plt.rcParams["font.family"] = "Times New Roman"
plt.rcParams.update({'font.size': 14})
plt.rcParams["font.weight"] = "bold"
plt.rcParams["axes.labelweight"] = "bold"

df = pd.DataFrame(data)

fig = plt.figure()
fig.add_subplot(111)

scatter = plt.scatter(df['input_size'], df['time'], c=df[f'{io}_channels'], cmap='cool', s=100)

if io == 'in':
    plt.colorbar(scatter, label='Input Channels')
else:
    plt.colorbar(scatter, label='Output Channels')

plt.xlabel('Input Size')
plt.ylabel('Time (s)')
plt.title(f'Convolution benchmark (Kernel Size: {kernel_size})')

fig.tight_layout(pad=0.1)
plt.savefig(f'figures/{op}_{io}_channels_{kernel_size}.png', dpi=500)
