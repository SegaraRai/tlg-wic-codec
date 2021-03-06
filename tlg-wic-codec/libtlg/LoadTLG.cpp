﻿//---------------------------------------------------------------------------
/*
	TVP2 ( T Visual Presenter 2 )  A script authoring tool
	Copyright (C) 2000 W.Dee <dee@kikyou.info> and contributors

	See details of license at "license.txt"
*/
//---------------------------------------------------------------------------
// TLG5/6 decoder
//---------------------------------------------------------------------------
#include "TLG.h"
#include "tvpgl.h"
#include <stdlib.h>
#include <string.h>

#define TJSAlignedAlloc _aligned_malloc
#define TJSAlignedDealloc _aligned_free

extern "C" void TVPCreateTable(void);

/*
	TLG5:
		Lossless graphics compression method designed for very fast decoding
		speed.

	TLG6:
		Lossless/near-lossless graphics compression method which is designed
		for high compression ratio and faster decoding. Decoding speed is
		somewhat slower than TLG5 because the algorithm is much more complex
		than TLG5. Though, the decoding speed (using SSE enabled code) is
		about 20 times faster than JPEG2000 lossless mode (using JasPer
		library) while the compression ratio can beat or compete with it.
		Summary of compression algorithm is described in
		environ/win32/krdevui/tpc/tlg6/TLG6Saver.cpp
		(in Japanese).
*/

//---------------------------------------------------------------------------
// TLG5 loading handler
//---------------------------------------------------------------------------
int TVPLoadTLG5(void* callbackdata, tTVPGraphicFormatCallback formatcallback, tTVPGraphicSizeCallback sizecallback, tTVPGraphicScanLineCallback scanlinecallback, tTJSBinaryStream* src, bool sds)

