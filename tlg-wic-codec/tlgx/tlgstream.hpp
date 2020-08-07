#pragma once

#include "../libtlg/TLG.h"
#include "../stdafx.hpp"

namespace tlgx {
  /**
	 * IStream -> tTJSBinaryStream
	 */
  class tMyStream : public tTJSBinaryStream {
    IStream* stream;

  public:
    tMyStream(IStream* stream);
    ~tMyStream();
    tjs_uint64 Seek(tjs_int64 offset, tjs_int whence) override;
    tjs_uint Read(void* buffer, tjs_uint read_size) override;
    tjs_uint Write(const void* buffer, tjs_uint write_size) override;
  };
} // namespace tlgx
