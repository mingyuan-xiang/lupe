"""UART receiver for calibrating adaptive layer generation."""

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
        if isinstance(msg, str) and msg == UARTIO_END_PRINT_STR:
            break
    par.close()

    config = {}
    for d in data:
        config[d] = 0

    result_queue.put(config)
