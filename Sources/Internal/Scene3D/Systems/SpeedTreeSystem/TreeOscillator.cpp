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


#include "Scene3D/Entity.h"
#include "Scene3D/Components/ComponentHelpers.h"
#include "Scene3D/Components/WindComponent.h"
#include "Scene3D/Components/TransformComponent.h"
#include "TreeOscillator.h"

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
 
TreeOscillator::TreeOscillator(float32 _distance, Entity * owner) :
    entityOwner(owner)
{
    influenceSqDistance = _distance * _distance;
}

bool TreeOscillator::HasInfluence(const Vector3 & forPosition, float32 * outSqDistance /* = 0 */) const
{
    Vector3 position = GetTransformComponent(entityOwner)->GetWorldTransform().GetTranslationVector();
    Vector2 direction2D = Vec3ToVec2(position - forPosition);
    float32 sqDistance = direction2D.SquareLength();
    
    if(outSqDistance)
        (*outSqDistance) = sqDistance;
    
    return sqDistance < influenceSqDistance;
}
    
////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////ImpulseOscillator//////////////////////////////////////////
    
ImpulseTreeOscillator::ImpulseTreeOscillator(float32 _distance, Entity * owner, float32 _forceValue) :
    TreeOscillator(_distance, owner),
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
    
    Vector3 position = GetTransformComponent(entityOwner)->GetWorldTransform().GetTranslationVector();
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
    
WindTreeOscillator::WindTreeOscillator(Entity * owner) :
    TreeOscillator(0.f, owner)
{}
    
void WindTreeOscillator::Update(float32 timeElapsed)
{
    time += timeElapsed;
}
    
Vector3 WindTreeOscillator::GetOsscilationTrunkOffset(const Vector3 & forPosition) const
{
    WindComponent * wind = GetWindComponent(entityOwner);
    return wind->GetWindDirection() * wind->GetWindForce() * (sinf(time) + .5f);
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
    
MovingTreeOscillator::MovingTreeOscillator(float32 distance, Entity * owner) :
    TreeOscillator(distance, owner)
{}
    
MovingTreeOscillator::~MovingTreeOscillator()
{}
    
void MovingTreeOscillator::Update(float32 timeElapsed)
{
    Vector3 currPos = GetTransformComponent(entityOwner)->GetWorldTransform().GetTranslationVector();
    currentSpeed = (currPos - prevUpdatePosition).Length() / timeElapsed;
    prevUpdatePosition = currPos;
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
    
};