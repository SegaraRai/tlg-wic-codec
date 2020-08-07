#include "TLGStream.hpp"

namespace tlgx {
  tMyStream::tMyStream(IStream* stream) : stream(stream) {
    stream->AddRef();
  }

  tMyStream::~tMyStream() {
    if (stream) {
      stream->Release();
      stream = 0;
    }
  }

  tjs_uint64 tMyStream::Seek(tjs_int64 offset, tjs_int whence) {
    if (stream) {
      DWORD origin;
      switch (whence) {
        case TJS_BS_SEEK_SET:
          origin = STREAM_SEEK_SET;
          break;
        case TJS_BS_SEEK_CUR:
          origin = STREAM_SEEK_CUR;
          break;
        case TJS_BS_SEEK_END:
          origin = STREAM_SEEK_END;
          break;
        default:
          origin = STREAM_SEEK_SET;
          break;
      }

      LARGE_INTEGER ofs;
      ULARGE_INTEGER newpos;

      ofs.QuadPart = offset;

      if (SUCCEEDED(stream->Seek(ofs, origin, &newpos))) {
        return newpos.QuadPart;
      }
    }
    return 0;
  }

  tjs_uint tMyStream::Read(void* buffer, tjs_uint read_size) {
    if (stream) {
      ULONG cb = read_size;
      ULONG read;
      if (SUCCEEDED(stream->Read(buffer, cb, &read))) {
        return read;
      }
    }
    return 0;
  }

  tjs_uint tMyStream::Write(const void* buffer, tjs_uint write_size) {
    if (stream) {
      ULONG cb = write_size;
      ULONG written;
      if (SUCCEEDED(stream->Write(buffer, cb, &written))) {
        return written;
      }
    }
    return 0;
  }
} // namespace tlgx
