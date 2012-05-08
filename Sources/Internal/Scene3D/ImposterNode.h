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

#ifndef __IMPOSTER_NODE_H__
#define __IMPOSTER_NODE_H__

#include "Scene3D/Scene.h"
#include "Render/RenderDataObject.h"

namespace DAVA
{

class ImposterManager;

class ImposterNode : public SceneNode
{
public:
	enum eState
	{
		STATE_3D = 0,
		STATE_IMPOSTER,
		STATE_ASK_FOR_REDRAW,
		STATE_QUEUED,
		STATE_REDRAW_APPROVED
	};

	ImposterNode(Scene * scene = 0);
	virtual ~ImposterNode();

	void UpdateState();
	virtual void Draw();
	virtual SceneNode* Clone(SceneNode *dstNode = NULL);
	virtual void SetScene(Scene * _scene);

	void UpdateImposter();
	void GeneralDraw();
	void DrawImposter();

	bool IsAskingForRedraw();
	bool IsQueued();
	void OnAddedToQueue();
	void ApproveRedraw();

private:
	void AskForRedraw();
	void ClearGeometry();
	void CreateGeometry();
	void RegisterInScene();
	void UnregisterInScene();
	
	bool IsRedrawApproved();
	bool IsImposterReady();

	bool isReady;

	Vector3 imposterVertices[4];
	RenderDataObject * renderData;
	Texture * fbo;

	Vector<float32> verts;
	Vector<float32> texCoords;
	Vector3 center;
	Vector3 direction;

	eState state;

	ImposterManager * manager;

	void RecreateFbo(const Vector2 & size);

	void HierarchicalRemoveCull(SceneNode * node);
};

};

#endif //__IMPOSTER_NODE_H__
