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

    Revision History:
        * Created by Vitaliy Borodovsky 
=====================================================================================*/

#include "Render/3D/StaticMesh.h"
#include "Scene3D/DataNode.h"
#include "Scene3D/Scene.h"
#include "Render/RenderManager.h"
#include "FileSystem/Logger.h"
#include "Render/Shader.h"
#include "Scene3D/SceneFileV2.h"

namespace DAVA 
{
    
REGISTER_CLASS(StaticMesh);

StaticMesh::StaticMesh(Scene * _scene)
	: DataNode(_scene)
{
    if (scene)
    {
        DataNode * staticMeshes = scene->GetStaticMeshes();
        staticMeshes->AddNode(this);
    }
}
    
void StaticMesh::SetScene(Scene * _scene)
{
    DVASSERT(scene == 0);
    scene = _scene;
    if (scene)
    {
        DataNode * staticMeshes = scene->GetStaticMeshes();
        staticMeshes->AddNode(this);
    }
}
    
int32 StaticMesh::Release()
{
    int32 retainCount = BaseObject::Release();
    if (retainCount == 1)
    {
        DataNode * staticMeshes = scene->GetStaticMeshes();
        staticMeshes->RemoveNode(this);
    }
    return retainCount;
}
	
StaticMesh::~StaticMesh()
{
}   
    
void StaticMesh::AddNode(DataNode * node)
{
    PolygonGroup * group = dynamic_cast<PolygonGroup*>(node);
    DVASSERT(group != 0);
    if (group)
    {
        DataNode::AddNode(group);
    }
}
	
uint32 StaticMesh::GetPolygonGroupCount()
{
    return (uint32)children.size();
}

PolygonGroup * StaticMesh::GetPolygonGroup(uint32 index)
{
    return reinterpret_cast<PolygonGroup*>(children[index]);
}
	

void StaticMesh::DrawPolygonGroup(int32 index, Material * material)
{
//    Matrix4 proj = RenderManager::Instance()->GetMatrix(RenderManager::MATRIX_PROJECTION);
//    Matrix4 glMatrixProj;
//    glGetFloatv(GL_PROJECTION_MATRIX, glMatrixProj.data);
//    
//    LOG_AS_MATRIX4(proj);
//    LOG_AS_MATRIX4(glMatrixProj);
//    
//    Matrix4 modelView = RenderManager::Instance()->GetMatrix(RenderManager::MATRIX_MODELVIEW);
//    Matrix4 glModelView;
//    glGetFloatv(GL_MODELVIEW_MATRIX, glModelView.data);
//    
//    LOG_AS_MATRIX4(modelView);
//    LOG_AS_MATRIX4(glModelView);
    
    
	PolygonGroup * group = reinterpret_cast<PolygonGroup*>(children[index]);

	RenderManager::Instance()->SetRenderData(group->renderDataObject);
	material->Bind();
	if (group->renderDataObject->GetIndexBufferID() != 0)
	{
		RenderManager::Instance()->HWDrawElements(PRIMITIVETYPE_TRIANGLELIST, group->indexCount, EIF_16, 0);
	}else
	{
		RenderManager::Instance()->HWDrawElements(PRIMITIVETYPE_TRIANGLELIST, group->indexCount, EIF_16, group->indexArray);
	}

	RenderManager::Instance()->SetTexture(0, 1); 
	RenderManager::Instance()->SetState(RenderStateBlock::DEFAULT_3D_STATE);
}



};