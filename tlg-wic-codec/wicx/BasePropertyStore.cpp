#include "BasePropertyStore.hpp"

#include <shared_mutex>

#include "Util.hpp"

namespace wicx {
  //----------------------------------------------------------------------------------------
  // BasePropertyStore implementation
  //----------------------------------------------------------------------------------------

  BasePropertyStore::BasePropertyStore() {
    // MessageBoxW(NULL, L"PropertyStore()", L"tlg_wic_codec", MB_OK);
  }

  BasePropertyStore::~BasePropertyStore() {
    // MessageBoxW(NULL, L"~PropertyStore()", L"tlg_wic_codec", MB_OK);
    ReleaseMembers();
  }

  void BasePropertyStore::ReleaseMembersWithoutLock() {
    WICX_RELEASE(m_pPropertyCache);
  }

  void BasePropertyStore::ReleaseMembers() {
    CheckMutex(m_mutex, __FUNCSIG__);
    std::lock_guard lock(m_mutex);

    ReleaseMembersWithoutLock();
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
    const auto count = m_unknownImpl.Release();
    if (count == 0) {
      delete this;
    }
    return count;
  }

  // ----- IInitializeWithStream interface ------------------------------------------------------------------

  STDMETHODIMP BasePropertyStore::Initialize(IStream* pstream, DWORD grfMode) {
    CheckMutex(m_mutex, __FUNCSIG__);
    std::lock_guard lock(m_mutex);

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
    ReleaseMembersWithoutLock();

    // create PropertyCache
    if (const auto result = PSCreateMemoryPropertyStore(IID_PPV_ARGS(&m_pPropertyCache)); FAILED(result)) {
      return result;
    }

    // load props
    if (const auto result = LoadProperties(m_pPropertyCache, pstream); FAILED(result)) {
      ReleaseMembersWithoutLock();
      return result;
    }

    return S_OK;
  }

  // ----- IPropertyStoreCapabilities interface ------------------------------------------------------------------

  STDMETHODIMP BasePropertyStore::IsPropertyWritable(REFPROPERTYKEY key) {
    CheckMutex(m_mutex, __FUNCSIG__);
    std::shared_lock lock(m_mutex);

    UNREFERENCED_PARAMETER(key);

    if (!m_pPropertyCache) {
      return E_UNEXPECTED;
    }

    return S_FALSE;
  }

  // ----- IPropertyStore interface ------------------------------------------------------------------

  STDMETHODIMP BasePropertyStore::Commit() {
    CheckMutex(m_mutex, __FUNCSIG__);
    std::shared_lock lock(m_mutex);

    if (!m_pPropertyCache) {
      return E_UNEXPECTED;
    }

    return S_OK;
  }

  STDMETHODIMP BasePropertyStore::GetAt(DWORD iProp, PROPERTYKEY* pKey) {
    CheckMutex(m_mutex, __FUNCSIG__);
    std::shared_lock lock(m_mutex);

    if (!m_pPropertyCache) {
      return E_UNEXPECTED;
    }

    return m_pPropertyCache->GetAt(iProp, pKey);
  }

  STDMETHODIMP BasePropertyStore::GetCount(DWORD* cProps) {
    CheckMutex(m_mutex, __FUNCSIG__);
    std::shared_lock lock(m_mutex);

    if (!m_pPropertyCache) {
      return E_UNEXPECTED;
    }

    return m_pPropertyCache->GetCount(cProps);
  }

  STDMETHODIMP BasePropertyStore::GetValue(REFPROPERTYKEY key, PROPVARIANT* pv) {
    CheckMutex(m_mutex, __FUNCSIG__);
    std::shared_lock lock(m_mutex);

    if (!m_pPropertyCache) {
      return E_UNEXPECTED;
    }

    return m_pPropertyCache->GetValue(key, pv);
  }

  STDMETHODIMP BasePropertyStore::SetValue(REFPROPERTYKEY key, REFPROPVARIANT propvar) {
    CheckMutex(m_mutex, __FUNCSIG__);
    std::shared_lock lock(m_mutex);

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
