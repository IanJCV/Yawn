#pragma once
#include "pch.h"

#if GAME_DLLCOMPILE
#define GAME_API __declspec(dllexport)
#else
#define GAME_API __declspec(dllimport)
#endif

#pragma comment( lib, "engine.lib" )