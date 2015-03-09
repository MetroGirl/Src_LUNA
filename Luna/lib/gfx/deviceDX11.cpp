#include "stdafx.h"
#include "lib/gfx/deviceDX11.h"

#if _DEBUG
#define V(hr, x) { hr = (x); if (FAILED(hr)) { dx_trace(__FILE__, __LINE__, hr, L#x, true); } }
#else 
#define V(hr, x) { hr = (x); }
#endif

namespace luna {
	LUNA_IMPLEMENT_SINGLETON(GraphicsDevice);

namespace {
  
inline HRESULT dx_trace(char const*     file,
                        unsigned int    line,
                        HRESULT         hr,
                        wchar_t const*  msg,
                        bool pop_msgbox)
{
  // TODO: DXTrace(file, line, hr, msg, pop_msgbox);
  return hr;
}

std::pair<ID3D11RenderTargetView*, ID3D11UnorderedAccessView*>
create_render_target_view(IDXGISwapChain& swap_chain, ID3D11Device& device);

std::tuple<ID3D11Texture2D*, ID3D11RenderTargetView*, ID3D11ShaderResourceView*>
create_buffer_and_view(ID3D11Device& device, int width, int height);

std::pair<ID3D11Texture2D*, ID3D11DepthStencilView*>
create_depth_stencil_and_view(ID3D11Device& device, int width, int height);

void create_dx11(HWND hwnd, int width, int height, bool fullscreen,
	ID3D11Device*& outDevice,
	ID3D11DeviceContext*& outContext,
	IDXGISwapChain*& outSwapChain,
	ID3D11RenderTargetView*& outRTV,
	ID3D11Texture2D*& outDepthStencil,
	ID3D11DepthStencilView*& outDSV)
{
	{// after party: calling CreateDXGIFactory() makes .kkapture happy.
		WORD wAdapterCount = 0;
		IDXGIFactory *lpDXGIFactory = 0;
		IDXGIAdapter *lpDXGIAdapter = 0;
		CreateDXGIFactory(__uuidof(IDXGIFactory), (void **)(&lpDXGIFactory));
		while (lpDXGIFactory->EnumAdapters(wAdapterCount, &lpDXGIAdapter) != DXGI_ERROR_NOT_FOUND)
		{
			DXGI_ADAPTER_DESC AdapterDesc;
			lpDXGIAdapter->GetDesc(&AdapterDesc);
			lpDXGIAdapter->Release();
			lpDXGIAdapter = 0;
			wAdapterCount++;
		}
		if (wAdapterCount == 0)
			return ;
	}

	DXGI_SWAP_CHAIN_DESC desc;
  ::ZeroMemory(&desc, sizeof(desc));
  desc.BufferCount        = 1;
  desc.BufferDesc.Width   = width;
  desc.BufferDesc.Height  = height;
  desc.BufferDesc.Format  = DXGI_FORMAT_R8G8B8A8_UNORM/*_SRGB*/;// after party: this change makes .kkapture happy.
  desc.BufferDesc.RefreshRate.Numerator   = SettingsManager::instance().getFrequency();
  desc.BufferDesc.RefreshRate.Denominator = 1;
  desc.BufferUsage        = DXGI_USAGE_RENDER_TARGET_OUTPUT;// | DXGI_USAGE_UNORDERED_ACCESS;
  desc.OutputWindow       = hwnd;
  desc.SampleDesc.Count   = 1;
  desc.SampleDesc.Quality = 0;
  desc.Windowed           = fullscreen ? FALSE : TRUE;
  
  ID3D11Device*           device;
  ID3D11DeviceContext*    device_context;
  IDXGISwapChain*         swap_chain;
  
  HRESULT hr;
  
  static D3D_FEATURE_LEVEL const feature_levels[] = {
    D3D_FEATURE_LEVEL_11_0,
		// after party:
    //D3D_FEATURE_LEVEL_10_1,        
    //D3D_FEATURE_LEVEL_10_0,
    //D3D_FEATURE_LEVEL_9_3,
    //D3D_FEATURE_LEVEL_9_2,
    //D3D_FEATURE_LEVEL_9_1,
		// :after party
	};
  static int const num_feature_levels = sizeof(feature_levels)/sizeof(D3D_FEATURE_LEVEL);
  
	DWORD creationFlags = 0;
#if LUNA_DEBUG
	creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

  D3D_FEATURE_LEVEL   feature_level = D3D_FEATURE_LEVEL_11_0;
  V(hr, D3D11CreateDeviceAndSwapChain(
      nullptr,
      D3D_DRIVER_TYPE_HARDWARE,
      0,
			creationFlags,
      feature_levels,
      num_feature_levels,
      D3D11_SDK_VERSION,
      &desc,
      &swap_chain,
      &device,
      &feature_level,
			&device_context));

	//  IDXGIOutput* dxgi_output;
	//  V(hr, swap_chain->GetContainingOutput(&dxgi_output));

	//get_display_mode_list(*dxgi_output);

	//  DXGI_OUTPUT_DESC dxgi_output_desc;
	//  V(hr, dxgi_output->GetDesc(&dxgi_output_desc));

	//  DXGI_SWAP_CHAIN_DESC dxgi_swap_chain_desc;
	//  V(hr, swap_chain->GetDesc(&dxgi_swap_chain_desc));

	{// Alt+Enter‚ð–³Œø‰»
		IDXGIDevice * dxgiDevicePtr;
		if (SUCCEEDED(device->QueryInterface(__uuidof(IDXGIDevice), (void **)&dxgiDevicePtr))){
			IDXGIAdapter * dxgiAdapterPtr;
			if (SUCCEEDED(dxgiDevicePtr->GetParent(__uuidof(IDXGIAdapter), (void **)&dxgiAdapterPtr))){
				IDXGIFactory * dxgiFactoryPtr;
				if (SUCCEEDED(dxgiAdapterPtr->GetParent(__uuidof(IDXGIFactory), (void **)&dxgiFactoryPtr))){
					dxgiFactoryPtr->MakeWindowAssociation(hwnd, DXGI_MWA_NO_ALT_ENTER);
				}
			}
		}
	}
  
  auto back_buffer_view   = create_render_target_view(*swap_chain, *device);
  auto depth_stencil_data = create_depth_stencil_and_view(*device, width, height);
  ID3D11Texture2D* depth_stencil = depth_stencil_data.first;
  ID3D11DepthStencilView* depth_stencil_view = depth_stencil_data.second;
     
  device_context->OMSetRenderTargets(1, &back_buffer_view.first, depth_stencil_view);
  
  D3D11_VIEWPORT vp;
  vp.Width    = static_cast<FLOAT>(width);
  vp.Height   = static_cast<FLOAT>(height);
  vp.MinDepth = 0.f;
  vp.MaxDepth = 1.f;
  vp.TopLeftX = 0;
  vp.TopLeftY = 0;
  device_context->RSSetViewports(1, &vp);
  
  outDevice                = device;
  outContext        = device_context;
  outSwapChain           = swap_chain;
  outRTV    = back_buffer_view.first;
  //d->unordered_access_view = back_buffer_view.second;
  outDSV    = depth_stencil_view;
  outDepthStencil         = depth_stencil;
  //swap_chain_desc       = desc;
  //d->dxgi_outputs          = dxgi_output;
}

std::pair<ID3D11RenderTargetView*, ID3D11UnorderedAccessView*>
create_render_target_view(IDXGISwapChain& swap_chain, ID3D11Device& device)
{
  ID3D11RenderTargetView*    render_target_view = nullptr;
  ID3D11UnorderedAccessView* unordered_access_view = nullptr;
  ID3D11Texture2D*           back_buffer = nullptr;
  HRESULT hr = S_OK;
  
  DXGI_SWAP_CHAIN_DESC swap_chain_desc;
  V(hr, swap_chain.GetDesc(&swap_chain_desc));
  
  V(hr, swap_chain.GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&back_buffer));
  
