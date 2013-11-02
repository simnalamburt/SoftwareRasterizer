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

// 사용자 헤더
#include "ErrorReporter.h"
#include "Uncopyable.h"
#include "Utility.h"
