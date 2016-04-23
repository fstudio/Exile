/**/
#ifndef GZIPSTREAM_H
#define GZIPSTREAM_H
#include <stdint.h>
#include <zlib.h>

BOOL GZipStreamWritePipe(HANDLE hWrite, uint8_t *source, size_t length);
#endif
