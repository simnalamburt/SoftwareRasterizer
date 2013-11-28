#pragma once
#include <minwindef.h>
#include <string>

// Window 클래스를 생성하기위한 설정지정을 도와주는 헬퍼 클래스입니다.
class WindowDesc
{
    friend class Window;
    static size_t Counter;

protected:
    HINSTANCE instanceHandle;
    uint width;
    uint height;
    std::wstring title;
    std::wstring className;
    bool fullscreen;
    bool resizeable;

public:
    // 새로운 WindowDesc 객체를 생성합니다.
    WindowDesc();

    virtual ~WindowDesc() { }

    // 명시적으로 생성될 윈도우가 종속될 프로그램의 인스턴스 핸들을 지정해줍니다.
    // 인스턴스 핸들을 지정해주지 않았을때의 기본값은, WindowDesc 객체를 소유한 스레드의 인스턴스입니다.
    // InstanceHandle : 생성될 윈도우가 종속될 프로그램의 인스턴스
    void Instance(HINSTANCE InstanceHandle) { instanceHandle = InstanceHandle; }

    // 명시적으로 생성될 윈도우의 가로 및 세로 길이를 지정해줍니다.
    // 가로길이와 세로길이의 지정하지 않았을때의 기본값은, CW_USEDEFAULT 입니다.
    // Width : 생성될 윈도우의 초기 가로길이
    // Height : 생성될 윈도우의 초기 세로길이
    void WindowSize(uint Width, uint Height) { width = Width; height = Height; }

    // 생성될 윈도우의 타이틀에 들어갈 문자열을 명시적으로 지정합니다.
    // 지정해주지 않으면, 타이틀에 아무 문자열도 표시되지 않습니다.
    // Title : 생성될 윈도우의 타이틀 문자열
    template <typename String> void WindowTitle(const String& Title) { title = Title; }
    // 생성될 윈도우의 타이틀에 들어갈 문자열을 명시적으로 지정합니다.
    // 지정해주지 않으면, 타이틀에 아무 문자열도 표시되지 않습니다.
    // Title : 생성될 윈도우의 타이틀 문자열
    template <typename String> void WindowTitle(String&& Title) { title = Title; }

    // 생성될 윈도우의 클래스 이름을 명시적으로 지정합니다.
    // ClassName : 생성될 윈도우의 클래스 이름
    template <typename String> void ClassName(const String& ClassName) { className = ClassName; }
    // 생성될 윈도우의 클래스 이름을 명시적으로 지정합니다.
    // ClassName : 생성될 윈도우의 클래스 이름
    template <typename String> void ClassName(String&& ClassName) { className = ClassName; }

    // 생성될 윈도우의 전체화면 여부를 명시적으로 지정합니다.
    // 지정하지 않을경우, 기본적으로 창모드로 생성됩니다.
    // FullScreen : 전체화면 여부
    void FullScreenState(bool Fullscreen) { fullscreen = Fullscreen; }

    // 사용자가 윈도우의 크기를 바꿀 수 있는지 여부를 명시적으로 지정합니다.
    void Resizeable(bool Resizeable) { resizeable = Resizeable; }
};
