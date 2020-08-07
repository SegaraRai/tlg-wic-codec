#include "TLGDecoder.hpp"

#include "TLGStream.hpp"

#include "../libtlg/tlg.h"

#include "../wicx/Util.hpp"

#include <string>

using namespace std::literals;

// {280BF6EC-0A7B-4870-8AB0-FC4DE12D0B7B}
const GUID CLSID_TLG_Container = { 0x280bf6ec, 0xa7b, 0x4870, { 0x8a, 0xb0, 0xfc, 0x4d, 0xe1, 0x2d, 0xb, 0x7b } };

// {05103AD4-28F3-4229-A9A3-2928A8CE5E9A}
const GUID CLSID_TLG_Decoder = { 0x5103ad4, 0x28f3, 0x4229, { 0xa9, 0xa3, 0x29, 0x28, 0xa8, 0xce, 0x5e, 0x9a } };

constexpr LARGE_INTEGER MakeLI(LONGLONG value) {
  LARGE_INTEGER li{};
  li.QuadPart = value;
  return li;
}

namespace tlgx {
  //----------------------------------------------------------------------------------------
  // TLG_FrameDecode implementation
  //----------------------------------------------------------------------------------------

  class TLG_FrameDecode : public BaseFrameDecode {
    std::mutex mutex;

    int width = 0;
    int height = 0;
    unsigned int pitch = 0;
    std::unique_ptr<unsigned char[]> outData{};

  public:
    TLG_FrameDecode(IWICImagingFactory* pIFactory, UINT num) : BaseFrameDecode(pIFactory, num) {}

    ~TLG_FrameDecode() {}

    void clear() {
      width = 0;
      height = 0;
      pitch = 0;
      outData.reset();
    }

    bool setSize(int w, int h) {
      try {
        clear();
        width = w;
        height = h;
        pitch = (width * (32 / 8) + 3) & ~3; /*4byte境界*/
        outData = std::make_unique<unsigned char[]>(pitch * height);
        return true;
      } catch (...) {}

      clear();

      return false;
    }

    void* getScanLine(int y) {
      if (y < 0 || !outData) {
        return nullptr;
      }
      return &outData[pitch * y];
    }

    static bool sizeCallback(void* callbackdata, unsigned int w, unsigned int h) {
      const auto decoder = static_cast<TLG_FrameDecode*>(callbackdata);
      return decoder->setSize(w, h);
    }

    static void* scanLineCallback(void* callbackdata, int y) {
      const auto decoder = static_cast<TLG_FrameDecode*>(callbackdata);
      return decoder->getScanLine(y);
    }

    /**
     * データ転送
     */
    HRESULT fill() {
      IWICImagingFactory* codecFactory = nullptr;

      if (const auto result = CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_IWICImagingFactory, reinterpret_cast<LPVOID*>(&codecFactory)); FAILED(result)) {
        WICX_RELEASE(codecFactory);
        return result;
      }

      if (!codecFactory) {
        return E_FAIL;
      }

      IWICBitmap* ptrBitmap = nullptr;

      if (const auto result = codecFactory->CreateBitmapFromMemory(width, height, GUID_WICPixelFormat32bppBGRA, pitch, pitch * height, outData.get(), &ptrBitmap); FAILED(result)) {
        WICX_RELEASE(codecFactory);
        return result;
      }

      WICX_RELEASE(codecFactory);

      if (!ptrBitmap) {
        return E_FAIL;
      }

      m_bitmapSource = static_cast<IWICBitmapSource*>(ptrBitmap);

      return S_OK;
    }