  V(hr, device.CreateRenderTargetView(back_buffer, 0, &render_target_view));
  if (swap_chain_desc.BufferUsage & DXGI_USAGE_UNORDERED_ACCESS) {
    V(hr, device.CreateUnorderedAccessView(back_buffer, 0, &unordered_access_view));
  }
  back_buffer->Release();
  
  return std::make_pair(render_target_view, unordered_access_view);
}
  
std::tuple<ID3D11Texture2D*, ID3D11RenderTargetView*, ID3D11ShaderResourceView*>
create_buffer_and_view(ID3D11Device& device, int width, int height)
{
  HRESULT hr = S_OK;
  ID3D11Texture2D*          buffer;
  ID3D11RenderTargetView*   rtv;
  ID3D11ShaderResourceView* srv;
  {    
    D3D11_TEXTURE2D_DESC desc;
    desc.Width      = width;
    desc.Height     = height;
    desc.MipLevels  = 1;
    desc.ArraySize  = 1;
    desc.Format     = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.Usage      = D3D11_USAGE_DEFAULT;
    desc.BindFlags  = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
    desc.CPUAccessFlags = 0;
    desc.MiscFlags      = 0;
    V(hr, device.CreateTexture2D(&desc, 0, &buffer));
  }
  {
    V(hr, device.CreateRenderTargetView(buffer, nullptr, &rtv));
  }
  {
    V(hr, device.CreateShaderResourceView(buffer, nullptr, &srv));
  }
  return std::make_tuple(buffer, rtv, srv);
}
  
std::pair<ID3D11Texture2D*, ID3D11DepthStencilView*>
create_depth_stencil_and_view(ID3D11Device& device, int width, int height)
{
  HRESULT hr = S_OK;
  
  ID3D11Texture2D* depth_stencil;
  D3D11_TEXTURE2D_DESC depth_desc;
  ZeroMemory(&depth_desc, sizeof(depth_desc));
  depth_desc.Width              = width;
  depth_desc.Height             = height;
  depth_desc.MipLevels          = 1;
  depth_desc.ArraySize          = 1;
  depth_desc.Format             = DXGI_FORMAT_D24_UNORM_S8_UINT;
  depth_desc.SampleDesc.Count   = 1;
  depth_desc.SampleDesc.Quality = 0;
  depth_desc.Usage              = D3D11_USAGE_DEFAULT;
  depth_desc.BindFlags          = D3D11_BIND_DEPTH_STENCIL;
  depth_desc.CPUAccessFlags     = 0;
  depth_desc.MiscFlags          = 0;
  V(hr, device.CreateTexture2D(&depth_desc, 0, &depth_stencil));
  
  ID3D11DepthStencilView* depth_stencil_view;
  D3D11_DEPTH_STENCIL_VIEW_DESC   dsv_desc;
  ZeroMemory(&dsv_desc, sizeof(dsv_desc));
  dsv_desc.Format             = depth_desc.Format;
  dsv_desc.ViewDimension      = D3D11_DSV_DIMENSION_TEXTURE2D;
  dsv_desc.Texture2D.MipSlice = 0;
  V(hr, device.CreateDepthStencilView(depth_stencil, &dsv_desc, &depth_stencil_view));
  
  return std::make_pair(depth_stencil, depth_stencil_view);
}

} // unnamed

GraphicsDevice::GraphicsDevice(HWND hwnd, u32 width, u32 height, bool fullscreen)
{
	create_dx11(hwnd, width, height, fullscreen, *mDevice.GetAddressOf(), *mContext.GetAddressOf(), *mSwapChain.GetAddressOf(), *mRTV.GetAddressOf(), *mDepthStencil.GetAddressOf(), *mDSV.GetAddressOf());

	mInstancePtr = this;//HACK
}

void GraphicsDevice::clear()
{
	f32 color[] = {0.2f, 0.2f, 0.8f, 1.f};
	mContext->ClearRenderTargetView(mRTV.Get(), color);
	mContext->ClearDepthStencilView(mDSV.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0);
}

void GraphicsDevice::present()
{
	mSwapChain->Present(SettingsManager::instance().isVSync() ? 1 : 0, 0);
}

}
