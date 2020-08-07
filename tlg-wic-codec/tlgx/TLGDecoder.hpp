#pragma once

#include "../wicx/BaseDecoder.hpp"
#include "../wicx/RegMan.hpp"

#include <mutex>

#include <Windows.h>

extern const GUID CLSID_TLG_Container;
extern const GUID CLSID_TLG_Decoder;

namespace tlgx {
  using namespace wicx;

  class TLG_Decoder : public BaseDecoder {
    std::mutex mutex;

  protected:
    BaseFrameDecode* CreateNewDecoderFrame(IWICImagingFactory* factory, UINT i) override;

  public:
    static void Register(RegMan& regMan);

    TLG_Decoder();
    ~TLG_Decoder();

    // IWICBitmapDecoder interface

    STDMETHOD(QueryCapability)(IStream* pIStream, DWORD* pCapability) override;
    STDMETHOD(Initialize)(IStream* pIStream, WICDecodeOptions cacheOptions) override;
  };
} // namespace tlgx
