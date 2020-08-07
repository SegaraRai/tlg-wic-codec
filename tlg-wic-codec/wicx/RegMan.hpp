#pragma once

#include <deque>
#include <stdexcept>
#include <string>

#include <Windows.h>

namespace wicx {
  class Win32Error : public std::runtime_error {
    DWORD m_error;

  public:
    Win32Error(DWORD error);
    
    DWORD GetError() const;
  };

  class RegMan {
  public:
    enum class Mode {
      Register,
      Unregister,
    };

  private:
    Mode m_mode;
    std::deque<std::wstring> m_keys;

    void SetRaw(const std::wstring& keyName, const std::wstring& valueName, DWORD type, const void* value, std::size_t valueSize);
    
  public:
    RegMan(Mode mode);

    void Unregister();

    void Create(const std::wstring& keyName);
    void SetSZ(const std::wstring& keyName, const std::wstring& valueName, const std::wstring& value);
    void SetDW(const std::wstring& keyName, const std::wstring& valueName, DWORD value);
    void SetBytes(const std::wstring& keyName, const std::wstring& valueName, const void* value, std::size_t count);
  };
} // namespace wicx
