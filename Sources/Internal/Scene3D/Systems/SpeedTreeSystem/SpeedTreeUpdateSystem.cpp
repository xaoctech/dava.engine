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
#include "Scene3D/Components/SpeedTreeComponents/SpeedTreeComponent.h"
#include "Scene3D/Systems/SpeedTreeSystem/TreeOscillator.h"
#include "Scene3D/Systems/EventSystem.h"
#include "Scene3D/Scene.h"
#include "Render/Highlevel/SpeedTreeObject.h"
#include "Utils/Random.h"
#include "Math/Math2D.h"

namespace DAVA
{
    
const uint32 SpeedTreeUpdateSystem::SPEED_TREE_UPDATE_SYSTEM_COMPONENTS_MASK = (1 << Component::SPEEDTREE_COMPONENT) | (1 << Component::WIND_COMPONENT) 
        | (1 << Component::IMPULSE_OSCILLATOR_COMPONENT) | (1 << Component::MOVING_OSCILLATOR_COMPONENT);

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
    uint32 oscCount = activeOscillators.size();
    for(uint32 oi = 0; oi < oscCount; ++oi)
    {
        SafeDelete(activeOscillators[oi]);
    }
    
    activeOscillators.clear();

    RenderOptions * options = RenderManager::Instance()->GetOptions();
    options->RemoveObserver(this);
}

