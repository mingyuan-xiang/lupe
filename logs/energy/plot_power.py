import os
import pandas as pd
import matplotlib.pyplot as plt

prefix = os.path.join('logs', 'power_snapshot')

file_name = [
    'inter_hawaii_resnet.csv',
]

def get_df(filepath):
    df = pd.read_csv(filepath)

    df['Power:float'] = pd.to_numeric(df['Power:float'], errors='coerce')
    df['Time:ulong'] = pd.to_numeric(df['Time:ulong'], errors='coerce')
    df['Time:ulong'] = (df['Time:ulong'] - df.iloc[0, 0]) / 1000000000

    return df

for f in file_name:
    filepath = os.path.join(prefix, f)
    df = get_df(filepath)

    plt.plot(df['Time:ulong'], df['Power:float'], linestyle='-')

plt.show()
