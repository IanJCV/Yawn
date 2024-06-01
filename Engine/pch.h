#ifndef ENGINE_PCH_H
#define ENGINE_PCH_H

#define WIN32_LEAN_AND_MEAN
#define UNICODE

#include <d3d11_1.h>       // D3D interface
#include <dxgi.h>        // DirectX driver interface
#include <d3dcompiler.h> // shader compiler
#include <DirectXTK/SimpleMath.h>
#include <DirectXTK/VertexTypes.h>

#include <cassert>
#include <corecrt_wstdio.h>
#include <string>
#include <vector>
#include <array>
#include <thread>
#include <memory>

#include "stbi_image.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <FMOD/fmod_studio.hpp>
#include <FMOD/fmod_errors.h>

#pragma comment( lib, "d3d11.lib" )			// direct3D library
#pragma comment( lib, "dxgi.lib" )			// directx graphics interface
#pragma comment( lib, "d3dcompiler.lib" )	// shader compiler
#pragma comment( lib, "fmodstudio_vc.lib" ) // FMOD

#if (_WIN32_WINNT >= 0x0A00 /*_WIN32_WINNT_WIN10*/)
#pragma comment( lib, "runtimeobject.lib" ) // Windows Runtime Library
#endif


#if _DEBUG
#pragma comment( lib, "assimp-d.lib" )
#pragma comment( lib, "zlibstaticd.lib" )
#else
#pragma comment( lib, "assimp.lib" )
#pragma comment( lib, "zlibstatic.lib" )
#endif

#endif // !ENGINE_PCH_H