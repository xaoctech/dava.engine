/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/


#include "SpeedTreeUpdateSystem.h"
#include "Scene3D/Entity.h"
#include "Scene3D/Components/ComponentHelpers.h"
#include "Scene3D/Components/RenderComponent.h"
#include "Scene3D/Components/TransformComponent.h"
#include "Scene3D/Components/SpeedTreeComponent.h"
#include "Scene3D/Systems/WindSystem.h"
#include "Scene3D/Systems/EventSystem.h"
#include "Scene3D/Scene.h"
#include "Render/Highlevel/SpeedTreeObject.h"
#include "Utils/Random.h"
#include "Math/Math2D.h"

namespace DAVA
{

SpeedTreeUpdateSystem::SpeedTreeUpdateSystem(Scene * scene)
:	SceneSystem(scene)
{
    RenderOptions * options = RenderManager::Instance()->GetOptions();
    options->AddObserver(this);
    isAnimationEnabled = options->IsOptionEnabled(RenderOptions::SPEEDTREE_ANIMATIONS);

	scene->GetEventSystem()->RegisterSystemForEvent(this, EventSystem::WORLD_TRANSFORM_CHANGED);
}

SpeedTreeUpdateSystem::~SpeedTreeUpdateSystem()
{
    RenderOptions * options = RenderManager::Instance()->GetOptions();
    options->RemoveObserver(this);
}

SpeedTreeUpdateSystem::TreeInfo::TreeInfo(Entity * _treeEntity)
{
    treeEntity = _treeEntity;

    Random * rnd = Random::Instance();
    SpeedTreeComponent * sc = GetSpeedTreeComponent(treeEntity);
    float32 spring = sc->GetTrunkOscillationSpringSqrt();
    spring *= spring;

#define BASE_OSCILLATION_VALUE (float32)((rnd->RandFloat() * 2.f - 1.f) * spring)

    leafTime = (float32)rnd->RandFloat(1000.f);
    oscVelocity = Vector3(BASE_OSCILLATION_VALUE, BASE_OSCILLATION_VALUE, BASE_OSCILLATION_VALUE);
    oscOffset = oscVelocity;

#undef BASE_OSCILLATION_VALUE

    Matrix4 wtMx = GetTransformComponent(treeEntity)->GetWorldTransform();
    wtPosition = wtMx.GetTranslationVector();
    wtMx.GetInverse(wtInvMx);
}

void SpeedTreeUpdateSystem::ImmediateEvent(Entity * entity, uint32 event)
{
	if(event == EventSystem::WORLD_TRANSFORM_CHANGED)
	{
        if(GetSpeedTreeComponent(entity))
        {
            uint32 treeCount = allTrees.size();
            for(uint32 i = 0; i < treeCount; ++i)
            {
                TreeInfo * info = allTrees[i];
                if(info->treeEntity == entity)
                {
                    Matrix4 wtMx = GetTransformComponent(entity)->GetWorldTransform();
                    info->wtPosition = wtMx.GetTranslationVector();
                    wtMx.GetInverse(info->wtInvMx);
                }
            }
        }
	}
}

void SpeedTreeUpdateSystem::AddEntity(Entity * entity)
{
    DVASSERT(GetSpeedTreeComponent(entity));

    SpeedTreeObject * treeObject = DynamicTypeCheck<SpeedTreeObject*>(GetRenderObject(entity));
    DVASSERT(treeObject);
    treeObject->SetAnimationFlag(isAnimationEnabled);

    TreeInfo * treeInfo = new TreeInfo(entity);
    allTrees.push_back(treeInfo);
}

void SpeedTreeUpdateSystem::RemoveEntity(Entity * entity)
{
    Vector<TreeInfo *>::iterator it = allTrees.begin();
    Vector<TreeInfo *>::iterator itEnd = allTrees.end();
    for(; it != itEnd; ++it)
    {
        TreeInfo * info = *it;
        if(info->treeEntity == entity)
        {
            SafeDelete(info);
            allTrees.erase(it);
            break;
        }
    }
}

void SpeedTreeUpdateSystem::Process(float32 timeElapsed)
{
    if(!isAnimationEnabled)
        return;
    
    WindSystem * windSystem = GetScene()->windSystem;

    //Update trees
    uint32 treeCount = allTrees.size();
    for(uint32 i = 0; i < treeCount; ++i)
    {
        TreeInfo * info = allTrees[i];
        
		SpeedTreeComponent * component = GetSpeedTreeComponent(info->treeEntity);
		SpeedTreeObject * treeObject = DynamicTypeCheck<SpeedTreeObject *>(GetRenderObject(info->treeEntity));
        
        if(treeObject->GetLodIndex() > component->GetMaxAnimatedLOD())
        {
            treeObject->SetAnimationFlag(false);
            continue;
        }
        treeObject->SetAnimationFlag(true);

        Vector3 windVec = windSystem->GetWind(info->wtPosition);
        float32 windForce = windVec.Length();

        float32 trunkAmplitude = component->GetTrunkOscillationAmplitude();
        float32 trunkSpring = component->GetTrunkOscillationSpringSqrt();
        info->oscVelocity += (windVec - info->oscOffset * trunkSpring * trunkSpring - info->oscVelocity * trunkSpring) * timeElapsed;
        info->oscOffset += info->oscVelocity * timeElapsed;
        
        info->leafTime += timeElapsed * sqrtf(windForce) * component->GetLeafsOscillationSpeed();

        float32 sine, cosine;
        SinCosFast(info->leafTime, sine, cosine);
        float32 leafsOscillationAmplitude = component->GetLeafsOscillationApmlitude();
        Vector2 leafOscillationParams(leafsOscillationAmplitude * sine, leafsOscillationAmplitude * cosine);
        
		Vector3 localOffset = MultiplyVectorMat3x3(info->oscOffset * component->GetTrunkOscillationAmplitude(), info->wtInvMx);
        treeObject->SetTreeAnimationParams(localOffset, leafOscillationParams);
    }
}

void SpeedTreeUpdateSystem::HandleEvent(Observable * observable)
{
    RenderOptions * options = static_cast<RenderOptions *>(observable);
    
    if(isAnimationEnabled != options->IsOptionEnabled(RenderOptions::SPEEDTREE_ANIMATIONS))
    {
        isAnimationEnabled = options->IsOptionEnabled(RenderOptions::SPEEDTREE_ANIMATIONS);
        
        if(!isAnimationEnabled)
        {
            uint32 treeCount = allTrees.size();
            for(uint32 i = 0; i < treeCount; ++i)
            {
                SpeedTreeObject * treeObject = DynamicTypeCheck<SpeedTreeObject *>(GetRenderObject(allTrees[i]->treeEntity));
                treeObject->SetAnimationFlag(false);
            }
        }
    }
}
    
};