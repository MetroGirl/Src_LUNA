#include "stdafx.h"
#include "lib/gfx/deviceDX11.h"
#include "app/shaderResource.h"
#include "stringHelper.h"

namespace luna {

LUNA_IMPLEMENT_CONCRETE(luna::ShaderResource);

namespace {

struct d3d_include : public ::ID3DInclude
{
  HRESULT __stdcall Open(D3D_INCLUDE_TYPE include_type, LPCSTR filename, LPCVOID parent_data, LPCVOID* out_data, ::UINT* bytes) override final
  {
    auto path = ascii_to_wide(std::string(filename));

		// D3D_INCLUDE_LOCAL/D3D_INCLUDE_SYSTEM‚Í–³Ž‹
    path = local_path_ + L"\\" + path;

		File f;
		if (!f.open(path.c_str(), File::OpenMode_Read)){
			LUNA_ERRORLINE(L"Shader compilation failed; file can't be opened.");
			return E_FAIL;
		}
		FileStream fs(&f);
		buffer_ = fs.readByteString();

    *out_data = buffer_.data();
    *bytes    = buffer_.size();
    return S_OK;
  }

  HRESULT __stdcall Close(LPCVOID data) override final
  {
    buffer_.clear();
    return S_OK;
  }

  d3d_include(std::wstring const& local_path)
    : local_path_(local_path)
  {}

  std::wstring                local_path_;
  std::vector<std::wstring>   system_pathes_; // TODO:
  std::vector<char>           buffer_;
};


	ShaderCompileInfo getShaderCompileInfo(ScriptContext& L, c8 const* table)
	{
		ShaderCompileInfo out;
		out.mTechniqueName = table;

		for (auto const& def : {
			std::make_pair("vs",&out.mVS),
			std::make_pair("hs",&out.mHS),
			std::make_pair("ds",&out.mDS),
			std::make_pair("gs",&out.mGS),
			std::make_pair("ps",&out.mPS),
			std::make_pair("cs",&out.mCS),
		}) {
			lua_pushstring(L, def.first);
			if (lua_gettable(L, -2)) {
				lua_pushinteger(L, 1);
				if (lua_gettable(L, -2)) {
					def.second->mFileName = lua_tostring(L, -1);
				}
				lua_pop(L, 1);

				lua_pushinteger(L, 2);
				if (lua_gettable(L, -2)) {
					def.second->mEntryPoint = lua_tostring(L, -1);
				}
				lua_pop(L, 1);
			}
			lua_pop(L, 1);
		}
		return out;
	}

	bool CompileShaderFromFile
		(
		const c16* srcDir,
		void* srcBuffer,
		size_t sizeSrcBuffer,
		LPCSTR srcName,
		LPCSTR entryPoint,
		LPCSTR shaderModel,
		ID3DBlob** blobOutPtrPtr
		)
	{
		DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined(DEBUG) || defined(_DEBUG)
		dwShaderFlags |= D3DCOMPILE_DEBUG;
#endif
		ID3DBlob* errorBlobPtr;

		d3d_include resolver(srcDir);
		HRESULT hr = D3DCompile(srcBuffer, sizeSrcBuffer, nullptr, nullptr, &resolver, entryPoint, shaderModel, dwShaderFlags, 0, blobOutPtrPtr, &errorBlobPtr);
		if (FAILED(hr))
		{
			if (errorBlobPtr)
			{
				LUNA_ERRORLINE(L"Compiling shader: %hs -> %hs() as %hs", srcName, entryPoint, shaderModel);
				LUNA_ERRORLINE(L"%hs", (char*)errorBlobPtr->GetBufferPointer());
			}
		}
		if (errorBlobPtr)
		{
			errorBlobPtr->Release();
		}

		return SUCCEEDED(hr);
	}

	ID3DBlob* compileShader(const ShaderCompileParam& param, const c8* shaderModel)
	{
		if (param.mFileName.empty()){
			return nullptr;
		}

		ID3DBlob* blobPtr = nullptr;

		File f;
		if (f.open(param.mFileName.c_str(), File::OpenMode_Read)){
			FileStream fs(&f);
			auto hlslSrc = fs.readByteString();

			CompileShaderFromFile(fs.getDir().c_str(), hlslSrc.data(), hlslSrc.size(), param.mFileName.c_str(), param.mEntryPoint.c_str(), shaderModel, &blobPtr);
		}
		else{
			LUNA_ERRORLINE(L"Shader compilation failed; file can't be opened.");
		}
		return blobPtr;
	}

