#include "pch.h"

using namespace std;
using namespace tbb;
using namespace DirectX;
using namespace DirectX::TriangleTests;
using namespace EasyD3D;

class PRIMITIVE
{
	array<XMVECTOR, 3> v;
	XMVECTOR plane;
	uint color;
	
public:
	PRIMITIVE() = default;
	PRIMITIVE(XMVECTOR A, XMVECTOR B, XMVECTOR C, uint Color)
		: plane(XMPlaneFromPoints(A, B, C)), color(Color)
	{
		v[0] = A;
		v[1] = B;
		v[2] = C;
	}
	
	bool Test(FXMVECTOR Point) const
	{
		XMVECTORF32 direction = { 0, 0, 1, 0 };
		float dist = 0.2f;
		return Intersects(Point, direction, v[0], v[1], v[2], dist);
	}

	float Z(FXMVECTOR Point) const
	{
		XMVECTORF32 delta = { 0, 0, 1, 0 };
		XMVECTOR Intersect = XMPlaneIntersectLine(plane, Point, Point + delta);
		return XMVectorGetZ(Intersect);
	}

	uint getColor() const { return color; }
	array<XMVECTOR, 3> getV() const { return v; }
};

class Edge
{
	float x_min;
	float y_min;
	float y_max;
	float m;
	size_t index;

public:
	Edge() = default;
	Edge(float X_MIN, float Y_MIN, float Y_MAX, float M, size_t I)
		: x_min(X_MIN), y_min(Y_MIN), y_max(Y_MAX), m(M), index(I) { }
	
	float getXMin() const { return x_min; }
	void setXMin(float X_MIN) { x_min = X_MIN; }
	float getYMin() const { return y_min; }
	float getYMax() const { return y_max; }
	float getM() const { return m; }
	size_t getI() const { return index; }
};

bool cmpX(Edge a, Edge b)
{
	return a.getXMin() < b.getXMin();
}

bool cmpY(Edge a, Edge b)
{
	return a.getYMin() < b.getYMin();
}

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
	vector<Edge> edges;
	vector<Edge> active;

	for (size_t n = 0; n < triangles.size(); n++)
	{
		array<XMVECTOR, 3> v = triangles[n].getV();
		for (size_t i = 0; i < v.size(); i++)
		{
			XMVECTOR a = v[i];
			XMVECTOR b = (i == v.size() - 1 ? v[0] : v[i + 1]);
			float x_a = XMVectorGetX(a), x_b = XMVectorGetX(b), y_a = XMVectorGetY(a), y_b = XMVectorGetY(b);
			if (y_a != y_b)
			{
				double m;
				float X_MIN, Y_MIN, Y_MAX;
				if (y_a < y_b)
				{
					X_MIN = x_a;
					Y_MIN = y_a;
					Y_MAX = y_b;
					m = (Y_MAX - Y_MIN) / (x_b - x_a);
				}
				else
				{
					X_MIN = x_b;
					Y_MIN = y_b;
					Y_MAX = y_a;
					m = (Y_MAX - Y_MIN) / (x_a - x_b);
				}
				edges.push_back(Edge(X_MIN, Y_MIN, Y_MAX, 1 / m, n));
			}
		}
	}

	sort(edges.begin(), edges.end(), cmpY);

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

		for (size_t y = 0; y < height; y++)
		{
			for (size_t x = 0; x < width; x++)
			{
				screen[y][x] = 0xffffff;
			}
			for (size_t i = 0; i < edges.size(); i++)
			{
				if (y < edges[i].getYMin())
					continue;
				if (y < edges[i].getYMax())
					active.push_back(edges[i]);
			}
			if (active.empty())
				continue;
			sort(active.begin(), active.end(), cmpX);
			Edge a = active[0];
			for (size_t i = 1; i < active.size(); i++)
			{
				Edge b = active[i];
				uint Color = triangles[a.getI()].getColor();
				if (i == active.size() - 1)
					Color = triangles[b.getI()].getColor();
				for (size_t x = a.getXMin(); x < b.getXMin(); x++)
					screen[y][x] = Color;
				if (i != active.size() - 1 && a.getI() != b.getI() && b.getI() != active[i + 1].getI())
				{
					a = b;
					b = active[++i];
					for (size_t x = a.getXMin(); x < b.getXMin(); x++)
					{
						float Z = numeric_limits<float>::infinity();
						XMVECTORF32 point = { (float)x, (float)y, 0, 1 };
						float z1 = triangles[a.getI()].Z(point);
						float z2 = triangles[b.getI()].Z(point);

						if (Z > z1)
						{
							Z = z1;
							screen[y][x] = triangles[a.getI()].getColor();
						}
						if (Z > z2)
						{
							Z = z2;
							screen[y][x] = triangles[b.getI()].getColor();
						}
					}
				}
				if (i != active.size() - 1 && a.getI() == b.getI())
					b = active[++i];
				a = b;
			}
			for (size_t i = 0; i < edges.size(); i++)
			{
				if (y < edges[i].getYMin())
					continue;
				if (y <= edges[i].getYMax())
					edges[i].setXMin(edges[i].getXMin() + edges[i].getM());
			}
			active.clear();
		}
        SetDIBitsToDevice(hdc, 0, 0, width, height, 0, 0, 0, height, &screen[0][0], &info, DIB_RGB_COLORS);
    });
    window.Run(nCmdShow);
}
