#include "BaseDecoder.hpp"

#include "Util.hpp"

#include <cassert>
#include <shared_mutex>

#include <Windows.h>

#include <guiddef.h>
#include <objbase.h>
#include <Unknwn.h>
#include <wincodec.h>
#include <winerror.h>

namespace wicx {
  //----------------------------------------------------------------------------------------
  // BaseFrameDecode implementation
  //----------------------------------------------------------------------------------------

  BaseFrameDecode::BaseFrameDecode(IWICImagingFactory* pIFactory, UINT num) : m_frameNumber(num), m_factory(pIFactory) {
    if (m_factory) {
      m_factory->AddRef();
    }
  }

  BaseFrameDecode::~BaseFrameDecode() {
    ReleaseMembers();
  }

  void BaseFrameDecode::ReleaseMembers() {
    CheckMutex(m_mutex, __FUNCSIG__);
    std::lock_guard lock(m_mutex);

    WICX_RELEASE(m_factory);
    WICX_RELEASE(m_bitmapSource);
    WICX_RELEASE(m_palette);
    for (auto& colorContext : m_colorContexts) {
      WICX_RELEASE(colorContext);
    }
    WICX_RELEASE(m_thumbnail);
    WICX_RELEASE(m_preview);
  }

  // ----- IUnknown interface ---------------------------------------------------------------------------

  STDMETHODIMP BaseFrameDecode::QueryInterface(REFIID iid, void** ppvObject) {
    CheckMutex(m_mutex, __FUNCSIG__);
    std::shared_lock lock(m_mutex);

    if (!ppvObject) {
      return E_INVALIDARG;
    }

    if (iid == IID_IUnknown) {
      *ppvObject = static_cast<IUnknown*>(this);
      AddRef();
    } else if (iid == IID_IWICBitmapFrameDecode) {
      *ppvObject = static_cast<IWICBitmapFrameDecode*>(this);
      AddRef();
    } else if (iid == IID_IWICBitmapSource) {
      if (!m_bitmapSource) {
        return E_NOINTERFACE;
      }
      *ppvObject = m_bitmapSource;
      m_bitmapSource->AddRef();
    } else {
      return E_NOINTERFACE;
    }

    return S_OK;
  }

  STDMETHODIMP_(ULONG) BaseFrameDecode::AddRef() {
    return m_unknownImpl.AddRef();
  }

  STDMETHODIMP_(ULONG) BaseFrameDecode::Release() {
    const auto count = m_unknownImpl.Release();
    if (count == 0) {
      delete this;
    }
    return count;
  }

  // ----- IWICBitmapFrameDecode interface --------------------------------------------------------------

  STDMETHODIMP BaseFrameDecode::GetMetadataQueryReader(IWICMetadataQueryReader** ppIMetadataQueryReader) {
    UNREFERENCED_PARAMETER(ppIMetadataQueryReader);
    return WINCODEC_ERR_UNSUPPORTEDOPERATION;
  }

  STDMETHODIMP BaseFrameDecode::GetColorContexts(UINT cCount, IWICColorContext** ppIColorContexts, UINT* pcActualCount) {
    CheckMutex(m_mutex, __FUNCSIG__);
    std::shared_lock lock(m_mutex);

    if (ppIColorContexts) {
      // return the actual color contexts

      if (cCount > m_colorContexts.size()) {
        return E_INVALIDARG;
      }

      for (size_t i = 0; i < cCount; i++) {
        ppIColorContexts[i] = m_colorContexts[i];
        m_colorContexts[i]->AddRef();
      }
    } else {
      // return the number of color contexts

      if (!pcActualCount) {
        return E_INVALIDARG;
      }

      *pcActualCount = static_cast<UINT>(m_colorContexts.size());
    }

    return S_OK;
  }

  STDMETHODIMP BaseFrameDecode::GetThumbnail(IWICBitmapSource** ppIThumbnail) {
    CheckMutex(m_mutex, __FUNCSIG__);
    std::shared_lock lock(m_mutex);

    if (!ppIThumbnail) {
      return E_INVALIDARG;
    }

    if (!m_thumbnail) {
      return WINCODEC_ERR_CODECNOTHUMBNAIL;
    }

    *ppIThumbnail = m_thumbnail;

    m_thumbnail->AddRef();

    return S_OK;
  }

  // ----- IWICBitmapSource interface -------------------------------------------------------------------

  STDMETHODIMP BaseFrameDecode::GetSize(UINT* puiWidth, UINT* puiHeight) {
    CheckMutex(m_mutex, __FUNCSIG__);
    std::shared_lock lock(m_mutex);

    if (!m_bitmapSource) {
      return E_UNEXPECTED;
    }

    return m_bitmapSource->GetSize(puiWidth, puiHeight);
  }

