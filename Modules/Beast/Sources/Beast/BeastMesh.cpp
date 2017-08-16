#ifdef __DAVAENGINE_BEAST__

#include "BeastMesh.h"
#include "BeastDebug.h"
#include "BeastManager.h"
#include "BeastMaterial.h"
#include "LandscapeGeometry.h"

BeastMesh::BeastMesh(const DAVA::String& name, BeastManager* manager)
    : BeastResource(name, manager)
{
}

void BeastMesh::InitWithSpeedTreeLeaf(DAVA::RenderBatch* batch, DAVA::int32 partIndex, const DAVA::Matrix4& leafTransform)
{
    DVASSERT(mesh == nullptr);

    DAVA::PolygonGroup* polygonGroup = batch->GetPolygonGroup();
    if (CheckVertexFormat(polygonGroup))
    {
        BEAST_VERIFY(DAVA_BEAST::ILBBeginMesh(manager->GetILBManager(), STRING_TO_BEAST_STRING(resourceName), &mesh));
        AddTranfromedVertices(polygonGroup, leafTransform);
        AddIndices(polygonGroup, batch->GetMaterial());
        textureCoordCount = polygonGroup->textureCoordCount;
        for (DAVA_BEAST::int32 i = 0; i < textureCoordCount; ++i)
        {
            BEAST_VERIFY(DAVA_BEAST::ILBBeginUVLayer(mesh, STRING_TO_BEAST_STRING(DAVA::Format("%d", i))));
            AddUV(polygonGroup, i);
            BEAST_VERIFY(DAVA_BEAST::ILBEndUVLayer(mesh));
        }
        BEAST_VERIFY(DAVA_BEAST::ILBEndMesh(mesh));
    }
}

void BeastMesh::InitWithMeshInstancePart(DAVA::RenderBatch* batch, DAVA::int32 partIndex)
{
    DVASSERT(mesh == nullptr);

    DAVA::PolygonGroup* polygonGroup = batch->GetPolygonGroup();
    if (CheckVertexFormat(polygonGroup))
    {
        BEAST_VERIFY(DAVA_BEAST::ILBBeginMesh(manager->GetILBManager(), STRING_TO_BEAST_STRING(resourceName), &mesh));
        AddVertices(polygonGroup);
        AddIndices(polygonGroup, batch->GetMaterial());
        textureCoordCount = polygonGroup->textureCoordCount;
        for (DAVA_BEAST::int32 i = 0; i < textureCoordCount; ++i)
        {
            BEAST_VERIFY(DAVA_BEAST::ILBBeginUVLayer(mesh, STRING_TO_BEAST_STRING(DAVA::Format("%d", i))));
            AddUV(polygonGroup, i);
            BEAST_VERIFY(DAVA_BEAST::ILBEndUVLayer(mesh));
        }
        BEAST_VERIFY(DAVA_BEAST::ILBEndMesh(mesh));
    }
}

bool BeastMesh::CheckVertexFormat(DAVA::PolygonGroup* polygonGroup)
{
    DAVA::int32 vertexFormat = polygonGroup->GetFormat();
    if ((vertexFormat & DAVA::EVF_VERTEX) == 0)
    {
        DAVA::Logger::Error("BeastMesh::CheckVertexFormat wrong vertex format");
        return false;
    }

    return true;
}

void BeastMesh::AddTranfromedVertices(DAVA::PolygonGroup* polygonGroup, const DAVA::Matrix4& transform)
{
    DAVA::int32 vertexCount = polygonGroup->GetVertexCount();
    DAVA::Vector<DAVA_BEAST::ILBVec3> coordData(vertexCount);
    DAVA::Vector<DAVA_BEAST::ILBVec3> normalData(vertexCount);

    DAVA::int32 vertexFormat = polygonGroup->GetFormat();
    DAVA::Vector4 vertexPivot;
    DAVA::Vector3 vertexDataCoord;
    DAVA::Vector3 vertexDataNormal;

    for (DAVA::int32 vertexIndex = 0; vertexIndex < vertexCount; ++vertexIndex)
    {
        polygonGroup->GetCoord(vertexIndex, vertexDataCoord);

        if (vertexFormat & DAVA::EVF_PIVOT4)
        {
            polygonGroup->GetPivot(vertexIndex, vertexPivot);
        }

        vertexDataCoord = DAVA::Vector3(vertexPivot) + DAVA::MultiplyVectorMat3x3(vertexDataCoord - DAVA::Vector3(vertexPivot), transform);

        if (vertexFormat & DAVA::EVF_NORMAL)
        {
            polygonGroup->GetNormal(vertexIndex, vertexDataNormal);
            vertexDataNormal = DAVA::MultiplyVectorMat3x3(vertexDataNormal, transform);
            vertexDataNormal.Normalize();
        }

        coordData[vertexIndex] = DAVA_BEAST::ILBVec3(vertexDataCoord.x, vertexDataCoord.y, vertexDataCoord.z);
        normalData[vertexIndex] = DAVA_BEAST::ILBVec3(vertexDataNormal.x, vertexDataNormal.y, vertexDataNormal.z);
    }

    BEAST_VERIFY(DAVA_BEAST::ILBAddVertexData(mesh, coordData.data(), normalData.data(), vertexCount));
}

