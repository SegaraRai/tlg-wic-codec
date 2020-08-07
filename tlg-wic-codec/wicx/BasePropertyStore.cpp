#include "BasePropertyStore.hpp"

#include "Util.hpp"

namespace wicx {
  //----------------------------------------------------------------------------------------
  // BasePropertyStore implementation
  //----------------------------------------------------------------------------------------

  BasePropertyStore::BasePropertyStore(GUID Me) : m_pPropertyCache(nullptr), m_CLSID_This(Me) {
    // MessageBoxW(NULL, L"PropertyStore()", L"tlg_wic_codec", MB_OK);
  }

  BasePropertyStore::~BasePropertyStore() {
    // MessageBoxW(NULL, L"~PropertyStore()", L"tlg_wic_codec", MB_OK);
    ReleaseMembers();
  }

  void BasePropertyStore::ReleaseMembers() {
    WICX_RELEASE(m_pPropertyCache);
  }

  // ----- IUnknown interface ---------------------------------------------------------------------------

  STDMETHODIMP BasePropertyStore::QueryInterface(REFIID iid, void** ppvObject) {
    if (!ppvObject) {
      return E_INVALIDARG;
    }

    *ppvObject = nullptr;

    if (iid == IID_IUnknown) {
      *ppvObject = static_cast<IUnknown*>(static_cast<IPropertyStore*>(this));
    } else if (iid == IID_IPropertyStore) {
      *ppvObject = static_cast<IPropertyStore*>(this);
    } else if (iid == IID_IPropertyStoreCapabilities) {
      *ppvObject = static_cast<IPropertyStoreCapabilities*>(this);
    } else if (iid == IID_IInitializeWithStream) {
      *ppvObject = static_cast<IInitializeWithStream*>(this);
    } else {
      return E_NOINTERFACE;
    }

    AddRef();

    return S_OK;
  }

  STDMETHODIMP_(ULONG) BasePropertyStore::AddRef() {
    return m_unknownImpl.AddRef();
  }

  STDMETHODIMP_(ULONG) BasePropertyStore::Release() {
    const ULONG result = m_unknownImpl.Release();

    if (result == 0) {
      delete this;
    }

    return result;
  }

  // ----- IInitializeWithStream interface ------------------------------------------------------------------

  STDMETHODIMP BasePropertyStore::Initialize(IStream* pstream, DWORD grfMode) {
    // return error if already initialized
    if (m_pPropertyCache) {
      return HRESULT_FROM_WIN32(ERROR_ALREADY_INITIALIZED);
    }

    // check arg
    if (!pstream) {
      return E_INVALIDARG;
    }

    // return error if stream is write only
    if ((grfMode & 3) == STGM_WRITE) {
      return E_ACCESSDENIED;
    }

    // release PropertyCache (should not occur but in case)
    ReleaseMembers();

    // create PropertyCache
    if (const auto result = PSCreateMemoryPropertyStore(IID_PPV_ARGS(&m_pPropertyCache)); FAILED(result)) {
      return result;
    }

    // load props
    const auto result = LoadProperties(m_pPropertyCache, pstream);

    if (FAILED(result)) {
      ReleaseMembers();
    }

    return result;
  }

  // ----- IPropertyStoreCapabilities interface ------------------------------------------------------------------

  STDMETHODIMP BasePropertyStore::IsPropertyWritable(REFPROPERTYKEY key) {
    UNREFERENCED_PARAMETER(key);

    if (!m_pPropertyCache) {
      return E_UNEXPECTED;
    }

    return S_FALSE;
  }

  // ----- IPropertyStore interface ------------------------------------------------------------------

  STDMETHODIMP BasePropertyStore::Commit() {
    if (!m_pPropertyCache) {
      return E_UNEXPECTED;
    }

    return S_OK;
  }

  STDMETHODIMP BasePropertyStore::GetAt(DWORD iProp, PROPERTYKEY* pKey) {
    if (!m_pPropertyCache) {
      return E_UNEXPECTED;
    }

    return m_pPropertyCache->GetAt(iProp, pKey);
  }

  STDMETHODIMP BasePropertyStore::GetCount(DWORD* cProps) {
    if (!m_pPropertyCache) {
      return E_UNEXPECTED;
    }

    return m_pPropertyCache->GetCount(cProps);
  }

  STDMETHODIMP BasePropertyStore::GetValue(REFPROPERTYKEY key, PROPVARIANT* pv) {
    if (!m_pPropertyCache) {
      return E_UNEXPECTED;
    }

    return m_pPropertyCache->GetValue(key, pv);
  }

  STDMETHODIMP BasePropertyStore::SetValue(REFPROPERTYKEY key, REFPROPVARIANT propvar) {
    if (!m_pPropertyCache) {
      return E_UNEXPECTED;
    }

    /*
    return m_propertyCache->SetValue(key, propvar);
    /*/
    UNREFERENCED_PARAMETER(key);
    UNREFERENCED_PARAMETER(propvar);

    return STG_E_ACCESSDENIED;
    //*/
  }
} // namespace wicx
