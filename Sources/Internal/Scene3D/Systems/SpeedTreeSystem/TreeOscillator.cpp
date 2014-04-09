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
#include "Scene3D/Components/SpeedTreeComponents/WindComponent.h"
#include "Scene3D/Components/SpeedTreeComponents/ImpulseOscillatorComponent.h"
#include "Scene3D/Components/SpeedTreeComponents/MovingOscillatorComponent.h"
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
 
TreeOscillator::TreeOscillator(Entity * owner) :
    entityOwner(owner),
	influenceSqDistance(0.f)
{}

void TreeOscillator::SetInfluenceDistance(float32 distance)
{
	influenceSqDistance = distance * distance;
}

bool TreeOscillator::HasInfluence(const Vector3 & forPosition)
{
    Vector3 position = GetTransformComponent(entityOwner)->GetWorldTransform().GetTranslationVector();
    Vector2 direction2D = Vec3ToVec2(position - forPosition);
    float32 sqDistance = direction2D.SquareLength();
    
    return sqDistance < influenceSqDistance;
}
    
////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////ImpulseOscillator//////////////////////////////////////////
    
ImpulseTreeOscillator::ImpulseTreeOscillator(Entity * owner) :
    TreeOscillator(owner),
    time(0.f),
	triggered(false)
{
	component = GetImpuleOscillatorComponent(owner);
	DVASSERT(component);

	SetInfluenceDistance(component->influenceDistance);
}
    
void ImpulseTreeOscillator::Update(float32 timeElapsed)
{
	if(triggered)
	{
		time += timeElapsed;

		if(time >= 5.f)
		{
			triggered = false;
		}
	}
}

bool ImpulseTreeOscillator::HasInfluence(const Vector3 & forPosition)
{
	return triggered && TreeOscillator::HasInfluence(forPosition);
}


void ImpulseTreeOscillator::Trigger()
{
	time = 0.f;
	triggered = true;
}

Vector3 ImpulseTreeOscillator::GetOscillationTrunkOffset(const Vector3 & forPosition) const
{
    Vector3 position = GetTransformComponent(entityOwner)->GetWorldTransform().GetTranslationVector();
	Vector2 direction2D = Vec3ToVec2(position - forPosition);

	float32 sqDistance = direction2D.SquareLength();
    sqDistance = Max(1.f, sqDistance);

    direction2D.Normalize();
    
	float32 forceValue = component->forceValue;
    Vector2 ret = direction2D / sqDistance * (pow((5.f - time), 3.f) / 150.f * sinf(time * 5.f)) * forceValue;
    
    return Vec2ToVec3(ret);
}
    
float32 ImpulseTreeOscillator::GetOscillationLeafsSpeed(const Vector3 & forPosition) const
{
    if(time < 1.f)
    {
        return 2.8f * (1.25f - .5f/(time + .5f) - (time - .5f) * (time - .5f));
    }
    else
    {
        return 4.f/(time + 0.5f) - .8f;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////WindOscillator///////////////////////////////////////////
    
WindTreeOscillator::WindTreeOscillator(Entity * owner) :
    TreeOscillator(owner),
	time(0.f)
{
    windComponent = GetWindComponent(entityOwner);
    DVASSERT(windComponent);

    windDirection = windComponent->GetWindDirection();

    SetInfluenceDistance(1e6);
}
    
void WindTreeOscillator::Update(float32 timeElapsed)
{
    time += timeElapsed;
}
    
Vector3 WindTreeOscillator::GetOscillationTrunkOffset(const Vector3 & forPosition) const
{
    float32 windForce = windComponent->GetWindForce();
    return windDirection * windForce * (sinf(time * windForce) + .5f);
}
    
float32 WindTreeOscillator::GetOscillationLeafsSpeed(const Vector3 & forPosition) const
{
    return windComponent->GetWindForce();
}

void WindTreeOscillator::UpdateWindDirection()
{
    windDirection = windComponent->GetWindDirection();
}

////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////MovingOscillator/////////////////////////////////////////
    
MovingTreeOscillator::MovingTreeOscillator(Entity * owner) :
    TreeOscillator(owner),
	currentSpeed(0.f)
{
	component = GetMovingOscillatorComponent(owner);
	DVASSERT(component);
}
    
MovingTreeOscillator::~MovingTreeOscillator()
{}
    
void MovingTreeOscillator::Update(float32 timeElapsed)
{
    Vector3 currPos = GetTransformComponent(entityOwner)->GetWorldTransform().GetTranslationVector();
    currentSpeed = (currPos - prevUpdatePosition).Length() / timeElapsed;
    prevUpdatePosition = currPos;
}
    
Vector3 MovingTreeOscillator::GetOscillationTrunkOffset(const Vector3 & forPosition) const
{
    return Vector3();
}
    
float32 MovingTreeOscillator::GetOscillationLeafsSpeed(const Vector3 & forPosition) const
{
    return Min(currentSpeed, component->speedClampValue);
}
    
};