#pragma once

#include <mutex>
#include <shared_mutex>
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

#ifdef NDEBUG
#  define CheckMutex(...) /**/
#else
  void CheckMutex(std::mutex& mutex, const std::string& funcName);
  void CheckMutex(std::shared_mutex& mutex, const std::string& funcName);
#endif

  std::wstring GetDLLFilepath();
} // namespace wicx
