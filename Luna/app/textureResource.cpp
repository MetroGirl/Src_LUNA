#include "stdafx.h"
#include "lib/gfx/deviceDX11.h"
#include "app/textureResource.h"
#include "DirectXTex/DirectXTex.h"

namespace luna {

LUNA_IMPLEMENT_CONCRETE(luna::TextureResource);

TextureResource::TextureResource()
{}

TextureResource::~TextureResource()
{}

bool TextureResource::load(Stream& fs)
{
	std::vector<c8> buf(fs.getSize());
	fs.read(buf.data(), buf.size());

	DirectX::TexMetadata metadata;
	DirectX::ScratchImage image;
	auto hr = fs.getExtension() == L"dds"?
		DirectX::LoadFromDDSMemory(buf.data(), buf.size(), 0, &metadata, image):
		DirectX::LoadFromWICMemory(buf.data(), buf.size(), 0, &metadata, image);
	if (FAILED(hr)) {
		return false;
	}

	auto& device = GraphicsDevice::instance().getDevice();
	hr = DirectX::CreateTexture(&device, image.GetImage(0, 0, 0), 1, metadata, reinterpret_cast<ID3D11Resource**>(mTexture.GetAddressOf()));
	if (FAILED(hr)) {
		return false;
	}
#if 0
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Format = metadata.format;
	srvDesc.Texture2D.MipLevels = metadata.mipLevels;
	srvDesc.Texture2D.MostDetailedMip = 0;
#endif
	hr = device.CreateShaderResourceView(mTexture.Get(), nullptr, &mSRV);
	if (FAILED(hr)) {
		return false;
	}

	return true;
}

bool TextureResource::isLoadable(Stream& fs)
{
	// ‚Æ‚è‚ ‚¦‚¸ .dds, .png ‚Ì‚Ý
	return fs.getExtension() == L"dds" || fs.getExtension() == L"png";
}

void TextureResource::freeResource()
{}

}
