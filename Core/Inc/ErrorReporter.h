#pragma once

enum class ErrorCodeType { HR, Nonzero, False, Custom };
enum class ErrorReportType { Error, Warning };

void Report(ErrorReportType ReportType, int ErrorCode, ErrorCodeType ErrorCodeType,
    const wchar_t* FileName, size_t LineNumber, const wchar_t* Function, const wchar_t* Command, const wchar_t* Message);



// Macro Body
#define _HR(x, msg, report_type) \
    do \
    { \
        int __code = (int)(x); \
        if (FAILED(__code)) \
            Report(report_type, __code, ErrorCodeType::HR, __FILEW__, __LINE__, __FUNCTIONW__, L#x, msg); \
    } \
    while (false)

#define _NONZERO(x, msg, report_type) \
    do \
    { \
        int __code = (int)(x); \
        if (__code) \
            Report(report_type, __code, ErrorCodeType::Nonzero, __FILEW__, __LINE__, __FUNCTIONW__, L#x, msg); \
    } \
    while (false)

#define _FALSE(x, msg, report_type) \
    do \
    { \
        int __code = (int)(x); \
        if (!__code) \
            Report(report_type, __code, ErrorCodeType::False, __FILEW__, __LINE__, __FUNCTIONW__, L#x, msg); \
    } \
    while (false)

#define _CUSTOM(x, msg, report_type) \
    Report(report_type, 0, ErrorCodeType::Custom, __FILEW__, __LINE__, __FUNCTIONW__, L"", msg);

// Macros
#define HR_ERROR_MSG(x, msg) 				_HR(x, msg, ErrorReportType::Error)
#define HR_WARNING_MSG(x, msg) 			_HR(x, msg, ErrorReportType::Warning)
#define NONZERO_ERROR_MSG(x, msg) 		_NONZERO(x, msg, ErrorReportType::Error)
#define NONZERO_WARNING_MSG(x, msg) 	_NONZERO(x, msg, ErrorReportType::Warning)
#define FALSE_ERROR_MSG(x, msg) 				_FALSE(x, msg, ErrorReportType::Error)
#define FALSE_WARNING_MSG(x, msg) 			_FALSE(x, msg, ErrorReportType::Warning)
#define ERROR_MSG(msg)							_CUSTOM(0, msg, ErrorReportType::Error)
#define WARNING_MSG(msg)						_CUSTOM(0, msg, ErrorReportType::Warning)

#define HR_ERROR(x) 								HR_ERROR_MSG(x, nullptr)
#define HR_WARNING(x) 							HR_WARNING_MSG(x, nullptr)
#define NONZERO_ERROR(x) 						NONZERO_ERROR_MSG(x, nullptr)
#define NONZERO_WARNING(x) 				NONZERO_WARNING_MSG(x, nullptr)
#define FALSE_ERROR(x) 							FALSE_ERROR_MSG(x, nullptr)
#define FALSE_WARNING(x) 						FALSE_WARNING_MSG(x, nullptr)
