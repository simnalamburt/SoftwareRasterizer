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



struct Edge
{
    XMFLOAT2A A, B; // A.y <= B.y
    const Primitive* pParent;

    Edge(FXMVECTOR a, FXMVECTOR b, const Primitive& Parent) : pParent(&Parent)
    {
        XMStoreFloat2A(&A, a);
        XMStoreFloat2A(&B, b);
        if (!(A.y <= B.y)) swap(A, B);
    }

    bool TestY(float Y) const { return A.y <= Y && Y < B.y; }
};

struct ActiveEdge
{
    float X;
    const Primitive* pParent;

    ActiveEdge(const Edge& TargetEdge, float TargetY) :
        X((TargetY - TargetEdge.A.y) / (TargetEdge.B.y - TargetEdge.A.y)*(TargetEdge.B.x - TargetEdge.A.x) + TargetEdge.A.x),
        pParent(TargetEdge.pParent) { }
};
bool operator<(const ActiveEdge& Left, const ActiveEdge& Right) { return Left.X < Right.X; }



int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
{
    size_t width = 800, height = 600;

    // 모델 정보
    vector<Vertex, _aligned_allocator<Vertex>> vertexBuffer =
    {
        { XMVectorSet(50, 150, 1, 1), XMVectorSet(0.6f, 0.43f, 1, 0) },
        { XMVectorSet(240, 440, 1, 1), XMVectorSet(0.3f, 0.53f, 1, 0) },
        { XMVectorSet(600, 275, 0, 1), XMVectorSet(0, 0.63f, 1, 0) },
        { XMVectorSet(700, 125, 1, 1), XMVectorSet(1, 0.65f, 0.05f, 0) },
        { XMVectorSet(660, 525, 1, 1), XMVectorSet(1, 0.35f, 0.35f, 0) },
        { XMVectorSet(275, 275, 0, 1), XMVectorSet(1, 0.05f, 0.65f, 0) },
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
                // AET 생성
                vector<ActiveEdge> activeTable;
                for (const auto& edge : edgeTable)
                if (edge.TestY((float)y)) activeTable.emplace_back(edge, (float)y);
                if (activeTable.empty()) continue;

                // x값 순으로 정렬
                sort(activeTable.rbegin(), activeTable.rend());
                unordered_set<const Primitive*> openedPrimitives;
                for (size_t x = 0; x < width; ++x)
                {
                    // 현재 점이 어느 도형 안에 포함되어있는지 체크
                    while (!activeTable.empty() && !(x < activeTable.back().X))
                    {
                        auto result = openedPrimitives.find(activeTable.back().pParent);
                        if (result == openedPrimitives.cend()) openedPrimitives.insert(activeTable.back().pParent);
                        else openedPrimitives.erase(result);
                        activeTable.pop_back();
                    }

                    // 0개 도형에 포함된경우 : skip
                    // 1개 도형에 포함된경우 : 바로 그리기 수행
                    // >2 도형에 포함된경우 : z값 검사 수행
                    size_t number = openedPrimitives.size();
                    if (number == 0) continue;
                    const Primitive* surface = nullptr;
                    XMVECTORF32 origin = { (float)x, (float)y, 0, 1 };
                    XMVECTORF32 direction = { 0, 0, 1, 0 };
                    if (number == 1) surface = *openedPrimitives.begin();
                    else
                    {
                        float z0 = numeric_limits<float>::infinity();
                        for (const Primitive* pPrimitive : openedPrimitives)
                        {
                            float z;
                            auto V = [&](size_t i){return vertexBuffer[pPrimitive->Indices[i]].Position; };
                            Intersects(origin, direction, V(0), V(1), V(2), z);

                            if (z < z0) z0 = z, surface = pPrimitive;
                        }
                    }
                    
                    // Lerp
                    XMVECTOR l0, l1, l2;
                    {
                        const XMVECTOR& A = vertexBuffer[surface->A].Position;
                        const XMVECTOR& B = vertexBuffer[surface->B].Position;
                        const XMVECTOR& C = vertexBuffer[surface->C].Position;

                        XMVECTOR ABC = XMVector2Cross(A - B, B - C);
                        l0 = XMVector2Cross(B - C, C - origin) / ABC;
                        l1 = XMVector2Cross(C - A, A - origin) / ABC;
                        l2 = XMVector2Cross(A - B, B - origin) / ABC;
                    }

                    auto C = [&](size_t i){ return vertexBuffer[surface->Indices[i]].Color; };
                    XMVECTOR Color = l0*C(0) + l1*C(1) + l2*C(2);

                    // 픽셀 셰이더
                    XMFLOAT3A color;
                    XMStoreFloat3A(&color, Color);
                    screenBuffer(x, y) = (uint(255 * color.x) & 0xFF) << 16 | (uint(255 * color.y) & 0xFF) << 8 | (uint(255 * color.z) & 0xFF);
                }
            }
        });

        screenBuffer.Present(HDC);
    });
    window.Run(nCmdShow);
}
