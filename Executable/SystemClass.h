#pragma once

class SystemClass : Uncopyable
{
private:
    const HINSTANCE instanceHandle;
    HWND windowHandle;

    const bool isFullScreen;
    const double freq;
    double last;

    function<bool(double)> idle;
    function<void(HDC)> draw;

public:
    // HINSTANCE : 생성될 윈도우가 종속될 프로그램의 인스턴스
    // Width : 생성될 윈도우의 초기 가로길이
    // Height : 생성될 윈도우의 초기 세로길이
    // FullScreen : 전체화면 여부
    SystemClass(HINSTANCE InstanceHandle, size_t Width, size_t Height, bool FullScreen);

    ~SystemClass();

    // 매 유휴시간마다 호출될 콜백함수를 정한다
    // bool Callback(double elapsed) : 매 유휴시간마다 호출될 콜백함수
    //     elapsed : 마지막 있었던 호출로부터 흐른 시간
    //     반환값 : true to receive more idle processing time; false if no more idle time is needed.
    void onIdle(const decltype(idle)& Callback) { idle = Callback; }

    // 그리기를 수행할 콜백함수를 정한다
    // void Callback(HDC hdc) : 매 렌더링 시점마다 호출될 콜백함수
    //     hdc : 렌더링에 사용될 DC 핸들
    void onDraw(const decltype(draw)& Callback) { draw = Callback; }

    // 만들어진 윈도우 인스턴스를 화면상에 표시하고, 메세지루프를 실행한다
    // ShowCommand : 윈도우가 생성되었을때 윈도우의 상태(초기화상태, 전체화면, 등..)
    void Run(int ShowCommand);

private:
    static LRESULT CALLBACK wndProc_temp(HWND, UINT, WPARAM, LPARAM);
    static LRESULT CALLBACK wndProc(HWND, UINT, WPARAM, LPARAM);
    LRESULT CALLBACK messageHandler(HWND, UINT, WPARAM, LPARAM);
};
