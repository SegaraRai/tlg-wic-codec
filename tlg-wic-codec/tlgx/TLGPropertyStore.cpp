#include "TLGPropertyStore.hpp"
#include "../libtlg/tlg.h"
#include "TLGStream.hpp"

#include <memory>
#include <stdexcept>
#include <string>

#define __STR2WSTR(str) L##str
#define _STR2WSTR(str) __STR2WSTR(str)

using namespace std::literals;

// {509DC48F-345D-4506-9FE2-7BDF4AB21CE4}
const GUID CLSID_TLG_PropertyStore = { 0x509dc48f, 0x345d, 0x4506, { 0x9f, 0xe2, 0x7b, 0xdf, 0x4a, 0xb2, 0x1c, 0xe4 } };

// {318AE181-30CB-4080-998F-263114758B6E}
const GUID PSGUID_TLGTAGS = { 0x318ae181, 0x30cb, 0x4080, { 0x99, 0x8f, 0x26, 0x31, 0x14, 0x75, 0x8b, 0x6e } };

const PROPERTYKEY PKEY_TLG_Tags{ PSGUID_TLGTAGS, PID_FIRST_USABLE };

constexpr LARGE_INTEGER MakeLI(LONGLONG value) {
  LARGE_INTEGER li{};
  li.QuadPart = value;
  return li;
}

std::wstring UTF8ToUTF16(const std::string& str) {
  // for <  U+10000 : UTF-8 = 1 - 3 bytes, UTF-16 = 2 bytes (1 unit)
  // for >= U+10000 : UTF-8 = 4     bytes, UTF-16 = 4 bytes (2 units)

  const auto bufferSize = static_cast<DWORD>(str.size() + 1);
  auto buffer = std::make_unique<wchar_t[]>(bufferSize);
  if (MultiByteToWideChar(CP_UTF8, MB_PRECOMPOSED, str.c_str(), static_cast<DWORD>(str.size() + 1), buffer.get(), bufferSize) == 0) {
    throw std::runtime_error("MultiByteToWideChar failed");
  }

  return std::wstring(buffer.get());
}

namespace tlgx {
  //----------------------------------------------------------------------------------------
  // TLG_PropertyStore implementation
  //----------------------------------------------------------------------------------------

  TLG_PropertyStore::TLG_PropertyStore() : BasePropertyStore(CLSID_TLG_PropertyStore) {}

  TLG_PropertyStore::~TLG_PropertyStore() {}

