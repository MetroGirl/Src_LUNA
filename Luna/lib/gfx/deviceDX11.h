#pragma once

#include "lib/standard.h"
#include "lib/type.h"

namespace luna {

class GraphicsDevice : public NonConstructableSingleton<GraphicsDevice>
{
public:
	GraphicsDevice(HWND hwnd, u32 width, u32 height, bool fullscreen);

	void clear();

	void present();

	ID3D11RenderTargetView& getBackBuffer() const
	{
		return *mRTV.Get();
	}

	ID3D11DepthStencilView& getDepthStencil() const
	{
		return *mDSV.Get();
	}

	ID3D11Device& getDevice() const
	{
		return *mDevice.Get();
	}

	ID3D11DeviceContext& getDeviceContext() const
	{
		return *mContext.Get();
	}

private:
	ComPtr<IDXGISwapChain> mSwapChain;
	ComPtr<ID3D11Device> mDevice;
	ComPtr<ID3D11DeviceContext> mContext;
	ComPtr<ID3D11RenderTargetView> mRTV;
	ComPtr<ID3D11Texture2D> mDepthStencil;
	ComPtr<ID3D11DepthStencilView> mDSV;
};

}
