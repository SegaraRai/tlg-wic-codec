#pragma once

#include "../StdAfx.hpp"

namespace wicx {
  class RegMan {
    std::vector<wchar_t const*> m_keys;

    void SetRaw(wchar_t const* keyName, wchar_t const* valueName, unsigned type, void const* value, size_t valueSize);

  public:
    void SetSZ(wchar_t const* keyName, wchar_t const* valueName, LPCWSTR value);
    void SetDW(wchar_t const* keyName, wchar_t const* valueName, DWORD value);
    void SetBytes(wchar_t const* keyName, wchar_t const* valueName, void const* value, size_t count);

    void Unregister();
  };
} // namespace wicx
