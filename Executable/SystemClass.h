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

    function<void(double)> IdleFunction;

private:
    static LRESULT CALLBACK WndProc_temp(HWND, UINT, WPARAM, LPARAM);
    static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
    LRESULT CALLBACK MessageHandler(HWND, UINT, WPARAM, LPARAM);

    void onIdle();
};
