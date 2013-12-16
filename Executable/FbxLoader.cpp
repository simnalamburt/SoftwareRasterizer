#include "pch.h"
#include "FbxLoader.h"

using namespace DirectX;



FbxLoader::FbxLoader(const char* FileName)
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
