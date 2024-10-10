from colorama import Fore, Style
import threading
import subprocess
import argparse
import sys
import json
import os
import time

sys.path.append("scripts/uart_commute")
from uart_dump import sync_reader, UARTIO_END_PRINT_STR

bound_list = [
    # (0, 0),
    # (164, 328),
    (1311, 1638),
    (2949, 3276),
]

def _banner_print(s):
    print(Fore.GREEN +
        '\n*******************************************************************')
    print(Fore.GREEN + s)
    print(Fore.GREEN +
        '\n*******************************************************************'
        + Fore.WHITE + Style.RESET_ALL)

def get_args():
    par = argparse.ArgumentParser()
    par.add_argument(
        '--port', type=str, default='/dev/ttyACM1', help='UART port'
    )
    par.add_argument('--baud', type=int, default=19200, help='UART baud rate')
    par.add_argument('--repeat', type=int, default=100, help='Repeat times for inference')

    return par.parse_args()

args = get_args()

def run_lupe(model, onnx_path, qf, config, dataset, repeat, lower, upper, hifram):
    if lower == upper:
        subprocess.run([
            './lupe.py', 'code-gen', '--model-name', model,
            '--model-path', onnx_path, '--qf', str(qf), '--config', config,
            '--intermittent', '--intermittent-repeat', str(repeat),
            '--intermittent-bound', str(lower),
            '--debug-random', '--debug-dataset', dataset,
            '--hifram-func', str(hifram)
        ], check=False)
    else:
        subprocess.run([
            './lupe.py', 'code-gen', '--model-name', model,
            '--model-path', onnx_path, '--qf', str(qf), '--config', config,
            '--intermittent', '--intermittent-repeat', str(repeat),
            '--intermittent-bound', str(lower), str(upper),
            '--debug-random', '--debug-dataset', dataset,
            '--hifram-func', str(hifram)
        ], check=False)

def enable_uart(baud, port, name):
    start = time.time()

    filename = "logs"
    os.makedirs(filename, exist_ok=True)

    filename = os.path.join(filename, f'{name}.log')
    if os.path.isfile(filename):
        os.remove(filename)

    f = open(filename, "w")

    par = sync_reader(port, baud)

    end = 0

    while True:
        msg = par.get_msg()
        if msg is None:
            continue

        if 'Restart times' in msg:
            end = time.time()

        print(msg)
        f.write(msg + "\n")
        f.flush()

        if isinstance(msg, str) and msg == UARTIO_END_PRINT_STR:
            break

    print(f"Elapsed time: {end - start} seconds")
    f.write(f"Elapsed time: {end - start} seconds")
    f.flush()

    par.close()
    f.close()

with open('scripts/intermittent_experiments/experiment_setting/lupe.json', "r", encoding="utf-8") as file:
    experiment_configs = json.load(file)

for bound in bound_list:
    for m in experiment_configs:
        c = experiment_configs[m]

        onnx_path = f'models/onnx/{m}.onnx'
        for config in c['config']:
            _banner_print(f"Generate Code for {m}")
            run_lupe(
                m, onnx_path, c['qf'], config, c['dataset'], args.repeat,
                bound[0],  bound[1], c['hifram-func']
            )

            config_name = config.split('/')[1]
            name = f'{bound[0]}_{bound[1]}_{m}_{config_name}'

            _banner_print('Compile and flash code')
            os.system(f"make apps/{m}/bld/gcc/prog")

            uart_thread = threading.Thread(
                target=enable_uart,
                args=(args.baud, args.port, name)
            )
            uart_thread.daemon = True
            uart_thread.start()

            uart_thread.join()
