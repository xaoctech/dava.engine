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
#include "Scene3D/Scene.h"

#include "Render/Texture.h"
#include "Render/Material.h"
#include "Render/3D/StaticMesh.h"
#include "Render/3D/AnimatedMesh.h"
#include "Scene3D/SceneNodeAnimationList.h"
#include "Scene3D/SceneFile.h"
#include "Scene3D/SceneFileV2.h"
#include "Scene3D/DataNode.h"
#include "Scene3D/ProxyNode.h"
#include "Platform/SystemTimer.h"
#include "FileSystem/FileSystem.h"
#include "Scene3D/ShadowVolumeNode.h"
#include "Scene3D/ShadowRect.h"

namespace DAVA 
{
    
REGISTER_CLASS(Scene);
    
Scene::Scene()
	:   SceneNode(0)
    ,   currentCamera(0)
    ,   clipCamera(0)
    ,   forceLodLayer(-1)
{   
}

Scene::~Scene()
{
	for (Vector<AnimatedMesh*>::iterator t = animatedMeshes.begin(); t != animatedMeshes.end(); ++t)
	{
		AnimatedMesh * obj = *t;
		obj->Release();
	}
	animatedMeshes.clear();
	
	for (Vector<Camera*>::iterator t = cameras.begin(); t != cameras.end(); ++t)
	{
		Camera * obj = *t;
		obj->Release();
	}
	cameras.clear();
    
    SafeRelease(currentCamera);
    SafeRelease(clipCamera);
    
    for (Map<String, ProxyNode*>::iterator it = rootNodes.begin(); it != rootNodes.end(); ++it)
    {
        SafeRelease(it->second);
    }
    rootNodes.clear();
}

//int32 Scene::GetMaterialCount()
//{
//    //DataNode * materialsNode = dynamic_cast<DataNode*>(this->FindByName("materials"));
//    List<DataNode*> dataNodes;
//    GetDataNodes(dataNodes);
//    
//    int32 matCount = 0;
//    const List<DataNode*>::iterator & end = dataNodes.end(); 
//    for (List<DataNode*>::iterator it = dataNodes.begin(); it != end; ++it)
//    {
//        if (dynamic_cast<Material*>(*it))
//            matCount++;
//    }
//    return matCount;
//}
//
//Material * Scene::GetMaterial(int32 index)
//{
//    //DataNode * materialsNode = dynamic_cast<DataNode*>(this->FindByName("materials"));
//	return dynamic_cast<Material*>(materials->GetChild(index));
//}

//int32 Scene::GetStaticMeshCount()
//{
//    return (int32)staticMeshes->GetChildrenCount();
//}
//
//StaticMesh * Scene::GetStaticMesh(int32 index)
//{
//	return dynamic_cast<StaticMesh*>(staticMeshes->GetChild(index));
//}
    
void Scene::AddAnimatedMesh(AnimatedMesh * mesh)
{
	if (mesh)
	{
		mesh->Retain();
		animatedMeshes.push_back(mesh);
	}	
}

void Scene::RemoveAnimatedMesh(AnimatedMesh * mesh)
{
	
}

AnimatedMesh * Scene::GetAnimatedMesh(int32 index)
{
	return animatedMeshes[index];
}
	
void Scene::AddAnimation(SceneNodeAnimationList * animation)
{
	if (animation)
	{
		animation->Retain();
		animations.push_back(animation);
	}
}

SceneNodeAnimationList * Scene::GetAnimation(int32 index)
{
	return animations[index];
}
	
SceneNodeAnimationList * Scene::GetAnimation(const String & name)
{
	int32 size = (int32)animations.size();
	for (int32 k = 0; k < size; ++k)
	{
		SceneNodeAnimationList * node = animations[k];
		if (node->GetName() == name)
			return node;
	}
	return 0;
}
	
	
	
void Scene::AddCamera(Camera * camera)
{
	if (camera)
	{
		camera->Retain();
		cameras.push_back(camera);
	}
}

Camera * Scene::GetCamera(int32 n)
{
	if (n >= 0 && n < (int32)cameras.size())
		return cameras[n];
	
	return NULL;
}


void Scene::AddRootNode(SceneNode *node, const String &rootNodePath)
{
    ProxyNode * proxyNode = new ProxyNode(this);
    proxyNode->SetNode(node);
    
    rootNodes[rootNodePath] = proxyNode;
    proxyNode->SetName(rootNodePath);
}

SceneNode *Scene::GetRootNode(const String &rootNodePath)
{
//    ProxyNode * proxyNode = dynamic_cast<ProxyNode*>(scenes->FindByName(rootNodePath));
//    if (proxyNode)
//    {
//        return proxyNode->GetNode();
//    }
//    
//    String ext = FileSystem::Instance()->GetExtension(rootNodePath);
//    if(ext == ".sce")
//    {
//        SceneFile *file = new SceneFile();
//        file->SetDebugLog(true);
//        file->LoadScene(rootNodePath, this);
//        SafeRelease(file);
//    }
//    else if(ext == ".sc2")
//    {
//        SceneFileV2 *file = new SceneFileV2();
//        file->EnableDebugLog(true);
//        file->LoadScene(rootNodePath.c_str(), this);
//        SafeRelease(file);
//    }
//
//    proxyNode = dynamic_cast<ProxyNode*>(scenes->FindByName(rootNodePath));
//    if (proxyNode)
//    {
//        return proxyNode->GetNode();
//    }
//    return 0;
    
	Map<String, ProxyNode*>::const_iterator it;
	it = rootNodes.find(rootNodePath);
	if (it != rootNodes.end())
	{
        ProxyNode * node = it->second;
		return node->GetNode();
	}
    
    String ext = FileSystem::Instance()->GetExtension(rootNodePath);
    if(ext == ".sce")
    {
        SceneFile *file = new SceneFile();
        file->SetDebugLog(true);
        file->LoadScene(rootNodePath, this);
        SafeRelease(file);
    }
    else if(ext == ".sc2")
    {
        SceneFileV2 *file = new SceneFileV2();
        file->EnableDebugLog(true);
        file->LoadScene(rootNodePath.c_str(), this);
        SafeRelease(file);
    }
    
	it = rootNodes.find(rootNodePath);
	if (it != rootNodes.end())
	{
        ProxyNode * node = it->second;
		return node->GetNode();
	}
    return 0;
}

void Scene::ReleaseRootNode(const String &rootNodePath)
{
	Map<String, ProxyNode*>::iterator it;
	it = rootNodes.find(rootNodePath);
	if (it != rootNodes.end())
	{
        it->second->Release();
        rootNodes.erase(it);
	}
}
    
void Scene::ReleaseRootNode(SceneNode *nodeToRelease)
{
//	for (Map<String, SceneNode*>::iterator it = rootNodes.begin(); it != rootNodes.end(); ++it)
//	{
//        if (nodeToRelease == it->second) 
//        {
//            SceneNode * obj = it->second;
//            obj->Release();
//            rootNodes.erase(it);
//            return;
//        }
//	}
}

    
void Scene::SetupTestLighting()
{
#ifdef __DAVAENGINE_IPHONE__
//	glShadeModel(GL_SMOOTH);
//	// enable lighting
//	glEnable(GL_LIGHTING);
//	glEnable(GL_NORMALIZE);
//	
//	// deactivate all lights
//	for (int i=0; i<8; i++)  glDisable(GL_LIGHT0 + i);
//	
//	// ambiental light to nothing
//	GLfloat ambientalLight[]= {0.2f, 0.2f, 0.2f, 1.0f};
//	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambientalLight);
//	
////	GLfloat light_ambient[] = { 0.0f, 0.0f, 0.0f, 1.0f };  // delete
//	//GLfloat light_specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
//	GLfloat light_specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
//	
//	GLfloat light_diffuse[4];
//	light_diffuse[0]=1.0f;
//	light_diffuse[1]=1.0f;
//	light_diffuse[2]=1.0f;
//	light_diffuse[3]=1.0f;
//	
//	GLfloat lightPos[] = { 0.0f, 0.0f, 1.0f, 0.0f };
//	
//	// activate this light
//	glEnable(GL_LIGHT0);
//	
//	//always position 0,0,0 because light  is moved with transformations
//	glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
//	
//	// colors 
//	glLightfv(GL_LIGHT0, GL_AMBIENT, light_diffuse); // now like diffuse color
//	glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
//	glLightfv(GL_LIGHT0, GL_SPECULAR,light_specular);
//	
//	//specific values for this light
//	glLightf(GL_LIGHT0, GL_CONSTANT_ATTENUATION, 1);
//	glLightf(GL_LIGHT0, GL_LINEAR_ATTENUATION, 0);
//	glLightf(GL_LIGHT0, GL_QUADRATIC_ATTENUATION, 0);
//	
//	//other values
//	glLightf(GL_LIGHT0, GL_SPOT_CUTOFF, 30.0f);
//	glLightf(GL_LIGHT0, GL_SPOT_EXPONENT, 0.0f);
//	GLfloat spotdirection[] = { 0.0f, 0.0f, -1.0f, 0.0f }; // irrelevant for this light (I guess)
//	glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION, spotdirection); 
#endif
}
    
void Scene::Update(float timeElapsed)
{
    uint64 time = SystemTimer::Instance()->AbsoluteMS();
    
	int32 size = (int32)animations.size();
	for (int32 animationIndex = 0; animationIndex < size; ++animationIndex)
	{
		SceneNodeAnimationList * anim = animations[animationIndex];
		anim->Update(timeElapsed);
	}
	
	SceneNode::Update(timeElapsed);
	
	size = (int32)animatedMeshes.size();
	for (int32 animatedMeshIndex = 0; animatedMeshIndex < size; ++animatedMeshIndex)
	{
		AnimatedMesh * mesh = animatedMeshes[animatedMeshIndex];
		mesh->Update(timeElapsed);
	}
    
    updateTime = SystemTimer::Instance()->AbsoluteMS() - time;
}		

void Scene::Draw()
{
    nodeCounter = 0;
    uint64 time = SystemTimer::Instance()->AbsoluteMS();

	shadowVolumes.clear();
    
    RenderManager::Instance()->SetCullMode(FACE_BACK);
    RenderManager::Instance()->SetState(RenderStateBlock::DEFAULT_3D_STATE);
    RenderManager::Instance()->FlushState();
    RenderManager::Instance()->ClearDepthBuffer();
    

    
	SetupTestLighting();
	
    if (currentCamera)
    {
        currentCamera->Set();
    }

	SceneNode::Draw();

	if(shadowVolumes.size() > 0)
	{
		//2nd pass
		RenderManager::Instance()->RemoveState(RenderStateBlock::STATE_CULL);
		RenderManager::Instance()->RemoveState(RenderStateBlock::STATE_DEPTH_WRITE);
		RenderManager::Instance()->AppendState(RenderStateBlock::STATE_BLEND);
		RenderManager::Instance()->SetBlendMode(BLEND_ZERO, BLEND_ONE);

		RenderManager::Instance()->ClearStencilBuffer(0);
		RenderManager::Instance()->AppendState(RenderStateBlock::STATE_STENCIL_TEST);
		
		RenderManager::State()->SetStencilFunc(FACE_FRONT_AND_BACK, CMP_ALWAYS);
		RenderManager::State()->SetStencilRef(1);
		RenderManager::State()->SetStencilMask(0xFFFFFFFF);

		RenderManager::State()->SetStencilPass(FACE_FRONT, STENCILOP_KEEP);
		RenderManager::State()->SetStencilFail(FACE_FRONT, STENCILOP_KEEP);
		RenderManager::State()->SetStencilZFail(FACE_FRONT, STENCILOP_DECR_WRAP);

		RenderManager::State()->SetStencilPass(FACE_BACK, STENCILOP_KEEP);
		RenderManager::State()->SetStencilFail(FACE_BACK, STENCILOP_KEEP);
		RenderManager::State()->SetStencilZFail(FACE_BACK, STENCILOP_INCR_WRAP);
		
		RenderManager::Instance()->FlushState();
		Vector<ShadowVolumeNode*>::iterator itEnd = shadowVolumes.end();
		for(Vector<ShadowVolumeNode*>::iterator it = shadowVolumes.begin(); it != itEnd; ++it)
		{
			(*it)->DrawShadow();
		}

		//3rd pass
		RenderManager::Instance()->RemoveState(RenderStateBlock::STATE_CULL);
		RenderManager::Instance()->RemoveState(RenderStateBlock::STATE_DEPTH_TEST);
		
		RenderManager::State()->SetStencilRef(0);
		RenderManager::State()->SetStencilFunc(FACE_FRONT_AND_BACK, CMP_NOTEQUAL);
		RenderManager::State()->SetStencilPass(FACE_FRONT_AND_BACK, STENCILOP_KEEP);
		RenderManager::State()->SetStencilFail(FACE_FRONT_AND_BACK, STENCILOP_KEEP);
		RenderManager::State()->SetStencilZFail(FACE_FRONT_AND_BACK, STENCILOP_KEEP);

		RenderManager::Instance()->SetBlendMode(BLEND_SRC_ALPHA, BLEND_ONE_MINUS_SRC_ALPHA);
		ShadowRect::Instance()->Draw();

		RenderManager::Instance()->SetBlendMode(BLEND_ONE, BLEND_ONE_MINUS_SRC_ALPHA);
	}

	RenderManager::Instance()->SetState(RenderStateBlock::DEFAULT_2D_STATE_BLEND);
	drawTime = SystemTimer::Instance()->AbsoluteMS() - time;
}

	
void Scene::StopAllAnimations(bool recursive )
{
	int32 size = (int32)animations.size();
	for (int32 animationIndex = 0; animationIndex < size; ++animationIndex)
	{
		SceneNodeAnimationList * anim = animations[animationIndex];
		anim->StopAnimation();
	}
	SceneNode::StopAllAnimations(recursive);
}
    
    
void Scene::SetCurrentCamera(Camera * _camera)
{
    SafeRelease(currentCamera);
    currentCamera = SafeRetain(_camera);
    SafeRelease(clipCamera);
    clipCamera = SafeRetain(_camera);
}

Camera * Scene::GetCurrentCamera() const
{
    return currentCamera;
}

void Scene::SetClipCamera(Camera * _camera)
{
    SafeRelease(clipCamera);
    clipCamera = SafeRetain(_camera);
}

Camera * Scene::GetClipCamera() const
{
    return clipCamera;
}
 
void Scene::SetForceLodLayer(int32 layer)
{
    forceLodLayer = layer;
}
int32 Scene::GetForceLodLayer()
{
    return forceLodLayer;
}

int32 Scene::RegisterLodLayer(float32 nearDistance, float32 farDistance)
{
    LodLayer newLevel;
    newLevel.nearDistance = nearDistance;
    newLevel.farDistance = farDistance;
    newLevel.nearDistanceSq = nearDistance * nearDistance;
    newLevel.farDistanceSq = farDistance * farDistance;
    int i = 0;
    
    for (Vector<LodLayer>::iterator it = lodLayers.begin(); it < lodLayers.end(); it++)
    {
        if (nearDistance < it->nearDistance)
        {
            lodLayers.insert(it, newLevel);
            return i;
        }
        i++;
    }
    
    lodLayers.push_back(newLevel);
    return i;
}
    
void Scene::ReplaceLodLayer(int32 layerNum, float32 nearDistance, float32 farDistance)
{
    DVASSERT(layerNum < (int32)lodLayers.size());
    
    lodLayers[layerNum].nearDistance = nearDistance;
    lodLayers[layerNum].farDistance = farDistance;
    lodLayers[layerNum].nearDistanceSq = nearDistance * nearDistance;
    lodLayers[layerNum].farDistanceSq = farDistance * farDistance;
    
    
//    LodLayer newLevel;
//    newLevel.nearDistance = nearDistance;
//    newLevel.farDistance = farDistance;
//    newLevel.nearDistanceSq = nearDistance * nearDistance;
//    newLevel.farDistanceSq = farDistance * farDistance;
//    int i = 0;
//    
//    for (Vector<LodLayer>::iterator it = lodLayers.begin(); it < lodLayers.end(); it++)
//    {
//        if (nearDistance < it->nearDistance)
//        {
//            lodLayers.insert(it, newLevel);
//            return i;
//        }
//        i++;
//    }
//    
//    lodLayers.push_back(newLevel);
//    return i;
}
    
    

void Scene::AddDrawTimeShadowVolume(ShadowVolumeNode * shadowVolume)
{
	shadowVolumes.push_back(shadowVolume);
}

    
void Scene::UpdateLights()
{
    
    
    
    
}
    
LightNode * Scene::GetNearestLight(LightNode::eType type, Vector3 position)
{
    switch(type)
    {
        case LightNode::TYPE_DIRECTIONAL:
            
            break;
    };

	return 0;
}

/*void Scene::Save(KeyedArchive * archive)
{
    // Perform refactoring and add Matrix4, Vector4 types to VariantType and KeyedArchive
    SceneNode::Save(archive);
    
    
    
    
    
}

void Scene::Load(KeyedArchive * archive)
{
    SceneNode::Load(archive);
}*/
    



};




