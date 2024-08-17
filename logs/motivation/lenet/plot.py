import matplotlib.pyplot as plt
import numpy as np

# Data provided
data = {
    "fir": {"conv1": 39248, "conv2": 173163, "conv3": 370838},
    "mac": {"conv1": 114591, "conv2": 200399, "conv3": 94564},
}

plt.rcParams["font.family"] = "Times New Roman"
plt.rcParams.update({'font.size': 12})
plt.rcParams["font.weight"] = "bold"
plt.rcParams["axes.labelweight"] = "bold"

conv_labels = ["conv1", "conv2", "conv3"]
fir_values = np.array([data["fir"][conv] for conv in conv_labels])
mac_values = np.array([data["mac"][conv] for conv in conv_labels])

normalized_fir = fir_values / fir_values
normalized_mac = mac_values / fir_values

fig, ax = plt.subplots(figsize=(6, 3))
width = 0.25

x = np.arange(len(conv_labels))

ax.bar(x - width/2, normalized_fir, width, label="FIR (LEA)")
ax.bar(x + width/2, normalized_mac, width, label="MAC (LEA)")

ax.set_xticks(x)
ax.set_xticklabels(conv_labels)
ax.legend()
fig.tight_layout(pad=0)

plt.savefig(f"lenet.png", bbox_inches='tight', dpi=500)
