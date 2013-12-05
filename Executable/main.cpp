#include "pch.h"

using namespace std;
using namespace tbb;
using namespace DirectX;
using namespace DirectX::TriangleTests;
using namespace EasyD3D;



int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
{
    size_t width = 800, height = 600;

    // 비트맵 정보파일
    BITMAPINFO info = {};
    info.bmiHeader.biSize = sizeof BITMAPINFOHEADER;
    info.bmiHeader.biWidth = width;
    info.bmiHeader.biHeight = height;
    info.bmiHeader.biPlanes = 1;
    info.bmiHeader.biBitCount = 32;
    info.bmiHeader.biCompression = BI_RGB;
    // 화면 버퍼
    unique_ptr<int[]> buffer(new int[width*height]);
    unique_ptr<int*[]> screen(new int*[height]);
    for (size_t i = 0; i < height; ++i) screen[i] = &buffer[i*width];

    // 창 띄우기
    WindowDesc desc;
    desc.WindowTitle(L"Multi-threaded Software Rasterizer!");
    desc.WindowSize(width, height);
    GdiWindow window(desc);
    window.setDraw([&](HDC hdc)
    {
        for (size_t y = 0; y < height; ++y)
        {
            for (size_t x = 0; x < width; ++x)
            {
                screen[y][x] = 0xc0ffee;
            }
        }

        SetDIBitsToDevice(hdc, 0, 0, width, height, 0, 0, 0, height, &screen[0][0], &info, DIB_RGB_COLORS);
    });
    window.Run(nCmdShow);
}
