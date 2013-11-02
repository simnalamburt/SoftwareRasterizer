#pragma once

class SystemClass : Uncopyable
{
private:
    HINSTANCE instanceHandle;
    bool isFullScreen;

    HWND windowHandle;

    const double freq;
    double last;

public:
    SystemClass(HINSTANCE InstanceHandle, size_t Width, size_t Height, bool FullScreen);
    ~SystemClass();

    void Run(int ShowCommand);
    LRESULT CALLBACK MessageHandler(HWND, UINT, WPARAM, LPARAM);
private:
    void onIdle();
};
