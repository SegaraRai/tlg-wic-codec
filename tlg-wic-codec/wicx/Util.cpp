#include "Util.hpp"

#include <stdexcept>
#include <string>

// see tlg-wic-codec.cpp
extern HINSTANCE g_hinstDLL;

namespace wicx {
  std::wstring GetDLLFilepath() {
    static std::wstring dllFilePath;

    if (!dllFilePath.empty()) {
      return dllFilePath;
    }

    HMODULE hModule = g_hinstDLL ? reinterpret_cast<HMODULE>(g_hinstDLL) : GetModuleHandleW(L"tlg-wic-codec.dll");
    if (!hModule) {
      throw std::runtime_error("failed to get hModule");
    }

    wchar_t filepath[MAX_PATH];
    if (GetModuleFileNameW(hModule, filepath, MAX_PATH) == 0) {
      throw std::runtime_error("GetModuleFileNameW failed");
    }

    return filepath;
  }
} // namespace wicx
