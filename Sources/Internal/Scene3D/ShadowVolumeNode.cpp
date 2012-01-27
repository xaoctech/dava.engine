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
#include "Render/3D/EdgeAdjacency.h"


namespace DAVA
{

REGISTER_CLASS(ShadowVolumeNode);

DAVA::ShadowVolumeNode::ShadowVolumeNode()
{
	shader = new Shader();
	shader->LoadFromYaml("~res:/Shaders/ShadowVolume/shadowvolume_debug.shader");
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

	PolygonGroup * group = currentLod->meshes[0]->GetPolygonGroup(currentLod->polygonGroupIndexes[0]);

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


void ShadowVolumeNode::CopyGeometryFrom(MeshInstanceNode * meshInstance)
{
	//TODO: copy only geometry
	//AddPolygonGroup(meshInstance->GetMeshes()[0], meshInstance->GetPolygonGroupIndexes()[0], meshInstance->GetMaterials()[0]);

	PolygonGroup * oldPolygonGroup = meshInstance->GetMeshes()[0]->GetPolygonGroup(meshInstance->GetPolygonGroupIndexes()[0]);

	EdgeAdjacency adjacency;
	adjacency.InitFromPolygonGroup(oldPolygonGroup);

	const Vector<EdgeAdjacency::Edge> & edges = adjacency.GetEdges();
	int32 sharedEdgesCount = adjacency.GetEdgesWithTwoTrianglesCount();

	PolygonGroup * newPolygonGroup = new PolygonGroup(GetScene());
	newPolygonGroup->AllocateData(EVF_VERTEX | EVF_NORMAL, oldPolygonGroup->GetVertexCount(), oldPolygonGroup->GetIndexCount() + sharedEdgesCount*2*3);//*2 - two triangles per edge, *3 - 3 indeces per triangle

	//copy old coord data
	int32 oldVertexCount = oldPolygonGroup->GetVertexCount();
	for(int32 i = 0; i < oldVertexCount; ++i)
	{
		Vector3 coord;
		oldPolygonGroup->GetCoord(i, coord);
		newPolygonGroup->SetCoord(i, coord);
	}

	//generate normal data
	int32 oldIndexCount = oldPolygonGroup->GetIndexCount();
	for(int32 i = 0; i < oldIndexCount; i += 3)
	{
		Vector3 p1;
		oldPolygonGroup->GetCoord(i, p1);
		Vector3 p2;
		oldPolygonGroup->GetCoord(i, p2);
		Vector3 p3;
		oldPolygonGroup->GetCoord(i, p3);

		Vector3 v1 = p2 - p1;
		Vector3 v2 = p3 - p1;
		Vector3 normal = v1.CrossProduct(v2);
		normal.Normalize();

		newPolygonGroup->SetNormal(i, normal);
	}
}

};
