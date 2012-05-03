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
#include "Platform/SystemTimer.h"

namespace DAVA
{

REGISTER_CLASS(ImposterNode);

ImposterNode::ImposterNode(Scene * scene)
:	SceneNode(scene)
{
	state = STATE_3D;
	renderData = 0;
	fbo = 0;
	manager = 0;
	isReady = false;
	RegisterInScene();
}

ImposterNode::~ImposterNode()
{
	UnregisterInScene();
	SafeRelease(fbo);
	SafeRelease(renderData);
}

void ImposterNode::UpdateState()
{
	if(GetChildrenCount() > 0)
	{
		DVASSERT(GetChildrenCount() == 1);
		AABBox3 bbox = GetChild(0)->GetWTMaximumBoundingBox();
		Vector3 bboxCenter = bbox.GetCenter();
		float32 distanceSquare = (scene->GetCurrentCamera()->GetPosition() - bboxCenter).SquareLength();
		distanceSquare *= scene->GetCurrentCamera()->GetZoomFactor() * scene->GetCurrentCamera()->GetZoomFactor();

		if((STATE_3D == state))
		{
			if(distanceSquare > 900)
			{
				AskForRedraw();
			}
			else
			{
				isReady = false;
			}
		}

		if(STATE_IMPOSTER == state)
		{
			if(distanceSquare < 900)
			{
				isReady = false;
				state = STATE_3D;
			}
			else
			{
				Vector3 newDirection = scene->GetCurrentCamera()->GetPosition()-center;
				newDirection.Normalize();
				if(newDirection.DotProduct(direction) < 0.996f)
				{
					AskForRedraw();
				}
			}
		}
	}
}

void ImposterNode::Draw()
{

}

void ImposterNode::GeneralDraw()
{
	if(IsRedrawApproved())
	{
		UpdateImposter();
	}

	if(IsImposterReady())
	{
		DrawImposter();
	}
	else
	{
		SceneNode::Draw();
	}
}

void ImposterNode::UpdateImposter()
{
	Camera * camera = scene->GetCurrentCamera();
	Camera * imposterCamera = new Camera();
	Vector3 cameraPos = camera->GetPosition();

	SceneNode * child = GetChild(0);
	AABBox3 bbox = child->GetWTMaximumBoundingBox();
	Vector3 bboxVertices[8];
	Vector3 screenVertices[8];
	bbox.GetCorners(bboxVertices);
	Vector3 bboxCenter = bbox.GetCenter();

	imposterCamera->Setup(90.f, 1.33f, 1.f, 1000.f);
	imposterCamera->SetTarget(bbox.GetCenter());
	imposterCamera->SetPosition(cameraPos);
	imposterCamera->SetUp(camera->GetUp());
	imposterCamera->SetLeft(camera->GetLeft());
	imposterCamera->Set();


	Rect viewport = RenderManager::Instance()->GetViewport();
	

	Matrix4 mvp = imposterCamera->GetUniformProjModelMatrix();

	AABBox3 screenBounds;
	for(int32 i = 0; i < 8; ++i)
	{
		//project
		Vector4 pv(bboxVertices[i]);
		pv = pv*mvp;
		pv.x = (viewport.dx * 0.5f) * (1.f + pv.x/pv.w)+viewport.x;
		pv.y = (viewport.dy * 0.5f) * (1.f + pv.y/pv.w)+viewport.y;
		pv.z = (pv.z/pv.w + 1.f) * 0.5f;

		screenVertices[i] = Vector3(pv.x, pv.y, pv.z);
		screenBounds.AddPoint(screenVertices[i]);
	}

	Vector4 pv(bboxCenter);
	pv = pv*mvp;
	pv.z = (pv.z/pv.w + 1.f) * 0.5f;
	float32 bboxCenterZ = pv.z;

	Vector2 screenSize = Vector2(screenBounds.max.x-screenBounds.min.x, screenBounds.max.y-screenBounds.min.y);

	Vector3 screenBillboardVertices[4];
	screenBillboardVertices[0] = Vector3(screenBounds.min.x, screenBounds.min.y, screenBounds.min.z);
	screenBillboardVertices[1] = Vector3(screenBounds.max.x, screenBounds.min.y, screenBounds.min.z);
	screenBillboardVertices[2] = Vector3(screenBounds.min.x, screenBounds.max.y, screenBounds.min.z);
	screenBillboardVertices[3] = Vector3(screenBounds.max.x, screenBounds.max.y, screenBounds.min.z);

	center = Vector3();
	Matrix4 invMvp = mvp;
	invMvp.Inverse();
	for(int32 i = 0; i < 4; ++i)
	{
		//unproject
		Vector4 out;
		out.x = 2.f*(screenBillboardVertices[i].x-viewport.x)/viewport.dx-1.f;
		out.y = 2.f*(screenBillboardVertices[i].y-viewport.y)/viewport.dy-1.f;
		out.z = 2.f*screenBillboardVertices[i].z-1.f;
		out.w = 1.f;

		out = out*invMvp;
		DVASSERT(out.w != 0.f);

		out.x /= out.w;
		out.y /= out.w;
		out.z /= out.w;

		imposterVertices[i] = Vector3(out.x, out.y, out.z);
		center += imposterVertices[i];
	}
	center /= 4.f;

	//draw
	RecreateFbo(screenSize);

	direction = camera->GetPosition()-center;
	direction.Normalize();

	float32 nearPlane = (center-cameraPos).Length();
	float32 farPlane = nearPlane + (bbox.max.z-bbox.min.z);
	float32 w = (imposterVertices[1]-imposterVertices[0]).Length();
	float32 h = (imposterVertices[2]-imposterVertices[0]).Length();
	imposterCamera->Setup(-w/2.f, w/2.f, -h/2.f, h/2.f, nearPlane, nearPlane+50.f);

	Rect oldViewport = RenderManager::Instance()->GetViewport();

	RenderManager::Instance()->SetRenderTarget(fbo);
	imposterCamera->SetTarget(center);
	imposterCamera->Set();
	RenderManager::Instance()->ClearWithColor(1.f, 1.f, 1.f, 0.f);
	RenderManager::Instance()->ClearDepthBuffer();

	child->RemoveFlag(NODE_CLIPPED_THIS_FRAME);
	child->Draw();

#ifdef __DAVAENGINE_OPENGL__
	BindFBO(RenderManager::Instance()->fboViewFramebuffer);
#endif

	RenderManager::Instance()->SetViewport(oldViewport, true);
		
	isReady = true;
	state = STATE_IMPOSTER;

	//unproject
	screenBillboardVertices[0] = Vector3(screenBounds.min.x, screenBounds.min.y, bboxCenterZ);
	screenBillboardVertices[1] = Vector3(screenBounds.max.x, screenBounds.min.y, bboxCenterZ);
	screenBillboardVertices[2] = Vector3(screenBounds.min.x, screenBounds.max.y, bboxCenterZ);
	screenBillboardVertices[3] = Vector3(screenBounds.max.x, screenBounds.max.y, bboxCenterZ);
	for(int32 i = 0; i < 4; ++i)
	{
		//unproject
		Vector4 out;
		out.x = 2.f*(screenBillboardVertices[i].x-viewport.x)/viewport.dx-1.f;
		out.y = 2.f*(screenBillboardVertices[i].y-viewport.y)/viewport.dy-1.f;
		out.z = 2.f*screenBillboardVertices[i].z-1.f;
		out.w = 1.f;

		out = out*invMvp;
		DVASSERT(out.w != 0.f);

		out.x /= out.w;
		out.y /= out.w;
		out.z /= out.w;

		imposterVertices[i] = Vector3(out.x, out.y, out.z);
	}

	camera->Set();
	SafeRelease(imposterCamera);

	ClearGeometry();
	CreateGeometry();
}

void ImposterNode::DrawImposter()
{
	Matrix4 modelViewMatrix = RenderManager::Instance()->GetMatrix(RenderManager::MATRIX_MODELVIEW); 
	const Matrix4 & cameraMatrix = scene->GetCurrentCamera()->GetMatrix();
	Matrix4 meshFinalMatrix;

	meshFinalMatrix = /*worldTransform **/ cameraMatrix;

	RenderManager::Instance()->SetMatrix(RenderManager::MATRIX_MODELVIEW, meshFinalMatrix);
	RenderManager::Instance()->SetRenderEffect(RenderManager::TEXTURE_MUL_FLAT_COLOR_ALPHA_TEST);

	RenderManager::Instance()->SetColor(1.f, 1.f, 1.f, 1.f);

	//RenderManager::Instance()->SetState(/*RenderStateBlock::STATE_BLEND |*/ RenderStateBlock::STATE_TEXTURE0/* | RenderStateBlock::STATE_CULL*/);
	RenderManager::Instance()->RemoveState(RenderStateBlock::STATE_CULL);
	//RenderManager::Instance()->RemoveState(RenderStateBlock::STATE_DEPTH_WRITE);
	//RenderManager::Instance()->AppendState(RenderStateBlock::STATE_BLEND);
	eBlendMode src = RenderManager::Instance()->GetSrcBlend();
	eBlendMode dst = RenderManager::Instance()->GetDestBlend();
	RenderManager::Instance()->SetBlendMode(BLEND_SRC_ALPHA, BLEND_ONE_MINUS_SRC_ALPHA);

	RenderManager::Instance()->SetTexture(fbo);

	RenderManager::Instance()->SetRenderData(renderData);

	RenderManager::Instance()->FlushState();
	RenderManager::Instance()->DrawArrays(PRIMITIVETYPE_TRIANGLESTRIP, 0, 4);

	//RenderManager::Instance()->AppendState(RenderStateBlock::STATE_DEPTH_WRITE);
	RenderManager::Instance()->SetState(RenderStateBlock::DEFAULT_3D_STATE);
	RenderManager::Instance()->SetBlendMode(src, dst);

	RenderManager::Instance()->SetMatrix(RenderManager::MATRIX_MODELVIEW, modelViewMatrix);
}

SceneNode* ImposterNode::Clone(SceneNode *dstNode /*= NULL*/)
{
	if (!dstNode) 
	{
		dstNode = new ImposterNode(scene);
	}

	SceneNode::Clone(dstNode);

	return dstNode;
}

void ImposterNode::ClearGeometry()
{
	SafeRelease(renderData);
	verts.clear();
	texCoords.clear();
}

void ImposterNode::CreateGeometry()
{
	DVASSERT(verts.empty() && texCoords.empty() && !renderData);

	verts.push_back(imposterVertices[0].x);
	verts.push_back(imposterVertices[0].y);
	verts.push_back(imposterVertices[0].z);

	verts.push_back(imposterVertices[1].x);
	verts.push_back(imposterVertices[1].y);
	verts.push_back(imposterVertices[1].z);

	verts.push_back(imposterVertices[2].x);
	verts.push_back(imposterVertices[2].y);
	verts.push_back(imposterVertices[2].z);

	verts.push_back(imposterVertices[3].x);
	verts.push_back(imposterVertices[3].y);
	verts.push_back(imposterVertices[3].z);

	texCoords.push_back(0);
	texCoords.push_back(0);

	texCoords.push_back(1);
	texCoords.push_back(0);

	texCoords.push_back(0);
	texCoords.push_back(1);

	texCoords.push_back(1);
	texCoords.push_back(1);

	renderData = new RenderDataObject();
	renderData->SetStream(EVF_VERTEX, TYPE_FLOAT, 3, 0, &(verts[0]));
	renderData->SetStream(EVF_TEXCOORD0, TYPE_FLOAT, 2, 0, &(texCoords[0]));
}

void ImposterNode::SetScene(Scene * _scene)
{
	SceneNode::SetScene(_scene);
	RegisterInScene();
}

void ImposterNode::RegisterInScene()
{
	if(scene)
	{
		scene->RegisterImposter(this);
	}
}

void ImposterNode::UnregisterInScene()
{
	scene->UnregisterImposter(this);
}

void ImposterNode::AskForRedraw()
{
	state = STATE_ASK_FOR_REDRAW;
}

bool ImposterNode::IsRedrawApproved()
{
	return (STATE_REDRAW_APPROVED == state);
}

bool ImposterNode::IsImposterReady()
{
	return isReady;
}

void ImposterNode::OnAddedToQueue()
{
	state = STATE_QUEUED;
}

bool ImposterNode::IsAskingForRedraw()
{
	return (STATE_ASK_FOR_REDRAW == state);
}

void ImposterNode::ApproveRedraw()
{
	state = STATE_REDRAW_APPROVED;
}

void ImposterNode::RecreateFbo(const Vector2 & size)
{
	SafeRelease(fbo);
	fbo = Texture::CreateFBO((uint32)(size.x*1.2f), (uint32)(size.y*1.2f), FORMAT_RGBA4444, Texture::DEPTH_RENDERBUFFER);
}

bool ImposterNode::IsQueued()
{
	return (STATE_QUEUED == state);
}


}
