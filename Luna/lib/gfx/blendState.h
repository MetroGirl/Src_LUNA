//
// blend state.
//

#ifndef LUNA_BLENDSTATE_H_INCLUDED
#define LUNA_BLENDSTATE_H_INCLUDED

namespace luna{
	enum BlendStateType
	{
		BlendStateType_One = 0,
		BlendStateType_Add,
		BlendStateType_Alpha,

		BlendStateType_Num,

		// alias
		BlendStateType_Begin = 0,
		BlendStateType_End = BlendStateType_Num,
	};

	//! @brief ブレンドステートオブジェクト
	class BlendState
	{
	public:
		BlendState(){}

		void consturct(BlendStateType t);

		ComPtr<ID3D11BlendState> getBlendState() const
		{
			return mBlendState;
		}

	private:
		ComPtr<ID3D11BlendState> mBlendState;

	private:
		BlendState(const BlendState&);
	};

	
}

#endif // LUNA_BLENDSTATE_H_INCLUDED