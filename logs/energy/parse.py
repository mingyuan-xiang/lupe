import os
import pandas as pd
import numpy as np
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

def process_file(filepath):
    df = pd.read_csv(filepath)

    df['Energy:float'] = pd.to_numeric(df['Energy:float'], errors='coerce')
    df['Power:float'] = pd.to_numeric(df['Power:float'], errors='coerce')

    compute = df[df['Power:float'] > 4]

    e = compute['Energy:float'].iloc[-1] - compute['Energy:float'].iloc[0]

    return e

def process_directory(directory):
    results = {}

    for filename in os.listdir(directory):
        if filename.endswith(".csv"):
            inter, config, model_name = parse_filename(filename)

            filepath = os.path.join(directory, filename)
            energy = process_file(filepath)

            if model_name not in results:
                results[model_name] = {}

            if config not in results[model_name]:
                results[model_name][config] = {}

            results[model_name][config][inter] = energy

    return results


result_dict = process_directory(os.path.join('logs', 'parsed_logs'))

with open('results.json', 'w') as json_file:
    json.dump(result_dict, json_file, indent=4)