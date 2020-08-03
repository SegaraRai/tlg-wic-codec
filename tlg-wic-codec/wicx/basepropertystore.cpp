#include "basepropertystore.hpp"

#define WICX_RELEASE(X) \
  if (X != nullptr) {   \
    X->Release();       \
    X = nullptr;        \
  }

namespace wicx {
  //----------------------------------------------------------------------------------------
  // BasePropertyStore implementation
  //----------------------------------------------------------------------------------------

  BasePropertyStore::BasePropertyStore(GUID Me) : m_pPropertyCache(nullptr), m_CLSID_This(Me) {
    // MessageBoxW(NULL, L"PropertyStore()", L"dds_wic_codec", MB_OK);
  }

  BasePropertyStore::~BasePropertyStore() {
    // MessageBoxW(NULL, L"~PropertyStore()", L"dds_wic_codec", MB_OK);
    ReleaseMembers();
  }

  void BasePropertyStore::ReleaseMembers() {
    WICX_RELEASE(m_pPropertyCache);
  }

  // ----- IUnknown interface ---------------------------------------------------------------------------

  STDMETHODIMP BasePropertyStore::QueryInterface(REFIID iid, void** ppvObject) {
    if (!ppvObject) {
      return S_OK;
    }

    *ppvObject = nullptr;

    if (iid == IID_IUnknown || iid == IID_IPropertyStore) {
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
      OutputDebugStringW(L"BasePropertyStore: already initialized\n");
      return HRESULT_FROM_WIN32(ERROR_ALREADY_INITIALIZED);
    }

    // check arg
    if (!pstream) {
      OutputDebugStringW(L"BasePropertyStore: invalid arg\n");
      return E_INVALIDARG;
    }

    // return error if stream is write only
    if ((grfMode & 3) == STGM_WRITE) {
      OutputDebugStringW(L"BasePropertyStore: invalid stream\n");
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
      OutputDebugStringW(L"BasePropertyStore: failed\n");
      ReleaseMembers();
    }

    OutputDebugStringW(L"BasePropertyStore: OK\n");

    return result;
  }

  // ----- IPropertyStoreCapabilities interface ------------------------------------------------------------------

  STDMETHODIMP BasePropertyStore::IsPropertyWritable(REFPROPERTYKEY key) {
    OutputDebugStringW(L"BasePropertyStore::IsPropertyWritable\n");

    UNREFERENCED_PARAMETER(key);

    if (!m_pPropertyCache) {
      return E_UNEXPECTED;
    }

    return S_FALSE;
  }

  // ----- IPropertyStore interface ------------------------------------------------------------------

  STDMETHODIMP BasePropertyStore::Commit() {
    OutputDebugStringW(L"BasePropertyStore::Commit\n");

    if (!m_pPropertyCache) {
      return E_UNEXPECTED;
    }

    return S_OK;
  }

  STDMETHODIMP BasePropertyStore::GetAt(DWORD iProp, PROPERTYKEY* pKey) {
    OutputDebugStringW(L"BasePropertyStore::GetAt\n");

    if (!m_pPropertyCache) {
      return E_UNEXPECTED;
    }

    return m_pPropertyCache->GetAt(iProp, pKey);
  }

  STDMETHODIMP BasePropertyStore::GetCount(DWORD* cProps) {
    OutputDebugStringW(L"BasePropertyStore::GetCount\n");

    if (!m_pPropertyCache) {
      return E_UNEXPECTED;
    }

    return m_pPropertyCache->GetCount(cProps);
  }

  STDMETHODIMP BasePropertyStore::GetValue(REFPROPERTYKEY key, PROPVARIANT* pv) {
    OutputDebugStringW(L"BasePropertyStore::GetValue ");

    PWSTR pstr = nullptr;
    if (SUCCEEDED(PSGetNameFromPropertyKey(key, &pstr)) && pstr != nullptr) {
      OutputDebugStringW(pstr);
      CoTaskMemFree(pstr);
      pstr = nullptr;
    }
    OutputDebugStringW(L"\n");

    if (!m_pPropertyCache) {
      return E_UNEXPECTED;
    }

    return m_pPropertyCache->GetValue(key, pv);
  }

  STDMETHODIMP BasePropertyStore::SetValue(REFPROPERTYKEY key, REFPROPVARIANT propvar) {
    OutputDebugStringW(L"BasePropertyStore::SetValue\n");

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
