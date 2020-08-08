#include "tlgx/TLGDecoder.hpp"
#include "tlgx/TLGPropertyStore.hpp"
#include "wicx/ClassFactory.hpp"
#include "wicx/RegMan.hpp"

#include <new>

#include <Windows.h>

#include <objbase.h>
#include <ShlObj.h>
#include <Unknwn.h>
#include <winerror.h>

HINSTANCE g_hinstDLL = NULL;

STDAPI DllRegisterServer() {
  try {
    wicx::RegMan regMan(wicx::RegMan::Mode::Register);

    tlgx::TLG_Decoder::Register(regMan);
    tlgx::TLG_PropertyStore::Register(regMan);
  } catch (wicx::Win32Error& error) {
    return HRESULT_FROM_WIN32(error.GetError());
  } catch (...) {
    return E_FAIL;
  }

  SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, NULL, NULL);

  return S_OK;
}

STDAPI DllUnregisterServer() {
  try {
    wicx::RegMan regMan(wicx::RegMan::Mode::Unregister);

    tlgx::TLG_Decoder::Register(regMan);
    tlgx::TLG_PropertyStore::Register(regMan);

    regMan.Unregister();
  } catch (wicx::Win32Error& error) {
    return HRESULT_FROM_WIN32(error.GetError());
  } catch (...) {
    return E_FAIL;
  }

  return S_OK;
}

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv) {
  if (!ppv) {
    return E_INVALIDARG;
  }

  IClassFactory* classFactory = nullptr;

  try {
    if (CLSID_TLG_Decoder == rclsid) {
      classFactory = new wicx::ClassFactory<tlgx::TLG_Decoder>();
    } else if (CLSID_TLG_PropertyStore == rclsid) {
      classFactory = new wicx::ClassFactory<tlgx::TLG_PropertyStore>();
    } else {
      return E_NOINTERFACE;
    }
  } catch (std::bad_alloc&) {
    // no memory
    return E_OUTOFMEMORY;
  } catch (...) {
    // unknown error
    return E_FAIL;
  }

  return classFactory->QueryInterface(riid, ppv);
}

BOOL APIENTRY DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
  UNREFERENCED_PARAMETER(lpvReserved);

  switch (fdwReason) {
    case DLL_PROCESS_ATTACH:
      DisableThreadLibraryCalls(hinstDLL);
      g_hinstDLL = hinstDLL;
      break;

    case DLL_PROCESS_DETACH:
      break;
  }

  return TRUE;
}