    HRESULT LoadImageFromStream(IStream* pIStream) {
      std::lock_guard lock(mutex);

      tMyStream stream(pIStream);

      if (const auto ret = TVPLoadTLG(this, sizeCallback, scanLineCallback, nullptr, &stream); ret != TLG_SUCCESS) {
        clear();
        return E_FAIL;
      }

      const auto result = fill();

      clear();

      return result;
    }
  };

  //----------------------------------------------------------------------------------------
  // TLG_Decoder implementation
  //----------------------------------------------------------------------------------------

  TLG_Decoder::TLG_Decoder() : BaseDecoder(CLSID_TLG_Decoder, CLSID_TLG_Container) {}

  TLG_Decoder::~TLG_Decoder() {}

  STDMETHODIMP TLG_Decoder::QueryCapability(IStream* pIStream, DWORD* pCapability) {
    if (!pIStream || !pCapability) {
      return E_INVALIDARG;
    }

    {
      // store stream seek position
      ULARGE_INTEGER pos{};
      if (const auto ret = pIStream->Seek({}, STREAM_SEEK_CUR, &pos); FAILED(ret)) {
        return ret;
      }

      bool callbackCalled = false;

      tMyStream stream(pIStream);

      const auto result = TVPLoadTLG(
        &callbackCalled,
        [](void* callbackCalled, tjs_uint w, tjs_uint h) -> bool {
          UNREFERENCED_PARAMETER(w);
          UNREFERENCED_PARAMETER(h);

          *static_cast<bool*>(callbackCalled) = true;

          return false;
        },
        nullptr,
        nullptr,
        &stream);

      // restore stream seek position
      if (const auto ret = pIStream->Seek(wicx::MakeLI(pos.QuadPart), STREAM_SEEK_SET, &pos); FAILED(ret)) {
        return ret;
      }

      if (!callbackCalled || result != TLG_ABORT) {
        // WebP WIC codec does so
        return WINCODEC_ERR_WRONGSTATE;
      }
    }

    *pCapability = WICBitmapDecoderCapabilityCanDecodeSomeImages;

    return S_OK;
  }

  STDMETHODIMP TLG_Decoder::Initialize(IStream* pIStream, WICDecodeOptions cacheOptions) {
    std::lock_guard lock(mutex);

    UNREFERENCED_PARAMETER(cacheOptions);

    // TODO: return error if already initialized?

    ReleaseMembers(true);

    if (!pIStream) {
      return E_INVALIDARG;
    }

    if (const auto result = EnsureFactory(); FAILED(result)) {
      return result;
    }

    const auto frame = static_cast<TLG_FrameDecode*>(CreateNewDecoderFrame(m_factory, 0));

    if (const auto result = frame->LoadImageFromStream(pIStream); FAILED(result)) {
      delete frame;
      return result;
    }

    AddDecoderFrame(frame);

    return S_OK;
  }

  BaseFrameDecode* TLG_Decoder::CreateNewDecoderFrame(IWICImagingFactory* factory, UINT i) {
    return new TLG_FrameDecode(factory, i);
  }

  void TLG_Decoder::Register(RegMan& regMan) {
    regMan.SetSZ(L"CLSID\\{7ED96837-96F0-4812-B211-F13C24117ED3}\\Instance\\{05103AD4-28F3-4229-A9A3-2928A8CE5E9A}"s, L"CLSID"s, L"{05103AD4-28F3-4229-A9A3-2928A8CE5E9A}"s);
    regMan.SetSZ(L"CLSID\\{7ED96837-96F0-4812-B211-F13C24117ED3}\\Instance\\{05103AD4-28F3-4229-A9A3-2928A8CE5E9A}"s, L"FriendlyName"s, L"TLG Decoder"s);

    regMan.SetSZ(L"CLSID\\{05103AD4-28F3-4229-A9A3-2928A8CE5E9A}"s, L"Version"s, L"1.0.0.1"s);
    regMan.SetSZ(L"CLSID\\{05103AD4-28F3-4229-A9A3-2928A8CE5E9A}"s, L"Date"s, _STR2CPPWSTR(__DATE__));
    regMan.SetSZ(L"CLSID\\{05103AD4-28F3-4229-A9A3-2928A8CE5E9A}"s, L"SpecVersion"s, L"1.0.0.0"s);
    regMan.SetSZ(L"CLSID\\{05103AD4-28F3-4229-A9A3-2928A8CE5E9A}"s, L"ColorManagementVersion"s, L"1.0.0.0"s);
    regMan.SetSZ(L"CLSID\\{05103AD4-28F3-4229-A9A3-2928A8CE5E9A}"s, L"MimeTypes"s, L"image/x-tlg"s);
    regMan.SetSZ(L"CLSID\\{05103AD4-28F3-4229-A9A3-2928A8CE5E9A}"s, L"FileExtensions"s, L".tlg"s);
    regMan.SetDW(L"CLSID\\{05103AD4-28F3-4229-A9A3-2928A8CE5E9A}"s, L"SupportsAnimation"s, 0);
    regMan.SetDW(L"CLSID\\{05103AD4-28F3-4229-A9A3-2928A8CE5E9A}"s, L"SupportChromakey"s, 1);
    regMan.SetDW(L"CLSID\\{05103AD4-28F3-4229-A9A3-2928A8CE5E9A}"s, L"SupportLossless"s, 1);
    regMan.SetDW(L"CLSID\\{05103AD4-28F3-4229-A9A3-2928A8CE5E9A}"s, L"SupportMultiframe"s, 1);
    regMan.SetSZ(L"CLSID\\{05103AD4-28F3-4229-A9A3-2928A8CE5E9A}"s, L"ContainerFormat"s, L"{280BF6EC-0A7B-4870-8AB0-FC4DE12D0B7B}"s);
    regMan.SetSZ(L"CLSID\\{05103AD4-28F3-4229-A9A3-2928A8CE5E9A}"s, L"Author"s, L"Go Watanabe"s);
    regMan.SetSZ(L"CLSID\\{05103AD4-28F3-4229-A9A3-2928A8CE5E9A}"s, L"Description"s, L"TLG(kirikiri) Format Decoder"s);
    regMan.SetSZ(L"CLSID\\{05103AD4-28F3-4229-A9A3-2928A8CE5E9A}"s, L"FriendlyName"s, L"TLG Decoder"s);

    regMan.Create(L"CLSID\\{05103AD4-28F3-4229-A9A3-2928A8CE5E9A}\\Formats"s);
    regMan.SetSZ(L"CLSID\\{05103AD4-28F3-4229-A9A3-2928A8CE5E9A}\\Formats\\{6FDDC324-4E03-4BFE-B185-3D77768DC90F}"s, L""s, L""s);

    regMan.SetSZ(L"CLSID\\{05103AD4-28F3-4229-A9A3-2928A8CE5E9A}\\InprocServer32"s, L""s, GetDLLFilepath());
    regMan.SetSZ(L"CLSID\\{05103AD4-28F3-4229-A9A3-2928A8CE5E9A}\\InprocServer32"s, L"ThreadingModel"s, L"Apartment"s);

    // パターン登録
    const char mask[] = "\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff";
    regMan.Create(L"CLSID\\{05103AD4-28F3-4229-A9A3-2928A8CE5E9A}\\Patterns"s);

    regMan.SetDW(L"CLSID\\{05103AD4-28F3-4229-A9A3-2928A8CE5E9A}\\Patterns\\0"s, L"Position"s, 0);
    regMan.SetDW(L"CLSID\\{05103AD4-28F3-4229-A9A3-2928A8CE5E9A}\\Patterns\\0"s, L"Length"s, 10);
    regMan.SetBytes(L"CLSID\\{05103AD4-28F3-4229-A9A3-2928A8CE5E9A}\\Patterns\\0"s, L"Pattern"s, "TLG0.0\x00sds\x1a", 10);
    regMan.SetBytes(L"CLSID\\{05103AD4-28F3-4229-A9A3-2928A8CE5E9A}\\Patterns\\0"s, L"Mask"s, mask, 10);

    regMan.SetDW(L"CLSID\\{05103AD4-28F3-4229-A9A3-2928A8CE5E9A}\\Patterns\\1"s, L"Position"s, 0);
    regMan.SetDW(L"CLSID\\{05103AD4-28F3-4229-A9A3-2928A8CE5E9A}\\Patterns\\1"s, L"Length"s, 10);
    regMan.SetBytes(L"CLSID\\{05103AD4-28F3-4229-A9A3-2928A8CE5E9A}\\Patterns\\1"s, L"Pattern"s, "TLG5.0\x00raw\x1a", 10);
    regMan.SetBytes(L"CLSID\\{05103AD4-28F3-4229-A9A3-2928A8CE5E9A}\\Patterns\\1"s, L"Mask"s, mask, 10);

    regMan.SetDW(L"CLSID\\{05103AD4-28F3-4229-A9A3-2928A8CE5E9A}\\Patterns\\2"s, L"Position"s, 0);
    regMan.SetDW(L"CLSID\\{05103AD4-28F3-4229-A9A3-2928A8CE5E9A}\\Patterns\\2"s, L"Length"s, 10);
    regMan.SetBytes(L"CLSID\\{05103AD4-28F3-4229-A9A3-2928A8CE5E9A}\\Patterns\\2"s, L"Pattern"s, "TLG6.0\x00raw\x1a", 10);
    regMan.SetBytes(L"CLSID\\{05103AD4-28F3-4229-A9A3-2928A8CE5E9A}\\Patterns\\2"s, L"Mask"s, mask, 10);
  }
} // namespace tlgx