void BeastMesh::AddVertices(DAVA::PolygonGroup* polygonGroup)
{
    DAVA::int32 vertexCount = polygonGroup->GetVertexCount();
    DAVA::Vector<DAVA_BEAST::ILBVec3> coordData(vertexCount);
    DAVA::Vector<DAVA_BEAST::ILBVec3> normalData(vertexCount);

    DAVA::int32 vertexFormat = polygonGroup->GetFormat();
    DAVA::Vector3 vertexDataCoord;
    DAVA::Vector3 vertexDataNormal;

    for (DAVA::int32 vertexIndex = 0; vertexIndex < vertexCount; ++vertexIndex)
    {
        polygonGroup->GetCoord(vertexIndex, vertexDataCoord);
        coordData[vertexIndex] = DAVA_BEAST::ILBVec3(vertexDataCoord.x, vertexDataCoord.y, vertexDataCoord.z);

        if (vertexFormat & DAVA::EVF_NORMAL)
        {
            polygonGroup->GetNormal(vertexIndex, vertexDataNormal);
            vertexDataNormal.Normalize();
        }

        normalData[vertexIndex] = DAVA_BEAST::ILBVec3(vertexDataNormal.x, vertexDataNormal.y, vertexDataNormal.z);
    }

    BEAST_VERIFY(DAVA_BEAST::ILBAddVertexData(mesh, coordData.data(), normalData.data(), vertexCount));
}

void BeastMesh::AddTangents(DAVA::PolygonGroup* polygonGroup)
{
    DAVA::int32 vertexCount = polygonGroup->GetVertexCount();
    DAVA::Vector<DAVA_BEAST::ILBVec3> tangentData(vertexCount);
    DAVA::Vector<DAVA_BEAST::ILBVec3> biTangentData(vertexCount);

    DAVA::Vector3 vertexDataNormal;
    DAVA::Vector3 vertexDataTangent;
    for (DAVA::int32 vertexIndex = 0; vertexIndex < vertexCount; ++vertexIndex)
    {
        polygonGroup->GetTangent(vertexIndex, vertexDataTangent);
        vertexDataTangent.Normalize();
        DAVA::Vector3 biTangent = vertexDataNormal.CrossProduct(vertexDataTangent);

        tangentData[vertexIndex] = DAVA_BEAST::ILBVec3(vertexDataTangent.x, vertexDataTangent.y, vertexDataTangent.z);
        biTangentData[vertexIndex] = DAVA_BEAST::ILBVec3(biTangent.x, biTangent.y, biTangent.z);
    }

    BEAST_VERIFY(DAVA_BEAST::ILBAddTangentData(mesh, tangentData.data(), biTangentData.data(), vertexCount));
}

void BeastMesh::AddIndices(DAVA::PolygonGroup* polygonGroup, DAVA::NMaterial* material)
{
    BEAST_VERIFY(DAVA_BEAST::ILBBeginMaterialGroup(mesh, STRING_TO_BEAST_STRING(PointerToString(material))));
    DAVA::int32 vertexCount = polygonGroup->GetVertexCount();
    indecesCount = polygonGroup->GetPrimitiveCount() * 3;
    DAVA::Vector<DAVA::int32> indeces(indecesCount);
    DAVA::int32 sourceIndex;
    for (DAVA::int32 indexOfIndex = 0; indexOfIndex < indecesCount; indexOfIndex++)
    {
        polygonGroup->GetIndex(indexOfIndex, sourceIndex);
        indeces[indexOfIndex] = sourceIndex;
    }
    BEAST_VERIFY(DAVA_BEAST::ILBAddTriangleData(mesh, indeces.data(), indecesCount));
    BEAST_VERIFY(DAVA_BEAST::ILBEndMaterialGroup(mesh));
}

