import matplotlib.pyplot as plt

no_opt = {
    "ResNet3" : (823084, 5219626),
    "MobileNetV2" : (306760, 3011707),
    "DS_CNN" : (1349284, 15796710),
    "LeNet" : (60540, 614736),
    "MLPClassifier" : (9535, 50125),
}

opt = {
    "ResNet3" : (1022005, 2079566),
    "MobileNetV2" : (405975, 988186),
    "DS_CNN" : (1913476, 5141198),
    "LeNet" : (85263, 211414),
    "MLPClassifier" : (10633, 27030),
}

models = list(no_opt.keys())

no_opt_ratio = [no_opt[model][0] / no_opt[model][1] * 100 for model in models]
opt_ratio = [opt[model][0] / opt[model][1] * 100 for model in models]

plt.rcParams["font.family"] = "Times New Roman"
plt.rcParams.update({'font.size': 20})
plt.rcParams["font.weight"] = "bold"
plt.rcParams["axes.labelweight"] = "bold"

plt.figure(figsize=(12, 8))
plt.scatter(models, no_opt_ratio, color='blue', marker='x', s=200, label='Bottom-up Implementation', zorder=3)
plt.scatter(models, opt_ratio, color='red', marker='o', s=200, label='Top-down Implementation', zorder=3)

plt.grid(True, color='grey', zorder=0)

plt.ylabel('LEA Utilization (%)')
plt.xticks()
plt.legend()
plt.tight_layout(pad=0.05)
plt.savefig(f'utilization.png', dpi=500, bbox_inches='tight')

