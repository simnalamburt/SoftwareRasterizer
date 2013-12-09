#include "pch.h"
#include "ScreenBuffer.h"

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



struct Edge
{
    const XMVECTOR& A, B;
    float minY, maxY;
    const Primitive* pParent;

    Edge(const XMVECTOR& A, const XMVECTOR& B, const Primitive& Parent) : A(A), B(B), pParent(&Parent)
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
    vector<Vertex> vertexBuffer =
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
        vector<Edge> edgeTable;
        for (const auto& primitive : indexBuffer)
        {
            for (size_t i = 0; i < 2; i++)
                edgeTable.emplace_back(vertexBuffer[primitive.Indices[i]].Position, vertexBuffer[primitive.Indices[i + 1]].Position, primitive);
            edgeTable.emplace_back(vertexBuffer[primitive.Indices[2]].Position, vertexBuffer[primitive.Indices[0]].Position, primitive);
        }

        // 스캔 라인 컨버전
        parallel_for(blocked_range<size_t>(0, height), [&](const blocked_range<size_t>& r)
        {
            for (size_t y = r.begin(); y < r.end(); ++y)
            {
                vector<ActiveEdge> activeTable;
                for (const auto& edge : edgeTable)
                    if (edge.TestY(y)) activeTable.emplace_back(edge, y);
                if (activeTable.empty()) continue;

                sort(activeTable.begin(), activeTable.end());
                for (size_t i = 0; i < activeTable.size()-1; ++i)
                {
                    ActiveEdge* pA = &activeTable[i];
                    ActiveEdge* pB = &activeTable[i + 1];

                    uint color = Color(vertexBuffer[pA->pParent->A].Color);
                    if (i == activeTable.size() - 1)
                        color = Color(vertexBuffer[pB->pParent->A].Color);
                    for (size_t x = pA->X; x < pB->X; x++)
                        screenBuffer(x, y) = color;
                    if (i != activeTable.size() - 2 &&
                        pA->pParent != pB->pParent &&
                        pB->pParent != activeTable[i + 2].pParent)
                    {
                        pA = pB;
                        pB = &activeTable[++i];
                        for (size_t x = pA->X; x < pB->X; x++)
                        {
                            float Z = numeric_limits<float>::infinity();

                            XMVECTORF32 point = { (float)x, (float)y, 0, 1 };
                            XMVECTORF32 direction = { 0, 0, 1, 0 };

                            float z1, z2;
                            Intersects((XMVECTOR)point, (XMVECTOR)direction,
                                vertexBuffer[pA->pParent->A].Position,
                                vertexBuffer[pA->pParent->B].Position,
                                vertexBuffer[pA->pParent->C].Position, z1);
                            Intersects((XMVECTOR)point, (XMVECTOR)direction,
                                vertexBuffer[pB->pParent->A].Position,
                                vertexBuffer[pB->pParent->B].Position,
                                vertexBuffer[pB->pParent->C].Position, z2);

                            if (Z > z1)
                            {
                                Z = z1;
                                screenBuffer(x, y) = Color(vertexBuffer[pA->pParent->A].Color);
                            }
                            if (Z > z2)
                            {
                                Z = z2;
                                screenBuffer(x, y) = Color(vertexBuffer[pB->pParent->A].Color);
                            }
                        }
                    }
                    if (i != activeTable.size() - 2 && pA->pParent != pB->pParent)
                    {
                        pA = pB;
                        pB = &activeTable[++i];
                    }
                }
            }
        });

        screenBuffer.Present(HDC);
    });
    window.Run(nCmdShow);
}
