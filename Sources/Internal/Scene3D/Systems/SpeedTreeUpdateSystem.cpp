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
    
Vector2 Vec3ToVec2(const Vector3 & v)
{
    return Vector2(v.x, v.y);
}

Vector3 Vec2ToVec3(const Vector2 & v)
{
    return Vector3(v.x, v.y, 0.f);
}
    
Vector3 Vec4ToVec3(const Vector4 & v)
{
    return Vector3(v.x, v.y, v.z);
}
    
Vector4 Vec3ToVec4(const Vector3 & v)
{
    return Vector4(v.x, v.y, v.z, 0.f);
}
    
////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////WindSystem/////////////////////////////////////////////
    
WindSystem::WindSystem(Scene * scene)
    : SceneSystem(scene)
{
    WindTreeOscillator * wind = new WindTreeOscillator(Vector3(1.f, 0.f, 0.f), 0.5f);
    GetScene()->speedTreeUpdateSystem->AddTreeOscillator(wind);
    wind->Release();
}
    
WindSystem::~WindSystem() {}
    
void WindSystem::AddEntity(Entity * entity)
{
    
}
    
void WindSystem::RemoveEntity(Entity * entity)
{
    
}

////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////TreeOscillator//////////////////////////////////////////

TreeOscillator::TreeOscillator(float32 _distance, const Vector3 & _worldPosition) :
    position(_worldPosition)
{
    influenceSqDistance = _distance * _distance;
}

bool TreeOscillator::HasInfluence(const Vector3 & forPosition, float32 * outSqDistance /* = 0 */) const
{
    Vector2 direction2D = Vec3ToVec2(position - forPosition);
    float32 sqDistance = direction2D.SquareLength();
    
    if(outSqDistance)
        (*outSqDistance) = sqDistance;
    
    return sqDistance < influenceSqDistance;
}
    
////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////ImpulseOscillator//////////////////////////////////////////
    
ImpulseTreeOscillator::ImpulseTreeOscillator(float32 _distance, const Vector3 & _worldPosition, float32 _forceValue) :
    TreeOscillator(_distance, _worldPosition),
    forceValue(_forceValue),
    time(0.f)
{}
    
void ImpulseTreeOscillator::Update(float32 timeElapsed)
{
    time += timeElapsed;
}
    
Vector3 ImpulseTreeOscillator::GetOsscilationTrunkOffset(const Vector3 & forPosition) const
{
    float32 squareDistance = 0.f;
    if(!HasInfluence(forPosition, &squareDistance))
        return Vector3();
    
    Vector2 direction2D = Vec3ToVec2(position - forPosition);
    squareDistance = Max(1.f, squareDistance);
    direction2D.Normalize();
    
    Vector2 ret = direction2D / squareDistance * (pow((5.f - time), 3.f) / 150.f * sinf(time * 5.f)) * forceValue;
    
    return Vec2ToVec3(ret);
}
    
float32 ImpulseTreeOscillator::GetOsscilationLeafsSpeed(const Vector3 & forPosition) const
{
    if(!HasInfluence(forPosition))
        return 0.f;
    
    if(time < 1.f)
    {
        return 2.8f * (1.25f - .5f/(time + .5f) - (time - .5f) * (time - .5f));
    }
    else
    {
        return 4.f/(time + 0.5) - .8f;
    }
}
    
bool ImpulseTreeOscillator::IsActive() const
{
    return time < 5.f;
}

////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////WindOscillator///////////////////////////////////////////
    
WindTreeOscillator::WindTreeOscillator(const Vector3 & _windDirection, float32 _windForce) :
    TreeOscillator(0.f, Vector3()),
    windForce(_windForce),
    windDirection(_windDirection)
{
    windForce = 0.25f;
    windDirection = Vector3(1.f, 0.f, 0.f);
}
    
void WindTreeOscillator::Update(float32 timeElapsed)
{
    time += timeElapsed;
}
    
Vector3 WindTreeOscillator::GetOsscilationTrunkOffset(const Vector3 & forPosition) const
{
    return windDirection * windForce * (sinf(time) + .5f);
}
    
float32 WindTreeOscillator::GetOsscilationLeafsSpeed(const Vector3 & forPosition) const
{
    return 1.f;
}
    
bool WindTreeOscillator::IsActive() const
{
    return true;
}
    
////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////MovingOscillator/////////////////////////////////////////
    
MovingTreeOscillator::MovingTreeOscillator(float32 distance, Entity * entity) :
    TreeOscillator(distance, Vector3()),
    movingEntity(entity)
{}
    
MovingTreeOscillator::~MovingTreeOscillator()
{}
    
void MovingTreeOscillator::Update(float32 timeElapsed)
{
    Vector3 currPos = GetTransformComponent(movingEntity)->GetWorldTransform().GetTranslationVector();
    currentSpeed = (currPos - position).Length() / timeElapsed;
    position = currPos;
}
    
Vector3 MovingTreeOscillator::GetOsscilationTrunkOffset(const Vector3 & forPosition) const
{
    return Vector3();
}
    
float32 MovingTreeOscillator::GetOsscilationLeafsSpeed(const Vector3 & forPosition) const
{
    if(!HasInfluence(forPosition))
        return 0.f;
    
    return Min(currentSpeed, 1.f);
}
    
bool MovingTreeOscillator::IsActive() const
{
    return true;
}

////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////SpeedTreeUpdateSystem///////////////////////////////////////
    
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
    Vector<TreeOscillator *>::iterator itEnd = activeOscillators.begin();
    while(it != itEnd)
    {
        TreeOscillator * osc = (*it);
        if(oscillator == osc)
        {
            activeOscillators.erase(it);
            SafeRelease(osc);
            break;
        }
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