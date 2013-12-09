#pragma once

class ScreenBuffer
{
    BITMAPINFO info;
    std::unique_ptr<EasyD3D::uint[]> buffer;
    std::unique_ptr<EasyD3D::uint*[]> bufferStride;

public:
    ScreenBuffer(size_t Width, size_t Height);

    EasyD3D::uint operator()(size_t X, size_t Y) const { return bufferStride[Y][X]; }
    EasyD3D::uint& operator()(size_t X, size_t Y) { return bufferStride[Y][X]; }

    void Present(HDC HDC) const;
    void ClearBuffer(EasyD3D::uint value = 0xffffff);
};