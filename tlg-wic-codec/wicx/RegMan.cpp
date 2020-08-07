#define NOMINMAX

#include "RegMan.hpp"

#include <stdexcept>
#include <string>

#include <Windows.h>

using namespace std::literals;

namespace wicx {
  RegMan::RegMan(RegMan::Mode mode) : m_mode(mode) {}

  void RegMan::Create(const std::wstring& keyName) {
    SetRaw(keyName, L""s, REG_NONE, nullptr, 0);
  }

  void RegMan::SetSZ(const std::wstring& keyName, const std::wstring& valueName, const std::wstring& value) {
    SetRaw(keyName, valueName, REG_SZ, value.c_str(), value.size() * sizeof(wchar_t));
  }

  void RegMan::SetDW(const std::wstring& keyName, const std::wstring& valueName, DWORD value) {
    SetRaw(keyName, valueName, REG_DWORD, &value, sizeof(value));
  }

  void RegMan::SetBytes(const std::wstring& keyName, const std::wstring& valueName, const void* value, std::size_t count) {
    SetRaw(keyName, valueName, REG_BINARY, value, count);
  }

  void RegMan::SetRaw(const std::wstring& keyName, const std::wstring& valueName, DWORD type, const void* value, std::size_t valueSize) {
    switch (m_mode) {
      case Mode::Register: {
        HKEY hKey = NULL;

        if (const auto result = RegCreateKeyExW(HKEY_CLASSES_ROOT, keyName.c_str(), 0, NULL, REG_OPTION_NON_VOLATILE, KEY_READ | KEY_WRITE, NULL, &hKey, NULL); result != ERROR_SUCCESS) {
          throw std::runtime_error("RegCreateKeyExW failed");
        }

        // skip setting value if valueName == "" and type == REG_NONE
        // though this is not a correct use of REG_NONE, it won't be a practical problem
        if (!valueName.empty() || type != REG_NONE) {
          if (const auto result = RegSetValueExW(hKey, valueName.c_str(), 0, type, reinterpret_cast<BYTE const*>(value), static_cast<DWORD>(valueSize)); result != ERROR_SUCCESS) {
            throw std::runtime_error("RegSetValueExW failed");
          }
        }

        break;
      }

      case Mode::Unregister: {
        if (m_keys.empty() || m_keys.front() != keyName) {
          // stack (FILO / LIFO)
          m_keys.push_front(keyName);
        }

        break;
      }
    }
  }

  void RegMan::Unregister() {
    if (m_mode == Mode::Register) {
      return;
    }

    for (const auto& key : m_keys) {
      if (const auto result = RegDeleteKeyExW(HKEY_CLASSES_ROOT, key.c_str(), 0, 0); result != ERROR_SUCCESS && result != ERROR_FILE_NOT_FOUND) {
        throw std::runtime_error("RegDeleteKeyExW failed");
      }
    }
  }
} // namespace wicx
