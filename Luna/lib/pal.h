// 
// platform abstraction layer. 
//

#ifndef LUNA_PAL_H_INCLUDED
#define LUNA_PAL_H_INCLUDED

#if LUNA_COMPILER_MSC
#define LUNA_WINDOWS (1)
#else
#define LUNA_WINDOWS (0)
#endif

#if LUNA_WINDOWS
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>
#include <shlwapi.h>
#include <shellapi.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <wrl/client.h>

#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "winmm.lib")

#pragma comment(lib, "dsound.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "D3DCompiler.lib")

#pragma comment(lib, "vorbis_static.lib")
#pragma comment(lib, "ogg_static.lib")
#pragma comment(lib, "vorbisfile_static.lib")

#pragma comment(linker, "/nodefaultlib:libcmt")

namespace luna{
	using Microsoft::WRL::ComPtr;
	using namespace DirectX;
}
#endif

#endif // LUNA_PAL_H_INCLUDED