#include "TLGStream.hpp"

#include "../wicx/Util.hpp"

namespace tlgx {
  tCOMStream::tCOMStream(IStream* stream) : m_stream(stream) {
    stream->AddRef();
  }

  tCOMStream::~tCOMStream() {
    wicx::WICX_RELEASE(m_stream);
  }

  tjs_uint64 tCOMStream::Seek(tjs_int64 offset, tjs_int whence) {
    if (!m_stream) {
      return 0;
    }

    DWORD origin = STREAM_SEEK_SET;
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
    }

    ULARGE_INTEGER newPos{};
    if (FAILED(m_stream->Seek(wicx::MakeLI(offset), origin, &newPos))) {
      return 0;
    }

    return newPos.QuadPart;
  }

  tjs_uint tCOMStream::Read(void* buffer, tjs_uint read_size) {
    if (!m_stream) {
      return 0;
    }

    ULONG read = 0;
    if (FAILED(m_stream->Read(buffer, read_size, &read))) {
      return 0;
    }

    return read;
  }

  tjs_uint tCOMStream::Write(const void* buffer, tjs_uint write_size) {
    if (!m_stream) {
      return 0;
    }

    ULONG written = 0;
    if (FAILED(m_stream->Write(buffer, write_size, &written))) {
      return 0;
    }

    return written;
  }
} // namespace tlgx
