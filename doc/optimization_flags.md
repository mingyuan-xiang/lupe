- [lea](#lea)
- [Overflow](#overflow)
  - [saturation](#saturation)
- [Optimization](#optimization)
  - [Device-*Independent*](#device-independent)
    - [layer-fusion](#layer-fusion)
    - [prop-const+loop-unroll](#prop-constloop-unroll)
    - [ring-buffer](#ring-buffer)
    - [concatenate FIR](#concatenate-fir)
  - [Device-*Dependent*](#device-dependent)
    - [dma (inter-module optimization)](#dma-inter-module-optimization)
    - [lea-opt (inter-module optimization)](#lea-opt-inter-module-optimization)
- [Intermittent support](#intermittent-support)
    - [inter](#inter)
- [Undone](#undone)
  - [CNN layers](#cnn-layers)
  - [RNN](#rnn)

## lea

This flag will enable
[low-energy accelerator (LEA)](https://www.ti.com/lit/an/slaa720/slaa720.pdf)
to speed up execution.

## Overflow

### saturation

Set the behavior of the saturation. There are three options here:

+ `none`: Use 16-bit for intermediate results.
+ `scaled`: Use 16-bit for intermediate results, but use the scaled LEA
commands.
+ `32bit`: Use 32-bit for intermediate results as well as the LEA commands
(q31).

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

#### ring-buffer

Update this code from Pouya.

```
void _conv_kernel(fixed *src_ptr, fixed *dst_ptr, mat_t *dst,
                  uint16_t src_col_size, uint16_t flt_length,
                  uint16_t row_count) {
  // out->strides[2] : length of each row of output
  // out->strides[1] : length of each channel of output

  /* Ring buffer opeartion
   *
   * Row X --- Row X + 1 ... Row X + N --- END
   * ^                       ^             ^
   * |-- _rb_start           |             |-- _rb_end
   * |-- _rb_top             |-- _rb_last
   *
   * At each iteration, the address at tmp_input_addr
   * is loaded onto _rb_last.
   *
   * The convolution is conducted.
   *
   * At the end of the convolution, _rb_top and _rb_last
   * are incermented by LINE_SIZE. Then, if either is equal
   * to _rb_end, it is set to _rb_start.
   *
   * INITALIZE: load everything except _rb_last, which should get loaded in the
   * for loop.
   *
   */

  fixed *tmp_input_addr = src_ptr;
  _q15 *_rb_start = _conv_src1;
  _q15 *_rb_top = (_q15 *)_rb_start;
  _q15 *_rb_last = (_q15 *)_rb_start;
  uint16_t src_line_size = MAKE_ALIGN_2(src_col_size);

  // Intilalize values
  for (uint16_t i = 0; i < row_count - 1; i++) {
    LEA_WTRANSFER(tmp_input_addr, _rb_last, src_col_size);
    _rb_last += src_line_size;
    tmp_input_addr += src_col_size;
  }
  _q15 *_rb_end = (_q15 *)_rb_last + src_line_size;

  // for each output row
  ADDR_FOR(out_ptr, dst_ptr, dst->strides[1], dst->strides[2]) {
    /* send partial output to LEA RAM (from other channel computes) */
    LEA_WTRANSFER(out_ptr, _conv_dst, dst->strides[2]);

    // Load a new row into __rb_last
    LEA_WTRANSFER(tmp_input_addr, _rb_last, src_col_size);
    tmp_input_addr += src_col_size;

    _q15 *_rb_cur = _rb_top;
    ADDR_FOR(flt_row, (fixed *)_conv_flt, flt_length,
             lea_fir_params.tapLength) {
      /* convolution */
      fir_q15((_q15 *)flt_row, _rb_cur);

      /* accumulate results for a 2D convolution */
      add_q15();

      _rb_cur += src_line_size;
      if (((uintptr_t)_rb_cur) == ((uintptr_t)_rb_end)) {
        _rb_cur = _rb_start;
      }
    }

    LEA_WTRANSFER(_conv_dst, out_ptr, dst->strides[2]);

    _rb_top += src_line_size;
    if (((uintptr_t)_rb_top) == ((uintptr_t)_rb_end)) {
      _rb_top = _rb_start;
    }
    _rb_last += src_line_size;
    if (((uintptr_t)_rb_last) == ((uintptr_t)_rb_end)) {
      _rb_last = _rb_start;
    }
  }
}
```

#### concatenate FIR

Calling FIR on small arrays is bad. We should either:
  + Concatenate multiple rows and call at once
  + Call mac on the whole matrix (we need to pad zeros at the end of each kernel row when the input row size is not the same as the kernel row)

### Device-*Dependent*

#### dma (inter-module optimization)

Optimize the DMA transaction functions.
#### lea-opt (inter-module optimization)

Optimize the LEA library APIs.

remove this
```
LEAPMS1 = MSP_LEA_CONVERT_ADDRESS(lea_add_params);
```

## Intermittent support

#### inter

Add intermittent support for neural networks

## Undone

### CNN layers

+ shortcut
+ batchnorm

### RNN

+ no implementation