{

  unsigned char buf[1];
  unsigned int colors;
  tjs_uint32 width, height, blockheight;
  if (!src->ReadBuffer(buf, 1)) {
    return false;
  }
  colors = buf[0];

  if (!src->ReadI32LE(width) || !src->ReadI32LE(height) || !src->ReadI32LE(blockheight)) {
    return false;
  }

  if (colors != 3 && colors != 4) {
    // "Unsupported color type."
    return false;
  }

  if (formatcallback) {
    formatcallback(callbackdata, sds ? TLGFormat::TLG50RAW_IN_TLG00SDS : TLGFormat::TLG50RAW, colors);
  }

  if (sizecallback && !sizecallback(callbackdata, width, height)) {
    return TLG_ABORT;
  }

  tjs_uint64 blockcount = (tjs_uint64)((height - 1) / blockheight) + 1;

  // skip block size section
  src->SetPosition(src->GetPosition() + blockcount * sizeof(tjs_uint32));

  // decomperss
  tjs_uint8* inbuf = NULL;
  tjs_uint8* outbuf[4];
  tjs_uint8* text = NULL;
  tjs_int slide_r = 0;
  for (unsigned int i = 0; i < colors; i++)
    outbuf[i] = NULL;

  int ret = TLG_SUCCESS;

  {
    text = (tjs_uint8*)TJSAlignedAlloc(4096, 4);
    memset(text, 0, 4096);

    inbuf = (tjs_uint8*)TJSAlignedAlloc(blockheight * width + 10, 4);
    for (tjs_uint i = 0; i < colors; i++)
      outbuf[i] = (tjs_uint8*)TJSAlignedAlloc(blockheight * width + 10, 4);

    tjs_uint8* prevline = NULL;
    for (tjs_uint y_blk = 0; y_blk < height; y_blk += blockheight) {
      // read file and decompress
      for (tjs_uint c = 0; c < colors; c++) {
        tjs_uint32 size;
        if (!src->ReadBuffer(buf, 1) || !src->ReadI32LE(size)) {
          ret = TLG_ERROR;
          goto errend;
        }
        if (buf[0] == 0) {
          // modified LZSS compressed data
          if (!src->ReadBuffer(inbuf, size)) {
            ret = TLG_ERROR;
            goto errend;
          }
          slide_r = TVPTLG5DecompressSlide(outbuf[c], inbuf, size, text, slide_r);
        } else {
          // raw data
          if (!src->ReadBuffer(outbuf[c], size)) {
            ret = TLG_ERROR;
            goto errend;
          }
        }
      }

      // compose colors and store
      tjs_uint y_lim = y_blk + blockheight;
      if (y_lim > height)
        y_lim = height;
      tjs_uint8* outbufp[4];
      for (tjs_uint c = 0; c < colors; c++)
        outbufp[c] = outbuf[c];
      for (tjs_uint y = y_blk; y < y_lim; y++) {
        tjs_uint8* current = (tjs_uint8*)scanlinecallback(callbackdata, y);
        if (current == NULL) {
          ret = TLG_ABORT;
          goto errend;
        }
        tjs_uint8* current_org = current;
        if (prevline) {
          // not first line
          switch (colors) {
            case 3:
              TVPTLG5ComposeColors3To4(current, prevline, outbufp, width);
              outbufp[0] += width;
              outbufp[1] += width;
              outbufp[2] += width;
              break;
            case 4:
              TVPTLG5ComposeColors4To4(current, prevline, outbufp, width);
              outbufp[0] += width;
              outbufp[1] += width;
              outbufp[2] += width;
              outbufp[3] += width;
              break;
          }
        } else {
          // first line
          switch (colors) {
            case 3:
              for (tjs_uint pr = 0, pg = 0, pb = 0, x = 0; x < width; x++) {
                tjs_uint b = outbufp[0][x];
                tjs_uint g = outbufp[1][x];
                tjs_uint r = outbufp[2][x];
                b += g;
                r += g;
                0 [current++] = pb += b;
                0 [current++] = pg += g;
                0 [current++] = pr += r;
                0 [current++] = 0xff;
              }
              outbufp[0] += width;
              outbufp[1] += width;
              outbufp[2] += width;
              break;
            case 4:
              for (tjs_uint pr = 0, pg = 0, pb = 0, pa = 0, x = 0; x < width; x++) {
                tjs_uint b = outbufp[0][x];
                tjs_uint g = outbufp[1][x];
                tjs_uint r = outbufp[2][x];
                tjs_uint a = outbufp[3][x];
                b += g;
                r += g;
                0 [current++] = pb += b;
                0 [current++] = pg += g;
                0 [current++] = pr += r;
                0 [current++] = pa += a;
              }
              outbufp[0] += width;
              outbufp[1] += width;
              outbufp[2] += width;
              outbufp[3] += width;
              break;
          }
        }
        scanlinecallback(callbackdata, -1);

        prevline = current_org;
      }
    }
  }

errend:
  if (inbuf)
    TJSAlignedDealloc(inbuf);
  if (text)
    TJSAlignedDealloc(text);
  for (tjs_uint i = 0; i < colors; i++)
    if (outbuf[i])
      TJSAlignedDealloc(outbuf[i]);

  return ret;
}
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// TLG6 loading handler
//---------------------------------------------------------------------------
int TVPLoadTLG6(void* callbackdata, tTVPGraphicFormatCallback formatcallback, tTVPGraphicSizeCallback sizecallback, tTVPGraphicScanLineCallback scanlinecallback, tTJSBinaryStream* src, bool sds) {
  TVPCreateTable();

  unsigned char buf[4];

  if (!src->ReadBuffer(buf, 4)) {
    return TLG_ERROR;
  }

  const unsigned int colors = buf[0]; // color component count

  if (colors != 1 && colors != 4 && colors != 3) {
    // "Unsupported color count"
    return TLG_ERROR;
  }

  if (buf[1] != 0) {
    // data flag
    // "Data flag must be 0 (any flags are not yet supported)";
    return TLG_ERROR;
  }

  if (buf[2] != 0) {
    // color type  (currently always zero)
    // "Unsupported color type"
    return TLG_ERROR;
  }

  if (buf[3] != 0) {
    // external golomb table (currently always zero)
    // "External golomb bit length table is not yet supported."
    return TLG_ERROR;
  }

  tjs_uint32 width, height, max_bit_length;

  if (!src->ReadI32LE(width) || !src->ReadI32LE(height) || !src->ReadI32LE(max_bit_length)) {
    return TLG_ERROR;
  }

  if (formatcallback) {
    formatcallback(callbackdata, sds ? TLGFormat::TLG60RAW_IN_TLG00SDS : TLGFormat::TLG60RAW, colors);
  }

  // set destination size
  if (sizecallback && !sizecallback(callbackdata, width, height)) {
    return TLG_ABORT;
  }

  // compute some values
  tjs_uint x_block_count = (tjs_uint)((width - 1) / TVP_TLG6_W_BLOCK_SIZE) + 1;
  tjs_uint y_block_count = (tjs_uint)((height - 1) / TVP_TLG6_H_BLOCK_SIZE) + 1;
  tjs_uint main_count = width / TVP_TLG6_W_BLOCK_SIZE;
  tjs_uint fraction = width - main_count * TVP_TLG6_W_BLOCK_SIZE;

  // prepare memory pointers
  tjs_uint8* bit_pool = NULL;
  tjs_uint32* pixelbuf = NULL; // pixel buffer
  tjs_uint8* filter_types = NULL;
  tjs_uint8* LZSS_text = NULL;
  tjs_uint32* zeroline = NULL;

  int ret = TLG_SUCCESS;

  // allocate memories
  bit_pool = (tjs_uint8*)TJSAlignedAlloc(max_bit_length / 8 + 5, 4);
  pixelbuf = (tjs_uint32*)TJSAlignedAlloc(sizeof(tjs_uint32) * width * TVP_TLG6_H_BLOCK_SIZE + 1, 4);
  filter_types = (tjs_uint8*)TJSAlignedAlloc(x_block_count * y_block_count, 4);
  zeroline = (tjs_uint32*)TJSAlignedAlloc(width * sizeof(tjs_uint32), 4);
  LZSS_text = (tjs_uint8*)TJSAlignedAlloc(4096, 4);

  if (bit_pool == NULL || pixelbuf == NULL || filter_types == NULL || zeroline == NULL || LZSS_text == NULL) {
    ret = TLG_ERROR;
    goto errend;
  }

  // initialize zero line (virtual y=-1 line)
  TVPFillARGB(zeroline, width, colors == 3 ? 0xff000000 : 0x00000000);
  // 0xff000000 for colors=3 makes alpha value opaque

  // initialize LZSS text (used by chroma filter type codes)
  {
    tjs_uint32* p = (tjs_uint32*)LZSS_text;
    for (tjs_uint32 i = 0; i < 32 * 0x01010101; i += 0x01010101) {
      for (tjs_uint32 j = 0; j < 16 * 0x01010101; j += 0x01010101)
        p[0] = i, p[1] = j, p += 2;
    }
  }

  // read chroma filter types.
  // chroma filter types are compressed via LZSS as used by TLG5.
  {
    tjs_uint32 inbuf_size;
    if (!src->ReadI32LE(inbuf_size)) {
      ret = TLG_ERROR;
      goto errend;
    }
    tjs_uint8* inbuf = (tjs_uint8*)TJSAlignedAlloc(inbuf_size, 4);
    if (!src->ReadBuffer(inbuf, inbuf_size)) {
      TJSAlignedDealloc(inbuf);
      ret = TLG_ERROR;
      goto errend;
    }
    TVPTLG5DecompressSlide(filter_types, inbuf, inbuf_size, LZSS_text, 0);
    TJSAlignedDealloc(inbuf);

    // for each horizontal block group ...
    tjs_uint32* prevline = zeroline;
    for (tjs_uint y = 0; y < height; y += TVP_TLG6_H_BLOCK_SIZE) {
      tjs_uint ylim = y + TVP_TLG6_H_BLOCK_SIZE;
      if (ylim >= height)
        ylim = height;

      tjs_uint pixel_count = (ylim - y) * width;

      // decode values
      for (tjs_uint c = 0; c < colors; c++) {
        // read bit length
        tjs_uint32 bit_length;
        if (!src->ReadI32LE(bit_length)) {
          ret = TLG_ERROR;
          goto errend;
        }

        // get compress method
        int method = (bit_length >> 30) & 3;
        bit_length &= 0x3fffffff;

        // compute byte length
        tjs_int byte_length = bit_length / 8;
        if (bit_length % 8)
          byte_length++;

        // read source from input
        if (!src->ReadBuffer(bit_pool, byte_length)) {
          ret = TLG_ERROR;
          goto errend;
        }

        // decode values
        // two most significant bits of bitlength are
        // entropy coding method;
        // 00 means Golomb method,
        // 01 means Gamma method (not yet suppoted),
        // 10 means modified LZSS method (not yet supported),
        // 11 means raw (uncompressed) data (not yet supported).

        switch (method) {
          case 0:
            if (c == 0 && colors != 1)
              TVPTLG6DecodeGolombValuesForFirst((tjs_int8*)pixelbuf, pixel_count, bit_pool);
            else
              TVPTLG6DecodeGolombValues((tjs_int8*)pixelbuf + c, pixel_count, bit_pool);
            break;
          default:
            // "Unsupported entropy coding method"
            ret = TLG_ERROR;
            goto errend;
        }
      }

      // for each line
      unsigned char* ft = filter_types + (y / TVP_TLG6_H_BLOCK_SIZE) * x_block_count;
      unsigned int skipbytes = (ylim - y) * TVP_TLG6_W_BLOCK_SIZE;

      for (unsigned int yy = y; yy < ylim; yy++) {
        tjs_uint32* curline = (tjs_uint32*)scanlinecallback(callbackdata, yy);
        if (curline == NULL) {
          ret = TLG_ABORT;
          goto errend;
        }
        int dir = (yy & 1) ^ 1;
        int oddskip = ((ylim - yy - 1) - (yy - y));
        if (main_count) {
          int start = ((width < TVP_TLG6_W_BLOCK_SIZE) ? width : TVP_TLG6_W_BLOCK_SIZE) * (yy - y);
          TVPTLG6DecodeLine(prevline, curline, width, main_count, ft, skipbytes, pixelbuf + start, colors == 3 ? 0xff000000 : 0, oddskip, dir);
        }

        if (main_count != x_block_count) {
          unsigned int ww = fraction;
          if (ww > TVP_TLG6_W_BLOCK_SIZE)
            ww = TVP_TLG6_W_BLOCK_SIZE;
          unsigned int start = ww * (yy - y);
          TVPTLG6DecodeLineGeneric(prevline, curline, width, main_count, x_block_count, ft, skipbytes, pixelbuf + start, colors == 3 ? 0xff000000 : 0, oddskip, dir);
        }

        scanlinecallback(callbackdata, -1);
        prevline = curline;
      }
    }
  }

errend:
  if (bit_pool)
    TJSAlignedDealloc(bit_pool);
  if (pixelbuf)
    TJSAlignedDealloc(pixelbuf);
  if (filter_types)
    TJSAlignedDealloc(filter_types);
  if (zeroline)
    TJSAlignedDealloc(zeroline);
  if (LZSS_text)
    TJSAlignedDealloc(LZSS_text);
  return ret;
}

