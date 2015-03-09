#pragma once

#include "lib/resourceObject.h"
#include "lib/gfx/deviceDX11.h"

namespace luna {

class TextureResource : public ResourceObject
{
	LUNA_DECLARE_CONCRETE(TextureResource, ResourceObject);

public:
	TextureResource();
	virtual ~TextureResource() override;

	ID3D11ShaderResourceView& getSRV() const
	{
		return *mSRV.Get();
	}

private:
	bool load(Stream&);
	bool isLoadable(Stream&) override;
	void freeResource() override;

	ComPtr<ID3D11Texture2D> mTexture;
	ComPtr<ID3D11ShaderResourceView> mSRV;
};

}
