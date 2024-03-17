- [lea](#lea)
- [model-fix](#model-fix)


## Important Note

Assume most of the optimization techniques are built on top of each other.
Some techniques can support out of order invocation, e.g. `lea`.

## lea

This flag will enable
[low-energy accelerator (LEA)](https://www.ti.com/lit/an/slaa720/slaa720.pdf)
to speed up execution.

## layer-fusion

Fuse layers to avoid control flow overhead

## loop-unroll

Unroll loops.

## tailored-api

Instead of having an unified ML layer API for the computation, Lupe will
generate a tailored layer for the corresponding model.

## prop-const

Propagate the constants so that the compiler can enable better optimizations.
This is only possible through a top-down view.

## dma

Optimize the DMA transaction functions. (**inter-module optimizations**)

## lea-opt

Optimize the LEA library APIs. (**inter-module optimizations**)

## loop-unroll

Unroll for loops