  HRESULT TLG_PropertyStore::LoadProperties(IPropertyStoreCache* pPropertyCache, IStream* pStream) {
    if (!pPropertyCache || !pStream) {
      OutputDebugStringW(L"PropertyStore: invalid arg\n");
      return E_INVALIDARG;
    }

    // store stream seek position
    ULARGE_INTEGER pos{};
    if (const auto ret = pStream->Seek({}, STREAM_SEEK_CUR, &pos); FAILED(ret)) {
      OutputDebugStringW(L"PropertyStore: failed to Seek\n");
      return ret;
    }

    tMyStream tjsStream(pStream);

    std::unordered_map<std::string, std::string> tagMap;

    // load TLG tags
    const auto result = TVPLoadTLG(
      pPropertyCache,
      [](void* ptr, unsigned int width, unsigned int height) -> bool {
        OutputDebugStringW(L"PropertyStore: callback\n");

        auto pPropertyCache = static_cast<IPropertyStoreCache*>(ptr);

        PROPVARIANT pv{};

        // set dimensions (w x h)
        {
          const std::wstring strDimensions = std::to_wstring(width) + L" x "s + std::to_wstring(height);
          if (SUCCEEDED(InitPropVariantFromString(strDimensions.c_str(), &pv))) {
            pPropertyCache->SetValueAndState(PKEY_Image_Dimensions, &pv, PSC_READONLY);
            PropVariantClear(&pv);
          }
        }

        // set width
        if (SUCCEEDED(InitPropVariantFromUInt32(width, &pv))) {
          pPropertyCache->SetValueAndState(PKEY_Image_HorizontalSize, &pv, PSC_READONLY);
          PropVariantClear(&pv);
        }

        // set height
        if (SUCCEEDED(InitPropVariantFromUInt32(height, &pv))) {
          pPropertyCache->SetValueAndState(PKEY_Image_VerticalSize, &pv, PSC_READONLY);
          PropVariantClear(&pv);
        }

        // set bit depth: 32bit
        if (SUCCEEDED(InitPropVariantFromUInt32(32, &pv))) {
          pPropertyCache->SetValueAndState(PKEY_Image_BitDepth, &pv, PSC_READONLY);
          PropVariantClear(&pv);
        }

        // set image ID: "TLG" (unknown) / "TLG5.0 raw" / "TLG6.0 raw" / "TLG5.0 raw in TLG0.0 sds" / "TLG6.0 raw in TLG0.0 sds"
        // TODO
        {
          const wchar_t* imageID = L"TLG";
          if (SUCCEEDED(InitPropVariantFromString(imageID, &pv))) {
            pPropertyCache->SetValueAndState(PKEY_Image_ImageID, &pv, PSC_READONLY);
            PropVariantClear(&pv);
          }
        }

        // stop processing
        return false;
      },
      nullptr,
      &tagMap,
      &tjsStream);

    // restore stream seek position
    if (const auto ret = pStream->Seek(MakeLI(pos.QuadPart), STREAM_SEEK_SET, &pos); FAILED(ret)) {
      OutputDebugStringW(L"PropertyStore: seek failed 2\n");
      return ret;
    }

    if (result != TLG_ABORT) {
      OutputDebugStringW(L"PropertyStore: result is not TLG_ABORT\n");
      return E_UNEXPECTED;
    }

    // set System.Kind and System.KindText
    {
      PROPVARIANT pv{};

      if (SUCCEEDED(InitPropVariantFromString(L"Picture", &pv))) {
        pPropertyCache->SetValueAndState(PKEY_Kind, &pv, PSC_READONLY);
        PropVariantClear(&pv);
      }

      if (SUCCEEDED(InitPropVariantFromString(L"TLG Image", &pv))) {
        pPropertyCache->SetValueAndState(PKEY_KindText, &pv, PSC_READONLY);
        PropVariantClear(&pv);
      }
    }

    /*
		// set tags
		// commented out because the tags of some TLG files are jsut binary...
		try {
			if (!tagMap.empty()) {
				std::vector<std::wstring> tags;
				std::wstring comment;
				for (const auto& [key, value] : tagMap) {
					// NOTE: tags are UTF-8

					try {
						const std::wstring wKey = UTF8ToUTF16(key);
						const std::wstring wValue = UTF8ToUTF16(value);

						comment += L"\n"s + wKey + L"="s + wValue;

						tags.push_back(wKey);
						tags.push_back(wValue);
					} catch (...) {
						// typically conversion error
						// do nothing
					}
				}

				comment = comment.substr(1);

				std::vector<const wchar_t*> tagsForPV;
				for (const auto& tag : tags) {
					tagsForPV.push_back(tag.c_str());
				}

				// NOTE: exceptions should not be thrown after this line

				PROPVARIANT pv{};

				if (SUCCEEDED(InitPropVariantFromString(comment.c_str(), &pv))) {
					pPropertyCache->SetValueAndState(PKEY_Comment, &pv, PSC_READONLY);
					PropVariantClear(&pv);
				}

				if (SUCCEEDED(InitPropVariantFromStringVector(tagsForPV.data(), static_cast<ULONG>(tagsForPV.size()), &pv))) {
					pPropertyCache->SetValueAndState(PKEY_TLG_Tags, &pv, PSC_READONLY);
					PropVariantClear(&pv);
				}

				if (SUCCEEDED(InitPropVariantFromStringVector(tagsForPV.data(), static_cast<ULONG>(tagsForPV.size()), &pv))) {
					pPropertyCache->SetValueAndState(PKEY_Keywords, &pv, PSC_READONLY);
					PropVariantClear(&pv);
				}

				if (SUCCEEDED(InitPropVariantFromStringVector(tagsForPV.data(), static_cast<ULONG>(tagsForPV.size()), &pv))) {
					pPropertyCache->SetValueAndState(PKEY_Supplemental_Tag, &pv, PSC_READONLY);
					PropVariantClear(&pv);
				}
			}
		} catch (...) {
			// typically memory error
			// do nothing
		}
		//*/

    OutputDebugStringW(L"PropertyStore: OK\n");

    return S_OK;
  }

  void TLG_PropertyStore::Register(RegMan& regMan) {
    HMODULE curModule = GetModuleHandleW(L"tlg-wic-codec.dll");
    wchar_t tempFileName[MAX_PATH];
    if (curModule != NULL)
      GetModuleFileNameW(curModule, tempFileName, MAX_PATH);

    // see https://docs.microsoft.com/ja-jp/windows/win32/properties/prophand-reg-dist

    regMan.SetSZ(L"CLSID\\{509DC48F-345D-4506-9FE2-7BDF4AB21CE4}"s, L"Version"s, L"1.0.0.1"s);
    regMan.SetSZ(L"CLSID\\{509DC48F-345D-4506-9FE2-7BDF4AB21CE4}"s, L"Date"s, _STR2WSTR(__DATE__));
    regMan.SetDW(L"CLSID\\{509DC48F-345D-4506-9FE2-7BDF4AB21CE4}"s, L"ManualSafeSave"s, 1);
    regMan.SetSZ(L"CLSID\\{509DC48F-345D-4506-9FE2-7BDF4AB21CE4}"s, L"Author"s, L"Go Watanabe, SegaraRai"s);
    regMan.SetSZ(L"CLSID\\{509DC48F-345D-4506-9FE2-7BDF4AB21CE4}"s, L"Description"s, L"TLG(kirikiri) Property Handler"s);
    regMan.SetSZ(L"CLSID\\{509DC48F-345D-4506-9FE2-7BDF4AB21CE4}"s, L"FriendlyName"s, L"TLG Property Handler"s);

    regMan.SetSZ(L"CLSID\\{509DC48F-345D-4506-9FE2-7BDF4AB21CE4}\\InprocServer32"s, L""s, tempFileName);
    regMan.SetSZ(L"CLSID\\{509DC48F-345D-4506-9FE2-7BDF4AB21CE4}\\InprocServer32"s, L"ThreadingModel"s, L"Apartment"s);
  }
} // namespace tlgx
