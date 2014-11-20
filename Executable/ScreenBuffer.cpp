#include "pch.h"
#include "ScreenBuffer.h"



using namespace EasyD3D;

ScreenBuffer::ScreenBuffer(size_t Width, size_t Height) :
buffer(new uint[Width*Height]), bufferStride(new uint*[Height])
{
    info.bmiHeader = { sizeof BITMAPINFOHEADER, Width, Height, 1, 32, BI_RGB }; // C2797 workaround

    for (size_t i = 0; i < Height; ++i) bufferStride[i] = &buffer[i*Width];
}

void ScreenBuffer::Present(HDC HDC) const
{
    SetDIBitsToDevice(HDC, 0, 0, info.bmiHeader.biWidth, info.bmiHeader.biHeight,
        0, 0, 0, info.bmiHeader.biHeight, &buffer[0], &info, DIB_RGB_COLORS);
}

void ScreenBuffer::ClearBuffer(uint value)
{
    const size_t count = info.bmiHeader.biWidth * info.bmiHeader.biHeight;
    for (size_t i = 0; i < count; ++i)
        buffer[i] = value;
}