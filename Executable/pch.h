#pragma once


// windows 헤더
#include <SDKDDKVer.h>
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

// C++ 헤더
#include <string>
#include <memory>
using std::unique_ptr;
using std::shared_ptr;
using std::weak_ptr;

// 사용자 헤더
#include <Uncopyable.h>
#include <ErrorReporter.h>
