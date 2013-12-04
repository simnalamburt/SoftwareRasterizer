#include "pch.h"

using namespace std;
using namespace tbb;
using namespace DirectX;
using namespace DirectX::TriangleTests;



class PRIMITIVE
{
    XMVECTOR a, b, c;
    XMVECTOR plane;
    uint color;

public:
    PRIMITIVE() = default;
    PRIMITIVE(FXMVECTOR A, FXMVECTOR B, FXMVECTOR C, uint Color)
        : a(A), b(B), c(C), plane(XMPlaneFromPoints(A, B, C)), color(Color) { }

    bool Test(FXMVECTOR Point) const
    {
        XMVECTORF32 direction = { 0, 0, 1, 0 };
        float dist = 0.2f;
        return Intersects(Point, direction, a, b, c, dist);
    }

    float Z(FXMVECTOR Point) const
    {
        XMVECTORF32 delta = { 0, 0, 1, 0 };
        XMVECTOR Intersect = XMPlaneIntersectLine(plane, Point, Point + delta);
        return XMVectorGetZ(Intersect);
    }

    uint getColor() const { return color; }
};

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
    array<PRIMITIVE, 2> triangles;
    {
        XMVECTORF32 A = { 50, 150, 1, 1 };
        XMVECTORF32 B = { 240, 440, 1, 1 };
        XMVECTORF32 C = { 600, 275, 0, 1 };
        triangles[0] = { A, B, C, 0x00a2ff };
    }
    {
        XMVECTORF32 A = { 700, 125, 1, 1 };
        XMVECTORF32 B = { 660, 525, 1, 1 };
        XMVECTORF32 C = { 275, 275, 0, 1 };
        triangles[1] = { A, B, C, 0xff5959 };
    }



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
                for (size_t x = 0; x < width; ++x)
                {
                    uint Color = 0xffffff;
                    float Z = numeric_limits<float>::infinity();

                    XMVECTORF32 point = { (float)x, (float)y, 0, 1 };

                    for (size_t i = 0; i < triangles.size(); ++i)
                    {
                        if (triangles[i].Test(point))
                        {
                            float z = triangles[i].Z(point);
                            if (Z > z)
                            {
                                Z = z;
                                Color = triangles[i].getColor();
                            }
                        }
                    }

                    screen[y][x] = Color;
                }
            }
        });

        SetDIBitsToDevice(hdc, 0, 0, width, height, 0, 0, 0, height, &screen[0][0], &info, DIB_RGB_COLORS);
    });
    window.Run(nCmdShow);
}
