#pragma once
#include "AlignedAllocator.h"

__declspec(align(16))
struct Vertex
{
    DirectX::XMVECTOR Position;
    DirectX::XMVECTOR Normal;
    DirectX::XMVECTOR Color;

    Vertex() = default;
    Vertex(DirectX::FXMVECTOR Position, DirectX::FXMVECTOR Normal, DirectX::FXMVECTOR Color)
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

class FbxLoader
{
    std::vector<Vertex, _aligned_allocator<Vertex>> vertexBuffer;
    std::vector<Primitive> indexBuffer;

public:
    explicit FbxLoader(const char* FileName);

    const decltype(vertexBuffer)& getVertexBuffer() const { return vertexBuffer; }
    const decltype(indexBuffer)& getIndexBuffer() const { return indexBuffer; }
};
