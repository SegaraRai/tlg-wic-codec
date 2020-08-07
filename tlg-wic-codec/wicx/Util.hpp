#pragma once

#include <string>

#include <Windows.h>

#define __STR2CPPWSTR(str) L##str##s
#define _STR2CPPWSTR(str) __STR2CPPWSTR(str)

namespace wicx {
  template<typename T>
  void WICX_RELEASE(T& pObject) {
    if (pObject != nullptr) {
      pObject->Release();
      pObject = nullptr;
    }
  }

  constexpr LARGE_INTEGER MakeLI(LONGLONG value) {
    LARGE_INTEGER li{};
    li.QuadPart = value;
    return li;
  }

  std::wstring GetDLLFilepath();
} // namespace wicx
