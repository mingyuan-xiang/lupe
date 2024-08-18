import matplotlib.pyplot as plt
import numpy as np

# Data provided
log = {
    "input size: 5" : {
        "fir": {
            "kernel size: 3" : { "total time" : 333424, "compute time" : 60122 },
            "kernel size: 5" : { "total time" : 197985, "compute time" : 31778 },
        },
        "mac": {
            "kernel size: 3" : { "total time" : 210341, "compute time" : 30870 },
            "kernel size: 5" : { "total time" : 57467, "compute time" : 7106 },
        }
    },
    "input size: 10" : {
        "fir": {
            "kernel size: 3" : { "total time" : 882501, "compute time" : 181937 },
            "kernel size: 5" : { "total time" : 1049447, "compute time" : 220027 },
        },
        "mac": {
            "kernel size: 3" : { "total time" : 1281898, "compute time" : 197568 },
            "kernel size: 5" : { "total time" : 824082, "compute time" : 130371 },
        }
    },
    "input size: 15" : {
        "fir": {
            "kernel size: 3" : { "total time" : 1494382, "compute time" : 350962 },
            "kernel size: 5" : { "total time" : 2007579, "compute time" : 494726 },
        },
        "mac": {
            "kernel size: 3" : { "total time" : 3328342, "compute time" : 516201 },
            "kernel size: 5" : { "total time" : 2686129, "compute time" : 430019 },
        }
    },
    "input size: 20" : {
        "fir": {
            "kernel size: 3" : { "total time" : 2139040, "compute time" : 537314 },
            "kernel size: 5" : { "total time" : 3024919, "compute time" : 808250 },
        },
        "mac": {
            "kernel size: 3" : { "total time" : 6348199, "compute time" : 986040 },
            "kernel size: 5" : { "total time" : 5642578, "compute time" : 905729 },
        }
    }
}

plt.rcParams["font.family"] = "Times New Roman"
plt.rcParams.update({'font.size': 16})
plt.rcParams["font.weight"] = "bold"
plt.rcParams["axes.labelweight"] = "bold"

fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(12, 3))

def plot(data, k, ax, labels):
    kernel_size = f"kernel size: {k}"

    input_sizes = [5, 10, 15, 20]
    fir_total_times = [data[f"input size: {size}"]["fir"][kernel_size]["total time"] for size in input_sizes]
    mac_total_times = [data[f"input size: {size}"]["mac"][kernel_size]["total time"] for size in input_sizes]

    fir_compute_times = [data[f"input size: {size}"]["fir"][kernel_size]["compute time"] for size in input_sizes]
    mac_compute_times = [data[f"input size: {size}"]["mac"][kernel_size]["compute time"] for size in input_sizes]

    normalized_mac_total_times = [mac_total_times[i] / fir_total_times[i] for i in range(len(input_sizes))]
    normalized_mac_compute_times = [mac_compute_times[i] / fir_total_times[i] for i in range(len(input_sizes))]

    bar_width = 0.25
    index = np.arange(len(input_sizes))

    ax.bar(
        index, np.array(fir_compute_times) / np.array(fir_total_times),
        bar_width, label=labels[0], color='mediumvioletred', hatch='\\\\'
    )
    ax.bar(
        index, 1-np.array(fir_compute_times) / np.array(fir_total_times),
        bar_width, bottom=np.array(fir_compute_times) / np.array(fir_total_times),
        color='pink', label=labels[1]
    )

    ax.bar(
        index + bar_width, normalized_mac_compute_times, bar_width,
        label=labels[2], color='seagreen', hatch='\\\\'
    )
    ax.bar(
        index + bar_width,
        np.array(normalized_mac_total_times) - np.array(normalized_mac_compute_times),
        bar_width, bottom=np.array(normalized_mac_compute_times), color='mediumspringgreen',
        label=labels[3]
    )

    ax.set_title(f"{k}x{k} Kernel")
    ax.set_xticks(index + bar_width / 2)
    ax.set_xticklabels(input_sizes)


plot(log, 3, ax1, labels=[
    "FIR (Compute Time)",
    "FIR (Total Time)",
    "MAC (Compute Time)",
    "MAC (Total Time)",
])
plot(log, 5, ax2, labels=[None]*4)

fig.legend(loc='lower center', bbox_to_anchor=(0.5, -0.12), ncol=4, fontsize=12)

plt.savefig(f"adaptive_conv.png", bbox_inches='tight', dpi=500)