void SpeedTreeUpdateSystem::ImmediateEvent(Entity * entity, uint32 event)
{
	if(event == EventSystem::WORLD_TRANSFORM_CHANGED)
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

bool SpeedTreeUpdateSystem::IsNeedProcessEntity(Entity * entity)
{
	uint32 componentsMask = entity->GetAvailableComponentFlags();
	return (componentsMask & SPEED_TREE_UPDATE_SYSTEM_COMPONENTS_MASK) > 0;
}

void SpeedTreeUpdateSystem::TriggerImpulseOscillator(Entity * entity)
{
	uint32 oscCount = activeOscillators.size();
	for(uint32 i = 0; i < oscCount; ++i)
	{
		TreeOscillator * osc = activeOscillators[i];
		if(osc->GetType() == TreeOscillator::OSCILLATION_TYPE_IMPULSE && osc->GetOwner() == entity)
		{
			ImpulseTreeOscillator * impulse = static_cast<ImpulseTreeOscillator *>(osc);
			impulse->Trigger();
		}
	}
}

void SpeedTreeUpdateSystem::AddEntity(Entity * entity)
{
	uint32 componentsMask = entity->GetAvailableComponentFlags();
	if(componentsMask & (1 << Component::SPEEDTREE_COMPONENT))
	{
		AddTreeEntity(entity);
	}
	else
	{
		AddOscillatorEntity(entity);
	}
}

void SpeedTreeUpdateSystem::RemoveEntity(Entity * entity)
{
	uint32 componentsMask = entity->GetAvailableComponentFlags();
	if(componentsMask & (1 << Component::SPEEDTREE_COMPONENT))
	{
		RemoveTreeEntity(entity);
	}
	else
	{
		RemoveOscillatorEntity(entity);
	}
}

void SpeedTreeUpdateSystem::AddTreeEntity(Entity * entity)
{
	DVASSERT(GetSpeedTreeComponent(entity));

	SpeedTreeObject * treeObject = DynamicTypeCheck<SpeedTreeObject*>(GetRenderObject(entity));
	DVASSERT(treeObject);

	treeObject->SetAnimationEnabled(isAnimationEnabled);

	TreeInfo * treeInfo = new TreeInfo((float32)Random::Instance()->RandFloat(1000.f));
	treeInfo->treeEntity = entity;

	Matrix4 wtMx = GetTransformComponent(entity)->GetWorldTransform();
	treeInfo->wtPosition = wtMx.GetTranslationVector();
	wtMx.GetInverse(treeInfo->wtInvMx);

	allTrees.push_back(treeInfo);
}

void SpeedTreeUpdateSystem::RemoveTreeEntity(Entity * entity)
{
	Vector<TreeInfo *>::iterator it = allTrees.begin();
	Vector<TreeInfo *>::iterator itEnd = allTrees.end();
	for(; it != itEnd; ++it)
	{
		if((*it)->treeEntity == entity)
		{
			allTrees.erase(it);
			break;
		}
	}
}

void SpeedTreeUpdateSystem::AddOscillatorEntity(Entity * entity)
{
	uint32 componentsFlags = entity->GetAvailableComponentFlags();
	if((componentsFlags & (1 << Component::WIND_COMPONENT)) != 0)
	{
		activeOscillators.push_back(new WindTreeOscillator(entity));
	}
	if((componentsFlags & (1 << Component::IMPULSE_OSCILLATOR_COMPONENT)) != 0)
	{
		activeOscillators.push_back(new ImpulseTreeOscillator(entity));
	}
	if((componentsFlags & (1 << Component::MOVING_OSCILLATOR_COMPONENT)) != 0)
	{
		activeOscillators.push_back(new MovingTreeOscillator(entity));
	}
}

void SpeedTreeUpdateSystem::RemoveOscillatorEntity(Entity * entity)
{
	Vector<TreeOscillator *>::iterator it = activeOscillators.begin();
	if(it != activeOscillators.end())
	{
		if((*it)->GetOwner() == entity)
		{
			SafeDelete((*it));
			it = activeOscillators.erase(it);
		}
		else
		{
			++it;
		}
	}
}

void SpeedTreeUpdateSystem::Process(float32 timeElapsed)
{
    if(!isAnimationEnabled)
        return;
    
    //Update oscillators
	uint32 oscCount = activeOscillators.size();
	for(uint32 i = 0; i < oscCount; ++i)
	{
		activeOscillators[i]->Update(timeElapsed);
	}
    
    //Update trees
    uint32 treeCount = allTrees.size();
    for(uint32 i = 0; i < treeCount; ++i)
    {
        TreeInfo * info = allTrees[i];
        
		SpeedTreeComponent * component = GetSpeedTreeComponent(info->treeEntity);
		SpeedTreeObject * treeObject = DynamicTypeCheck<SpeedTreeObject *>(GetRenderObject(info->treeEntity));
        
        if(treeObject->GetLodIndex() > component->GetMaxAnimatedLOD())
        {
            treeObject->SetAnimationEnabled(false);
            continue;
        }
        treeObject->SetAnimationEnabled(true);
        
		Vector3 treePosition = info->wtPosition;

        Vector3 oscillationOffsetAll;
        float32 leafSpeedAll = 0.f;
        
        uint32 oscCount = activeOscillators.size();
        for(uint32 oi = 0; oi < oscCount; ++oi)
        {
            TreeOscillator * osc = activeOscillators[oi];
			if(!osc->HasInfluence(treePosition))
				continue;
            
            oscillationOffsetAll += osc->GetOscillationTrunkOffset(treePosition);
            
            float32 leafSpeed = osc->GetOscillationLeafsSpeed(treePosition);
            if(osc->GetType() == TreeOscillator::OSCILLATION_TYPE_MOVING)
            {
                leafSpeed *= component->GetMovingOscillationLeafsSpeed();
            }
            
            leafSpeedAll += leafSpeed;
        }
        
        info->elapsedTime += timeElapsed * (component->GetLeafOscillationSpeed() * leafSpeedAll);
        
        float32 sine, cosine;
        SinCosFast(info->elapsedTime, sine, cosine);
        float32 leafsOscillationAmplitude = component->GetLeafsOscillationApmlitude();
        Vector2 leafOscillationParams(leafsOscillationAmplitude * sine, leafsOscillationAmplitude * cosine);
        
		Vector3 localOffset = MultiplyVectorMat3x3(oscillationOffsetAll * component->GetTrunkOscillationAmplitude(), info->wtInvMx);
        treeObject->SetTreeAnimationParams(localOffset, leafOscillationParams);
    }
}
    
void SpeedTreeUpdateSystem::HandleEvent(Observable * observable)
{
    RenderOptions * options = static_cast<RenderOptions *>(observable);
    
    if(isAnimationEnabled != options->IsOptionEnabled(RenderOptions::SPEEDTREE_ANIMATIONS))
    {
        isAnimationEnabled = options->IsOptionEnabled(RenderOptions::SPEEDTREE_ANIMATIONS);
        
        uint32 treeCount = allTrees.size();
        for(uint32 i = 0; i < treeCount; ++i)
        {
			SpeedTreeObject * treeObject = DynamicTypeCheck<SpeedTreeObject *>(GetRenderObject(allTrees[i]->treeEntity));
            treeObject->SetAnimationEnabled(isAnimationEnabled);
        }
    }
}
    
};