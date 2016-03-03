#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <zlib.h>
#define segment_size 1460 // largest tcp data segment
int ungzip(char *source, int len, char *des) {
  int ret, have;
  int offset = 0;
  z_stream d_stream;
  Byte compr[segment_size] = {0}, uncompr[segment_size * 4] = {0};
  memcpy(compr, (Byte *)source, len);
  uLong comprLen, uncomprLen;
  comprLen = len;
  uncomprLen = segment_size * 4;
  strcpy((char *)uncompr, "garbage");
  d_stream.zalloc = Z_NULL;
  d_stream.zfree = Z_NULL;
  d_stream.opaque = Z_NULL;
  d_stream.next_in = Z_NULL;
  d_stream.avail_in = 0;
  ret = inflateInit2(&d_stream, 47);
  if (ret != Z_OK) {
    printf("inflateInit2 error:%d", ret);
    return ret;
  }
  d_stream.next_in = compr;
  d_stream.avail_in = comprLen;
  do {
    d_stream.next_out = uncompr;
    d_stream.avail_out = uncomprLen;
    ret = inflate(&d_stream, Z_NO_FLUSH);
    assert(ret != Z_STREAM_ERROR);
    switch (ret) {
    case Z_NEED_DICT:
      ret = Z_DATA_ERROR;
    case Z_DATA_ERROR:
    case Z_MEM_ERROR:
      (void)inflateEnd(&d_stream);
      return ret;
    }
    have = uncomprLen - d_stream.avail_out;
    memcpy(des + offset, uncompr, have);
    offset += have;
  } while (d_stream.avail_out == 0);
  inflateEnd(&d_stream);
  memcpy(des + offset, "\0", 1);
  return ret;
}

int inflate_read(char *source, int len, char **dest, int gzip) {
  int ret;
  unsigned have;
  z_stream strm;
  unsigned char out[CHUNK];
  int totalsize = 0;
  /*  allocate  inflate  state  */
  strm.zalloc = Z_NULL;
  strm.zfree = Z_NULL;
  strm.opaque = Z_NULL;
  strm.avail_in = 0;
  strm.next_in = Z_NULL;

  if (gzip)
    ret = inflateInit2(&strm, 47);
  else
    ret = inflateInit(&strm);

  if (ret != Z_OK)
    return ret;

  strm.avail_in = len;
  strm.next_in = source;

  /*  run  inflate()  on  input  until  output  buffer  not  full  */
  do {
    strm.avail_out = CHUNK;
    strm.next_out = out;
    ret = inflate(&strm, Z_NO_FLUSH);
    assert(ret != Z_STREAM_ERROR); /*  state  not  clobbered  */
    switch (ret) {
    case Z_NEED_DICT:
      ret = Z_DATA_ERROR; /*  and  fall  through  */
    case Z_DATA_ERROR:
    case Z_MEM_ERROR:
      inflateEnd(&strm);
      return ret;
    }
    have = CHUNK - strm.avail_out;
    totalsize += have;
    *dest = realloc(*dest, totalsize);
    memcpy(*dest + totalsize - have, out, have);
  } while (strm.avail_out == 0);

  /*  clean  up  and  return  */
  (void)inflateEnd(&strm);
  return ret == Z_STREAM_END ? Z_OK : Z_DATA_ERROR;
}
