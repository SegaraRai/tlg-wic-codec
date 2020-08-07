#include "RegMan.hpp"

namespace wicx {
  void RegMan::SetSZ(wchar_t const* keyName, wchar_t const* valueName, LPCWSTR value) {
    SetRaw(keyName, valueName, REG_SZ, value, sizeof(wchar_t) * (wcslen(value)));
  }

  void RegMan::SetDW(wchar_t const* keyName, wchar_t const* valueName, DWORD value) {
    SetRaw(keyName, valueName, REG_DWORD, &value, sizeof(value));
  }

  void RegMan::SetBytes(wchar_t const* keyName, wchar_t const* valueName, void const* value, size_t count) {
    SetRaw(keyName, valueName, REG_BINARY, value, count);
  }

  void RegMan::SetRaw(wchar_t const* keyName, wchar_t const* valueName, unsigned type, void const* value, size_t valueSize) {
    m_keys.push_back(keyName);

    HKEY hKey;
    long err = RegCreateKeyW(HKEY_CLASSES_ROOT, keyName, &hKey);
    if (err == ERROR_SUCCESS) {
      err = RegSetValueExW(hKey, valueName, 0, type, reinterpret_cast<BYTE const*>(value), static_cast<DWORD>(valueSize));
    }
  }

  void RegMan::Unregister() {
    for (std::size_t i = m_keys.size(); i > 0; i--) {
      RegDeleteKeyW(HKEY_CLASSES_ROOT, m_keys[i - 1]);
    }
  }
} // namespace wicx
