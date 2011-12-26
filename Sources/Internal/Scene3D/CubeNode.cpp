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
#include "Scene3D/CubeNode.h"
#include "Render/3D/PolygonGroup.h"
#include "Render/3D/StaticMesh.h"
#include "Render/RenderManager.h"
#include "Render/RenderHelper.h"

namespace DAVA 
{
CubeNode::CubeNode(Scene * _scene)
:	MeshInstanceNode(_scene)
{
    cubeMesh = NULL;
}
	
CubeNode::~CubeNode()
{
    SafeRelease(cubeMesh);
}

void CubeNode::Update(float32 timeElapsed)
{
    MeshInstanceNode::Update(timeElapsed);
}
    
void CubeNode::Draw()
{
    MeshInstanceNode::Draw();
}

SceneNode* CubeNode::Clone(SceneNode *dstNode)
{
    if (!dstNode) 
    {
        dstNode = new CubeNode(scene);
    }

    MeshInstanceNode::Clone(dstNode);
    
    ((CubeNode*)dstNode)->cubeMesh = (StaticMesh *)cubeMesh->Clone();


    return dstNode;
}

void CubeNode::CreateCube(Vector3 size, Color c)
{
    RGBColor color(c.r * 255, c.g * 255, c.b * 255, c.a * 255);
    
    Vector3 halfSize = size / 2;
    float vertices[] = 
    {
		-halfSize.x,    -halfSize.y,   -halfSize.z,
		halfSize.x,     -halfSize.y,   -halfSize.z,
		halfSize.x,     halfSize.y,    -halfSize.z,
		-halfSize.x,    halfSize.y,    -halfSize.z,
		-halfSize.x,    -halfSize.y,   halfSize.z,
		halfSize.x,     -halfSize.y,   halfSize.z,
		halfSize.x,     halfSize.y,    halfSize.z,
		-halfSize.x,    halfSize.y,    halfSize.z,
	};

	int32 indices[] = 
	{
		0, 4, 5,    0, 5, 1,
		1, 5, 6,    1, 6, 2,
		2, 6, 7,    2, 7, 3,
		3, 7, 4,    3, 4, 0,
		4, 7, 6,    4, 6, 5,
		3, 0, 1,    3, 1, 2
	};
	
	SafeRelease(cubeMesh);
    cubeMesh = new StaticMesh(GetScene());
    cubeMesh->Create(1);
    
    PolygonGroup * cube = cubeMesh->GetPolygonGroup(0);
//	cube->AllocateData( EVF_VERTEX | EVF_COLOR, 12, 36, 0);  //почему 12?
    cube->AllocateData( EVF_VERTEX | EVF_COLOR, 8, 36, 0); 
    
	for (int32 i = 0; i < 8 ; ++i)
	{
		cube->SetCoord(i, Vector3(vertices[i * 3 + 0], vertices[i * 3 + 1], vertices[i * 3 + 2]));
		cube->SetColor(i, color);
	}
	for (int32 i = 0; i < 36; ++i)
	{				   
		cube->SetIndex(i, indices[i]);
	}

    AddPolygonGroup(cubeMesh, 0, NULL);
}

};
