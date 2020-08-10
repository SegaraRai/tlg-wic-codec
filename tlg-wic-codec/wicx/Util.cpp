#include "Util.hpp"

#include "../Version.h"

#include <mutex>
#include <shared_mutex>
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

    HMODULE hModule = g_hinstDLL ? reinterpret_cast<HMODULE>(g_hinstDLL) : GetModuleHandleW(_STR2WSTR(TLG_WIC_CODEC_FILENAME));
    if (!hModule) {
      throw std::runtime_error("failed to get hModule");
    }

    wchar_t filepath[MAX_PATH];
    if (GetModuleFileNameW(hModule, filepath, MAX_PATH) == 0) {
      throw std::runtime_error("GetModuleFileNameW failed");
    }

    return filepath;
  }

#ifndef NDEBUG
  void CheckMutex(std::mutex& mutex, const std::string& funcName) {
    using namespace std::literals;

    bool lockableExclusive = false;

    if (mutex.try_lock()) {
      lockableExclusive = true;
      mutex.unlock();
    }

    const std::string str = funcName + ": x="s + (lockableExclusive ? "OK"s : "NG"s) + "\n"s;
    OutputDebugStringA(str.c_str());
  }

  void CheckMutex(std::shared_mutex& mutex, const std::string& funcName) {
    using namespace std::literals;

    bool lockableExclusive = false;
    bool lockableShared = false;

    if (mutex.try_lock_shared()) {
      lockableShared = true;
      mutex.unlock_shared();
    }

    if (mutex.try_lock()) {
      lockableExclusive = true;
      mutex.unlock();
    }

    const std::string str = funcName + ": x="s + (lockableExclusive ? "OK"s : "NG"s) + ", s="s + (lockableShared ? "OK"s : "NG"s) + "\n"s;
    OutputDebugStringA(str.c_str());
  }
#endif
} // namespace wicx
