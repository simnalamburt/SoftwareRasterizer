#include "pch.h"

using namespace std;
using namespace DirectX;
using namespace tbb;



inline int ARGB(BYTE A, BYTE R, BYTE G, BYTE B)
{
    return (A << 24) | (R << 16) | (G << 8) | B;
}

inline int ARGB(BYTE R, BYTE G, BYTE B)
{
    return (R << 16) | (G << 8) | B;
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
{
    size_t width = 800, height = 600;

    // ºñÆ®¸Ê Á¤º¸ÆÄÀÏ
    BITMAPINFO info = {};
    info.bmiHeader.biSize = sizeof BITMAPINFOHEADER;
    info.bmiHeader.biWidth = width;
    info.bmiHeader.biHeight = height;
    info.bmiHeader.biPlanes = 1;
    info.bmiHeader.biBitCount = 32;
    info.bmiHeader.biCompression = BI_RGB;

    // ºñÆ®¸Ê ¹öÆÛ
    XMFLOAT4A A(380, 130, 0, 1);
    XMFLOAT4A B(170, 360, 0, 1);
    XMFLOAT4A C(600, 400, 0, 1);
    if (A.y > B.y) swap(A, B);
    if (A.y > C.y) swap(A, C);
    if (B.y > C.y) swap(B, C);

    XMVECTOR a = XMLoadFloat4A(&A);
    XMVECTOR b = XMLoadFloat4A(&B);
    XMVECTOR c = XMLoadFloat4A(&C);
    XMVECTOR ab = b - a, bc = c - b, ca = a - c;
    XMFLOAT4A AB, BC, CA;
    XMStoreFloat4A(&AB, ab);
    XMStoreFloat4A(&BC, bc);
    XMStoreFloat4A(&CA, ca);
    
    

    unique_ptr<int[]> buffer(new int[width*height]);
    unique_ptr<int*[]> screen(new int*[height]);
    for (size_t i = 0; i < height; ++i) screen[i] = &buffer[i*width];

    // Ã¢ ¶ç¿ì±â
    WindowDesc desc;
    desc.WindowTitle(L"Multi-threaded Software Rasterizer!");
    desc.WindowSize(width, height);
    GdiWindow window(desc);
    window.setDraw([&](HDC hdc)
    {
        parallel_for(blocked_range<size_t>(0, height), [&](const blocked_range<size_t>& r)
        {
            for (size_t y = r.begin(); y < r.end(); ++y)
            {
                float AB_X = AB.x / AB.y * (y - A.y) + A.x;
                float BC_X = BC.x / BC.y * (y - B.y) + B.x;
                float CA_X = CA.x / CA.y * (y - C.y) + C.x;

                function<void()> code;
                if (A.y <= y && y < B.y)
                {
                    for (size_t x = 0; x < width; ++x)
                    {
                        screen[y][x] = ((CA_X <= x && x < AB_X) || (AB_X <= x && x <CA_X)) ? 0x4a8eff : 0xffffff;
                    }
                }
                else if (B.y <= y && y< C.y)
                {
                    for (size_t x = 0; x < width; ++x)
                    {
                        screen[y][x] = ((CA_X <= x && x < BC_X) || (BC_X <= x && x <CA_X)) ? 0x4a8eff : 0xffffff;
                    }
                }
                else
                {
                    for (size_t x = 0; x < width; ++x)
                    {
                        screen[y][x] = 0xffffff;
                    }
                }
            }
        });

        SetDIBitsToDevice(hdc, 0, 0, width, height, 0, 0, 0, height, &screen[0][0], &info, DIB_RGB_COLORS);
    });
    window.Run(nCmdShow);
}
