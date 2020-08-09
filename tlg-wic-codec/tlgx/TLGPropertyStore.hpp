#pragma once

#include "../wicx/BasePropertyStore.hpp"
#include "../wicx/RegMan.hpp"

#include <mutex>

#include <Windows.h>

#include <ObjIdl.h>
#include <propsys.h>

extern const GUID CLSID_TLG_PropertyStore;

namespace tlgx {
  class TLG_PropertyStore final : public wicx::BasePropertyStore {
    std::mutex mutex;

  protected:
    HRESULT LoadProperties(IPropertyStoreCache* pPropertyCache, IStream* pStream) override;

  public:
    static void Register(wicx::RegMan& regMan);

    TLG_PropertyStore();
    ~TLG_PropertyStore();
  };
} // namespace tlgx
