//
// frame buffer.
//

#ifndef LUNA_FRAMEBUFFER_H_INCLUDED
#define LUNA_FRAMEBUFFER_H_INCLUDED

namespace luna{
	enum FrameBufferType
	{
		FrameBufferType_SceneTexture0,
		FrameBufferType_SceneTexture1,
		FrameBufferType_SceneTexture2,
		FrameBufferType_SceneTexture3,
		FrameBufferType_WorkBuffer0,
		FrameBufferType_WorkBuffer1,
		FrameBufferType_WorkBuffer2,
		FrameBufferType_WorkBuffer3,
		FrameBufferType_CaptureTexture0,
		FrameBufferType_CaptureTexture1,
		FrameBufferType_CaptureTexture2,
		FrameBufferType_CaptureTexture3,
		FrameBufferType_CaptureTexture4,
		FrameBufferType_CaptureTexture5,
		FrameBufferType_CaptureTexture6,
		FrameBufferType_CaptureTexture7,
		FrameBufferType_CaptureTexture8,
		FrameBufferType_CaptureTexture9,
		FrameBufferType_Num,

		// alias
		FrameBufferType_SceneTextureBegin = FrameBufferType_SceneTexture0,
		FrameBufferType_SceneTextureEnd = FrameBufferType_WorkBuffer0,
		FrameBufferType_WorkBufferBegin = FrameBufferType_WorkBuffer0,
		FrameBufferType_WorkBufferEnd = FrameBufferType_CaptureTexture0,
		FrameBufferType_CaptureTextureBegin = FrameBufferType_CaptureTexture0,
		FrameBufferType_CaptureTextureEnd = FrameBufferType_Num,
	};

	//! @brief フレームバッファオブジェクト
	class FrameBuffer
	{
	public:
		enum ScreenBufferType{
			ScreenBufferType_ColorOnly = 0x01,
			ScreenBufferType_DepthOnly = 0x02,
			ScreenBufferType_ColorAndDepth = ScreenBufferType_ColorOnly | ScreenBufferType_DepthOnly,
		};

		FrameBuffer(){}

		void consturct(s32 width, s32 height, ScreenBufferType type, DXGI_FORMAT colorFormat);

		ComPtr<ID3D11Texture2D> getColor() const
		{
			return mColor;
		}
		ComPtr<ID3D11RenderTargetView> getColorRTV() const
		{
			return mColorRTV;
		}
		ComPtr<ID3D11ShaderResourceView> getColorSRV() const
		{
			return mColorSRV;
		}

		ComPtr<ID3D11Texture2D> getDepth() const
		{
			return mDepth;
		}
		ComPtr<ID3D11DepthStencilView> getDepthDSV() const
		{
			return mDepthDSV;
		}
		ComPtr<ID3D11ShaderResourceView> getDepthSRV() const
		{
			return mDepthSRV;
		}

	private:
		ComPtr<ID3D11Texture2D> mColor;
		ComPtr<ID3D11RenderTargetView> mColorRTV;
		ComPtr<ID3D11ShaderResourceView> mColorSRV;

		ComPtr<ID3D11Texture2D> mDepth;
		ComPtr<ID3D11DepthStencilView> mDepthDSV;
		ComPtr<ID3D11ShaderResourceView> mDepthSRV;

	private:
		FrameBuffer(const FrameBuffer&);
	};

	
}

#endif // LUNA_FRAMEBUFFER_H_INCLUDED