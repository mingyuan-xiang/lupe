- [lea](#lea)
- [model-fix](#model-fix)


## lea

This flag will enable
[low-energy accelerator (LEA)](https://www.ti.com/lit/an/slaa720/slaa720.pdf)
to speed up execution.

## model-fix

Fix the kernel size (for input and output) for each layer. This is the first
step of top-down optimization for Lupe. The reason for this flag is that the
model is fixed at deployment time, so we can have a series of tailored layers
instead of generic computation layers, i.e., 2D convolution, etc.