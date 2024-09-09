import os
import re
import json

dir_path = '../'

model_opt_pattern = re.compile(r'Running (\w+) \((.+)\) total cycles')
frequency_pattern = re.compile(r'cycles \((\d+) Hz\)')
count_pattern = re.compile(r'CNT=(\d+)\)')
total_cycles_pattern = re.compile(r'total cycles \(\d+ Hz\) \(CNT=\d+\): (\d+)')
accuracy_pattern = re.compile(r'accumulative accuracy: ([\d\.]+)')

results = {}

for file_name in os.listdir(dir_path):
    if file_name.endswith('.log'):
        with open(os.path.join(dir_path, file_name), 'r') as file:
            content = file.read()

            model_opt_match = model_opt_pattern.search(content)
            if model_opt_match:
                model_name = model_opt_match.group(1)
                opt_flags = model_opt_match.group(2)

                frequency_match = frequency_pattern.search(content)
                count_match = count_pattern.search(content)
                total_cycles_matches = total_cycles_pattern.findall(content)
                accuracy_matches = accuracy_pattern.findall(content)

                if frequency_match and count_match and total_cycles_matches and accuracy_matches:
                    frequency = int(frequency_match.group(1))
                    count = int(count_match.group(1))
                    total_cycles = sum(map(int, total_cycles_matches))
                    last_accuracy = float(accuracy_matches[-1])
                    total_images = len(accuracy_matches)

                    if model_name not in results:
                        results[model_name] = []
                    results[model_name].append({
                        "optimization_flags": opt_flags,
                        "frequency": frequency,
                        "count": count,
                        "total_images": total_images,
                        "last_accuracy": last_accuracy,
                        "total_cycles": total_cycles
                    })

output_file = 'results.json'
with open(output_file, 'w') as json_file:
    json.dump(results, json_file, indent=4)
