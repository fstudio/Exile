/**/
#ifndef GZIPSTREAM_H
#define GZIPSTREAM_H
#include <zlib.h>
#include <stdint.h>
BOOL GZipStreamWritePipe(HANDLE hWrite, uint8_t *source, size_t length);
#endif
