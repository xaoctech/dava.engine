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


#include "Scene3D/Systems/SpeedTreeUpdateSystem.h"
#include "Scene3D/Systems/EventSystem.h"
#include "Scene3D/Entity.h"
#include "Scene3D/Components/ComponentHelpers.h"
#include "Scene3D/Components/RenderComponent.h"
#include "Scene3D/Components/TransformComponent.h"
#include "Render/Highlevel/SpeedTreeObject.h"

#include "Math/Math2D.h"

namespace DAVA
{
    
float32 GetForceOsscilation(float32 t, float32 phi)
{
    return (pow((5.f - t), 3.f) / 150.f * sinf(t * phi));
}
    
SpeedTreeUpdateSystem::SpeedTreeUpdateSystem(Scene * scene)
:	SceneSystem(scene),
    globalTime(0.f),
    windDirection(Vector3(1.f, 0.f, 0.f)),
    timerTime(0.f),
    timerTime2(0.f)
{
//    scene->GetEventSystem()->RegisterSystemForEvent(this, EventSystem::WORLD_TRANSFORM_CHANGED);
}

SpeedTreeUpdateSystem::~SpeedTreeUpdateSystem()
{
}

void SpeedTreeUpdateSystem::ImmediateEvent(Entity * entity, uint32 event)
{
}
    
void SpeedTreeUpdateSystem::AddEntity(Entity * entity)
{
    RenderObject * renderObject = GetRenderComponent(entity)->GetRenderObject();
    if (!renderObject) return;
    
    SpeedTreeObject * treeObject = cast_if_equal<SpeedTreeObject*>(renderObject);
    if(!treeObject) return;
    
    entity->AddComponent(new SpeedTreeComponent());
    
    treeObject->SetAnimationEnabled(true);
    
    TreeInfo * treeInfo = new TreeInfo(treeObject);
    treeInfo->position = GetTransformComponent(entity)->GetWorldTransform().GetTranslationVector();
    treeInfo->component = GetSpeedTreeComponent(entity);
    allTrees.push_back(treeInfo);
}

void SpeedTreeUpdateSystem::RemoveEntity(Entity * entity)
{
    RenderObject * renderObject = GetRenderComponent(entity)->GetRenderObject();
    if (!renderObject) return;
    
    SpeedTreeObject * treeObject = cast_if_equal<SpeedTreeObject*>(renderObject);
    if(!treeObject) return;
    
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

void SpeedTreeUpdateSystem::AddForce(const Vector3 & position, float32 forceValue)
{
    Force force;
    force.position = position;
    force.value = forceValue;
    force.time = 0.f;
    
    activeForces.push_back(force);
}
    
void SpeedTreeUpdateSystem::Process(float32 timeElapsed)
{
    globalTime += timeElapsed;
    
    //Update forces
    Vector<Force>::iterator it = activeForces.begin();
    while(it != activeForces.end())
    {
        Force & force = (*it);
        force.time += timeElapsed;
        if(force.time > 5.f)
        {
            it = activeForces.erase(it);
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
        
        Vector2 forcesOffset;
        float32 forcesAngleOffset = 0.f;
        uint32 forcesCount = activeForces.size();
        for(uint32 fi = 0; fi < forcesCount; ++fi)
        {
            Force & force = activeForces[fi];
            
            Vector3 direction = force.position - info->position;
            Vector2 direction2D = Vector2(direction.x, direction.y);
            float32 squareDistance = direction2D.SquareLength();
            if(squareDistance > 100.f)
                continue;
            
            squareDistance = Max(1.f, squareDistance);
            direction2D.Normalize();
            
            float32 linearOscillation = GetForceOsscilation(force.time, 5.f);
            forcesAngleOffset += linearOscillation * params.leafsForceOscillation;

            forcesOffset += direction2D / squareDistance * force.value * linearOscillation * params.trunkForceOscillationAmplitude;
        }
        
        Vector3 windOffset = windDirection * ((sinf(globalTime * params.trunkOscillationSpeed) + .5f) * params.trunkOscillationAmplitude);
        Vector3 trunkOscillationParams = windOffset + Vector3(forcesOffset.x, forcesOffset.y, 0.f);
        
        float32 sine, cosine;
        SinCosFast(globalTime * params.leafsOscillationSpeed + forcesAngleOffset, sine, cosine);
        Vector2 leafOscillationParams(params.leafsOscillationAmplitude * sine, params.leafsOscillationAmplitude * cosine);
        
        info->treeObject->SetTreeAnimationParams(trunkOscillationParams, leafOscillationParams);
    }
    
//    timerTime += timeElapsed;
//    if(timerTime > 15.f)
//    {
//        timerTime = 0.f;
//        AddForce(Vector3(1.f, 0.f, 0.f), 1.f);
//    }
//    
//    timerTime2 += timeElapsed;
//    if(timerTime2 > 16.f)
//    {
//        timerTime2 = 1.f;
//        AddForce(Vector3(-1.f, 0.f, 0.f), 3.f);
//    }
}

};