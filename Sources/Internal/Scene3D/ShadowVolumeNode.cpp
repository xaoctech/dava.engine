/*==================================================================================
    Copyright (c) 2008, DAVA Consulting, LLC
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the DAVA Consulting, LLC nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE DAVA CONSULTING, LLC AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL DAVA CONSULTING, LLC BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/

#include "ShadowVolumeNode.h"
#include "Render/RenderManager.h"
#include "Render/3D/StaticMesh.h"
#include "Scene3D/Scene.h"



namespace DAVA
{

REGISTER_CLASS(ShadowVolumeNode);

DAVA::ShadowVolumeNode::ShadowVolumeNode()
{
	shader = new Shader();
	shader->LoadFromYaml("~res:/Shaders/ShadowVolume/shadowvolume.shader");
	shader->Recompile();
}

DAVA::ShadowVolumeNode::~ShadowVolumeNode()
{
	SafeRelease(shader);
}

void DAVA::ShadowVolumeNode::Draw()
{
	scene->AddDrawTimeShadowVolume(this);
}

void DAVA::ShadowVolumeNode::DrawShadow()
{
	Matrix4 prevMatrix = RenderManager::Instance()->GetMatrix(RenderManager::MATRIX_MODELVIEW); 
	Matrix4 meshFinalMatrix = worldTransform * prevMatrix;
	RenderManager::Instance()->SetMatrix(RenderManager::MATRIX_MODELVIEW, meshFinalMatrix);

	Matrix4 projMatrix = RenderManager::Instance()->GetMatrix(RenderManager::MATRIX_PROJECTION);

	PolygonGroup * group = shadowPolygonGroup;

	RenderManager::Instance()->SetShader(shader);
	RenderManager::Instance()->SetRenderData(group->renderDataObject);
	RenderManager::Instance()->FlushState();

	int32 uniformLightPosition0 = shader->FindUniformLocationByName("lightPosition0");
	if (uniformLightPosition0 != -1)
	{
		Vector3 lightPosition0(100.f, 100.f, 100.0f);
		const Matrix4 & matrix = scene->GetCurrentCamera()->GetMatrix();
		lightPosition0 = lightPosition0 * matrix;

		shader->SetUniformValue(uniformLightPosition0, lightPosition0); 
	}

	if (group->renderDataObject->GetIndexBufferID() != 0)
	{
		RenderManager::Instance()->HWDrawElements(PRIMITIVETYPE_TRIANGLELIST, group->indexCount, EIF_16, 0);
	}
	else
	{
		RenderManager::Instance()->HWDrawElements(PRIMITIVETYPE_TRIANGLELIST, group->indexCount, EIF_16, group->indexArray);
	}

	RenderManager::Instance()->SetMatrix(RenderManager::MATRIX_MODELVIEW, prevMatrix);
}



int32 ShadowVolumeNode::FindEdgeInMappingTable(int32 nV1, int32 nV2, EdgeMapping* mapping, int32 count)
{
	for( int i = 0; i < count; ++i )
	{
		// If both vertex indexes of the old edge in mapping entry are -1, then
		// we have searched every valid entry without finding a match.  Return
		// this index as a newly created entry.
		if( ( mapping[i].oldEdge[0] == -1 && mapping[i].oldEdge[1] == -1 ) ||

			// Or if we find a match, return the index.
			( mapping[i].oldEdge[1] == nV1 && mapping[i].oldEdge[0] == nV2 ) )
		{
			return i;
		}
	}

	DVASSERT(0);
	return -1;  // We should never reach this line
}


void ShadowVolumeNode::CopyGeometryFrom(MeshInstanceNode * meshInstance)
{
	PolygonGroup * oldPolygonGroup = meshInstance->GetPolygonGroups()[0]->GetPolygonGroup();

	

	int32 numEdges = oldPolygonGroup->GetIndexCount();
	int32 oldIndexCount = oldPolygonGroup->GetIndexCount();
	EdgeMapping * mapping = new EdgeMapping[numEdges];
	int32 numMaps = 0;

	//generate adjacency
	int32 oldVertexCount = oldPolygonGroup->GetVertexCount();
	int32 * adjacency = new int32[oldVertexCount];
	Memset(adjacency, -1, oldVertexCount*sizeof(int32));
	for(int32 i = 0; i < oldVertexCount; ++i)
	{
		Vector3 newFoundCoord;
		oldPolygonGroup->GetCoord(i, newFoundCoord);
		adjacency[i] = i;
		for(int32 j = 0; j < i; ++j)
		{
			Vector3 oldCoord;
			oldPolygonGroup->GetCoord(j, oldCoord);
			if(EdgeAdjacency::IsPointsEqual(newFoundCoord, oldCoord))
			{
				adjacency[i] = j;
				break;
			}
		}
	}

	PolygonGroup * newPolygonGroup = new PolygonGroup(GetScene());
	newPolygonGroup->AllocateData(EVF_VERTEX | EVF_NORMAL, oldIndexCount, oldIndexCount + numEdges*2*3);
	int32 nextIndex = 0;

	//int32 nextOutVertex = 0;

	int32 facesCount = oldIndexCount/3;
	for(int32 f = 0; f < facesCount; ++f)
	{
		//copy old vertex data
		int32 oldIndex0, oldIndex1, oldIndex2;
		Vector3 oldPos0, oldPos1, oldPos2;
		oldPolygonGroup->GetIndex(f*3+0, oldIndex0);
		oldPolygonGroup->GetCoord(oldIndex0, oldPos0);
		newPolygonGroup->SetCoord(f*3+0, oldPos0);
		newPolygonGroup->SetIndex(nextIndex++, f*3+0);

		oldPolygonGroup->GetIndex(f*3+1, oldIndex1);
		oldPolygonGroup->GetCoord(oldIndex1, oldPos1);
		newPolygonGroup->SetCoord(f*3+1, oldPos1);
		newPolygonGroup->SetIndex(nextIndex++, f*3+1);

		oldPolygonGroup->GetIndex(f*3+2, oldIndex2);
		oldPolygonGroup->GetCoord(oldIndex2, oldPos2);
		newPolygonGroup->SetCoord(f*3+2, oldPos2);
		newPolygonGroup->SetIndex(nextIndex++, f*3+2);

		//generate new normals
		Vector3 v0 = oldPos1 - oldPos0;
		Vector3 v1 = oldPos2 - oldPos0;
		Vector3 normal = v0.CrossProduct(v1);
		normal.Normalize();

		newPolygonGroup->SetNormal(f*3+0, normal);
		newPolygonGroup->SetNormal(f*3+1, normal);
		newPolygonGroup->SetNormal(f*3+2, normal);


		//edge 1
		int32 nIndex;
		int32 vertIndex[3] = 
		{
			adjacency[oldIndex0],
			adjacency[oldIndex1],
			adjacency[oldIndex2]
		};
		nIndex = FindEdgeInMappingTable(vertIndex[0], vertIndex[1], mapping, numEdges);

		if(mapping[nIndex].oldEdge[0] == -1 && mapping[nIndex].oldEdge[1] == -1)
		{
			// No entry for this edge yet.  Initialize one.
			mapping[nIndex].oldEdge[0] = vertIndex[0];
			mapping[nIndex].oldEdge[1] = vertIndex[1];
			mapping[nIndex].newEdge[0][0] = f*3 + 0;
			mapping[nIndex].newEdge[0][1] = f*3 + 1;

			++numMaps;
		}
		else
		{
			// An entry is found for this edge.  Create
			// a quad and output it.
			mapping[nIndex].newEdge[1][0] = f*3 + 0;
			mapping[nIndex].newEdge[1][1] = f*3 + 1;

			// First triangle
			newPolygonGroup->SetIndex(nextIndex++, mapping[nIndex].newEdge[0][1]);
			newPolygonGroup->SetIndex(nextIndex++, mapping[nIndex].newEdge[0][0]);
			newPolygonGroup->SetIndex(nextIndex++, mapping[nIndex].newEdge[1][0]);

			// Second triangle
			newPolygonGroup->SetIndex(nextIndex++, mapping[nIndex].newEdge[1][1]);
			newPolygonGroup->SetIndex(nextIndex++, mapping[nIndex].newEdge[1][0]);
			newPolygonGroup->SetIndex(nextIndex++, mapping[nIndex].newEdge[0][0]);

			// pMapping[nIndex] is no longer needed. Copy the last map entry
			// over and decrement the map count.
			mapping[nIndex] = mapping[numMaps - 1];
			Memset(&mapping[numMaps - 1], 0xFF, sizeof(mapping[numMaps - 1]));
			--numMaps;
		}

		//edge 2
		nIndex = FindEdgeInMappingTable(vertIndex[1], vertIndex[2], mapping, numEdges);

		if(mapping[nIndex].oldEdge[0] == -1 && mapping[nIndex].oldEdge[1] == -1)
		{
			mapping[nIndex].oldEdge[0] = vertIndex[1];
			mapping[nIndex].oldEdge[1] = vertIndex[2];
			mapping[nIndex].newEdge[0][0] = f*3 + 1;
			mapping[nIndex].newEdge[0][1] = f*3 + 2;

			++numMaps;
		}
		else
		{
			mapping[nIndex].newEdge[1][0] = f*3 + 1;
			mapping[nIndex].newEdge[1][1] = f*3 + 2;

			newPolygonGroup->SetIndex(nextIndex++, mapping[nIndex].newEdge[0][1]);
			newPolygonGroup->SetIndex(nextIndex++, mapping[nIndex].newEdge[0][0]);
			newPolygonGroup->SetIndex(nextIndex++, mapping[nIndex].newEdge[1][0]);

			newPolygonGroup->SetIndex(nextIndex++, mapping[nIndex].newEdge[1][1]);
			newPolygonGroup->SetIndex(nextIndex++, mapping[nIndex].newEdge[1][0]);
			newPolygonGroup->SetIndex(nextIndex++, mapping[nIndex].newEdge[0][0]);

			mapping[nIndex] = mapping[numMaps - 1];
			Memset(&mapping[numMaps - 1], 0xFF, sizeof(mapping[numMaps - 1]));
			--numMaps;
		}

		//edge 3
		nIndex = FindEdgeInMappingTable(vertIndex[2], vertIndex[0], mapping, numEdges);

		if(mapping[nIndex].oldEdge[0] == -1 && mapping[nIndex].oldEdge[1] == -1)
		{
			mapping[nIndex].oldEdge[0] = vertIndex[2];
			mapping[nIndex].oldEdge[1] = vertIndex[0];
			mapping[nIndex].newEdge[0][0] = f*3 + 2;
			mapping[nIndex].newEdge[0][1] = f*3 + 0;

			++numMaps;
		}
		else
		{
			mapping[nIndex].newEdge[1][0] = f*3 + 2;
			mapping[nIndex].newEdge[1][1] = f*3 + 0;

			newPolygonGroup->SetIndex(nextIndex++, mapping[nIndex].newEdge[0][1]);
			newPolygonGroup->SetIndex(nextIndex++, mapping[nIndex].newEdge[0][0]);
			newPolygonGroup->SetIndex(nextIndex++, mapping[nIndex].newEdge[1][0]);

			newPolygonGroup->SetIndex(nextIndex++, mapping[nIndex].newEdge[1][1]);
			newPolygonGroup->SetIndex(nextIndex++, mapping[nIndex].newEdge[1][0]);
			newPolygonGroup->SetIndex(nextIndex++, mapping[nIndex].newEdge[0][0]);

			mapping[nIndex] = mapping[numMaps - 1];
			Memset(&mapping[numMaps - 1], 0xFF, sizeof(mapping[numMaps - 1]));
			--numMaps;
		}
	}

	Logger::Debug("faces to path: %d", numMaps);

	//EdgeAdjacency oldAdjacency;
	//oldAdjacency.InitFromPolygonGroup(oldPolygonGroup, oldPolygonGroup->GetIndexCount());
	//int32 sharedEdgesCount = oldAdjacency.GetEdgesWithTwoTrianglesCount();

	//shadowPolygonGroup = new PolygonGroup(GetScene());
	//shadowPolygonGroup->AllocateData(EVF_VERTEX | EVF_NORMAL, oldPolygonGroup->GetIndexCount() + sharedEdgesCount*2*2,//*2 - two triangles per edge, *2 - 2 vertices per triangle
	//	oldPolygonGroup->GetIndexCount() + sharedEdgesCount*2*3);//*2 - two triangles per edge, *3 - 3 indeces per triangle

	////copy old index data
	//int32 oldIndexCount = oldPolygonGroup->GetIndexCount();
	////for(int32 i = 0; i < oldIndexCount; ++i)
	////{
	////	int32 index;
	////	oldPolygonGroup->GetIndex(i, index);
	////	shadowPolygonGroup->SetIndex(i, index);
	////}

	//int32 i0, i1, i2;
	//Vector3 p0, p1, p2;
	//for(int32 i = 0; i < oldIndexCount; i += 3)
	//{
	//	oldPolygonGroup->GetIndex(i+0, i0);
	//	oldPolygonGroup->GetIndex(i+1, i1);
	//	oldPolygonGroup->GetIndex(i+2, i2);
	//	oldPolygonGroup->GetCoord(i0, p0);
	//	oldPolygonGroup->GetCoord(i1, p1);
	//	oldPolygonGroup->GetCoord(i2, p2);

	//	Vector3 v0 = p1 - p0;
	//	Vector3 v1 = p2 - p0;
	//	Vector3 normal = v0.CrossProduct(v1);
	//	normal.Normalize();

	//	shadowPolygonGroup->SetIndex(i+0, i+0);
	//	shadowPolygonGroup->SetIndex(i+1, i+1);
	//	shadowPolygonGroup->SetIndex(i+2, i+2);
	//	shadowPolygonGroup->SetCoord(i+0, p0);
	//	shadowPolygonGroup->SetCoord(i+1, p1);
	//	shadowPolygonGroup->SetCoord(i+2, p2);
	//	shadowPolygonGroup->SetNormal(i+0, normal);
	//	shadowPolygonGroup->SetNormal(i+1, normal);
	//	shadowPolygonGroup->SetNormal(i+2, normal);
	//}


	//EdgeAdjacency newAdjacency;
	//newAdjacency.InitFromPolygonGroup(shadowPolygonGroup, oldPolygonGroup->GetIndexCount());
	//Vector<EdgeAdjacency::Edge> & edges = newAdjacency.GetEdges();

	////copy old coord/normal data
	////int32 oldVertexCount = oldPolygonGroup->GetVertexCount();
	////for(int32 i = 0; i < oldVertexCount; ++i)
	////{
	////	Vector3 coord;
	////	oldPolygonGroup->GetCoord(i, coord);
	////	shadowPolygonGroup->SetCoord(i, coord);

	////	Vector3 normal;
	////	oldPolygonGroup->GetNormal(i, normal);
	////	shadowPolygonGroup->SetNormal(i, normal);
	////}



	////for(int32 i = 0; i < oldIndexCount; i += 3)
	////{
	////	Vector3 p1;// = vertices[indices[i+0]].position;
	////	Vector3 p2;// = vertices[indices[i+1]].position;
	////	Vector3 p3;// = vertices[indices[i+2]].position;
	////	int32 i1, i2, i3;
	////	shadowPolygonGroup->GetIndex(i+0, i1);
	////	shadowPolygonGroup->GetIndex(i+1, i2);
	////	shadowPolygonGroup->GetIndex(i+2, i3);
	////	shadowPolygonGroup->GetCoord(i1, p1);
	////	shadowPolygonGroup->GetCoord(i2, p2);
	////	shadowPolygonGroup->GetCoord(i3, p3);

	////	Vector3 v1 = p2 - p1;
	////	Vector3 v2 = p3 - p1;
	////	Vector3 normal = v1.CrossProduct(v2);
	////	normal.Normalize();

	////	shadowPolygonGroup->SetNormal(i1, normal);
	////	shadowPolygonGroup->SetNormal(i2, normal);
	////	shadowPolygonGroup->SetNormal(i3, normal);
	////}

	//Vector<EdgeAdjacency::Edge> nonShared;
	////generate degenerate quads
	//newIndexCount = oldIndexCount;
	//newVertexCount = oldIndexCount;
	//int32 edgesCount = edges.size();
	//for(int32 i = 0; i < edgesCount; ++i)
	//{
	//	const EdgeAdjacency::Edge & edge = edges[i];
	//	if(edge.sharedTriangles.size() == 2)
	//	{
	//		//indeces
	//		int32 i0[3];
	//		i0[0] = edge.sharedTriangles[0].i0;
	//		i0[1] = edge.sharedTriangles[0].i1;
	//		i0[2] = edge.sharedTriangles[0].i2;

	//		int32 i1[3];
	//		i1[0] = edge.sharedTriangles[1].i0;
	//		i1[1] = edge.sharedTriangles[1].i1;
	//		i1[2] = edge.sharedTriangles[1].i2;

	//		//normals
	//		Vector3 n0 = CalculateNormalForVertex(i0);
	//		Vector3 n1 = CalculateNormalForVertex(i1);

	//		//triangles
	//		int32 index0AtT0 = FindIndexInTriangleForPointInEdge(i0, 0, edge);
	//		int32 newI0T0 = DuplicateVertexAndSetNormalAtIndex(n0, index0AtT0);

	//		int32 index1AtT0 = FindIndexInTriangleForPointInEdge(i0, 1, edge);
	//		int32 newI1T0 = DuplicateVertexAndSetNormalAtIndex(n0, index1AtT0);

	//		int32 index0AtT1 = FindIndexInTriangleForPointInEdge(i1, 0, edge);
	//		int32 newI0T1 = DuplicateVertexAndSetNormalAtIndex(n1, index0AtT1);

	//		int32 index1AtT1 = FindIndexInTriangleForPointInEdge(i1, 1, edge);
	//		int32 newI1T1 = DuplicateVertexAndSetNormalAtIndex(n1, index1AtT1);

	//		//new triangles
	//		shadowPolygonGroup->SetIndex(newIndexCount++, newI0T1);
	//		shadowPolygonGroup->SetIndex(newIndexCount++, newI1T0);
	//		shadowPolygonGroup->SetIndex(newIndexCount++, newI0T0);

	//		shadowPolygonGroup->SetIndex(newIndexCount++, newI1T1);
	//		shadowPolygonGroup->SetIndex(newIndexCount++, newI1T0);
	//		shadowPolygonGroup->SetIndex(newIndexCount++, newI0T1);
	//	}
	//	else
	//	{
	//		nonShared.push_back(edge);
	//	}
	//}

	//int i = 0;
}


//int32 ShadowVolumeNode::DuplicateVertexAndSetNormalAtIndex(const Vector3 & normal, int32 index)
//{
//	Vector3 coord;
//	shadowPolygonGroup->GetCoord(index, coord);
//	shadowPolygonGroup->SetCoord(newVertexCount, coord);
//
//	shadowPolygonGroup->SetNormal(newVertexCount, normal);
//
//	newVertexCount++;
//
//	return newVertexCount-1;
//}
//
//Vector3 ShadowVolumeNode::CalculateNormalForVertex(int32 * originalTriangleVertices)
//{
//	Vector3 p1, p2, p3;
//	shadowPolygonGroup->GetCoord(originalTriangleVertices[0], p1);
//	shadowPolygonGroup->GetCoord(originalTriangleVertices[1], p2);
//	shadowPolygonGroup->GetCoord(originalTriangleVertices[2], p3);
//
//	Vector3 v1 = p2 - p1;
//	Vector3 v2 = p3 - p1;
//	Vector3 normal = v1.CrossProduct(v2);
//	normal.Normalize();
//
//	return normal;
//}
//
//int32 ShadowVolumeNode::FindIndexInTriangleForPointInEdge(int32 * triangleStartIndex, int32 pointInEdge, const EdgeAdjacency::Edge & edge)
//{
//	Vector3 coord = edge.points[pointInEdge];
//
//	Vector3 c0;
//	shadowPolygonGroup->GetCoord(triangleStartIndex[0], c0);
//	if(EdgeAdjacency::IsPointsEqual(c0, coord))
//	{
//		return triangleStartIndex[0];
//	}
//	else
//	{
//		Vector3 c1;
//		shadowPolygonGroup->GetCoord(triangleStartIndex[1], c1);
//		if(EdgeAdjacency::IsPointsEqual(c1, coord))
//		{
//			return triangleStartIndex[1];
//		}
//		else
//		{
//			Vector3 c2;
//			shadowPolygonGroup->GetCoord(triangleStartIndex[2], c2);
//			if(EdgeAdjacency::IsPointsEqual(c2, coord))
//			{
//				return triangleStartIndex[2];
//			}
//			else
//			{
//				DVASSERT(0);
//				return -1;
//			}
//		}
//	}
//}



};
