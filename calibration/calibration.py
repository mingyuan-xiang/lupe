"""UART receiver for calibrating adaptive layer generation."""
import re

import numpy as np

from .uart_dump import sync_reader, UARTIO_END_PRINT_STR

def calibration(baud, port, result_queue):
    """Get the acceleration choices for each layer
    
    Returns:
        Return dictionary of layer names and acceleration choices
    """

    par = sync_reader(port, baud)

    data = []
    np.set_printoptions(edgeitems=30, linewidth=100000)
    while True:
        msg = par.get_msg()
        if msg is None:
            continue
        data.append(msg)
        print(msg)
        if isinstance(msg, str) and msg == UARTIO_END_PRINT_STR:
            break
    par.close()

    pattern = r'(?P<layer_name>\w+(?:_\w+)*) \((?P<acceleration>\w+(?:_\w+)?)\) takes (?P<cycles>\d+) cycles to execute \(REPEAT: (?P<repeat>\d+)\)'

    config = {}
    for d in data:
        if not isinstance(d, str):
            continue
        match = re.search(pattern, d)
        if match:
            layer = match.group('layer_name')
            acc = match.group('acceleration')
            cycles = int(match.group('cycles'))
            repeat = int(match.group('repeat'))

            if layer in config:
                if cycles < config[layer]['cycles']:
                    config[layer]['acceleration'] = acc
                    config[layer]['cycles'] = cycles
                    config[layer]['repeat'] = repeat
            else:
                config[layer] = {
                    'acceleration' : acc,
                    'cycles' : cycles,
                    'repeat' : repeat,
                }

    result_queue.put(config)

def update_calibration_config(old_config, new_config):
    """Update the old configuration based on new configuration"""
    config = old_config
    for layer in new_config:
        d = {
            'acceleration' : new_config[layer]['acceleration'],
            'cycles' : new_config[layer]['cycles'],
            'repeat' : new_config[layer]['repeat'],
        }
        if layer in config:
            new_time = d['cycles'] / d['repeat']
            old_time = config[layer]['cycles'] / config[layer]['repeat']
            if new_time < old_time:
                config[layer] = d
        else:
            config[layer] = d
    return config
