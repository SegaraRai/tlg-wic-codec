#pragma once

#include "UnknownImpl.hpp"

#include <shared_mutex>
#include <vector>

#include <Windows.h>

#include <wincodec.h>

namespace wicx {
  class BaseFrameDecode : public IWICBitmapFrameDecode {
    std::shared_mutex m_mutex;
    UnknownImpl m_unknownImpl;

    void ReleaseMembers();

  protected:
    UINT m_frameNumber = 0;
    IWICImagingFactory* m_factory = nullptr;
    std::vector<IWICColorContext*> m_colorContexts{};
    IWICPalette* m_palette = nullptr;
    IWICBitmapSource* m_bitmapSource = nullptr;
    IWICBitmapSource* m_preview = nullptr;
    IWICBitmapSource* m_thumbnail = nullptr;

  public:
    BaseFrameDecode(IWICImagingFactory* pIFactory, UINT num);
    virtual ~BaseFrameDecode();

    // IUnknown interface

    STDMETHOD(QueryInterface)(REFIID riid, void** ppv);
    STDMETHOD_(ULONG, AddRef)();
    STDMETHOD_(ULONG, Release)();

    // IWICBitmapFrameDecode interface

    STDMETHOD(GetMetadataQueryReader)(IWICMetadataQueryReader** ppIMetadataQueryReader);
    STDMETHOD(GetColorContexts)(UINT cCount, IWICColorContext** ppIColorContexts, UINT* pcActualCount);
    STDMETHOD(GetThumbnail)(IWICBitmapSource** ppIThumbnail);

    // IWICBitmapSource interface

    STDMETHOD(GetSize)(UINT* puiWidth, UINT* puiHeight);
    STDMETHOD(GetPixelFormat)(WICPixelFormatGUID* pPixelFormat);
    STDMETHOD(GetResolution)(double* pDpiX, double* pDpiY);
    STDMETHOD(CopyPalette)(IWICPalette* pIPalette);
    STDMETHOD(CopyPixels)(const WICRect* prc, UINT cbStride, UINT cbPixelsSize, BYTE* pbPixels);
  };

  class BaseDecoder : public IWICBitmapDecoder {
    std::shared_mutex m_mutex;
    UnknownImpl m_unknownImpl;

  protected:
    GUID const m_CLSID_Container{};
    GUID const m_CLSID_This{};

    IWICImagingFactory* m_factory = nullptr;
    std::vector<BaseFrameDecode*> m_frames{};
    std::vector<IWICColorContext*> m_colorContexts{};
    IWICPalette* m_palette = nullptr;
    IWICBitmapSource* m_preview = nullptr;
    IWICBitmapSource* m_thumbnail = nullptr;

    virtual BaseFrameDecode* CreateNewDecoderFrame(IWICImagingFactory* factory, UINT i) = 0;
    virtual void AddDecoderFrame(BaseFrameDecode* frame);
    HRESULT EnsureFactory();
    void ReleaseMembers(bool releaseFactory);

  public:
    BaseDecoder(GUID Me, GUID Container);
    virtual ~BaseDecoder();

    // IUnknown interface

    STDMETHOD(QueryInterface)(REFIID riid, void** ppv);
    STDMETHOD_(ULONG, AddRef)();
    STDMETHOD_(ULONG, Release)();

    // IWICBitmapDecoder interface

    STDMETHOD(QueryCapability)
    (IStream* pIStream, DWORD* pCapability) = 0;
    STDMETHOD(Initialize)(IStream* pIStream, WICDecodeOptions cacheOptions) = 0;
    STDMETHOD(GetContainerFormat)(GUID* pguidContainerFormat);
    STDMETHOD(GetDecoderInfo)(IWICBitmapDecoderInfo** ppIDecoderInfo);
    STDMETHOD(CopyPalette)(IWICPalette* pIPalette);
    STDMETHOD(GetMetadataQueryReader)(IWICMetadataQueryReader** ppIMetadataQueryReader);
    STDMETHOD(GetPreview)(IWICBitmapSource** ppIPreview);
    STDMETHOD(GetColorContexts)(UINT cCount, IWICColorContext** ppIColorContexts, UINT* pcActualCount);
    STDMETHOD(GetThumbnail)(IWICBitmapSource** ppIThumbnail);
    STDMETHOD(GetFrameCount)(UINT* pCount);
    STDMETHOD(GetFrame)(UINT index, IWICBitmapFrameDecode** ppIBitmapFrame);
  };
} // namespace wicx
