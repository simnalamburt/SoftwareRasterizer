#pragma once

class SystemClass : Uncopyable
{
private:
    const HINSTANCE instanceHandle;
    HWND windowHandle;

    const bool isFullScreen;
    const double freq;
    double last;

    function<void(double)> idle;
    function<void(HDC)> draw;

public:
    SystemClass(HINSTANCE InstanceHandle, size_t Width, size_t Height, bool FullScreen);
    ~SystemClass();

    void onIdle(const decltype(idle)& Callback) { idle = Callback; }
    void onDraw(const decltype(draw)& Callback) { draw = Callback; }
    void Run(int ShowCommand);
private:
    static LRESULT CALLBACK wndProc_temp(HWND, UINT, WPARAM, LPARAM);
    static LRESULT CALLBACK wndProc(HWND, UINT, WPARAM, LPARAM);
    LRESULT CALLBACK messageHandler(HWND, UINT, WPARAM, LPARAM);
};
