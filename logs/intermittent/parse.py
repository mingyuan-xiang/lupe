import os
import re
import json

restart_time_pattern = r"Restart times: (\d+)"
elapsed_time_pattern = r"Elapsed time: ([\d.]+)"
repeat_pattern = r"repeat: (\d+)"
rng_pattern = r"\[([0-9]+),\s*([0-9]+)\]"
filename_pattern = r"\d+_\d+_(LeNet|ResNet3|MobileNetV2|DS_CNN|MLPClassifier)_(.+)\.json\.log"
related_filename_pattern = r"\d+_\d+_(.+)_(LeNet|ResNet3|MobileNetV2|DS_CNN|MLPClassifier)_(.+)\.log"

dir_path = os.path.dirname(os.path.abspath(__file__))
continuous_path = os.path.join(os.path.dirname(dir_path), 'continuous', 'results.json')
related_continuous_path = os.path.join(os.path.dirname(dir_path), 'continuous', 'related_results.json')
log_path = os.path.join(dir_path, 'logs')

with open(continuous_path, 'r') as f:
    continuous_results = json.load(f)

with open(related_continuous_path, 'r') as f:
    related_continuous_results = json.load(f)

results = {}

for file_name in os.listdir(log_path):
    if file_name.endswith('.log'):
        with open(os.path.join(log_path, file_name), 'r') as file:
            content = file.read()

            restart_time = re.search(restart_time_pattern, content).group(1)
            elapsed_time = re.search(elapsed_time_pattern, content).group(1)
            repeat = re.search(repeat_pattern, content).group(1)
            rng_match = re.search(rng_pattern, content)
            rng_lower = rng_match.group(1)
            rng_upper = rng_match.group(2)
            filename_match = re.search(filename_pattern, file_name)
            if filename_match:
                model_name = filename_match.group(1)
                config = filename_match.group(2)
            else:
                filename_match = re.search(related_filename_pattern, file_name)
                model_name = filename_match.group(2)
                config = filename_match.group(1)

            flag = False
            for r in continuous_results[model_name]:
                if r['optimization_flags'] == config:
                    continuous_time = r['total_cycles'] / (r['total_images'] * 32768)
                    flag = True
                    break

            if not flag:
                for r in related_continuous_results[model_name]:
                    if r['optimization_flags'] == config:
                        continuous_time = r['total_cycles'] / (r['total_images'] * 32768)
                        break
            flag = False

            if rng_lower == "0":
                restart_time = 1
            else:
                restart_time = float(restart_time) / float(repeat)

            d = {
                'config' : config,
                'restart' : restart_time,
                'intermittent_time' : float(elapsed_time) / float(repeat),
                'continuous_time' : continuous_time,
            }

            r = str((int(rng_lower), int(rng_upper)))
            if r not in results:
                results[r] = { model_name : [d] }
            else:
                if model_name not in results[r]:
                    results[r][model_name] = [d]
                else:
                    results[r][model_name].append(d)

output_file = os.path.join(dir_path, 'results.json')
with open(output_file, 'w') as json_file:
    json.dump(results, json_file, indent=4)
