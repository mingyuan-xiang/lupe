#ifndef MAT_H
#define MAT_H

#include <libfixedAbstract/fixed.h>
#include <libmspprintf/mspprintf.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#ifndef CONFIG_MATCONSOLE
#define MATPRINTF(fmt, ...) (void)0
#define MAT_DUMP(m, w) (void)(0)
#else
#define MATPRINTF msp_printf
#define MAT_DUMP(m, w) (mat_dump(m, w))
#endif

typedef struct {
  uint16_t dims[4];
  uint16_t len_dims;
  uint16_t strides[4];
  fixed* data;
} mat_t;

#define MAT_NUMARGS(...) (sizeof((uint16_t[]){__VA_ARGS__}) / sizeof(uint16_t))

// Reshapes the Matrix
#define MAT_RESHAPE(m, ...)                                                    \
  (mat_reshape(m, ((uint16_t[]){__VA_ARGS__}), MAT_NUMARGS(__VA_ARGS__)))

// Reshapes the matrix so dst's shape similar to src
#define MAT_SAMESHAPE(dst, src) (mat_sameshape(dst, src))

// Constrains the matrix by 1 dimension/axis
#define MAT_CONSTRAIN(m, ...)                                                  \
  (mat_constrain(m, ((uint16_t[]){__VA_ARGS__}), MAT_NUMARGS(__VA_ARGS__)))

/* Gets a mtrix index
 * mat_get is too expensive to be called for the most common cases,
 * so this cascade of cases for the most common cases is enough to
 * reduce overhead.
 */
#define MAT_GET(m, ...)                                                        \
  ((MAT_NUMARGS(__VA_ARGS__) == 1)                                             \
       ? *((m)->data + ((uint16_t[]){__VA_ARGS__})[0])                         \
   : (MAT_NUMARGS(__VA_ARGS__) == 2)                                           \
       ? *((m)->data + ((uint16_t[]){__VA_ARGS__})[0] * (m)->strides[0] +      \
           ((uint16_t[]){__VA_ARGS__})[1])                                     \
   : (MAT_NUMARGS(__VA_ARGS__) == 3)                                           \
       ? *((m)->data + ((uint16_t[]){__VA_ARGS__})[0] * (m)->strides[0] +      \
           (m)->strides[1] * ((uint16_t[]){__VA_ARGS__})[1] +                  \
           ((uint16_t[]){__VA_ARGS__})[2])                                     \
       : mat_get(m, ((uint16_t[]){__VA_ARGS__}), MAT_NUMARGS(__VA_ARGS__)))

/* Gets a mtrix index ptr
 * mat_ptr is too expensive to be called for the most common cases,
 * so this cascade of cases for the most common cases is enough to
 * reduce overhead.
 */
#define MAT_PTR(m, ...)                                                        \
  ((MAT_NUMARGS(__VA_ARGS__) == 1)                                             \
       ? ((m)->data + ((uint16_t[]){__VA_ARGS__})[0])                          \
   : (MAT_NUMARGS(__VA_ARGS__) == 2)                                           \
       ? ((m)->data + ((uint16_t[]){__VA_ARGS__})[0] * (m)->strides[0] +       \
          ((uint16_t[]){__VA_ARGS__})[1])                                      \
   : (MAT_NUMARGS(__VA_ARGS__) == 3)                                           \
       ? ((m)->data + ((uint16_t[]){__VA_ARGS__})[0] * (m)->strides[0] +       \
          (m)->strides[1] * ((uint16_t[]){__VA_ARGS__})[1] +                   \
          ((uint16_t[]){__VA_ARGS__})[2])                                      \
       : mat_ptr(m, ((uint16_t[]){__VA_ARGS__}), MAT_NUMARGS(__VA_ARGS__)))

/* Sets a mtrix index
 * mat_set is too expensive to be called for the most common cases,
 * so this cascade of cases for the most common cases is enough to
 * reduce overhead.
 */
#define MAT_SET(m, val, ...)                                                   \
  ((MAT_NUMARGS(__VA_ARGS__) == 1)                                             \
       ? *((m)->data + ((uint16_t[]){__VA_ARGS__})[0]) = val                   \
   : (MAT_NUMARGS(__VA_ARGS__) == 2)                                           \
       ? *((m)->data + ((uint16_t[]){__VA_ARGS__})[0] * (m)->strides[0] +      \
           ((uint16_t[]){__VA_ARGS__})[1]) = val                               \
   : (MAT_NUMARGS(__VA_ARGS__) == 3)                                           \
       ? *((m)->data + ((uint16_t[]){__VA_ARGS__})[0] * (m)->strides[0] +      \
           (m)->strides[1] * ((uint16_t[]){__VA_ARGS__})[1] +                  \
           ((uint16_t[]){__VA_ARGS__})[2]) = val                               \
       : mat_set(m, val, ((uint16_t[]){__VA_ARGS__}),                          \
                 MAT_NUMARGS(__VA_ARGS__)))

// Basic matrix information functionality
#define MAT_GET_DIM(m, axis) (mat_get_dim(m, axis))
#define MAT_GET_STRIDE(m, axis) (mat_get_stride(m, axis))
#define MAT_GET_SIZE(m) (mat_get_size(m))

// Matrix special ops that only touch matrix meta data, not
// the actual contained data
#define MAT_TRANSPOSE(m) (mat_transpose(m))

// checks if two matrix data are the same.
// Both need to have the same number of elements or return false
#define MAT_SAME(dst, src) (mat_same(dst, src))
#define MAT_CLOSE(dst, src, close) (mat_close(dst, src, F_ABS(close)))

// Debug purposes, dumps layer v of m into d
#define MAT_DEBUG_DUMP(m, v, d) (mat_debug_dump(m, v, d))

void mat_reshape(mat_t *, uint16_t[], uint16_t);
void mat_sameshape(mat_t *, mat_t *);
mat_t mat_constrain(mat_t *, uint16_t[], uint16_t);
fixed mat_get(mat_t *, uint16_t[], uint16_t);
fixed *mat_ptr(mat_t *, uint16_t[], uint16_t);
void mat_set(mat_t *, fixed, uint16_t[], uint16_t);
uint16_t mat_get_dim(mat_t *, uint16_t);
uint16_t mat_get_stride(mat_t *, uint16_t);
size_t mat_get_size(mat_t *);
void mat_transpose(mat_t *);
bool mat_same(mat_t *, mat_t *);
bool mat_close(mat_t *, mat_t *, fixed);
void mat_dump(mat_t *, uint16_t);
void mat_debug_dump(mat_t *, uint16_t, fixed *);

#endif