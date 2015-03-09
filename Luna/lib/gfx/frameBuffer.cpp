//
// frame buffer.
//
#include "stdafx.h"
#include "frameBuffer.h"
#include "deviceDX11.h"

namespace luna{
	void FrameBuffer::consturct(s32 width, s32 height, ScreenBufferType type, DXGI_FORMAT colorFormat)
	{
		HRESULT hr;

		// Color
		if (type&ScreenBufferType_ColorOnly){
			D3D11_TEXTURE2D_DESC texDesc = {};
			texDesc.Usage = D3D11_USAGE_DEFAULT;
			texDesc.Format = colorFormat;
			texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
			texDesc.Width = width;
			texDesc.Height = height;
			texDesc.CPUAccessFlags = 0;
			texDesc.MipLevels = 1;
			texDesc.ArraySize = 1;
			texDesc.SampleDesc.Count = 1;
			texDesc.SampleDesc.Quality = 0;

			hr = GraphicsDevice::instance().getDevice().CreateTexture2D(&texDesc, nullptr, mColor.GetAddressOf());
			if (FAILED(hr))
			{
				LUNA_ASSERT(0, L"");
			}

			D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
			rtvDesc.Format = texDesc.Format;
			rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
			rtvDesc.Texture2D.MipSlice = 0;

			hr = GraphicsDevice::instance().getDevice().CreateRenderTargetView(mColor.Get(), &rtvDesc, mColorRTV.GetAddressOf());
			if (FAILED(hr))
			{
				LUNA_ASSERT(0, L"");
			}

			D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
			srvDesc.Format = texDesc.Format;
			srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
			srvDesc.Texture2D.MostDetailedMip = 0;
			srvDesc.Texture2D.MipLevels = 1;

			hr = GraphicsDevice::instance().getDevice().CreateShaderResourceView(mColor.Get(), &srvDesc, mColorSRV.GetAddressOf());
			if (FAILED(hr))
			{
				LUNA_ASSERT(0, L"");
			}
		}

		if (type&ScreenBufferType_DepthOnly){
			D3D11_TEXTURE2D_DESC texDesc = {};
			texDesc.Usage = D3D11_USAGE_DEFAULT;
			texDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
			texDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
			texDesc.Width = width;
			texDesc.Height = height;
			texDesc.CPUAccessFlags = 0;
			texDesc.MipLevels = 1;
			texDesc.ArraySize = 1;
			texDesc.SampleDesc.Count = 1;
			texDesc.SampleDesc.Quality = 0;

			hr = GraphicsDevice::instance().getDevice().CreateTexture2D(&texDesc, nullptr, mDepth.GetAddressOf());
			if (FAILED(hr))
			{
				LUNA_ASSERT(0, L"");
			}

			D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
			dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
			dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;

			hr = GraphicsDevice::instance().getDevice().CreateDepthStencilView(mDepth.Get(), &dsvDesc, mDepthDSV.GetAddressOf());
			if (FAILED(hr))
			{
				LUNA_ASSERT(0, L"");
			}

			D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
			srvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
			srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
			srvDesc.Texture2D.MostDetailedMip = 0;
			srvDesc.Texture2D.MipLevels = 1;

			hr = GraphicsDevice::instance().getDevice().CreateShaderResourceView(mDepth.Get(), &srvDesc, mDepthSRV.GetAddressOf());
			if (FAILED(hr))
			{
				LUNA_ASSERT(0, L"");
			}

		}
	}
}