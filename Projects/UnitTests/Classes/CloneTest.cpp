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
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTR ACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

    Revision History:
        * Created by Ivan "Dizz" Petrochenko
=====================================================================================*/

#include "CloneTest.h"

CloneTest::CloneTest()
: TestTemplate<CloneTest>("CloneTest")
{
    RegisterFunction(this, &CloneTest::CloneOneType, "CloneOneType", NULL);
    RegisterFunction(this, &CloneTest::CloneTwoTypes, "CloneTwoTypes", NULL);
}

void CloneTest::LoadResources()
{
    GetBackground()->SetColor(Color(0.0f, 1.0f, 0.0f, 1.0f));
}


void CloneTest::UnloadResources()
{
    RemoveAllControls();
}

template <class Node>
bool CloneNode()
{
    Node *srcNode = new Node();
    srcNode->SetName("Tested Node");
    
    Node *dstNullNode = dynamic_cast<Node *>(srcNode->Clone());
    
    Node *dstNode = NULL;
    dstNode = dynamic_cast<Node *>(srcNode->Clone(dstNode));
    
	if(dstNode && dstNullNode)
	{
		bool compareResult = (dstNode->GetName() == dstNullNode->GetName());
		SafeRelease(srcNode);
		SafeRelease(dstNode);
		SafeRelease(dstNullNode);
		
		return compareResult;
	}

    return false;
}

void CloneTest::CloneOneType(PerfFuncData * data)
{
//    TEST_VERIFY(CloneNode<BillboardNode>());
//    TEST_VERIFY(CloneNode<BoneNode>());
//    TEST_VERIFY(CloneNode<BVNode>());
    
    TEST_VERIFY(CloneNode<Camera>());
    
    TEST_VERIFY(CloneNode<ImposterNode>());
//    TEST_VERIFY(CloneNode<LandscapeNode>()); // Do wee need Clone() ?
    TEST_VERIFY(CloneNode<LightNode>());
    TEST_VERIFY(CloneNode<LodNode>());
    TEST_VERIFY(CloneNode<MeshInstanceNode>());
    TEST_VERIFY(CloneNode<ParticleEffectNode>());
    TEST_VERIFY(CloneNode<ParticleEmitterNode>());
    TEST_VERIFY(CloneNode<ReferenceNode>());
    
//    TEST_VERIFY(CloneNode<Scene>()); //Do we need Clone() ?
    TEST_VERIFY(CloneNode<SceneNode>());
//    TEST_VERIFY(CloneNode<SceneNodeAnimationList>()); //Do we need Clone() ?
    TEST_VERIFY(CloneNode<ShadowVolumeNode>());
//    TEST_VERIFY(CloneNode<SkeletonNode>()); //Do we need Clone() ?
    
//    TEST_VERIFY(CloneNode<SphereNode>());
    
//    TEST_VERIFY(CloneNode<SpriteNode>()); //Do we need Clone()?
    TEST_VERIFY(CloneNode<UserNode>());
}


template <class SrcNode, class DestNode>
bool CloneDifferentNodes()
{
    SrcNode *srcNode = new SrcNode();
    srcNode->SetName("Tested Node");
    
    DestNode *dstNullNode = dynamic_cast<DestNode *>(srcNode->Clone());
    
    DestNode *dstNode = NULL;
    dstNode = dynamic_cast<DestNode *>(srcNode->Clone(dstNode));
    
    
    if(dstNode && dstNullNode)
    {
        bool compareResult = (dstNode->GetName() == dstNullNode->GetName());
        SafeRelease(srcNode);
        SafeRelease(dstNode);
        SafeRelease(dstNullNode);
        return compareResult;
    }
    
    return false;
}

void CloneTest::CloneTwoTypes(PerfFuncData * data)
{
    bool ret = CloneDifferentNodes<SceneNode, Camera>();
    TEST_VERIFY(ret == false);
    
    ret = CloneDifferentNodes<SceneNode, ImposterNode>();
    TEST_VERIFY(ret == false);
    
//    ret = CloneDifferentNodes<SceneNode, LandscapeNode>();
//    TEST_VERIFY(ret == false);

    ret = CloneDifferentNodes<SceneNode, LightNode>();
    TEST_VERIFY(ret == false);

    ret = CloneDifferentNodes<SceneNode, LodNode>();
    TEST_VERIFY(ret == false);

    ret = CloneDifferentNodes<SceneNode, MeshInstanceNode>();
    TEST_VERIFY(ret == false);

    ret = CloneDifferentNodes<SceneNode, ParticleEffectNode>();
    TEST_VERIFY(ret == false);

    ret = CloneDifferentNodes<SceneNode, ParticleEmitterNode>();
    TEST_VERIFY(ret == false);

    ret = CloneDifferentNodes<SceneNode, ReferenceNode>();
    TEST_VERIFY(ret == false);
    
//    ret = CloneDifferentNodes<SceneNode, Scene>();
//    TEST_VERIFY(ret == false);
    
//    ret = CloneDifferentNodes<SceneNode, SceneNodeAnimationList>();
//    TEST_VERIFY(ret == false);

    ret = CloneDifferentNodes<SceneNode, ShadowVolumeNode>();
    TEST_VERIFY(ret == false);

//    ret = CloneDifferentNodes<SceneNode, SkeletonNode>();
//    TEST_VERIFY(ret == false);

//    ret = CloneDifferentNodes<SceneNode, SpriteNode>();
//    TEST_VERIFY(ret == false);

    ret = CloneDifferentNodes<SceneNode, UserNode>();
    TEST_VERIFY(ret == false);
}

