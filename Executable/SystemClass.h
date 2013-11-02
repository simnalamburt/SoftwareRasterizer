#pragma once

class SystemClass : Uncopyable
{
private:
    const HINSTANCE instanceHandle;
    HWND windowHandle;

    const bool isFullScreen;
    const double freq;
    double last;

public:
    SystemClass(HINSTANCE InstanceHandle, size_t Width, size_t Height, bool FullScreen);
    ~SystemClass();

    void Run(int ShowCommand);

    function<void(double)> onIdle;
    function<void(HDC)> onDraw;

private:
    static LRESULT CALLBACK wndProc_temp(HWND, UINT, WPARAM, LPARAM);
    static LRESULT CALLBACK wndProc(HWND, UINT, WPARAM, LPARAM);
    LRESULT CALLBACK messageHandler(HWND, UINT, WPARAM, LPARAM);
};