  STDMETHODIMP BaseFrameDecode::GetPixelFormat(WICPixelFormatGUID* pPixelFormat) {
    CheckMutex(m_mutex, __FUNCSIG__);
    std::shared_lock lock(m_mutex);

    if (!m_bitmapSource) {
      return E_UNEXPECTED;
    }

    return m_bitmapSource->GetPixelFormat(pPixelFormat);
  }

  STDMETHODIMP BaseFrameDecode::GetResolution(double* pDpiX, double* pDpiY) {
    CheckMutex(m_mutex, __FUNCSIG__);
    std::shared_lock lock(m_mutex);

    if (!m_bitmapSource) {
      return E_UNEXPECTED;
    }

    return m_bitmapSource->GetResolution(pDpiX, pDpiY);
  }

  STDMETHODIMP BaseFrameDecode::CopyPalette(IWICPalette* pIPalette) {
    CheckMutex(m_mutex, __FUNCSIG__);
    std::shared_lock lock(m_mutex);

    if (!pIPalette) {
      return E_INVALIDARG;
    }

    if (!m_palette) {
      return E_UNEXPECTED;
    }

    pIPalette->InitializeFromPalette(m_palette);

    return S_OK;
  }

  STDMETHODIMP BaseFrameDecode::CopyPixels(WICRect const* prc, UINT cbStride, UINT cbPixelsSize, BYTE* pbPixels) {
    CheckMutex(m_mutex, __FUNCSIG__);
    std::shared_lock lock(m_mutex);

    if (!m_bitmapSource) {
      return E_UNEXPECTED;
    }

    return m_bitmapSource->CopyPixels(prc, cbStride, cbPixelsSize, pbPixels);
  }

  //----------------------------------------------------------------------------------------
  // BaseDecoder implementation
  //----------------------------------------------------------------------------------------

  BaseDecoder::BaseDecoder(GUID Me, GUID Container) : m_CLSID_Container(Container), m_CLSID_This(Me) {
    // MessageBoxW(NULL, L"Decoder()", L"tlg_wic_codec", MB_OK);
  }

  BaseDecoder::~BaseDecoder() {
    // MessageBoxW(NULL, L"~Decoder()", L"tlg_wic_codec", MB_OK);
    ReleaseMembers(true);
  }

  void BaseDecoder::ReleaseMembers(bool releaseFactory) {
    CheckMutex(m_mutex, __FUNCSIG__);
    std::lock_guard lock(m_mutex);

    if (releaseFactory) {
      WICX_RELEASE(m_factory);
    }

    for (auto& frame : m_frames) {
      WICX_RELEASE(frame);
    }
    m_frames.clear();

    WICX_RELEASE(m_palette);

    for (auto& colorContext : m_colorContexts) {
      WICX_RELEASE(colorContext);
    }
    m_colorContexts.clear();

    WICX_RELEASE(m_thumbnail);

    WICX_RELEASE(m_preview);
  }

