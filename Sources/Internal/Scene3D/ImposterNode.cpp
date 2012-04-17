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

#include "ImposterNode.h"
#include "Utils/Utils.h"
#include "Render/Image.h"

namespace DAVA
{

REGISTER_CLASS(ImposterNode);

ImposterNode::ImposterNode(Scene * scene)
:	SceneNode(scene)
{
	//renderData = new RenderDataObject();
	//renderData->SetStream(EVF_VERTEX, TYPE_FLOAT, 3, 0, &verts.front());

	fbo = Texture::CreateFBO(512, 512, FORMAT_RGBA8888, Texture::DEPTH_RENDERBUFFER);
}

ImposterNode::~ImposterNode()
{
	SafeRelease(fbo);
	//SafeRelease(renderData);
}

void ImposterNode::Update(float32 timeElapsed)
{

}

void ImposterNode::Draw()
{
	if(GetChildrenCount() > 0)
	{
		DVASSERT(GetChildrenCount() == 1);
		float32 dst = (scene->GetCurrentCamera()->GetPosition() - GetWorldTransform().GetTranslationVector()).SquareLength();
		dst *= scene->GetCurrentCamera()->GetZoomFactor() * scene->GetCurrentCamera()->GetZoomFactor();

		if(dst > 300.f)
		{
			//switch to imposter
			UpdateImposter();
		}
		else
		{
			SceneNode::Draw();
		}
	}
}

void ImposterNode::UpdateImposter()
{
	SceneNode * child = GetChild(0);
	AABBox3 bbox = child->GetWTMaximumBoundingBox();
	Vector3 bboxVertices[8];
	Vector3 screenVertices[8];
	bbox.GetCorners(bboxVertices);

	Rect viewport = Rect(/*310*/0, /*61*/0, 1180, 829);//RenderManager::Instance()->GetViewport();
	Camera * camera = scene->GetCurrentCamera();

	Matrix4 mvp = camera->GetUniformProjModelMatrix();

	AABBox3 screenBounds;
	for(int32 i = 0; i < 8; ++i)
	{
		//project
		Vector4 pv(bboxVertices[i]);
		pv = pv*mvp;
		pv.x = (viewport.dx * 0.5f) * (1.f + pv.x/pv.w) + viewport.x;
		pv.y = (viewport.dy * 0.5f) * (1.f + pv.y/pv.w) + viewport.y;
		pv.z = (pv.z/pv.w + 1.f) * 0.5f;

		screenVertices[i] = Vector3(pv.x, pv.y, pv.z);
		screenBounds.AddPoint(screenVertices[i]);
	}

	Vector3 screenBillboardVertices[4];
	screenBillboardVertices[0] = Vector3(screenBounds.min.x, screenBounds.min.y, screenBounds.min.z);
	screenBillboardVertices[1] = Vector3(screenBounds.max.x, screenBounds.min.y, screenBounds.min.z);
	screenBillboardVertices[2] = Vector3(screenBounds.max.x, screenBounds.max.y, screenBounds.min.z);
	screenBillboardVertices[3] = Vector3(screenBounds.min.x, screenBounds.max.y, screenBounds.min.z);

	center = Vector3();
	for(int32 i = 0; i < 4; ++i)
	{
		imposterVertices[i] = camera->UnProject(screenBillboardVertices[i].x, screenBillboardVertices[i].y, screenBillboardVertices[i].z, viewport);
		center += imposterVertices[i];
	}
	center /= 4.f;

	direction = camera->GetPosition()-center;
	direction.Normalize();

	Camera * imposterCamera = new Camera();
	float32 nearPlane = (center-camera->GetPosition()).Length();
	float32 farPlane = nearPlane + (bbox.max.z-bbox.min.z);
	float32 w = (imposterVertices[1]-imposterVertices[0]).Length();
	float32 h = (imposterVertices[3]-imposterVertices[0]).Length();
	imposterCamera->Setup(-w/2.f, w/2.f, -h/2.f, h/2.f, nearPlane, farPlane);
	//imposterCamera->Setup(90, 1.33f, 1, 1000);
	imposterCamera->SetTarget(center);
	imposterCamera->SetPosition(camera->GetPosition());
	
	imposterCamera->Set();
	BindFBO(fbo->fboID);
	Rect oldViewport = RenderManager::Instance()->GetViewport();
	RenderManager::Instance()->SetViewport(Rect(0, 0, 512, 512), true);
	RenderManager::Instance()->ClearWithColor(1.f, 0.f, 0.f, 1.f);
	RenderManager::Instance()->ClearDepthBuffer();

	child->Draw();

	BindFBO(RenderManager::Instance()->fboViewFramebuffer);
	Image * img = fbo->CreateImageFromMemory();
	img->Save("imposter.png");
	RenderManager::Instance()->SetViewport(oldViewport, true);
	camera->Set();
	
	SafeRelease(imposterCamera);

}



}
