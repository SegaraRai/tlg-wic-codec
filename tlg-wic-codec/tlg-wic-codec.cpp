#include "StdAfx.hpp"
#include "tlgx/TLGDecoder.hpp"
#include "tlgx/TLGPropertyStore.hpp"
#include "wicx/ClassFactory.hpp"
#include "wicx/RegMan.hpp"

#include <shlobj.h>

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

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, void** ppv) {
  HRESULT result = E_INVALIDARG;

  if (NULL != ppv) {
    IClassFactory* classFactory = NULL;

    if (CLSID_TLG_Decoder == rclsid) {
      result = S_OK;
      classFactory = new wicx::ClassFactory<tlgx::TLG_Decoder>();
    } else if (CLSID_TLG_PropertyStore == rclsid) {
      result = S_OK;
      classFactory = new wicx::ClassFactory<tlgx::TLG_PropertyStore>();
    } else
      result = E_NOINTERFACE;

    if (SUCCEEDED(result)) {
      if (NULL != classFactory)
        result = classFactory->QueryInterface(riid, ppv);
      else
        result = E_OUTOFMEMORY;
    }
  }

  return result;
}

BOOL APIENTRY DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
  UNREFERENCED_PARAMETER(lpvReserved);

  switch (fdwReason) {
    case DLL_PROCESS_ATTACH:
      DisableThreadLibraryCalls(hinstDLL);
      break;

    case DLL_PROCESS_DETACH:
      break;
  }

  return TRUE;
}
