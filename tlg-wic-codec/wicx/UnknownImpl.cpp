#include "UnknownImpl.hpp"

#include <mutex>

#include <Windows.h>

namespace wicx {
  ULONG STDMETHODCALLTYPE UnknownImpl::AddRef() {
    std::lock_guard lock(m_mutex);
    return ++m_numReferences;
  }

  ULONG STDMETHODCALLTYPE UnknownImpl::Release() {
    std::lock_guard lock(m_mutex);

    if (m_numReferences > 0) {
      m_numReferences--;
    }

    return m_numReferences;
  }
} // namespace wicx
