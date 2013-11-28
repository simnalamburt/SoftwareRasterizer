#pragma once


// windows 헤더
#include <SDKDDKVer.h>
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

// C++ 헤더
#include <string>
#include <memory>
using std::unique_ptr;
using std::shared_ptr;
using std::weak_ptr;

#include <functional>
using std::function;

// Intel TBB
#include <tbb/parallel_for.h>

// Easy D3D
#pragma comment ( lib, "Utility.lib" )
#include <EasyD3D/Utility.h>
#pragma comment ( lib, "Core.lib" )
#include <EasyD3D/EasyD3D.h>