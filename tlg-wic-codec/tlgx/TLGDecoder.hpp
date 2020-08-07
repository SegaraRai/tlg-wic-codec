#pragma once

#include "../wicx/BaseDecoder.hpp"
#include "../wicx/RegMan.hpp"

#include <mutex>

#include <Windows.h>

#include <ObjIdl.h>
#include <wincodec.h>

extern const GUID CLSID_TLG_Container;
extern const GUID CLSID_TLG_Decoder;

namespace tlgx {
  class TLG_Decoder : public wicx::BaseDecoder {
    std::mutex mutex;

  protected:
    wicx::BaseFrameDecode* CreateNewDecoderFrame(IWICImagingFactory* factory, UINT i) override;

  public:
    static void Register(wicx::RegMan& regMan);

    TLG_Decoder();
    ~TLG_Decoder();

    // IWICBitmapDecoder interface

    STDMETHOD(QueryCapability)(IStream* pIStream, DWORD* pCapability) override;
    STDMETHOD(Initialize)(IStream* pIStream, WICDecodeOptions cacheOptions) override;
  };
} // namespace tlgx
