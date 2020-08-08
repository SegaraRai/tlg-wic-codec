#pragma once

#include "../StdAfx.hpp"
#include "UnknownImpl.hpp"

namespace wicx {
  template<typename T>
  class ClassFactory : public IClassFactory {
  public:
    // IUnknown interface

    STDMETHOD(QueryInterface)(REFIID riid, void** ppv) {
      HRESULT result = E_INVALIDARG;

      if (ppv) {
        if (riid == IID_IUnknown) {
          *ppv = static_cast<IUnknown*>(this);
          AddRef();

          result = S_OK;
        } else if (riid == IID_IClassFactory) {
          *ppv = static_cast<IClassFactory*>(this);
          AddRef();

          result = S_OK;
        } else {
          result = E_NOINTERFACE;
        }
      }

      return result;
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

      HRESULT result = E_INVALIDARG;

      if (NULL != ppv) {
        T* obj = new T();

        if (NULL != obj) {
          result = obj->QueryInterface(riid, ppv);
        } else {
          *ppv = NULL;
          result = E_OUTOFMEMORY;
        }
      }

      return result;
    }

    STDMETHOD(LockServer)(BOOL fLock) {
      return CoLockObjectExternal(this, fLock, FALSE);
    }

  private:
    UnknownImpl unknownImpl;
  };
} // namespace wicx
