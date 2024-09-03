import re
import matplotlib.pyplot as plt
import numpy as np

def parse_log_data(filename):
    results = {}

    with open(filename, 'r') as file:
        log_data = file.read()

    method_patterns = re.finditer(r"===+\s*(?P<method>[^=]+)===+", log_data)

    methods = []
    for match in method_patterns:
        methods.append((match.group('method').strip(), match.end()))

    for i, m in enumerate(methods):
        method_name, start_index = m
        end_index = len(log_data) if i == len(methods) - 1 else methods[i + 1][1] - 1
        section_data = log_data[start_index:end_index]

        size_time_dict = {}

        entries = re.findall(r"\(REPEAT=\d+\) .+; size: (\d+), time: (\d+)", section_data)
        for size, time in entries:
            size_time_dict[int(size)] = int(time)

        results[method_name] = size_time_dict

    return results

parsed_data = parse_log_data('log')

# plot the logs
plt.rcParams["font.family"] = "Times New Roman"
plt.rcParams.update({'font.size': 14})

fig, ax = plt.subplots()
ax.set_xlabel('Words (2 Btyes)')
ax.set_ylabel('Time (MicroSec)')
ax.set_xlim([0, 100])
ax.set_ylim([0, 180])

freq = ((2 ** 15 - 1) * 10000) / 1000000

for method, data in parsed_data.items():
    sizes = sorted(data.keys())
    times = [data[size] / freq for size in sizes]
    ax.plot(sizes, times, label=method)

# Find the intersection of 'DMA (Optimized)' and 'Loop Assign (Unrolled)'
def regression(data):
    sizes = np.array(sorted(data.keys()))
    times = np.array([data[size] for size in sizes]) / freq
    return np.polyfit(sizes, times, 1)

regression_results = {}
for method, data in parsed_data.items():
    slope, intercept = regression(data)
    regression_results[method] = (slope, intercept)

slope1, intercept1 = regression_results['DMA (Optimized)']
slope2, intercept2 = regression_results['Loop Copy (Unrolled)']
x_intersection = int((intercept2 - intercept1) / (slope1 - slope2))
y_intersection = slope1 * x_intersection + intercept1

ax.plot(x_intersection, y_intersection, 'ro')
ax.plot([x_intersection, x_intersection], [y_intersection, 0], 'gray', linestyle='dashed')

# Print the x-coordinate of the intersection directly on the plot
ax.text(x_intersection, -0.026, f'{x_intersection}', horizontalalignment='center', verticalalignment='top', transform=ax.get_xaxis_transform())

ax.legend()

plt.xticks(np.arange(0, 100, 30))

plt.savefig('data_movement.png', dpi=500)
