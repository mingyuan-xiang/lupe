import os
import pandas as pd
import numpy as np
from scipy.stats import linregress
import json

model_name_map = {
    'mlp' : 'MLPClassifier',
    'lenet' : 'LeNet',
    'mobilenet' : 'MobileNetV2',
    'resnet' : 'ResNet3',
    'dscnn' : 'DS-CNN',
}

config_map = {
    'tails' : 'Tails',
    'hawaii' : 'Hawaii',
    'lupe_no_opt' : 'Bottom-up',
    'lupe' : 'Lupe',
}

def parse_filename(filename):
    filename = filename.replace('.csv', '')
    parts = filename.split('_')
    if filename.startswith('inter'):
        config = '_'.join(parts[1:-1])
        model_name = parts[-1]
        inter = 'intermittent'
    else:
        config = '_'.join(parts[:-1])
        model_name = parts[-1]
        inter = 'continuous'
    return inter, config_map[config], model_name_map[model_name]

def calculate_slope(time_ns, energy_microjoule):
    time_sec = time_ns * 1e-9
    energy_joules = energy_microjoule * 1e-6

    valid_mask = ~np.isnan(energy_joules)
    time_sec = time_sec[valid_mask]
    energy_joules = energy_joules[valid_mask]

    if len(time_sec) < 2:
        return None

    slope, intercept, r_value, p_value, std_err = linregress(time_sec, energy_joules)
    return slope

def process_file(filepath):
    df = pd.read_csv(filepath)

    time_ns = df['Time:ulong'].astype(float)
    energy_microjoule = df['Energy:float'].replace('', np.nan).astype(float)

    slope = calculate_slope(time_ns, energy_microjoule)

    return slope

def process_directory(directory):
    results = {}

    for filename in os.listdir(directory):
        if filename.endswith(".csv"):
            inter, config, model_name = parse_filename(filename)

            filepath = os.path.join(directory, filename)
            slope = process_file(filepath)

            if model_name not in results:
                results[model_name] = {}

            if config not in results[model_name]:
                results[model_name][config] = {}

            results[model_name][config][inter] = slope

    return results


def get_data(d, inter, config, model):
    if config == 'Bottom-up':
        config = 'no_opt'
    if config == 'Lupe':
        config = 'dma_lea_opt_adaptive_buffer_mem_acc'

    if model == 'DS-CNN':
        model = 'DS_CNN'

    for l in d[model]:
        if l['config'] == config:
            if inter == 'intermittent':
                return l['intermittent_time']
            if inter == 'continuous':
                return l['continuous_time']

            raise ValueError("Should not reach here")

result_dict = process_directory('logs')

with open('../intermittent/results.json', 'r') as f:
    times = json.load(f)['(0, 0)']

for m in result_dict:
    for c in result_dict[m]:
        for i in result_dict[m][c]:
            r = result_dict[m][c][i]
            result_dict[m][c][i] = r * get_data(times, i, c, m)

with open('results.json', 'w') as json_file:
    json.dump(result_dict, json_file, indent=4)