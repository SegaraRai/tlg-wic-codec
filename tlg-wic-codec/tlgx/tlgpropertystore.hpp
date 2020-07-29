#pragma once

#include "../stdafx.hpp"
#include "../wicx/basepropertystore.hpp"
#include "../wicx/regman.hpp"

extern const GUID CLSID_TLG_PropertyStore;

namespace tlgx
{
	using namespace wicx;

	class TLG_PropertyStore : public BasePropertyStore {
	public:
		static void Register(RegMan& regMan);

		TLG_PropertyStore();
		~TLG_PropertyStore();

	protected:
		HRESULT LoadProperties(IPropertyStoreCache* pPropertyCache, IStream* pStream) override;
	};
}
