#include <libmatAbstract/mat.h>
#include <stdint.h>
#include <string.h>

/* Given a matrix, calculates which linear index should be accessed to get the
 * appropriate offset given the matrix dimensions and the reqested indices in
 * the virtual matrix.
 */
inline uint16_t _offset_calc(mat_t *m, uint16_t idxs[], uint16_t len) {
  uint16_t offset = 0;
  for (uint16_t i = 0; i < len; i++) {
    offset += m->strides[i] * idxs[i];
  }
  return offset;
}

/* Virtually reshapes the matrix by changing how the strides functions. */
void mat_reshape(mat_t *m, uint16_t dims[], uint16_t len) {
  m->len_dims = len;
  uint16_t running_stride = 1;
  memset(m->strides, 1, sizeof(uint16_t) * (len + 1));
  for (uint16_t i = 0; i < len; i++) {
    m->dims[i] = dims[i];
    m->strides[len - i - 1] = running_stride;
    running_stride *= dims[len - i - 1];
  }
}

/* Gets a value given the indices from the matrix
 * NOTE: This is rather expensive, specially if your are accessing
 * elements that would normally be one after the other.
 */
fixed mat_get(mat_t *m, uint16_t idxs[], uint16_t len) {
  return *(m->data + _offset_calc(m, idxs, len));
}

/* Sets a value in the matrix given the indices
 * NOTE: This is rather expensive, specially if your are accessing
 * elements that would normally be one after the other.
 */
void mat_set(mat_t *m, fixed v, uint16_t idxs[], uint16_t len) {
  *(m->data + _offset_calc(m, idxs, len)) = v;
}

/* Returns a pointer to specified indices */
fixed *mat_ptr(mat_t *m, uint16_t idxs[], uint16_t len) {
  return m->data + _offset_calc(m, idxs, len);
}

uint16_t mat_get_dim(mat_t *m, uint16_t axis) { return m->dims[axis]; }
uint16_t mat_get_stride(mat_t *m, uint16_t axis) { return m->strides[axis]; }

/* Returns the total size of the matrix */
size_t mat_get_size(mat_t *m) {
  size_t size = 1;
  for (uint16_t i = 0; i < m->len_dims; i++) {
    size *= m->dims[i];
  }
  return size;
}

/* Virtually transposes the matrix */
void mat_transpose(mat_t *m) {
  uint16_t start = 0;
  uint16_t end = m->len_dims - 1;
  while (start < end) {
    uint16_t tmp = m->dims[start];
    m->dims[start] = m->dims[end];
    m->dims[end] = tmp;

    tmp = m->strides[start];
    m->strides[start] = m->strides[end];
    m->strides[end] = tmp;

    start++;
    end--;
  }
}

/* Checks if two matrix data are the same.
 * Both matrices needs to have the same element count.
 */
bool mat_same(mat_t *dst, mat_t *src) {
  size_t src_size = mat_get_size(src);
  size_t dst_size = mat_get_size(dst);
  if (dst_size != src_size) {
    MATPRINTF("NOT SAME: matrices are not the same shape\r\n");
    return false;
  }

  while (src_size != 0) {
    src_size--;
    if (src->data[src_size] != dst->data[src_size]) {
      MATPRINTF("NOT SAME: At index %u", src_size);
      MATPRINTF(" src is %u", src->data[src_size]);
      MATPRINTF(" and dst is %u\r\n", dst->data[src_size]);
      return false;
    }
  }
  return true;
}

bool mat_close(mat_t *dst, mat_t *src, fixed close) {
  size_t src_size = mat_get_size(src);
  size_t dst_size = mat_get_size(dst);
  if (dst_size != src_size) {
    MATPRINTF("NOT SAME: matrices are not the same shape\r\n");
    return false;
  }

  while (src_size != 0) {
    src_size--;
    fixed diff;
    if (src->data[src_size] > dst->data[src_size])
      diff = src->data[src_size] - dst->data[src_size];
    else
      diff = dst->data[src_size] - src->data[src_size];

    if (diff > close) {
      MATPRINTF("NOT CLOSE: At index %u", src_size);
      MATPRINTF(" src is %u", src->data[src_size]);
      MATPRINTF(" and dst is %u\r\n", dst->data[src_size]);
      return false;
    }
  }
  return true;
}
/* Prints a 2D matrix or a 2D section of a 3D matrix.
 * Use which to specify which subsection should be printed.
 */
void mat_dump(mat_t *m, uint16_t which) {
  uint16_t rows = MAT_GET_DIM(m, m->len_dims - 2);
  uint16_t cols = MAT_GET_DIM(m, m->len_dims - 1);
  MATPRINTF("\r\n=====================");
  MATPRINTF("\r\nRows: %u\r\n", rows);
  MATPRINTF("Cols: %u\r\n", cols);
  for (uint16_t i = 0; i < rows; i++) {
    for (uint16_t j = 0; j < cols; j++) {
      if (m->len_dims == 2) {
        MATPRINTF("%i ", MAT_GET(m, i, j));
      } else {
        MATPRINTF("%i ", MAT_GET(m, which, i, j));
      }
    }
    MATPRINTF("\r\n");
  }
  MATPRINTF("done ");
  MATPRINTF("===================== \r\n");
}

/* Copies over a 2D subsection of a 3D matrix into dest
 * Use which to specify which subsection should be copied
 */
void mat_debug_dump(mat_t *m, uint16_t which, fixed *dst) {
  fixed *dest_ptr = dst;
  uint16_t rows = MAT_GET_DIM(m, m->len_dims - 2);
  uint16_t cols = MAT_GET_DIM(m, m->len_dims - 1);
  for (uint16_t i = 0; i < rows; i++) {
    for (uint16_t j = 0; j < cols; j++) {
      *dest_ptr = MAT_GET(m, which, i, j);
      dest_ptr++;
    }
  }
}