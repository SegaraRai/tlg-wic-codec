#pragma once

#include <deque>
#include <string>

#include <Windows.h>

namespace wicx {
  class RegMan {
    std::deque<std::wstring> m_keys;

    void SetRaw(const std::wstring& keyName, const std::wstring& valueName, unsigned type, const void* value, size_t valueSize);

  public:
    void SetSZ(const std::wstring& keyName, const std::wstring& valueName, const std::wstring& value);
    void SetDW(const std::wstring& keyName, const std::wstring& valueName, DWORD value);
    void SetBytes(const std::wstring& keyName, const std::wstring& valueName, const void* value, size_t count);

    void Unregister();
  };
} // namespace wicx