//---------------------------------------------------------------------------
// TLG loading handler
//---------------------------------------------------------------------------
static int TVPInternalLoadTLG(void* callbackdata,
                              tTVPGraphicFormatCallback formatcallback,
                              tTVPGraphicSizeCallback sizecallback,
                              tTVPGraphicScanLineCallback scanlinecallback,
                              tTJSBinaryStream* src,
                              bool sds) {
  // read header
  unsigned char mark[11];
  if (!src->ReadBuffer(mark, 11)) {
    return TLG_ERROR;
  }

  // check for TLG raw data
  if (!memcmp("TLG5.0\x00raw\x1a", mark, 11)) {
    return TVPLoadTLG5(callbackdata, formatcallback, sizecallback, scanlinecallback, src, sds);
  } else if (!memcmp("TLG6.0\x00raw\x1a", mark, 11)) {
    return TVPLoadTLG6(callbackdata, formatcallback, sizecallback, scanlinecallback, src, sds);
  } else {
    return TLG_ERROR;
  }
}
//---------------------------------------------------------------------------

// check is TLG file
bool TVPCheckTLG(tTJSBinaryStream* src) {
  src->Seek(0, TJS_BS_SEEK_SET); // rewind
  bool ret = false;
  // read header
  unsigned char mark[11];
  if (src->ReadBuffer(mark, 11)) {
    // check for TLG0.0 sds
    if (!memcmp("TLG0.0\x00sds\x1a", mark, 11) || !memcmp("TLG5.0\x00raw\x1a", mark, 11) || !memcmp("TLG6.0\x00raw\x1a", mark, 11)) {
      ret = true;
    }
  }
  return ret;
}

