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
#include "Scene3D/Systems/SpeedTreeSystem/TreeOscillator.h"
#include "Render/Highlevel/SpeedTreeObject.h"

#include "Math/Math2D.h"

namespace DAVA
{
    
SpeedTreeUpdateSystem::SpeedTreeUpdateSystem(Scene * scene)
:	SceneSystem(scene)
{
}

SpeedTreeUpdateSystem::~SpeedTreeUpdateSystem()
{
    uint32 oscCount = activeOscillators.size();
    for(uint32 oi = 0; oi < oscCount; ++oi)
    {
        SafeRelease(activeOscillators[oi]);
    }
    
    activeOscillators.clear();
}

void SpeedTreeUpdateSystem::ImmediateEvent(Entity * entity, uint32 event)
{
}
    
void SpeedTreeUpdateSystem::AddEntity(Entity * entity)
{
    SpeedTreeObject * treeObject = DynamicTypeCheck<SpeedTreeObject*>(GetRenderObject(entity));
    DVASSERT(treeObject);

    treeObject->SetAnimationEnabled(true);
    
    TreeInfo * treeInfo = new TreeInfo(treeObject);
    treeInfo->position = GetTransformComponent(entity)->GetWorldTransform().GetTranslationVector();
    treeInfo->component = GetSpeedTreeComponent(entity);
    allTrees.push_back(treeInfo);
}

void SpeedTreeUpdateSystem::RemoveEntity(Entity * entity)
{
    SpeedTreeObject * treeObject = DynamicTypeCheck<SpeedTreeObject*>(GetRenderObject(entity));
    DVASSERT(treeObject);
    
    Vector<TreeInfo *>::iterator it = allTrees.begin();
    Vector<TreeInfo *>::iterator itEnd = allTrees.end();
    for(; it != itEnd; ++it)
    {
        if((*it)->treeObject == treeObject)
        {
            allTrees.erase(it);
            break;
        }
    }
}

void SpeedTreeUpdateSystem::AddTreeOscillator(TreeOscillator * oscillator)
{
    if(oscillator->IsActive())
    {
        activeOscillators.push_back(oscillator);
        SafeRetain(oscillator);
    }
}

void SpeedTreeUpdateSystem::ForceRemoveTreeOscillator(TreeOscillator * oscillator)
{
    Vector<TreeOscillator *>::iterator it = activeOscillators.begin();
    Vector<TreeOscillator *>::iterator itEnd = activeOscillators.end();
    while(it != itEnd)
    {
        TreeOscillator * osc = (*it);
        if(oscillator == osc)
        {
            activeOscillators.erase(it);
            SafeRelease(osc);
            break;
        }
		++it;
    }
}

void SpeedTreeUpdateSystem::Process(float32 timeElapsed)
{
    //Update oscillators
    Vector<TreeOscillator *>::iterator it = activeOscillators.begin();
    while(it != activeOscillators.end())
    {
        TreeOscillator * oscillator = (*it);
        oscillator->Update(timeElapsed);
        if(!oscillator->IsActive())
        {
            it = activeOscillators.erase(it);
            SafeRelease(oscillator);
        }
        else
        {
            ++it;
        }
    }
    
    //Update trees
    uint32 treeCount = allTrees.size();
    for(uint32 i = 0; i < treeCount; ++i)
    {
        TreeInfo * info = allTrees[i];
        
        if(!info->component)
            continue;
        
        const SpeedTreeComponent::OscillationParams & params = info->component->GetOcciliationParameters();
        
        Vector3 oscillationOffsetAll;
        float32 leafSpeedAll = 0.f;
        
        uint32 oscCount = activeOscillators.size();
        for(uint32 oi = 0; oi < oscCount; ++oi)
        {
            TreeOscillator * osc = activeOscillators[oi];
            
            oscillationOffsetAll += osc->GetOsscilationTrunkOffset(info->position) * params.trunkOscillationAmplitude;
            
            float32 leafSpeed = osc->GetOsscilationLeafsSpeed(info->position);
            if(osc->GetType() == TreeOscillator::OSCILLATION_TYPE_MOVING)
            {
                leafSpeed *= params.movingOscillationLeafsSpeed;
            }
            
            leafSpeedAll += leafSpeed;
        }
        
        info->elapsedTime += timeElapsed * (params.leafsOscillationSpeed * leafSpeedAll);
        
        float32 sine, cosine;
        SinCosFast(info->elapsedTime, sine, cosine);
        Vector2 leafOscillationParams(params.leafsOscillationAmplitude * sine, params.leafsOscillationAmplitude * cosine);
        
        info->treeObject->SetTreeAnimationParams(oscillationOffsetAll, leafOscillationParams);
    }
}

};