void BeastMesh::AddUV(DAVA::PolygonGroup* polygonGroup, DAVA::int32 textureCoordIndex)
{
    DAVA::Vector2 texCoord;
    DAVA::int32 vertexCount = polygonGroup->GetVertexCount();
    DAVA::Vector<DAVA_BEAST::ILBVec2> uvData(vertexCount);
    for (DAVA::int32 i = 0; i < vertexCount; ++i)
    {
        polygonGroup->GetTexcoord(textureCoordIndex, i, texCoord);
        uvData[i] = DAVA_BEAST::ILBVec2(texCoord.x, 1.f - texCoord.y);
    }
    BEAST_VERIFY(DAVA_BEAST::ILBAddUVData(mesh, uvData.data(), vertexCount));
}

void BeastMesh::InitWithLandscape(LandscapeGeometry* geometry, BeastMaterial* material)
{
    if (nullptr == mesh)
    {
        BEAST_VERIFY(DAVA_BEAST::ILBBeginMesh(manager->GetILBManager(), STRING_TO_BEAST_STRING(resourceName), &mesh));
        AddVertices(geometry);
        AddIndices(geometry, material);

        BEAST_VERIFY(DAVA_BEAST::ILBBeginUVLayer(mesh, CONST_STRING_TO_BEAST_STRING("0")));
        AddUV(geometry);
        BEAST_VERIFY(DAVA_BEAST::ILBEndUVLayer(mesh));

        BEAST_VERIFY(DAVA_BEAST::ILBEndMesh(mesh));
    }
}

void BeastMesh::AddVertices(LandscapeGeometry* geometry)
{
    DAVA::size_type vertexCount = geometry->vertices.size();
    DAVA::Vector<DAVA_BEAST::ILBVec3> coordData(vertexCount);
    DAVA::Vector<DAVA_BEAST::ILBVec3> normalData(vertexCount);

    for (DAVA::size_type i = 0; i < vertexCount; ++i)
    {
        const DAVA::Vector3& position = geometry->vertices[i].position;
        const DAVA::Vector3& normal = geometry->normals[i];

        coordData[i] = DAVA_BEAST::ILBVec3(position.x, position.y, position.z);
        normalData[i] = DAVA_BEAST::ILBVec3(normal.x, normal.y, normal.z);
    }

    BEAST_VERIFY(DAVA_BEAST::ILBAddVertexData(mesh, coordData.data(), normalData.data(), static_cast<DAVA_BEAST::int32>(vertexCount)));
}

void BeastMesh::AddIndices(LandscapeGeometry* geometry, BeastMaterial* material)
{
    BEAST_VERIFY(DAVA_BEAST::ILBBeginMaterialGroup(mesh, STRING_TO_BEAST_STRING(material->GetName())));
    BEAST_VERIFY(DAVA_BEAST::ILBAddTriangleData(mesh, geometry->indices.data(), static_cast<DAVA_BEAST::int32>(geometry->indices.size())));
    BEAST_VERIFY(DAVA_BEAST::ILBEndMaterialGroup(mesh));
}

void BeastMesh::AddUV(LandscapeGeometry* geometry)
{
    DAVA::size_type vertexCount = geometry->vertices.size();
    DAVA::Vector<DAVA_BEAST::ILBVec2> uvData(vertexCount);
    for (DAVA::size_type i = 0; i < vertexCount; ++i)
    {
        const DAVA::Vector2& texCoord = geometry->vertices[i].texCoord;
        uvData[i] = DAVA_BEAST::ILBVec2(texCoord.x, texCoord.y);
    }
    BEAST_VERIFY(DAVA_BEAST::ILBAddUVData(mesh, uvData.data(), static_cast<DAVA::int32>(vertexCount)));
}

DAVA_BEAST::ILBMeshHandle BeastMesh::GetILBMesh() const
{
    return mesh;
}

DAVA::int32 BeastMesh::GetTextureCoordCount() const
{
    return textureCoordCount;
}

bool BeastMesh::HasTangents(DAVA::PolygonGroup* polygonGroup)
{
    return (polygonGroup->GetFormat() & DAVA::EVF_TANGENT ? true : false);
}

#endif //__DAVAENGINE_BEAST__