struct SizeInfo {
  int width;
  int height;
};

static bool getSize(void* callbackdata, tjs_uint w, tjs_uint h) {
  SizeInfo* info = (SizeInfo*)callbackdata;
  info->width = w;
  info->height = h;
  return false;
}

bool TVPGetInfoTLG(tTJSBinaryStream* src, int* width, int* height) {
  SizeInfo size;
  if (TVPLoadTLG(&size, NULL, getSize, NULL, NULL, src) == TLG_ABORT) {
    if (width) {
      *width = size.width;
    }
    if (height) {
      *height = size.height;
    }
    return true;
  }
  return false;
}

int TVPLoadTLG(void* callbackdata,
               tTVPGraphicFormatCallback formatcallback,
               tTVPGraphicSizeCallback sizecallback,
               tTVPGraphicScanLineCallback scanlinecallback,
               std::unordered_map<std::string, std::string>* tags,
               tTJSBinaryStream* src) {
  src->Seek(0, TJS_BS_SEEK_SET); // rewind
  // read header
  unsigned char mark[11];
  if (!src->ReadBuffer(mark, 11)) {
    return TLG_ERROR;
  }

  // check for TLG0.0 sds
  if (!memcmp("TLG0.0\x00sds\x1a", mark, 11)) {
    // read TLG0.0 Structured Data Stream

    // TLG0.0 SDS tagged data is simple "NAME=VALUE," string;
    // Each NAME and VALUE have length:content expression.
    // eg: 4:LEFT=2:20,3:TOP=3:120,4:TYPE=1:3,
    // The last ',' cannot be ommited.
    // Each string (name and value) must be encoded in utf-8.

    // read raw data size
    tjs_uint32 rawlen;
    if (!src->ReadI32LE(rawlen)) {
      return TLG_ERROR;
    }

    // seek to meta info data point
    src->Seek((tjs_uint64)rawlen + 11 + 4, TJS_BS_SEEK_SET);

    // read tag data

    bool check = true;

    while (check) {
      char chunkname[4];
      if (4 != src->Read(chunkname, 4))
        break;
      // cannot read more
      tjs_uint chunksize;

      if (!src->ReadI32LE(chunksize)) {
        break;
      }

      if (!memcmp(chunkname, "tags", 4)) {
        // tag information
        char* tag = NULL;
        std::string name;
        std::string value;

        tag = new char[chunksize + 1];
        if (!src->ReadBuffer(tag, chunksize)) {
          break;
        }
        tag[chunksize] = 0;

        if (tags) {

          const char* tagp = tag;
          const char* tagp_lim = tag + chunksize;
          while (tagp < tagp_lim) {
            tjs_uint namelen = 0;
            while (*tagp >= '0' && *tagp <= '9') {
              namelen = namelen * 10 + *tagp - '0', tagp++;
            }
            if (*tagp != ':') {
              // Malformed TLG SDS tag structure, missing colon after name length
              check = false;
              break;
            }
            tagp++;
            name = std::string(tagp, namelen);
            tagp += namelen;
            if (*tagp != '=') {
              // Malformed TLG SDS tag structure, missing equals after name
              check = false;
              break;
            }
            tagp++;
            tjs_uint valuelen = 0;
            while (*tagp >= '0' && *tagp <= '9')
              valuelen = valuelen * 10 + *tagp - '0', tagp++;
            if (*tagp != ':') {
              // Malformed TLG SDS tag structure, missing colon after value length
              check = false;
              break;
            }
            tagp++;
            value = std::string(tagp, valuelen);
            tagp += valuelen;
            if (*tagp != ',') {
              // Malformed TLG SDS tag structure, missing comma after a tag
              check = false;
              break;
            }
            tagp++;

            // insert into name-value pairs ... TODO: utf-8 decode
            (*tags)[name] = value;
          }
        }
        if (tag)
          delete[] tag;
      } else {
        // skip the chunk
        src->SetPosition(src->GetPosition() + chunksize);
      }
    } // while

    // seek to TLG raw data point
    src->Seek(11 + 4, TJS_BS_SEEK_SET);

    // try to load TLG raw data
    return TVPInternalLoadTLG(callbackdata, formatcallback, sizecallback, scanlinecallback, src, true);
  } else {
    src->Seek(0, TJS_BS_SEEK_SET); // rewind

    // try to load TLG raw data
    return TVPInternalLoadTLG(callbackdata, formatcallback, sizecallback, scanlinecallback, src, false);
  }
}

//---------------------------------------------------------------------------
