# Optimization Flags

Lupe takes a json file as arguments for optimizations. Each flag takes a `true`
or a `false` value (except for `lea_size` that takes an integer).
The list of flags (with corresponding sections of the paper)
are listed below. Since batching would always improve performance, it is automatically
enabled when `adaptive_gen_lea` is enabled. By default, we use unbatched FIR
instantiation for all 2D convolutions.

+ `dma_opt`: Enable DMA optimization (Section 3.1.1)
+ `lea_opt`: Enable LEA optimization for reducing SRAM preparation overhead (Section 3.1.1)
+ `adaptive_gen_lea`: Generate adaptive DNN layers (Section 3.2). This flag assumes
  DNN calibration is already invoked. Or, the calibration json file already exists.
+ `adaptive_gen_mem`: Generate adaptive data movement (Section 3.1.3).
+ `global_mem_buffer`: Use a global memory buffer so that data management can be dynamic.
  Essentially, we use a big buffer and dynamically change the pointer location
  based on each DNN layer configuration.
+ `lea_size`: An integer value that specifies how large LEA buffer would be.
  If `global_mem_buffer` is enabled, it is the size of the total global buffer.
  Otherwise, it is the size of each LEA buffer, which we have 4 of them.

One example of optimization flags would look like:

```
{
    "dma_opt" : true,
    "lea_opt" : true,
    "adaptive_gen_lea" : true,
    "adaptive_gen_mem" : true,
    "global_mem_buffer" : true
}
```