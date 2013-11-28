#pragma once
#include "Type.h"

#pragma region Uncopyable
// 클래스 Uncopyable. 이 클래스를 상속하는 모든 클래스는 복사연산이 불가능해진다.
// Usage : class MyClass : Uncopyable { /* declarations */ };
class Uncopyable
{
protected:
    Uncopyable() = default;
    ~Uncopyable() = default;
    Uncopyable(const Uncopyable&) = delete;
    Uncopyable& operator=(const Uncopyable&) = delete;
};
#pragma endregion
#pragma region Stopwatch
enum class StopwatchStatus { Paused, Running };

// 스톱워치 클래스. 시간 측정에 쓰인다.
class Stopwatch
{
    const static llong CountPerSecond;
    llong countStarted;
    llong countPaused;
    llong countLastTick;

public:
    // 새 스톱워치 객체를 생성한다
    // Status : 새로 생성된 스톱워치가 일시정지된 상태일지, 실행중인 상태일지 결정한다 (기본값 : Paused)
    Stopwatch(StopwatchStatus Status = StopwatchStatus::Paused);

    // 멈춰있는 스톱워치를 작동시킨다. 이미 작동중이었을경우, 아무런일도 일어나지 않는다.
    void Start();
    // 작동중인 스톱워치를 정지시킨다. 이미 멈춰있었을경우, 아무런일도 일어나지 않는다.
    void Pause();

    // 스톱워치가 시작되고나서 총 경과한 시간을 반환한다.
    // 반환값 : 스톱워치가 시작되고나서 경과된 총 시간. (단위 : 초)
    double getTime() const;
    // 스톱워치가 현재 시간을 재는중인지, 일시정지된 상태인지 여부를 반환한다.
    // 반환값 : 현재 스톱워치의 상태
    StopwatchStatus getStatus() const { return countPaused ? StopwatchStatus::Paused : StopwatchStatus::Running; }

    // 마지막으로 Tick() 함수를 호출한지 시간이 얼마나 지났는지 반환한다.
    // 반환값 : 마지막으로 Tick() 함수를 호출하고나서 경과된 총 시간. (단위 : 초)
    double Tick();
};
#pragma endregion
#pragma region RAII
#include <functional>

class Raii : Uncopyable
{
    std::function<void()> code;
public:
    template <typename Func> Raii(Func&& Code) : code(std::move(Code)) { }
    ~Raii() { code(); }
};

#define RAII_HELPER2(n) __raiiObject##n
#define RAII_HELPER1(n) Raii RAII_HELPER2(n) = [&]()

// Usage
//     RAII { /* Do something release in here */ };
//     RAII
//     {
//         CoUninitialize();
//         CoTaskMemFree(mem);
//     };
//
// RAII가 선언된 범위의 스코프가 종료되면, RAII 안에 선언된 코드가 실행된다
#define RAII RAII_HELPER1(__COUNTER__)

#pragma endregion
#pragma region ErrorReporter
enum class ErrorCodeType { HR, Nonzero, False, Custom };
enum class ErrorReportType { Error, Warning };

void Report(ErrorReportType ReportType, int ErrorCode, ErrorCodeType ErrorCodeType,
    const wchar* FileName, size_t LineNumber, const wchar* Function, const wchar* Command, const wchar* Message);



// Macro Bodies
#define _HR(x, report_type, msg) \
do \
{ \
    int __code = (int)(x); \
if (FAILED(__code)) \
    Report(report_type, __code, ErrorCodeType::HR, __FILEW__, __LINE__, __FUNCTIONW__, L#x, msg); \
} \
while (false)

#define _NONZERO(x, report_type, msg) \
do \
{ \
    int __code = (int)(x); \
if (__code) \
    Report(report_type, __code, ErrorCodeType::Nonzero, __FILEW__, __LINE__, __FUNCTIONW__, L#x, msg); \
} \
while (false)

#define _FALSE(x, report_type, msg) \
do \
{ \
    int __code = (int)(x); \
if (!__code) \
    Report(report_type, __code, ErrorCodeType::False, __FILEW__, __LINE__, __FUNCTIONW__, L#x, msg); \
} \
while (false)

#define _CUSTOM(x, report_type, msg) \
    Report(report_type, 0, ErrorCodeType::Custom, __FILEW__, __LINE__, __FUNCTIONW__, L"", msg);


// Interface Macros
#define HR_ERROR_MSG(x, msg) _HR(x, ErrorReportType::Error, msg)
#define NONZERO_ERROR_MSG(x, msg) _NONZERO(x, ErrorReportType::Error, msg)
#define FALSE_ERROR_MSG(x, msg) _FALSE(x, ErrorReportType::Error, msg)
#define ERROR_MSG(msg) _CUSTOM(0, ErrorReportType::Error, msg)

#ifdef _DEBUG
#define HR_WARNING_MSG(x, msg) _HR(x, ErrorReportType::Warning, msg)
#define NONZERO_WARNING_MSG(x, msg) _NONZERO(x, ErrorReportType::Warning, msg)
#define FALSE_WARNING_MSG(x, msg) _FALSE(x, ErrorReportType::Warning, msg)
#define WARNING_MSG(msg) _CUSTOM(0, ErrorReportType::Warning, msg)
#else
#define HR_WARNING_MSG(x, msg) (x)
#define NONZERO_WARNING_MSG(x, msg) (x)
#define FALSE_WARNING_MSG(x, msg) (x)
#define WARNING_MSG(msg)
#endif

#define HR_ERROR(x) HR_ERROR_MSG(x, nullptr)
#define HR_WARNING(x) HR_WARNING_MSG(x, nullptr)
#define NONZERO_ERROR(x) NONZERO_ERROR_MSG(x, nullptr)
#define NONZERO_WARNING(x) NONZERO_WARNING_MSG(x, nullptr)
#define FALSE_ERROR(x) FALSE_ERROR_MSG(x, nullptr)
#define FALSE_WARNING(x) FALSE_WARNING_MSG(x, nullptr)
#pragma endregion
