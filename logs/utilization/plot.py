import matplotlib.pyplot as plt
import numpy as np

data = {
    'fir' : {
        'no_opt' : {
            "ResNet3" : (824636, 5219626),
            "MobileNetV2" : (307880, 3011707),
            "DS_CNN" : (1354654, 15796710),
            "LeNet" : (60785, 614736),
            "MLPClassifier" : (9528, 50125),
        },
        'opt' : {
            "ResNet3" : (1022099, 2079566),
            "MobileNetV2" : (406033, 988186),
            "DS_CNN" : (1915694, 5141198),
            "LeNet" : (85239, 211414),
            "MLPClassifier" : (10640, 27030),
        }
    },
    'mac' : {
        'no_opt' : {
            "ResNet3" : (849592, 11984904),
            "MobileNetV2" : (493065, 8101754),
            "DS_CNN" : (2396954, 36466877),
            "LeNet" : (32229, 442206),
            "MLPClassifier" : (9300, 102698),
        },
        'opt' : {
            "ResNet3" : (1428844, 3941291),
            "MobileNetV2" : (903463, 2537070),
            "DS_CNN" : (4272157, 11284501),
            "LeNet" : (48409, 177313),
            "MLPClassifier" : (12347, 45069),
        }
    }
}

models = list(data['fir']['no_opt'].keys())

fir_no_opt_values = [(data['fir']['no_opt'][model][0] / data['fir']['no_opt'][model][1]) * 100 for model in models]
fir_opt_values = [
    (data['fir']['opt'][model][0] / data['fir']['opt'][model][1] - data['fir']['no_opt'][model][0] / data['fir']['no_opt'][model][1]) * 100
    for model in models
]

mac_no_opt_values = [(data['mac']['no_opt'][model][0] / data['mac']['no_opt'][model][1]) * 100 for model in models]
mac_opt_values = [
    (data['mac']['opt'][model][0] / data['mac']['opt'][model][1] - data['mac']['no_opt'][model][0] / data['mac']['no_opt'][model][1]) * 100
    for model in models
]

x = np.arange(len(models))
width = 0.2
cmap = plt.get_cmap('tab10')
colors = cmap(np.linspace(0, 1, 4))

plt.rcParams["font.family"] = "Times New Roman"
plt.rcParams.update({'font.size': 20})
plt.rcParams["font.weight"] = "bold"
plt.rcParams["axes.labelweight"] = "bold"

fig, ax = plt.subplots(figsize=(12, 6))

ax.bar(x - width/2, fir_no_opt_values, width, label='FIR-Inst. Bottom-up', zorder=3, color=colors[0], hatch='\\\\')
ax.bar(x - width/2, fir_opt_values, width, bottom=fir_no_opt_values, label='FIR-Inst. Top-down', zorder=3, color=colors[1])

ax.bar(x + width/2, mac_no_opt_values, width, label='MAC-Inst. Bottom-up', zorder=3, color=colors[2], hatch='\\\\')
ax.bar(x + width/2, mac_opt_values, width, bottom=mac_no_opt_values, label='MAC-Inst. Top-down', zorder=3, color=colors[3])

plt.grid(True, axis='y', color='grey', zorder=0)
ax.set_ylabel('LEA Utilization (%)')
ax.set_xticks(x)
ax.set_xticklabels(models)
ax.xaxis.set_tick_params(length=0)
ax.legend(ncol=2, loc='upper center', edgecolor='black', bbox_to_anchor=(0.5, 1.02), framealpha=1)

plt.tight_layout()
plt.tight_layout(pad=0.05)
plt.savefig(f'utilization.png', dpi=500, bbox_inches='tight')

print(f'avg (fir, no opt): {round(sum(fir_no_opt_values) / 5, 2)}')
print(f'avg (fir, opt): {round((sum(fir_no_opt_values) + sum(fir_opt_values)) / 5, 2)}')
print(f'avg (mac, no opt): {round(sum(mac_no_opt_values) / 5, 2)}')
print(f'avg (mac, opt): {round((sum(mac_no_opt_values) + sum(mac_opt_values)) / 5, 2)}')