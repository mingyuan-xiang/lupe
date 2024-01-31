"""Dump UART data from the serial port

Quickly invoke UART for debugging:

    python3 uart_dump.py --port <uart port> --baud <baud rate>
"""

import argparse

import serial
import numpy as np

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

class SyncReader:
    """
    Synchronize reader class for parsing UART data.

    Due to the limited sizes of the UART buffer, we chop the data into chunks
    when send/receive data to/from msp430. We also send/receive an ACK to/from
    the msp430 to mark the finish of the transaction.

    Attributes:
        port: UART port
        baud: Baud rate
        ser: Serial port instance
        ack: ACK number
    """
    def __init__(self, port, baud) -> None:
        """Initialize serial port.

        Args:
            port: UART port
            baud: Baud rate
        """
        self.port = port
        self.baud = baud
        self.ser = serial.Serial(self.port, self.baud)
        if not self.ser.is_open:
            self.ser.open()
        self.ack = 0

    def close(self):
        """Close the serial port."""
        self.ser.close()

    def _get_16bit(self, is_signed=True):
        """Read a 16-bit integer from the UART port.
        
        Args:
            is_signed: Indicate if the integer is signed.

        Returns:
            An integer of 16-bit.
        """
        mat_bytes = []
        mat_bytes.append(self.ser.read())
        mat_bytes.append(self.ser.read())
        return int.from_bytes(
            b''.join(mat_bytes), byteorder='big', signed=is_signed
        )

    def _update_ack(self):
        """Update the ack number"""
        self.ack = (self.ack + 1) % 256

    def _send_ack(self):
        """Send the ack number back for synchronization.
        
        Update the ACK once finished.
        """
        self.ser.write(self.ack.to_bytes(1, byteorder='big'))
        self._update_ack()

    def get_msg(self):
        """
        Get a message from the serial port

        Send an ack after receiving the message

        Returns:
            Return the message from the msp430.
            The returned message is based on the following types:
                + UARTIO_MSG_TYPE_PRINTF_STR: a printf string
                + UARTIO_MSG_TYPE_BYTES: an array of unsigned 16-bit integers
                + UARTIO_MSG_TYPE_MAT: a numpy array of a tensor
        """
        # Read the message type
        msg = None
        msg_type = self.ser.read()
        if msg_type == UARTIO_MSG_TYPE_PRINTF_STR:
            msg = ''
            # Send an ack
            self._send_ack()

            # Read the string
            while True:
                result = self.ser.read()
                if result == UARTIO_STRING_END:
                    break
                msg += result.decode('utf-8')
            # Send an ack
            self._send_ack()
        if msg_type == UARTIO_MSG_TYPE_BYTES:
            msg = []
            # Receive the number of bytes
            num_bytes = self._get_16bit(is_signed=False)
            # Send an ack
            self._send_ack()

            # Read the bytes
            while num_bytes > 0:
                read_num_bytes = min(num_bytes, UARTIO_BUFFER_SIZE)
                num_bytes -= read_num_bytes
                result = self.ser.read(size=read_num_bytes)
                msg += result
                # Send an ack
                self._send_ack()
        if msg_type == UARTIO_MSG_TYPE_MAT:
            # Send an ack
            self._send_ack()
            # Get the number of dimensions
            n_dim = self._get_16bit(False)
            # Get the length of each dimension
            dim = []
            size = 1
            for _ in range(n_dim):
                d = self._get_16bit(False)
                dim.append(d)
                size *= d
            # Get the data
            data = []
            while size > 0:
                read_size = min(size, UARTIO_16BIT_BUFFER_SIZE)
                size -= read_size
                for _ in range(read_size):
                    data.append(self._get_16bit())
                # Send an ack
                self._send_ack()
            # Reshape the data
            data = np.array(data)
            msg = data.reshape(dim)

        return msg

    def put_arr(self, arr):
        """Send an array of 16-bit signed integers to msp430"""
        int_bytes = arr.newbyteorder('>').tobytes()
        self.ser.write(int_bytes)

        while self.ser.read() != UARTIO_MSG_TYPE_RECV_DONE:
            pass

    def read(self):
        """
        Read the message from the serial port

        Read util the msp430 sends an end string.

        Returns:
            Return a list of messages.
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
    """Get command line arguments for port and baud rate
    
    Returns:
        Return an argparse instance
    """
    par = argparse.ArgumentParser()
    par.add_argument(
        '--port', type=str, default='/dev/cu.usbmodem1103', help='UART port'
    )
    par.add_argument('--baud', type=int, default=19200, help='UART baud rate')
    args = par.parse_args()
    return args

def main():
    """Quickly invoke uart_dump"""
    args = get_args()
    par = SyncReader(args.port, args.baud)
    par.read()

    par.close()

if __name__ == '__main__':
    main()
