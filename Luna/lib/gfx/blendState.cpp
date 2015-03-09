//
// blend state.
//
#include "stdafx.h"
#include "blendState.h"
#include "deviceDX11.h"

namespace luna{
	void BlendState::consturct(BlendStateType t)
	{
		HRESULT hr;

		D3D11_BLEND_DESC BlendDesc = {};
		BlendDesc.AlphaToCoverageEnable = FALSE;
		BlendDesc.IndependentBlendEnable = FALSE;

		switch (t)
		{
		case luna::BlendStateType_One:
			BlendDesc.RenderTarget[0].BlendEnable = FALSE;
			BlendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
			BlendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ZERO;
			BlendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
			BlendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
			BlendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
			BlendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
			BlendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
			break;
		case luna::BlendStateType_Add:
			BlendDesc.RenderTarget[0].BlendEnable = TRUE;
			BlendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
			BlendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
			BlendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
			BlendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
			BlendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
			BlendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
			BlendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
			break;
		case luna::BlendStateType_Alpha:
			BlendDesc.RenderTarget[0].BlendEnable = TRUE;
			BlendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
			BlendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
			BlendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
			BlendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ZERO;
			BlendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
			BlendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
			BlendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
			break;
		default:
			break;
		}

		hr = GraphicsDevice::instance().getDevice().CreateBlendState(&BlendDesc, mBlendState.GetAddressOf());
		if (FAILED(hr))
		{
			LUNA_ASSERT(0, L"");
		}
	}
}