#pragma once

#include "../StdAfx.hpp"
#include "UnknownImpl.hpp"

namespace wicx {
  class BasePropertyStore
    : public IPropertyStore
    , public IPropertyStoreCapabilities
    , public IInitializeWithStream {
    UnknownImpl m_unknownImpl;
    IPropertyStoreCache* m_pPropertyCache;

  protected:
    GUID const m_CLSID_This;

    virtual void ReleaseMembers();
    virtual HRESULT LoadProperties(IPropertyStoreCache* pPropertyCache, IStream* pStream) = 0;

  public:
    BasePropertyStore(GUID Me);
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
