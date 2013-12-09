#include "pch.h"
#include <fbxsdk.h>
#include <iostream>

using namespace std;
using namespace tbb;
using namespace DirectX;
using namespace DirectX::TriangleTests;
using namespace EasyD3D;

struct MyVertex
{
	float pos[3];
	uint color;
};

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

FbxManager* g_pFbxSdkManager = nullptr;

HRESULT LoadFBX(vector<MyVertex>* pOutVertexVector, const char* path);
void DisplayTexture(FbxGeometry* pGeometry);
void FindAndDisplayTextureInfoByProperty(FbxProperty pProperty, bool& pDisplayHeader, int pMaterialIndex);

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
    // 화면 버퍼
    unique_ptr<int[]> buffer(new int[width*height]);
    unique_ptr<int*[]> screen(new int*[height]);
    for (size_t i = 0; i < height; ++i) screen[i] = &buffer[i*width];

    // 창 띄우기
    WindowDesc desc;
    desc.WindowTitle(L"Multi-threaded Software Rasterizer!");
    desc.WindowSize(width, height);
    GdiWindow window(desc);

	uint Color;
	vector<MyVertex> *VertexVector = new vector<MyVertex>;
	if (LoadFBX(VertexVector, "./../Model/box.fbx") == S_OK) Color = 0xc0ffee;
	else return -1;
	vector<PRIMITIVE> triangles;
	MyVertex v;

	cout << VertexVector->size() << endl;

	for (size_t i = 0; i < VertexVector->size(); i++)
	{
		if (i + 3 > VertexVector->size())
			break;
		v = VertexVector->at(i++);
		XMVECTORF32 A = { (float)v.pos[0], (float)v.pos[1], (float)v.pos[2], 1 };
		v = VertexVector->at(i++);
		XMVECTORF32 B = { (float)v.pos[0], (float)v.pos[1], (float)v.pos[2], 1 };
		v = VertexVector->at(i);
		XMVECTORF32 C = { (float)v.pos[0], (float)v.pos[1], (float)v.pos[2], 1 };
		triangles.push_back({ A, B, C, 0xff5959 });
	}

    window.setDraw([&](HDC hdc)
    {
		for (size_t y = 0; y < height; ++y)
		{
			for (size_t x = 0; x < width; ++x)
			{
				Color = 0xc0ffee;
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
        SetDIBitsToDevice(hdc, 0, 0, width, height, 0, 0, 0, height, &screen[0][0], &info, DIB_RGB_COLORS);
    });
    window.Run(nCmdShow);
}


HRESULT LoadFBX(vector<MyVertex>* pOutVertexVector, const char* path)
{
	if (g_pFbxSdkManager == nullptr)
	{
		g_pFbxSdkManager = FbxManager::Create();

		FbxIOSettings* pIOsettings = FbxIOSettings::Create(g_pFbxSdkManager, IOSROOT);
		g_pFbxSdkManager->SetIOSettings(pIOsettings);
	}

	FbxImporter* pImporter = FbxImporter::Create(g_pFbxSdkManager, "");
	FbxScene* pFbxScene = FbxScene::Create(g_pFbxSdkManager, "");

	bool bSuccess = pImporter->Initialize(path, -1, g_pFbxSdkManager->GetIOSettings());
	if (!bSuccess) return E_FAIL;

	bSuccess = pImporter->Import(pFbxScene);
	if (!bSuccess) return E_FAIL;

	pImporter->Destroy();

	FbxNode* pFbxRootNode = pFbxScene->GetRootNode();

	if (pFbxRootNode)
	{
		for (int i = 0; i < pFbxRootNode->GetChildCount(); i++)
		{
			FbxNode* pFbxChildNode = pFbxRootNode->GetChild(i);

			if (pFbxChildNode->GetNodeAttribute() == NULL)
				continue;

			FbxNodeAttribute::EType AttributeType = pFbxChildNode->GetNodeAttribute()->GetAttributeType();

			if (AttributeType != FbxNodeAttribute::eMesh)
				continue;

			FbxMesh* pMesh = (FbxMesh*)pFbxChildNode->GetNodeAttribute();

			FbxVector4* pVertices = pMesh->GetControlPoints();
			int vertexCount = 0;
			for (int j = 0; j < pMesh->GetPolygonCount(); j++)
			{
				int iNumVertices = pMesh->GetPolygonSize(j);
				assert(iNumVertices == 3);

				for (int k = 0; k < iNumVertices; k++)
				{
					int iControlPointIndex = pMesh->GetPolygonVertex(j, k);

					MyVertex vertex;
					vertex.pos[0] = (float)pVertices[iControlPointIndex].mData[0];
					vertex.pos[1] = (float)pVertices[iControlPointIndex].mData[1];
					vertex.pos[2] = (float)pVertices[iControlPointIndex].mData[2];
					pOutVertexVector->push_back(vertex);
					vertexCount++;
				}
			}
		}
	}
	return S_OK;
}

/*void FindAndDisplayTextureInfoByProperty(FbxProperty pProperty, bool& pDisplayHeader, int pMaterialIndex){

	if (pProperty.IsValid())
	{
		//int lTextureCount = pProperty.GetSrcObjectCount();

		for (int j = 0; j < lTextureCount; ++j)
		{
			//Here we have to check if it's layeredtextures, or just textures:
			FbxLayeredTexture *lLayeredTexture = FbxCast <FbxLayeredTexture>(pProperty.GetSrcObject(FbxLayeredTexture::ClassId, j));
			if (lLayeredTexture)
			{
				FbxLayeredTexture *lLayeredTexture = FbxCast <FbxLayeredTexture>(pProperty.GetSrcObject(FbxLayeredTexture::ClassId, j));
				int lNbTextures = lLayeredTexture->GetSrcObjectCount(FbxTexture::ClassId);
				for (int k = 0; k<lNbTextures; ++k)
				{
					FbxTexture* lTexture = FbxCast <FbxTexture>(lLayeredTexture->GetSrcObject(FbxTexture::ClassId, k));
					if (lTexture)
					{
						if (pDisplayHeader){
							pDisplayHeader = false;
						}

						//NOTE the blend mode is ALWAYS on the LayeredTexture and NOT the one on the texture.
						//Why is that?  because one texture can be shared on different layered textures and might
						//have different blend modes.

						FbxLayeredTexture::EBlendMode lBlendMode;
						lLayeredTexture->GetTextureBlendMode(k, lBlendMode);
					}

				}
			}
			else
			{
				//no layered texture simply get on the property
				FbxTexture* lTexture = FbxCast <FbxTexture>(pProperty.GetSrcObject(FbxTexture::ClassId, j));
				if (lTexture)
				{
					//display connected Material header only at the first time
					if (pDisplayHeader){
						pDisplayHeader = false;
					}

				}
			}
		}
	}//end if pProperty

}

void DisplayTexture(FbxGeometry* pGeometry)
{
	int lMaterialIndex;
	FbxProperty lProperty;
	if (pGeometry->GetNode() == NULL)
		return;
	int lNbMat = pGeometry->GetNode()->GetSrcObjectCount(FbxSurfaceMaterial::ClassId);
	for (lMaterialIndex = 0; lMaterialIndex < lNbMat; lMaterialIndex++){
		FbxSurfaceMaterial *lMaterial = (FbxSurfaceMaterial *)pGeometry->GetNode()->GetSrcObject(FbxSurfaceMaterial::ClassId, lMaterialIndex);
		bool lDisplayHeader = true;

		//go through all the possible textures
		if (lMaterial){

			int lTextureIndex;
			FBXSDK_FOR_EACH_TEXTURE(lTextureIndex)
			{
				lProperty = lMaterial->FindProperty(FbxLayerElement::sTextureChannelNames[lTextureIndex]);
				FindAndDisplayTextureInfoByProperty(lProperty, lDisplayHeader, lMaterialIndex);
			}

		}//end if(lMaterial)

	}// end for lMaterialIndex     
}*/