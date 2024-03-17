# Notes for code generation implementation

### Data structure

Assume the data is always 4D, i.e. the shape of the data has 4 dimensions.
However, the data is actually stored in 1D C array. The strides are used to get
the offsets of each dimension.

Moreover, for input/output, the first dimension will be 1 because it will always
be 3D or 2D.