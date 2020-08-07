#pragma once

#include "../libtlg/TLG.h"

#include <Windows.h>

#include <ObjIdl.h>

namespace tlgx {
  /**
   * IStream -> tTJSBinaryStream
   */
  class tCOMStream : public tTJSBinaryStream {
    IStream* m_stream;

  public:
    tCOMStream(IStream* stream);
    ~tCOMStream();

    tjs_uint64 Seek(tjs_int64 offset, tjs_int whence) override;
    tjs_uint Read(void* buffer, tjs_uint read_size) override;
    tjs_uint Write(const void* buffer, tjs_uint write_size) override;
  };
} // namespace tlgx
