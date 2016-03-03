/*

*/
#include "Precompiled.h"
#include "GZipStream.h"
#include <assert.h>
#define CHUNK 4096

BOOL GZipStreamWritePipe(HANDLE hWrite, uint8_t *source, size_t length)
{
	if (hWrite == nullptr || source == nullptr || length == 0)
		return FALSE;
	DWORD dwWrite = 0;
	z_stream stream;
	unsigned have;
	int ret;
	unsigned char out[CHUNK];
	stream.zalloc = Z_NULL;
	stream.zfree = Z_NULL;
	stream.opaque = Z_NULL;
	stream.avail_in = 0;
	stream.next_in = Z_NULL;
	ret = inflateInit2(&stream, 47);
	if (ret != Z_OK)
		return FALSE;
	stream.avail_in = length;
	stream.next_in = source;
	do {
		stream.avail_out = CHUNK;
		stream.next_out = out;
		ret = inflate(&stream, Z_NO_FLUSH);
		assert(ret != Z_STREAM_ERROR);
		switch (ret) {
			case Z_NEED_DICT:
			ret = Z_DATA_ERROR; /*  and  fall  through  */
			case Z_DATA_ERROR:
			case Z_MEM_ERROR:
			inflateEnd(&stream);
			return ret;
		}
		have = CHUNK - stream.avail_out;
		WriteFile(hWrite, out, have,&dwWrite, NULL);
	} while (stream.avail_out == 0);
	return FALSE;
}
