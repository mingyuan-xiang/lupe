import matplotlib.pyplot as plt

model = ('LeNet5')
data = {
    'conv1' : 9699,
    'pool1' : 1856,
    'conv2' : 27994,
    'pool2' : 646,
    'conv3' : 59268,
    'fc1' : 667,
    'fc2' : 52,
}

width = 0.01

fig = plt.figure(figsize=(2.5, 16))
ax = fig.add_subplot(111)
plt.axis('off')
ax.set_ylim([0, 1])
bottom = 0

sum = 0
for layer, d in data.items():
    sum += d

print(f"conv1: {data['conv1'] / sum * 100}")
print(f"conv2: {data['conv2'] / sum * 100}")
print(f"conv3: {data['conv3'] / sum * 100}")

for layer, d in reversed(data.items()):
    d = d / sum
    p = ax.bar(model, d, width, label=layer, bottom=bottom)
    bottom += d

fig.tight_layout()
plt.savefig('lenet_total_breakdown.png', bbox_inches='tight')