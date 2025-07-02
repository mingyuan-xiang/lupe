# Script to dump UART data from the serial port
# The script will do a synchronize read

import numpy as np
import argparse

import serial

# Message type
UARTIO_MSG_TYPE_PRINTF_STR = b'\x1f'
UARTIO_MSG_TYPE_BYTES = b'\x2f'
UARTIO_MSG_TYPE_MAT = b'\x3f'
UARTIO_MSG_TYPE_RECV_DONE = b'\x4f'

UARTIO_STRING_END = b'\xf0'

# Buffer size
UARTIO_BUFFER_SIZE = 100
UARTIO_16BIT_BUFFER_SIZE = int(UARTIO_BUFFER_SIZE / 2)

# End UART print string
UARTIO_END_PRINT_STR = '@@@@'

class sync_reader:
    """
    Synchronize reader class for parsing UART data
    """
    def __init__(self, port, baud) -> None:
        # Initialize serial port
        self.port = port
        self.baud = baud
        self.ser = serial.Serial(self.port, self.baud, timeout=10000)
        if not self.ser.is_open:
            self.ser.open()
        self.ack = 0

    def close(self):
        self.ser.close()

    def _get_16bit(self, is_signed=True):
        mat_bytes = []
        mat_bytes.append(self.ser.read())
        mat_bytes.append(self.ser.read())
        return int.from_bytes(b''.join(mat_bytes), byteorder='big', signed=is_signed)

    def _update_ack(self):
        self.ack = (self.ack + 1) % 256

    def send_ack(self):
        self.ser.write(self.ack.to_bytes(1, byteorder='big'))
        self._update_ack()

    def get_msg(self):
        """
        Get a message from the serial port
        Send an ack after receiving the message
        Return the message
        """
        # Read the message type
        msg = None
        msg_type = self.ser.read()
        if msg_type == UARTIO_MSG_TYPE_PRINTF_STR:
            msg = ''
            # Send an ack
            self.send_ack()

            # Read the string
            while True:
                result = self.ser.read()
                if result == UARTIO_STRING_END:
                    break
                else:
                    msg += result.decode('utf-8')
            # Send an ack
            self.send_ack()
        if msg_type == UARTIO_MSG_TYPE_BYTES:
            msg = []
            # Receive the number of bytes
            num_bytes = self._get_16bit(is_signed=False)
            # Send an ack
            self.send_ack()

            # Read the bytes
            while num_bytes > 0:
                read_num_bytes = min(num_bytes, UARTIO_BUFFER_SIZE)
                num_bytes -= read_num_bytes
                result = self.ser.read(size=read_num_bytes)
                msg += result
                # Send an ack
                self.send_ack()
        if msg_type == UARTIO_MSG_TYPE_MAT:
            # Send an ack
            self.send_ack()
            # Get the number of dimensions
            n_dim = self._get_16bit(False)
            # Get the length of each dimension
            dim = []
            size = 1
            for i in range(n_dim):
                d = self._get_16bit(False)
                dim.append(d)
                size *= d
            # Get the data
            data = []
            while size > 0:
                read_size = min(size, UARTIO_16BIT_BUFFER_SIZE)
                size -= read_size
                for i in range(read_size):
                    data.append(self._get_16bit())
                # Send an ack
                self.send_ack()
            # Reshape the data
            data = np.array(data)
            msg = data.reshape(dim)

        return msg

    def put_arr(self, arr):
        b = arr.view(arr.dtype.newbyteorder('>')).tobytes()
        self.ser.write(b)

        while self.ser.read() != UARTIO_MSG_TYPE_RECV_DONE:
            pass

        return

    def read(self):
        """
        Read the message from the serial port
        """
        data = []
        np.set_printoptions(edgeitems=30, linewidth=100000)
        while True:
            msg = self.get_msg()
            if msg is None:
                continue
            print(msg)
            data.append(msg)
            if isinstance(msg, str) and msg == UARTIO_END_PRINT_STR:
                break

        return data

def get_args():
    par = argparse.ArgumentParser()
    par.add_argument(
        '--port', type=str, default='/dev/cu.usbmodem1203', help='UART port'
    )
    par.add_argument('--baud', type=int, default=19200, help='UART baud rate')
    args = par.parse_args()
    return args

def main():
    args = get_args()
    par = sync_reader(args.port, args.baud)
    par.read()

    par.close()

if __name__ == '__main__':
    main()
