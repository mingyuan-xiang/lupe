#include <libmspsyncioutils/mspsyncioutils.h>
#include <libmspsyncioutils/uartio_msg.h>

#define _PUTC(c) PUTC(CONFIG_PRINT_PORT, c)

static uint8_t ack;

void wait_ack() {
  while (uartio_getchar(CONFIG_PRINT_PORT) != ack)
    ;
  ack++;
}

/*
* Send a 16-bit number in big endian
*/
size_t _PUT16(uint16_t num) {
  size_t ret = 0;
  ret += _PUTC((uint8_t)((num >> 8) & 0xFF));
  ret += _PUTC((uint8_t)(num & 0xFF));
  return ret;
}

/* send a matrix in a synchronized way */
size_t msp_send_mat(mat_t* mat) {
  size_t ret = 0;
  // send the type
  ret += _PUTC(UARTIO_MSG_TYPE_MAT);
  // send the matrix shape
  ret += _PUT16(mat->len_dims);
  for (int i = 0; i < mat->len_dims; i++) {
    ret += _PUT16(mat->dims[i]);
  }
  // wait for acknowledgement for sent header
  wait_ack();

  // send the matrix data
  size_t size = MAT_GET_SIZE(mat);
  size_t st = 0;
  size_t ed = UARTIO_16BIT_BUFFER_SIZE;
  while (size > UARTIO_16BIT_BUFFER_SIZE) {
    for (int i = st; i < ed; i++) {
      ret += _PUT16((uint16_t)(mat->data[i]));
    }
    // wait for acknowledgement for sent data
    wait_ack();

    st += UARTIO_16BIT_BUFFER_SIZE;
    ed += UARTIO_16BIT_BUFFER_SIZE;
    size -= UARTIO_16BIT_BUFFER_SIZE;
  }

  ed = MAT_GET_SIZE(mat);
  for (int i = st; i < ed; i++) {
    ret += _PUT16((uint16_t)(mat->data[i]));
  }
  // wait for acknowledgement for sent data
  wait_ack();

  return ret;
}

/* send a byte array in a synchronized way */
size_t msp_send_bytes(uint8_t* bytes, size_t len) {
  size_t ret = 0;
  // send the type
  ret += _PUTC(UARTIO_MSG_TYPE_BYTES);
  // send the array size
  ret += _PUT16(len);
  // wait for acknowledgement for sent header
  wait_ack();

  // send the data
  size_t size = len;
  uint8_t* ptr = bytes;
  while (size > UARTIO_BUFFER_SIZE) {
    ret += uartio_send_sync(CONFIG_PRINT_PORT, ptr, UARTIO_BUFFER_SIZE);
    // wait for acknowledgement for sent data
    wait_ack();

    ptr += UARTIO_BUFFER_SIZE;
    size -= UARTIO_BUFFER_SIZE;
  }

  ret += uartio_send_sync(CONFIG_PRINT_PORT, ptr, size);
  // wait for acknowledgement for sent data
  wait_ack();

  return ret;
}

/* send a printf string in a synchronized way */
size_t msp_send_printf(const char *format, ...) {
  size_t ret = 0;
  // send the type
  ret += _PUTC(UARTIO_MSG_TYPE_PRINTF_STR);
  // wait for acknowledgement for sent header
  wait_ack();

  va_list a;
  va_start(a, format);
  ret += _msp_port_vprintf(CONFIG_PRINT_PORT, format, a);
  va_end(a); 
  // send the end of string symbol
  ret += _PUTC(UARTIO_STRING_END);

  // wait for acknowledgement for sent the string
  wait_ack();

  return ret;
}

size_t msp_end_printing() {
  return msp_send_printf("@@@@");
}

/*
* send UARTIO_MSG_TYPE_RECV_DONE to notify the receive is done
*/
void ack_recv() {
  _PUTC(UARTIO_MSG_TYPE_RECV_DONE);
}

size_t msp_recv_mat(mat_t* mat) {
  uint16_t size = mat->strides[0] * mat->dims[0] * sizeof(int16_t);
  uartio_recv_sync(CONFIG_PRINT_PORT, (uint8_t*)(mat->data), size);

  /* send notification to mark the receiving is done */
  ack_recv();
  
  return size;
}