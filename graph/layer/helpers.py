"""Helper functions"""

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
