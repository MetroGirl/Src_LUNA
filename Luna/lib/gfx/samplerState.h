//
// sampler state.
//

#ifndef LUNA_SAMPLERSTATE_H_INCLUDED
#define LUNA_SAMPLERSTATE_H_INCLUDED

namespace luna{
	enum SamplerStateType
	{
		SamplerStateType_Nearest_Clamp = 0,
		SamplerStateType_Nearest_Repeat,
		SamplerStateType_Nearest_Mirror,
		SamplerStateType_Bilinear_Clamp,
		SamplerStateType_Bilinear_Repeat,
		SamplerStateType_Bilinear_Mirror,
		SamplerStateType_Trilinear_Clamp,
		SamplerStateType_Trilinear_Repeat,
		SamplerStateType_Trilinear_Mirror,
		SamplerStateType_Anisotropic_Clamp,
		SamplerStateType_Anisotropic_Repeat,
		SamplerStateType_Anisotropic_Mirror,

		SamplerStateType_Num,

		// alias
		SamplerStateType_Begin = 0,
		SamplerStateType_End = SamplerStateType_Num,
	};

	//! @brief サンプラオブジェクト
	class SamplerState
	{
	public:
		SamplerState(){}

		void consturct(SamplerStateType t);

		ComPtr<ID3D11SamplerState> getSampler() const
		{
			return mSampler;
		}

	private:
		ComPtr<ID3D11SamplerState> mSampler;

	private:
		SamplerState(const SamplerState&);
	};

	
}

#endif // LUNA_FRAMEBUFFER_H_INCLUDED