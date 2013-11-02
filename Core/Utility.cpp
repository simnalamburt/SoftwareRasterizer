#include "pch.h"
#include "Utility.h"
#include "ErrorReporter.h"



std::wstring LoadString(_In_opt_ HINSTANCE InstanceHandle, _In_ UINT uID)
{
    const wchar_t* src; int len;
    FALSE_ERROR( len = LoadString(InstanceHandle, uID, reinterpret_cast<LPWSTR>( &src ), 0) );
    return std::wstring(src, len);
}

double getTimerFreq()
{
    LARGE_INTEGER freq;
    FALSE_ERROR( QueryPerformanceFrequency(&freq) );
    return static_cast<double>( freq.QuadPart );
}

double getTimerCount()
{
    LARGE_INTEGER count;
    FALSE_ERROR( QueryPerformanceCounter(&count) );
    return static_cast<double>( count.QuadPart );
}
