#pragma once
#include <windef.h>
#include <string>
#include <functional>
#include <EasyD3D/Utility.h>
class WindowDesc;

enum class WindowStatus { Ready, Running, Finished };

// 새 윈도우를 생성하는 클래스이다. WinAPI 함수들을 래핑하는 클래스이다.
class Window
{
private:
    const HINSTANCE instanceHandle;
    const std::wstring className;
    const bool fullscreen;

    HWND windowHandle;
    WindowStatus status;
    Stopwatch stopwatch;
    std::function<bool(double)> idle;
    std::function<void(ushort, ushort)> resize;

public:
    // 새로운 Window 클래스의 인스턴스를 생성한다.
    // Description : 윈도우 생성에 쓰일 설정
    Window(const WindowDesc& Description);

    virtual ~Window();

    // 매 유휴시간마다 호출될 콜백함수를 정한다.
    // bool Callback(double Elapsed) : 매 유휴시간마다 호출될 콜백
    //     Elapsed : 마지막 있었던 호출로부터 흐른 시간
    //     반환값 : true to receive more idle processing time; false if no more idle time is needed.
    void setIdle(const decltype(idle)& Callback) { idle = Callback; }
    // 매 유휴시간마다 호출될 콜백함수를 정한다.
    // bool Callback(double Elapsed) : 매 유휴시간마다 호출될 콜백
    //     Elapsed : 마지막 있었던 호출로부터 흐른 시간
    //     반환값 : true to receive more idle processing time; false if no more idle time is needed.
    void setIdle(decltype(idle) && Callback) { idle = Callback; }

    // 윈도우의 크기가 변할때마다 호출될 콜백을 지정한다.
    // void Callback(ushort Width, ushort Height) : 윈도우의 크기가 변할때마다 호출될 콜백
    //     Width : 윈도우의 새 가로길이
    //     Height : 윈도우의 새 세로길이
    void setResize(const decltype(resize)& Callback) { resize = Callback; }
    // 윈도우의 크기가 변할때마다 호출될 콜백을 지정한다.
    // void Callback(ushort Width, ushort Height) : 윈도우의 크기가 변할때마다 호출될 콜백
    //     Width : 윈도우의 새 가로길이
    //     Height : 윈도우의 새 세로길이
    void setResize(decltype(resize) && Callback) { resize = Callback; }

    // 만들어진 윈도우 인스턴스를 화면상에 표시하고, 메세지루프를 실행한다.
    // Run() 함수는 한 객체에 대해 2회 이상 호출될 수 없다.
    // ShowCommand : 윈도우가 생성되었을때 윈도우의 상태(초기화상태, 전체화면, 등..)
    // 반환값 : The exit code given in the PostQuitMessage function
    WPARAM Run(int ShowCommand);

    // 윈도우가 종속되어있는 인스턴스 핸들(HINSTANCE)을 반환한다.
    // 반환값 : 인스턴스 핸들(HINSTANCE)
    HINSTANCE getInstanceHandle() const { return instanceHandle; }
    // 윈도우의 클래스 이름을 반환한다.
    // 반환값 : 클래스 이름
    const std::wstring& getClassName() const { return className; }
    // 윈도우의 전체화면 여부를 반환한다.
    // 반환값 : 전체화면 여부
    bool getFullscreenStatus() const { return fullscreen; }
    // Window 클래스에 의해 생성된 윈도우 핸들(HWND)을 반환한다.
    // 반환값 : 윈도우 핸들(HWND)
    HWND getWindowHandle() const { return windowHandle; }
    // 현재 Window 클래스가 실행대기중인지, 실행중인지, 실행이 끝났는지 여부를 반환한다.
    // 반환값 : WindowStatus 값
    WindowStatus getStatus() const { return status; }

private:
    static LRESULT CALLBACK wndProc_temp(HWND, UINT, WPARAM, LPARAM);
    static LRESULT CALLBACK wndProc(HWND, UINT, WPARAM, LPARAM);
    LRESULT messageHandler(HWND, UINT, WPARAM, LPARAM);

protected:
    // WM_PAINT 메세지가 들어올 때 수행될 동작이다. 순수가상함수.
    virtual void onDraw() = 0;
    // WM_SIZE 메세지가 들어올 떄 수행될 동작이다. 오버라이드될 수 있다.
    virtual void onResize(ushort Width, ushort Height) { }
    // 윈도우가 전체화면모드로 들어갈 때 수행될 동작이다. 오버라이드될 수 있다.
    virtual void onEnterFullscreenMode();
    // 윈도우가 전체화면모드에서 빠져나올 떄 수행될 동작이다. 오버라이드될 수 있다.
    virtual void onLeaveFullscreenMode();
};
