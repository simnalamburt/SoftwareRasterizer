#pragma once
#define _CRT_SECURE_NO_WARNINGS

// windows 헤더
#include <SDKDDKVer.h>
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

// DirectXMath.h
#include <DirectXMath.h>
#include <DirectXCollision.h>

// C++ 헤더
#include <array>
#include <vector>
#include <unordered_set>
#include <memory>

// Intel TBB
#include <tbb/parallel_for.h>

// FBX SDK
#pragma comment(lib,"libfbxsdk.lib")
#include <fbxsdk.h>

// Easy D3D
#include <EasyD3D/EasyD3D.h>