	ID3D11VertexShader* createShaderVS(ID3DBlob* blobPtr)
	{
		if (!blobPtr){
			return nullptr;
		}
		ID3D11VertexShader* shaderPtr = nullptr;
		luna::GraphicsDevice::instance().getDevice().CreateVertexShader(blobPtr->GetBufferPointer(), blobPtr->GetBufferSize(), nullptr, &shaderPtr);
		return shaderPtr;
	}
	ID3D11HullShader* createShaderHS(ID3DBlob* blobPtr)
	{
		if (!blobPtr){
			return nullptr;
		}
		ID3D11HullShader* shaderPtr = nullptr;
		luna::GraphicsDevice::instance().getDevice().CreateHullShader(blobPtr->GetBufferPointer(), blobPtr->GetBufferSize(), nullptr, &shaderPtr);
		return shaderPtr;
	}
	ID3D11DomainShader* createShaderDS(ID3DBlob* blobPtr)
	{
		if (!blobPtr){
			return nullptr;
		}
		ID3D11DomainShader* shaderPtr = nullptr;
		luna::GraphicsDevice::instance().getDevice().CreateDomainShader(blobPtr->GetBufferPointer(), blobPtr->GetBufferSize(), nullptr, &shaderPtr);
		return shaderPtr;
	}
	ID3D11GeometryShader* createShaderGS(ID3DBlob* blobPtr)
	{
		if (!blobPtr){
			return nullptr;
		}
		ID3D11GeometryShader* shaderPtr = nullptr;
		luna::GraphicsDevice::instance().getDevice().CreateGeometryShader(blobPtr->GetBufferPointer(), blobPtr->GetBufferSize(), nullptr, &shaderPtr);
		return shaderPtr;
	}
	ID3D11ComputeShader* createShaderCS(ID3DBlob* blobPtr)
	{
		if (!blobPtr){
			return nullptr;
		}
		ID3D11ComputeShader* shaderPtr = nullptr;
		luna::GraphicsDevice::instance().getDevice().CreateComputeShader(blobPtr->GetBufferPointer(), blobPtr->GetBufferSize(), nullptr, &shaderPtr);
		return shaderPtr;
	}
	ID3D11PixelShader* createShaderPS(ID3DBlob* blobPtr)
	{
		if (!blobPtr){
			return nullptr;
		}
		ID3D11PixelShader* shaderPtr = nullptr;
		luna::GraphicsDevice::instance().getDevice().CreatePixelShader(blobPtr->GetBufferPointer(), blobPtr->GetBufferSize(), nullptr, &shaderPtr);
		return shaderPtr;
	}
}

std::map<std::string, ShaderCompileInfo> getShaderCompileInfo(ScriptContext& L)
{
	std::map<std::string, ShaderCompileInfo> info;
	lua_getglobal(L, "shaders");
	lua_pushnil(L);
	while (lua_next(L, -2)) {
		lua_pushvalue(L, -2);
		auto const* table_name = lua_tostring(L, -1);
		lua_pop(L, 1);
		info.insert(std::make_pair(table_name, getShaderCompileInfo(L, table_name)));
		lua_pop(L, 1);
	}
	lua_pop(L, 1);
	return info;
}

ShaderResource::ShaderResource()
{}

ShaderResource::~ShaderResource()
{}

bool ShaderResource::load(Stream& fs)
{
	auto buffer = fs.readByteString();
	ScriptContext context;
	luaL_loadbuffer(context, buffer.data(), buffer.size(), "tmp");
	if (lua_pcall(context, 0, 0, 0)) {
		LUNA_ERRORLINE(L"LUA: %hs", lua_tostring(context, -1));
		return false;
	}

	auto infoTbl = getShaderCompileInfo(context);
	mBlobTbl.reserve(infoTbl.size());

	for (auto& def : infoTbl){
		auto& shaderTbl = def.second;

		ShaderBlobInfo info;
		info.mTechniqueName = shaderTbl.mTechniqueName;
		info.mVS.blobPtr = compileShader(shaderTbl.mVS, "vs_5_0");
		info.mVS.shaderPtr = createShaderVS(info.mVS.blobPtr);
		info.mHS.blobPtr = compileShader(shaderTbl.mHS, "hs_5_0");
		info.mHS.shaderPtr = createShaderHS(info.mHS.blobPtr);
		info.mDS.blobPtr = compileShader(shaderTbl.mDS, "ds_5_0");
		info.mDS.shaderPtr = createShaderDS(info.mDS.blobPtr);
		info.mGS.blobPtr = compileShader(shaderTbl.mGS, "gs_5_0");
		info.mGS.shaderPtr = createShaderGS(info.mGS.blobPtr);
		info.mCS.blobPtr = compileShader(shaderTbl.mCS, "cs_5_0");
		info.mCS.shaderPtr = createShaderCS(info.mCS.blobPtr);
		info.mPS.blobPtr = compileShader(shaderTbl.mPS, "ps_5_0");
		info.mPS.shaderPtr = createShaderPS(info.mPS.blobPtr);
		mBlobTbl.push_back(info);
	}

	return true;
}

bool ShaderResource::isLoadable(Stream& fs)
{
	if (fs.getExtension() != L"lua"){
		return false;
	}

	auto buffer = fs.readByteString();

	ScriptContext context;
	luaL_loadbuffer(context, buffer.data(), buffer.size(), "tmp");
	if (lua_pcall(context, 0, 0, 0)) {
		LUNA_ERRORLINE(L"%hs", lua_tostring(context, -1));
		return false;
	}

	lua_getglobal(context, "ID");
	const c8* idStr = lua_tostring(context, -1);
	if (!idStr || strcmp(idStr, "Shader") != 0){
		return false;
	}
	return true;
}

void ShaderResource::freeResource()
{
	for (auto& blob : mBlobTbl){
		blob.mVS.release();
		blob.mHS.release();
		blob.mDS.release();
		blob.mGS.release();
		blob.mCS.release();
		blob.mPS.release();
	}
	mBlobTbl.clear();
}

}
