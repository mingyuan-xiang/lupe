"""Helper functions"""

from math import log2, ceil
import numpy as np

def get_stride(shape, idx):
    """Get the stride at the given index

    The data in MSP430 is stored in 1D arrays. This function is used to
    calculate the stride for n-dimensional arrays.
    
    Args:
        shape: The tuple of the shape of the array
        idx: The index to get the stride for
    """
    stride = 1
    for i in range(idx + 1, len(shape)):
        stride *= shape[i]

    return stride

def get_qf(data, per, qf_offset=0):
    """Get the fixed-point format of the data

    Ignore the outlier based on given per

    Args:
        data: numpy array
        per: percentile to exclude the outliers
        qf_offset: smaller qf to prevent overflow
    """

    lower = np.percentile(data, per)
    upper = np.percentile(data, 100 - per)
    x = max(abs(lower), abs(upper))

    if x < 1:
        return qf_offset

    return ceil(log2(x + 0.0000005)) + qf_offset
