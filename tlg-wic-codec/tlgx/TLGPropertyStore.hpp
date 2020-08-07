#pragma once

#include "../StdAfx.hpp"
#include "../wicx/BasePropertyStore.hpp"
#include "../wicx/RegMan.hpp"

extern const GUID CLSID_TLG_PropertyStore;

namespace tlgx {
  using namespace wicx;

  class TLG_PropertyStore : public BasePropertyStore {
  public:
    static void Register(RegMan& regMan);

    TLG_PropertyStore();
    ~TLG_PropertyStore();

  protected:
    HRESULT LoadProperties(IPropertyStoreCache* pPropertyCache, IStream* pStream) override;
  };
} // namespace tlgx
