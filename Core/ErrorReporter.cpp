#include "pch.h"
#include "ErrorReporter.h"



void Report(ErrorReportType ReportType, int ErrorCode, ErrorCodeType ErrorCodeType,
    const wchar_t* FileName, size_t LineNumber, const wchar_t* Function, const wchar_t* Command, const wchar_t* Message)
{
    std::wostringstream buffer;

    buffer << L"\n\n"
        L"==============================================================================\n" <<
        FileName << L'(' << LineNumber << L")\n"
        L"Function \t: " << Function << L"\n"
        L"Command \t: " << Command << L'\n';

    switch ( ErrorCodeType )
    {
    case ErrorCodeType::HR:
        if ( ErrorCode ) buffer << L"HRESULT \t: " << _com_error(ErrorCode).ErrorMessage() << L" (0x" << ErrorCode << L")\n";
        break;
    case ErrorCodeType::Nonzero:
        if ( ErrorCode ) buffer << L"ErrorCode \t: " << ErrorCode << L'\n';
        break;
    case ErrorCodeType::False:
        buffer << L"Command has returned false(0)\n";
        break;
    }

    DWORD LastError = GetLastError();
    if ( LastError ) buffer << L"GetLastError() \t: " << _com_error(LastError).ErrorMessage() << L" (" << LastError << L")\n";
    if ( Message )
    {
        size_t len = wcslen(Message);
        if ( len > 0 )
        {
            buffer << Message;
            if ( Message[len - 1] != L'\n' )
                buffer << L'\n';
        }
    }

    buffer << L"------------------------------------------------------------------------------\n";

    if ( IsDebuggerPresent() )
    {
        OutputDebugString( buffer.str().c_str() );
        DebugBreak();
    }
    else
    {
        AllocConsole();
        AttachConsole( GetCurrentProcessId() );
        FILE* dummy;
        _wfreopen_s(&dummy, L"CON", L"w", stdout);
        _wsetlocale(LC_ALL, L"");
        _putws( buffer.str().c_str() );
        _wsystem(L"pause");
        FreeConsole();
    }

    if ( ReportType == ErrorReportType::Error )
    {
        ExitProcess(EXIT_FAILURE);
    }
}
