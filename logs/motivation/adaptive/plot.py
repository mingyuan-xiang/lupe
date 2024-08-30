import matplotlib.pyplot as plt
import matplotlib.ticker as ticker
import numpy as np

# Data provided
log_base = {
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

log_lupe = {
    "input size: 5" : {
        "fir": {
            "kernel size: 3" : { "total time" : 54725, "compute time" : 29491 },
            "kernel size: 5" : { "total time" : 60730, "compute time" : 35155 },
        },
        "mac": {
            "kernel size: 3" : { "total time" : 16576, "compute time" : 6106 },
            "kernel size: 5" : { "total time" : 4736, "compute time" : 1961 },
        }
    },
    "input size: 10" : {
        "fir": {
            "kernel size: 3" : { "total time" : 126787, "compute time" : 74802 },
            "kernel size: 5" : { "total time" : 162499, "compute time" : 109377 },
        },
        "mac": {
            "kernel size: 3" : { "total time" : 107810, "compute time" : 38018 },
            "kernel size: 5" : { "total time" : 113908, "compute time" : 39264 },
        }
    },
    "input size: 15" : {
        "fir": {
            "kernel size: 3" : { "total time" : 251219, "compute time" : 157370 },
            "kernel size: 5" : { "total time" : 356015, "compute time" : 255940 },
        },
        "mac": {
            "kernel size: 3" : { "total time" : 281945, "compute time" : 98875 },
            "kernel size: 5" : { "total time" : 378992, "compute time" : 129909 },
        }
    },
    "input size: 20" : {
        "fir": {
            "kernel size: 3" : { "total time" : 425251, "compute time" : 274384 },
            "kernel size: 5" : { "total time" : 636218, "compute time" : 469439 },
        },
        "mac": {
            "kernel size: 3" : { "total time" : 538991, "compute time" : 188769 },
            "kernel size: 5" : { "total time" : 799956, "compute time" : 273852 },
        }
    }
}

plt.rcParams["font.family"] = "Times New Roman"
plt.rcParams.update({'font.size': 16})
plt.rcParams["font.weight"] = "bold"
plt.rcParams["axes.labelweight"] = "bold"

fig, axs = plt.subplots(1, 2, figsize=(12, 4))

def plot(data, k, ax, labels):
    kernel_size = f"kernel size: {k}"

    input_sizes = [5, 10, 15, 20]
    fir_total_times = [data[f"input size: {size}"]["fir"][kernel_size]["total time"] for size in input_sizes]
    mac_total_times = [data[f"input size: {size}"]["mac"][kernel_size]["total time"] for size in input_sizes]

    normalized_fir_total_times = [fir_total_times[i] / fir_total_times[i] for i in range(len(input_sizes))]
    normalized_mac_total_times = [mac_total_times[i] / fir_total_times[i] for i in range(len(input_sizes))]

    bar_width = 0.25
    index = np.arange(len(input_sizes))

    ax.bar(
        index, normalized_fir_total_times,
        bar_width, label=labels[0], color='firebrick', hatch='++'
    )
    ax.bar(
        index + bar_width, normalized_mac_total_times, bar_width,
        label=labels[1], color='royalblue', hatch='\\\\'
    )

    ax.set_ylim([0, 1.5])
    ax.set_title(f"{k}x{k} Kernel")
    ax.set_xlabel('Input Size')
    ax.set_xticks(index + bar_width / 2)
    ax.set_xticklabels(input_sizes)
    ax.yaxis.set_major_formatter(ticker.FormatStrFormatter('%.1f'))


# plot(log_base, 3, axs[0, 0], True, labels=[None]*4)
# plot(log_base, 5, axs[0, 1], True, labels=[None]*4)
plot(log_lupe, 3, axs[0], labels=[
    "FIR",
    "MAC",
])
plot(log_lupe, 5, axs[1], labels=[None]*4)

yticks = axs[0].yaxis.get_major_ticks()
yticks[0].label1.set_visible(False)

axs[0].set_ylabel('Normalized to FIR\n(Top-down)')
axs[1].set_yticklabels([])
axs[1].yaxis.set_tick_params(length=0)

fig.legend(loc='lower center', ncol=4, fontsize=12, bbox_to_anchor=(0.54, -0.02))
fig.tight_layout(pad=0, rect=[0, 0.08, 1, 1])

plt.subplots_adjust(hspace=0)
plt.subplots_adjust(wspace=0)

# plt.show()
plt.savefig(f"adaptive_conv.png", dpi=500)