  HRESULT BaseDecoder::EnsureFactory() {
    CheckMutex(m_mutex, __FUNCSIG__);
    std::lock_guard lock(m_mutex);

    if (m_factory) {
      return S_OK;
    }

    return CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_IWICImagingFactory, reinterpret_cast<LPVOID*>(&m_factory));
  }

  void BaseDecoder::AddDecoderFrame(BaseFrameDecode* frame) {
    CheckMutex(m_mutex, __FUNCSIG__);
    std::lock_guard lock(m_mutex);

    assert(frame);

    frame->AddRef();
    m_frames.push_back(frame);
  }

  // ----- IUnknown interface ---------------------------------------------------------------------------

  STDMETHODIMP BaseDecoder::QueryInterface(REFIID iid, void** ppvObject) {
    CheckMutex(m_mutex, __FUNCSIG__);
    std::shared_lock lock(m_mutex);

    if (!ppvObject) {
      return E_INVALIDARG;
    }

    if (iid == IID_IUnknown) {
      *ppvObject = static_cast<IUnknown*>(this);
    } else if (iid == IID_IWICBitmapDecoder) {
      *ppvObject = static_cast<IWICBitmapDecoder*>(this);
    } else {
      return E_NOINTERFACE;
    }

    AddRef();

    return S_OK;
  }

  STDMETHODIMP_(ULONG) BaseDecoder::AddRef() {
    return m_unknownImpl.AddRef();
  }

  STDMETHODIMP_(ULONG) BaseDecoder::Release() {
    const auto count = m_unknownImpl.Release();
    if (count == 0) {
      delete this;
    }
    return count;
  }

  // ----- IWICBitmapDecoder interface ------------------------------------------------------------------

  STDMETHODIMP BaseDecoder::GetContainerFormat(GUID* pguidContainerFormat) {
    CheckMutex(m_mutex, __FUNCSIG__);
    std::shared_lock lock(m_mutex);

    if (!pguidContainerFormat) {
      return E_INVALIDARG;
    }

    *pguidContainerFormat = m_CLSID_Container;

    return S_OK;
  }

  STDMETHODIMP BaseDecoder::GetDecoderInfo(IWICBitmapDecoderInfo** ppIDecoderInfo) {
    if (const auto result = EnsureFactory(); FAILED(result)) {
      return result;
    }

    {
      CheckMutex(m_mutex, __FUNCSIG__);
      std::shared_lock lock(m_mutex);

      if (!m_factory) {
        return E_FAIL;
      }

      IWICComponentInfo* compInfo = nullptr;

      if (const auto result = m_factory->CreateComponentInfo(m_CLSID_This, &compInfo); FAILED(result)) {
        WICX_RELEASE(compInfo);
        return result;
      }

      if (!compInfo) {
        return E_FAIL;
      }

      if (const auto result = compInfo->QueryInterface(IID_IWICBitmapDecoderInfo, reinterpret_cast<void**>(ppIDecoderInfo)); FAILED(result)) {
        WICX_RELEASE(compInfo);
        return result;
      }

      WICX_RELEASE(compInfo);
    }

    return S_OK;
  }

  STDMETHODIMP BaseDecoder::CopyPalette(IWICPalette* pIPalette) {
    CheckMutex(m_mutex, __FUNCSIG__);
    std::shared_lock lock(m_mutex);

    if (!pIPalette) {
      return E_INVALIDARG;
    }

    if (!m_palette) {
      return E_UNEXPECTED;
    }

    return pIPalette->InitializeFromPalette(m_palette);
  }

  STDMETHODIMP BaseDecoder::GetMetadataQueryReader(IWICMetadataQueryReader** ppIMetadataQueryReader) {
    UNREFERENCED_PARAMETER(ppIMetadataQueryReader);

    return E_NOTIMPL;
  }

  STDMETHODIMP BaseDecoder::GetPreview(IWICBitmapSource** ppIPreview) {
    CheckMutex(m_mutex, __FUNCSIG__);
    std::shared_lock lock(m_mutex);

    if (!ppIPreview) {
      return E_INVALIDARG;
    }

    if (!m_preview) {
      return E_UNEXPECTED;
    }

    return m_preview->QueryInterface(IID_IWICBitmapSource, reinterpret_cast<void**>(ppIPreview));
  }

  STDMETHODIMP BaseDecoder::GetColorContexts(UINT cCount, IWICColorContext** ppIColorContexts, UINT* pcActualCount) {
    CheckMutex(m_mutex, __FUNCSIG__);
    std::shared_lock lock(m_mutex);

    if (ppIColorContexts) {
      // return the actual color contexts

      if (cCount > m_colorContexts.size()) {
        return E_INVALIDARG;
      }

      for (size_t i = 0; i < cCount; i++) {
        ppIColorContexts[i] = m_colorContexts[i];
        m_colorContexts[i]->AddRef();
      }
    } else {
      // return the number of color contexts

      if (!pcActualCount) {
        return E_INVALIDARG;
      }

      *pcActualCount = static_cast<UINT>(m_colorContexts.size());
    }

    return S_OK;
  }

  STDMETHODIMP BaseDecoder::GetThumbnail(IWICBitmapSource** ppIThumbnail) {
    CheckMutex(m_mutex, __FUNCSIG__);
    std::shared_lock lock(m_mutex);

    if (!ppIThumbnail) {
      return E_INVALIDARG;
    }

    if (!m_thumbnail) {
      return WINCODEC_ERR_CODECNOTHUMBNAIL;
    }

    return m_thumbnail->QueryInterface(IID_IWICBitmapSource, reinterpret_cast<void**>(ppIThumbnail));
  }

  STDMETHODIMP BaseDecoder::GetFrameCount(UINT* pCount) {
    CheckMutex(m_mutex, __FUNCSIG__);
    std::shared_lock lock(m_mutex);

    if (!pCount) {
      return E_INVALIDARG;
    }

    *pCount = static_cast<UINT>(m_frames.size());

    return S_OK;
  }

  STDMETHODIMP BaseDecoder::GetFrame(UINT index, IWICBitmapFrameDecode** ppIBitmapFrame) {
    CheckMutex(m_mutex, __FUNCSIG__);
    std::shared_lock lock(m_mutex);

    if (!ppIBitmapFrame || index >= m_frames.size()) {
      return E_INVALIDARG;
    }

    return m_frames[index]->QueryInterface(IID_IWICBitmapFrameDecode, reinterpret_cast<void**>(ppIBitmapFrame));
  }
} // namespace wicx
