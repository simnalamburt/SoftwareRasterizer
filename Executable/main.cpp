#include "pch.h"
#include "ScreenBuffer.h"
#include "AlignedAllocator.h"

using namespace std;
using namespace tbb;
using namespace DirectX;
using namespace DirectX::TriangleTests;
using namespace EasyD3D;



__declspec(align(16))
struct Vertex
{
    XMVECTOR Position;
    XMVECTOR Color;
};

struct Primitive
{
    union {
        struct { size_t A, B, C; };
        size_t Indices[3];
    };

    Primitive() = default;
    Primitive(size_t A, size_t B, size_t C) : A(A), B(B), C(C) {}
};



__declspec(align(16))
struct Edge
{
    XMVECTOR A, B;
    float minY, maxY;
    const Primitive* pParent;

    Edge(FXMVECTOR A, FXMVECTOR B, const Primitive& Parent) : A(A), B(B), pParent(&Parent)
    {
        minY = XMVectorGetY(A); maxY = XMVectorGetY(B);
        if (minY > maxY) swap(minY, maxY);
    }

    bool TestY(float Y) const { return minY <= Y && Y < maxY; }
};

struct ActiveEdge
{
    float X, Z;
    const Primitive* pParent;

    ActiveEdge(const Edge& TargetEdge, float TargetY) : pParent(TargetEdge.pParent)
    {
        XMVECTOR Delta = TargetEdge.B - TargetEdge.A;

        float AY = XMVectorGetY(TargetEdge.A);
        float DeltaY = XMVectorGetY(Delta);

        XMVECTOR Result = XMVectorReplicate((TargetY - AY) / DeltaY)*Delta + TargetEdge.A;
        XMFLOAT3A result;
        XMStoreFloat3A(&result, Result);

        X = result.x;
        Z = result.z;
    }
};
bool operator<(const ActiveEdge& Left, const ActiveEdge& Right) { return Left.X < Right.X; }

uint Color(FXMVECTOR ColorVector)
{
    XMFLOAT3A color;
    XMStoreFloat3A(&color, ColorVector);
    return (uint(255 * color.x) & 0xFF) << 16 | (uint(255 * color.y) & 0xFF) << 8 | (uint(255 * color.z) & 0xFF);
}



int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
{
    size_t width = 800, height = 600;

    // 모델 정보
    vector<Vertex, _aligned_allocator<Vertex>> vertexBuffer =
    {
        { XMVectorSet(50, 150, 1, 1), XMVectorSet(0, 0.63f, 1, 0) },
        { XMVectorSet(240, 440, 1, 1), XMVectorSet(0, 0.63f, 1, 0) },
        { XMVectorSet(600, 275, 0, 1), XMVectorSet(0, 0.63f, 1, 0) },
        { XMVectorSet(700, 125, 1, 1), XMVectorSet(1, 0.35f, 0.35f, 0) },
        { XMVectorSet(660, 525, 1, 1), XMVectorSet(1, 0.35f, 0.35f, 0) },
        { XMVectorSet(275, 275, 0, 1), XMVectorSet(1, 0.35f, 0.35f, 0) },
    };
    vector<Primitive> indexBuffer =
    {
        { 0, 1, 2 },
        { 3, 4, 5 }
    };

    // 스크린 버퍼
    ScreenBuffer screenBuffer(width, height);

    // 창 띄우기
    WindowDesc desc;
    desc.WindowTitle(L"Multi-threaded Software Rasterizer!");
    desc.WindowSize(width, height);
    GdiWindow window(desc);
    window.setDraw([&](HDC HDC)
    {
        // 버퍼 클리어
        screenBuffer.ClearBuffer();

        // 에지 테이블 생성
        vector<Edge, _aligned_allocator<Edge>> edgeTable;
        for (const auto& primitive : indexBuffer)
        {
            auto V = [&](size_t i){ return vertexBuffer[primitive.Indices[i]].Position; };
            edgeTable.emplace_back(V(0), V(1), primitive);
            edgeTable.emplace_back(V(1), V(2), primitive);
            edgeTable.emplace_back(V(2), V(0), primitive);
        }

        // 스캔 라인 컨버전
        parallel_for(blocked_range<size_t>(0, height), [&](const blocked_range<size_t>& r)
        {
            for (size_t y = r.begin(); y < r.end(); ++y)
            {
                vector<ActiveEdge> activeTable;
                for (const auto& edge : edgeTable)
                if (edge.TestY((float)y)) activeTable.emplace_back(edge, (float)y);
                if (activeTable.empty()) continue;

                sort(activeTable.rbegin(), activeTable.rend());
                hash_set<const Primitive*> openedPrimitives;
                for (size_t x = 0; x < width; ++x)
                {
                    while (!activeTable.empty() && !(x < activeTable.back().X))
                    {
                        auto result = openedPrimitives.find(activeTable.back().pParent);
                        if (result == openedPrimitives.cend())
                            openedPrimitives.insert(activeTable.back().pParent);
                        else
                            openedPrimitives.erase(result);
                        activeTable.pop_back();
                    }

                    size_t number = openedPrimitives.size();
                    if (number == 1)
                    {
                        screenBuffer(x, y) = Color(vertexBuffer[(*openedPrimitives.begin())->A].Color);
                    }
                    else
                    {
                        float z0 = numeric_limits<float>::infinity();
                        for (const Primitive* pPrimitive : openedPrimitives)
                        {
                            // pPrimitive
                            XMVECTORF32 origin = { (float)x, (float)y, 0, 1 };
                            XMVECTORF32 direction = { 0, 0, 1, 0 };

                            float z;
                            auto V = [&](size_t i){return vertexBuffer[pPrimitive->Indices[i]].Position; };
                            Intersects(origin, direction, V(0), V(1), V(2), z);

                            if (z < z0)
                            {
                                z0 = z;
                                screenBuffer(x, y) = Color(vertexBuffer[pPrimitive->A].Color);
                            }
                        }
                    }
                }
            }
        });

        screenBuffer.Present(HDC);
    });
    window.Run(nCmdShow);
}
