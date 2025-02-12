import os
import pandas as pd
import matplotlib.pyplot as plt
import numpy as np

prefix = os.path.join('logs', 'parsed_logs')

files = [
    'tails_resnet.csv',
    'lupe_no_opt_resnet.csv',
    'hawaii_resnet.csv',
    'lupe_resnet.csv',
]

cmap1 = plt.get_cmap('hsv')
N = len(files)
colors1 = cmap1(np.linspace(0, 1, N))
files = [(files[i], colors1[i]) for i in range(N)]

width = 0.35
plt.rcParams["font.family"] = "Times New Roman"
plt.rcParams.update({'font.size': 20})
plt.rcParams["font.weight"] = "bold"
plt.rcParams["axes.labelweight"] = "bold"


def get_df(filepath):
    df = pd.read_csv(filepath)

    df['Power:float'] = pd.to_numeric(df['Power:float'], errors='coerce')
    df['Time:ulong'] = pd.to_numeric(df['Time:ulong'], errors='coerce')
    df['Time:ulong'] = (df['Time:ulong'] - df.iloc[0, 0]) / 1000000000

    return df

alpha = 1

for f, c in files:
    filepath = os.path.join(prefix, f)
    df = get_df(filepath)

    plt.plot(df['Time:ulong'], df['Power:float'], linestyle='-', color=c, alpha=alpha)

    alpha -= 0.3

plt.show()
