#define NOMINMAX

#include "RegMan.hpp"

#include <string>

#include <Windows.h>

namespace wicx {
  void RegMan::SetSZ(const std::wstring& keyName, const std::wstring& valueName, const std::wstring& value) {
    SetRaw(keyName, valueName, REG_SZ, value.c_str(), value.size() * sizeof(wchar_t));
  }

  void RegMan::SetDW(const std::wstring& keyName, const std::wstring& valueName, DWORD value) {
    SetRaw(keyName, valueName, REG_DWORD, &value, sizeof(value));
  }

  void RegMan::SetBytes(const std::wstring& keyName, const std::wstring& valueName, const void* value, size_t count) {
    SetRaw(keyName, valueName, REG_BINARY, value, count);
  }

  void RegMan::SetRaw(const std::wstring& keyName, const std::wstring& valueName, unsigned type, const void* value, size_t valueSize) {
    // stack (FILO / LIFO)
    m_keys.push_front(keyName);

    HKEY hKey;
    long err = RegCreateKeyW(HKEY_CLASSES_ROOT, keyName.c_str(), &hKey);
    if (err == ERROR_SUCCESS) {
      err = RegSetValueExW(hKey, valueName.c_str(), 0, type, reinterpret_cast<BYTE const*>(value), static_cast<DWORD>(valueSize));
    }
  }

  void RegMan::Unregister() {
    for (const auto& key : m_keys) {
      RegDeleteKeyW(HKEY_CLASSES_ROOT, key.c_str());
    }
  }
} // namespace wicx
