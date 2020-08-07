#pragma once

#include "../StdAfx.hpp"

#include <mutex>

namespace wicx {
  class UnknownImpl {
    std::mutex m_mutex;
    ULONG m_numReferences = 0;

  public:
    UnknownImpl() = default;

    ULONG STDMETHODCALLTYPE AddRef();
    ULONG STDMETHODCALLTYPE Release();
  };
} // namespace wicx
