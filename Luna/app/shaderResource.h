#pragma once

#include "lib/resourceObject.h"
#include "lib/scriptContext.h"

namespace luna {

// ‚Æ‚è‚ ‚¦‚¸‚±‚±‚Å
	struct ShaderCompileParam
	{
		std::string mFileName;
		std::string mEntryPoint;
	};

	struct ShaderCompileInfo
	{
		std::string mTechniqueName;
		ShaderCompileParam mVS, mHS, mDS, mGS, mPS, mCS;
	};

	struct VSInfo
	{
		ID3DBlob* blobPtr;
		ID3D11VertexShader* shaderPtr;

		void release()
		{
			if (blobPtr){
				blobPtr->Release();
			}
			if (shaderPtr){
				shaderPtr->Release();
			}
		}
	};

	struct HSInfo
	{
		ID3DBlob* blobPtr;
		ID3D11HullShader* shaderPtr;

		void release()
		{
			if (blobPtr){
				blobPtr->Release();
			}
			if (shaderPtr){
				shaderPtr->Release();
			}
		}
	};

	struct DSInfo
	{
		ID3DBlob* blobPtr;
		ID3D11DomainShader* shaderPtr;

		void release()
		{
			if (blobPtr){
				blobPtr->Release();
			}
			if (shaderPtr){
				shaderPtr->Release();
			}
		}
	};

	struct GSInfo
	{
		ID3DBlob* blobPtr;
		ID3D11GeometryShader* shaderPtr;

		void release()
		{
			if (blobPtr){
				blobPtr->Release();
			}
			if (shaderPtr){
				shaderPtr->Release();
			}
		}
	};

	struct CSInfo
	{
		ID3DBlob* blobPtr;
		ID3D11ComputeShader* shaderPtr;

		void release()
		{
			if (blobPtr){
				blobPtr->Release();
			}
			if (shaderPtr){
				shaderPtr->Release();
			}
		}
	};

	struct PSInfo
	{
		ID3DBlob* blobPtr;
		ID3D11PixelShader* shaderPtr;

		void release()
		{
			if (blobPtr){
				blobPtr->Release();
			}
			if (shaderPtr){
				shaderPtr->Release();
			}
		}
	};

	struct ShaderBlobInfo
	{
		std::string mTechniqueName;
		VSInfo mVS;
		HSInfo mHS;
		DSInfo mDS;
		GSInfo mGS;
		CSInfo mCS;
		PSInfo mPS;
	};

	std::map<std::string, ShaderCompileInfo> getShaderCompileInfo(ScriptContext& sc);

	class ShaderResource : public ResourceObject
	{
		LUNA_DECLARE_CONCRETE(ShaderResource, ResourceObject);

	public:
		ShaderResource();
		virtual ~ShaderResource() override;

		const ShaderBlobInfo* findTechnique(const c8* techniqueName)
		{
			auto it = std::find_if(mBlobTbl.begin(), mBlobTbl.end(), [&](const ShaderBlobInfo& a){ return a.mTechniqueName == techniqueName; });
			return it == mBlobTbl.end() ? nullptr : &(*it);
		}

		const vector<ShaderBlobInfo>& getBlobTbl() const
		{
			return mBlobTbl;
		}

	private:
		bool load(Stream&) override;
		bool isLoadable(Stream&) override;
		void freeResource() override;

		vector<ShaderBlobInfo> mBlobTbl;
	};

}
