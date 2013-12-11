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
    XMVECTOR Normal;
    XMVECTOR Color;

    Vertex() = default;
    Vertex(FXMVECTOR Position, FXMVECTOR Normal, FXMVECTOR Color)
        : Position(Position), Normal(Normal), Color(Color) { }
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
struct Light
{
    XMVECTOR Direction;
    XMVECTOR Color;
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



struct FbxPtrDeleter
{
    void operator()(FbxObject* ptr) const
    {
        ptr->Destroy();
    }
};

template <typename T>
using FbxPtr = unique_ptr<T, FbxPtrDeleter>;


class FbxLoader
{
    vector<Vertex, _aligned_allocator<Vertex>> vertexBuffer;
    vector<Primitive> indexBuffer;

public:
    explicit FbxLoader(const char* FileName)
    {
        FbxManager* manager = FbxManager::Create();
        RAII{ manager->Destroy(); };
        FbxIOSettings* ios = FbxIOSettings::Create(manager, IOSROOT);
        RAII{ ios->Destroy(true); };
        manager->SetIOSettings(ios);

        FbxImporter* importer = FbxImporter::Create(manager, "");
        RAII{ importer->Destroy(true); };
        FbxScene* scene = FbxScene::Create(manager, "");
        RAII{ scene->Destroy(true); };

        FALSE_ERROR(importer->Initialize(FileName, -1, manager->GetIOSettings()));
        FALSE_ERROR(importer->Import(scene));

        FbxNode* rootNode = scene->GetRootNode();
        RAII{ rootNode->Destroy(true); };
        if (rootNode)
        {
            for (int i = 0; i < rootNode->GetChildCount(); i++)
            {
                FbxNode* childNode = rootNode->GetChild(i);

                if (childNode->GetNodeAttribute() == NULL)
                    continue;

                FbxNodeAttribute::EType AttributeType = childNode->GetNodeAttribute()->GetAttributeType();

                if (AttributeType != FbxNodeAttribute::eMesh)
                    continue;

                FbxMesh* mesh = (FbxMesh*)childNode->GetNodeAttribute();
                FbxVector4* vertices = mesh->GetControlPoints();

                int vertexCount = 0;
                for (int j = 0; j < mesh->GetPolygonCount(); j++)
                {
                    assert(mesh->GetPolygonSize(j) == 3);
                    for (int k = 0; k < 3; k++)
                    {
                        const double* data = vertices[mesh->GetPolygonVertex(j, k)].mData;
                        for (int l = 0; l < mesh->GetElementNormalCount(); l++)
                        {
                            FbxLayerElementNormal* leNormal = mesh->GetElementNormal(l);
                            int id = 0;
                            if (leNormal->GetMappingMode() == FbxGeometryElement::eByPolygonVertex)
                            {
                                switch (leNormal->GetReferenceMode())
                                {
                                case FbxGeometryElement::eDirect:
                                    id = vertexCount + k;
                                    break;
                                case FbxGeometryElement::eIndexToDirect:
                                    id = leNormal->GetIndexArray().GetAt(vertexCount + k);
                                    break;
                                default:
                                    break;
                                }
                            }
                            FbxVector4& norm = leNormal->GetDirectArray().GetAt(id);
                            vertexBuffer.emplace_back(
                                XMVectorSet(data[0], data[1], data[2], 1.0f),
                                XMVectorSet(norm[0], norm[1], norm[2], 0.0f),
                                XMVectorSet(1.0f, 1.0f, 1.0f, 0.0f));
                        }
                    }
                    indexBuffer.emplace_back(vertexCount, vertexCount + 1, vertexCount + 2);
                    vertexCount += 3;
                }
            }
        }
    }

    const decltype(vertexBuffer)& getVertexBuffer() const { return vertexBuffer; }
    const decltype(indexBuffer)& getIndexBuffer() const { return indexBuffer; }
};



int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
{
    size_t width = 800, height = 600;

    // 모델, 광원 정보
    FbxLoader Model("box.fbx");
    Light light = { XMVectorSet(0, -0.707107f, -0.707107f, 0), XMVectorSet(1, 1, 1, 0) };
    // 스크린 버퍼
    ScreenBuffer screenBuffer(width, height);

    // 창 띄우기
    WindowDesc desc;
    desc.WindowTitle(L"Multi-threaded Software Rasterizer!");
    desc.WindowSize(width, height);
    GdiWindow window(desc);
    window.setDraw([&](HDC HDC)
    {
        // 스크린버퍼 클리어
        screenBuffer.ClearBuffer();

        // 버퍼 받아옴
        auto vertexBuffer = Model.getVertexBuffer();
        XMVECTORF32 Eye = { 10, 10, -25, 1 };
        XMVECTORF32 Focus = { 0, 0, 0, 1 };
        XMVECTORF32 Up = { 0, 1, 0, 0 };
        for (auto& vertex : vertexBuffer)
        {
            auto& Pos = vertex.Position;

            // 정점 셰이더
            Pos = XMVector4Transform(Pos, XMMatrixLookAtRH(Eye, Focus, Up));
            Pos = XMVector4Transform(Pos, XMMatrixPerspectiveFovRH(XM_PIDIV4, (float)width / (float)height, 0.1f, 100.1f));
            Pos = XMVector4Transform(Pos, XMMatrixTranslation(1.0f, 1.0f, 0.0f));
            Pos = XMVector4Transform(Pos, XMMatrixScaling((float)width / 2.0f, (float)height / 2.0f, 1.0f));
            Pos /= XMVectorGetW(Pos);
        }
        const auto& indexBuffer = Model.getIndexBuffer();

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
                {
                    if (edge.TestY((float)y)) activeTable.emplace_back(edge, (float)y);
                }
                if (activeTable.empty()) continue;
                // x값 순으로 정렬
                sort(activeTable.begin(), activeTable.end());

                unordered_set<const Primitive*> openedPrimitives;
                auto currentEdge = activeTable.cbegin();
                for (size_t x = 0; x < width; ++x)
                {
                    // 현재 점이 어느 도형 안에 포함되어있는지 체크
                    while (currentEdge != activeTable.cend() && !(x < currentEdge->X))
                    {
                        auto result = openedPrimitives.find(currentEdge->pParent);
                        if (result == openedPrimitives.cend())
                            openedPrimitives.insert(currentEdge->pParent);
                        else
                            openedPrimitives.erase(result);
                        ++currentEdge;
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
                            if (!Intersects(origin, direction, V(0), V(1), V(2), z))
                                continue;

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

                    auto N = [&](size_t i){ return vertexBuffer[surface->Indices[i]].Normal; };
                    auto C = [&](size_t i){ return vertexBuffer[surface->Indices[i]].Color; };
                    XMVECTOR Normal = XMVector3NormalizeEst(l0*N(0) + l1*N(1) + l2*N(2));
                    XMVECTOR Color = l0*C(0) + l1*C(1) + l2*C(2);

                    // 픽셀 셰이더
                    XMVECTOR Result = XMVector3Dot(Normal, -light.Direction)*Color*light.Color;

                    XMFLOAT3A result;
                    XMStoreFloat3A(&result, Result);
                    screenBuffer(x, y) = (uint(255 * result.x) & 0xFF) << 16 | (uint(255 * result.y) & 0xFF) << 8 | (uint(255 * result.z) & 0xFF);
                }
            }
        });

        screenBuffer.Present(HDC);
    });
    window.Run(nCmdShow);
}
