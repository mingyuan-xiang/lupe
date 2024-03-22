- [lea](#lea)
- [Optimization](#optimization)
  - [Device-*Independent*](#device-independent)
    - [layer-fusion](#layer-fusion)
    - [prop-const+loop-unroll](#prop-constloop-unroll)
  - [Device-*Dependent*](#device-dependent)
    - [dma](#dma)
    - [lea-opt](#lea-opt)
- [Intermittent support](#intermittent-support)
    - [inter](#inter)
- [Undone](#undone)
  - [CNN layers](#cnn-layers)
  - [RNN](#rnn)

## lea

This flag will enable
[low-energy accelerator (LEA)](https://www.ti.com/lit/an/slaa720/slaa720.pdf)
to speed up execution.

## Optimization

Use a **top-down** approach that could enable inter-module optimizations

### Device-*Independent*

#### layer-fusion

Fuse layers to avoid control flow overhead, e.g.
    + fuse batchnorm with convolution 
    + fuse flatten (reshape)
    + fuse bias addition

#### prop-const+loop-unroll

Propagate the constants so that the compiler can enable better loop-unrolling.
This is only possible through a top-down view.

Note: By default GCC will only do loop unrolling in O3, which will fail
to compile under CMU maker (*It could be my setting issue*)

### Device-*Dependent*

#### dma

Optimize the DMA transaction functions.
#### lea-opt

Optimize the LEA library APIs.

## Intermittent support

#### inter

Add intermittent support for neural networks

## Undone

### CNN layers

+ shortcut
+ batchnorm

### RNN

+ no implementation