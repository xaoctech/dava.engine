#ifdef __DAVAENGINE_BEAST__

#include "BeastMesh.h"
#include "BeastDebug.h"

BeastMesh::BeastMesh(ILBManagerHandle _manager)
:	manager(_manager)
{
}

BeastMesh::~BeastMesh()
{

}

void BeastMesh::SetStaticMesh(DAVA::StaticMesh * staticMesh)
{
	DAVA::uint32 polygonGroupsCount = staticMesh->GetPolygonGroupCount();
	for(DAVA::uint32 polygonGroupIndex = 0; polygonGroupIndex < polygonGroupsCount; ++polygonGroupIndex)
	{
		DAVA::PolygonGroup * polygonGroup = staticMesh->GetPolygonGroup(polygonGroupIndex);
		
		CheckVertexFormat(polygonGroup);
		AddVertices(polygonGroup);
		AddIndices(polygonGroup);
		AddUV(polygonGroup);
	}
}

bool BeastMesh::CheckVertexFormat(DAVA::PolygonGroup * polygonGroup)
{
	DAVA::int32 vertexFormat = polygonGroup->GetFormat();
	if(!((vertexFormat & DAVA::EVF_VERTEX) && (vertexFormat & DAVA::EVF_NORMAL)))
	{
		DAVA::Logger::Error("BeastMesh::AddStaticMesh wrong vertex format");
		return false;
	}

	return true;
}

void BeastMesh::AddVertices(DAVA::PolygonGroup * polygonGroup)
{
	DAVA::int32 vertexCount = polygonGroup->GetVertexCount();
	ILBVec3 * coordData = new ILBVec3[vertexCount];
	ILBVec3 * normalData = new ILBVec3[vertexCount];

	DAVA::Vector3 vertexData;
	for(DAVA::int32 vertexIndex = 0; vertexIndex < vertexCount; ++vertexIndex)
	{
		polygonGroup->GetCoord(vertexIndex, vertexData);
		coordData[vertexIndex] = ILBVec3(vertexData.x, vertexData.y, vertexData.z);

		polygonGroup->GetNormal(vertexIndex, vertexData);
		normalData[vertexIndex] = ILBVec3(vertexData.x, vertexData.y, vertexData.z);
	}

	BEAST_VERIFY(ILBAddVertexData(mesh, coordData, normalData, vertexCount));
	DAVA::SafeDeleteArray(coordData);
	DAVA::SafeDeleteArray(normalData);
}

void BeastMesh::AddIndices(DAVA::PolygonGroup * polygonGroup)
{
	BEAST_VERIFY(ILBBeginMaterialGroup(mesh, "dummy"));
	DAVA::int32 indecesCount = polygonGroup->GetIndexCount();
	DAVA::int32 * indeces = new DAVA::int32[indecesCount];
	DAVA::int32 sourceIndex;
	for(DAVA::int32 indexOfIndex = 0; indexOfIndex < indecesCount; indexOfIndex++)
	{
		polygonGroup->GetIndex(indexOfIndex, sourceIndex);
		indeces[indexOfIndex] = sourceIndex;
	}
	BEAST_VERIFY(ILBAddTriangleData(mesh, indeces, indecesCount));
	DAVA::SafeDeleteArray(indeces);

	BEAST_VERIFY(ILBEndMaterialGroup(mesh));
}

void BeastMesh::AddUV(DAVA::PolygonGroup * polygonGroup)
{
	BEAST_VERIFY(ILBBeginUVLayer(mesh, "0"));

	DAVA::int32 vertexCount = polygonGroup->GetVertexCount();
	ILBVec2 * uvData = new ILBVec2[vertexCount];
	DAVA::Vector2 texCoord;
	for(DAVA::int32 vertexIndex = 0; vertexIndex < vertexCount; ++vertexIndex)
	{
		polygonGroup->GetTexcoord(0, vertexIndex, texCoord);
		uvData[vertexIndex] = ILBVec2(texCoord.x, texCoord.y);
	}
	BEAST_VERIFY(ILBAddUVData(mesh, uvData, vertexCount));
	DAVA::SafeDeleteArray(uvData);

	BEAST_VERIFY(ILBEndUVLayer(mesh));
}

#endif //__DAVAENGINE_BEAST__
