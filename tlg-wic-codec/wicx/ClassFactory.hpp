#pragma once

#include "UnknownImpl.hpp"

#include <new>

#include <Windows.h>

#include <Unknwn.h>
#include <objbase.h>
#include <winerror.h>

namespace wicx {
  template<typename T>
  class ClassFactory : public IClassFactory {
    UnknownImpl unknownImpl;

  public:
    // IUnknown interface

    STDMETHOD(QueryInterface)(REFIID riid, void** ppv) {
      if (!ppv) {
        return E_INVALIDARG;
      }

      if (riid == IID_IUnknown) {
        *ppv = static_cast<IUnknown*>(this);
      } else if (riid == IID_IClassFactory) {
        *ppv = static_cast<IClassFactory*>(this);
      } else {
        return E_NOINTERFACE;
      }

      AddRef();

      return S_OK;
    }

    STDMETHOD_(ULONG, AddRef)() {
      return unknownImpl.AddRef();
    }

    STDMETHOD_(ULONG, Release)() {
      const auto count = unknownImpl.Release();
      if (count == 0) {
        delete this;
      }
      return count;
    }

    // IClassFactory interface

    STDMETHOD(CreateInstance)(IUnknown* pUnkOuter, REFIID riid, void** ppv) {
      UNREFERENCED_PARAMETER(pUnkOuter);

      if (!ppv) {
        return E_INVALIDARG;
      }

      try {
        T* obj = new T();
        return obj->QueryInterface(riid, ppv);
      } catch (std::bad_alloc&) {
        // no memory
        return E_OUTOFMEMORY;
      } catch (...) {
        // unknown exception
        return E_FAIL;
      }
    }

    STDMETHOD(LockServer)(BOOL fLock) {
      return CoLockObjectExternal(this, fLock, FALSE);
    }
  };
} // namespace wicx
