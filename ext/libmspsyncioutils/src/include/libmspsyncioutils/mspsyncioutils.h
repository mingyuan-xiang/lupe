#ifndef INCLUDE_MSPSYNCIOUTILS_H
#define INCLUDE_MSPSYNCIOUTILS_H

#include <stdarg.h>
#include <stdint.h>

#include <libmspio/uartio.h>
#include <libmspprintf/mspprintf.h>
#include <libmatAbstract/mat.h>

/*
* This is a wrapper on libmspio library.
*
* To send a byte array or a string, the library will send a header first,
* which contains a 1-byte data type description (`uartio_msg.h`) and a few bytes for the
* data description (in big endian) in one message. Then, the library will send the data
* in one or multiple messages.
*
* The library can only receive byte array. It will received a 3-byte header first,
* with a 1-byte data type description and a 2-byte size. Then, the library will receive
* the data.
*
* The uart buffer size is defined as `UARTIO_BUFFER_SIZE` (in `uartio_msg.h`).
* Any array that is bigger than this will need to be sent/received in multiple message chunks.
*
* Each sent/received message needs to be communicated in a synchronized. In other words,
* each sent message will wait for a responding acknowledgement, and each received message
* will send an acknowledgement. The acknowledgement will be increased each time.
*/

#ifdef CONFIG_PRINT_PORT

/*
* send a matrix in a synchronized way
* The header message will be:
*   1. a type description byte (UARTIO_MSG_TYPE_MAT)
*   2. 2 bytes of dimension array size
*   3. 2*N bytes of sizes in each dimension, N is the dimension
*/
size_t msp_send_mat(mat_t* mat);

/*
* send a byte array in a synchronized way
* The header message will be:
*   1. a type description byte (UARTIO_MSG_TYPE_BYTES)
*   2. 2 bytes array size
*/
size_t msp_send_bytes(uint8_t* bytes, size_t len);

/*
* send a printf string in a synchronized way
* The header message will be:
*   1. a type description byte (UARTIO_MSG_TYPE_PRINTF_STR)
*/
size_t msp_send_printf(const char *format, ...);

/*
* end the printing
*/
size_t msp_end_printing();

/*
* receive a matrix in a synchronized way
* the matrix size is defined in the mat header
*/
size_t msp_recv_mat(mat_t* mat);

#endif

#endif /* INCLUDE_MSPSYNCIOUTILS_H */