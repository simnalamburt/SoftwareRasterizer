#include "pch.h"
#include "SystemClass.h"



inline int ARGB(BYTE A, BYTE R, BYTE G, BYTE B)
{
    return (A << 24) | (R << 16) | (G << 8) | B;
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
{
    size_t width = 800;
    size_t height = 600;


    // ºñÆ®¸Ê Á¤º¸ÆÄÀÏ
    BITMAPINFO info = {};
    info.bmiHeader.biSize = sizeof BITMAPINFOHEADER;
    info.bmiHeader.biWidth = width;
    info.bmiHeader.biHeight = height;
    info.bmiHeader.biPlanes = 1;
    info.bmiHeader.biBitCount = 32;
    info.bmiHeader.biCompression = BI_RGB;

    // ºñÆ®¸Ê ¹öÆÛ
    unique_ptr<int[]> buffer(new int[width*height]);
    unique_ptr<int*[]> screen(new int*[height]);
    for( size_t i=0; i<height; ++i ) screen[i] = &buffer[i*width];


    // Ã¢ ¶ç¿ì±â
    SystemClass window(hInstance, width, height, false);
    window.onDraw = [&info, &screen](HDC hdc)
    {
        size_t width = info.bmiHeader.biWidth;
        size_t height = info.bmiHeader.biHeight;

        for( size_t y=0; y<height; ++y )
        {
            for( size_t x=0; x<width; ++x )
            {
                screen[y][x] = 0x47a6ff;
            }
        }

        SetDIBitsToDevice(hdc, 0, 0, width, height, 0, 0, 0, height, &screen[0][0], &info, DIB_RGB_COLORS);
    };
    window.Run(nCmdShow);
}
