#pragma once

#include "UnknownImpl.hpp"

#include <shared_mutex>

#include <Windows.h>

#include <ObjIdl.h>
#include <PropIdl.h>
#include <propsys.h>
#include <wtypes.h>

namespace wicx {
  class BasePropertyStore
    : public IPropertyStore
    , public IPropertyStoreCapabilities
    , public IInitializeWithStream {
    std::shared_mutex m_mutex;
    UnknownImpl m_unknownImpl;

    IPropertyStoreCache* m_pPropertyCache = nullptr;

  protected:
    void ReleaseMembersWithoutLock();
    void ReleaseMembers();

    virtual HRESULT LoadProperties(IPropertyStoreCache* pPropertyCache, IStream* pStream) = 0;

  public:
    BasePropertyStore();
    virtual ~BasePropertyStore();

    // IUnknown interface

    STDMETHOD(QueryInterface)(REFIID riid, void** ppv);
    STDMETHOD_(ULONG, AddRef)();
    STDMETHOD_(ULONG, Release)();

    // IInitializeWithStream interface

    STDMETHOD(Initialize)(IStream* pstream, DWORD grfMode);

    // IPropertyStoreCapabilities interface

    STDMETHOD(IsPropertyWritable)(REFPROPERTYKEY key);

    // IPropertyStore interface

    STDMETHOD(Commit)();
    STDMETHOD(GetAt)(DWORD iProp, PROPERTYKEY* pKey);
    STDMETHOD(GetCount)(DWORD* cProps);
    STDMETHOD(GetValue)(REFPROPERTYKEY key, PROPVARIANT* pv);
    STDMETHOD(SetValue)(REFPROPERTYKEY key, REFPROPVARIANT propvar);
  };
} // namespace wicx
