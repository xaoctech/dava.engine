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


#ifndef __DAVAENGINE_SCENE3D_TREE_OSCILLATOR_H__
#define	__DAVAENGINE_SCENE3D_TREE_OSCILLATOR_H__

#include "Base/BaseTypes.h"
#include "Base/BaseMath.h"
#include "Base/BaseObject.h"

namespace DAVA
{
class Entity;

class TreeOscillator
{
public:
    enum eType
    {
        OSCILLATION_TYPE_WIND = 0,
        OSCILLATION_TYPE_IMPULSE,
        OSCILLATION_TYPE_MOVING,
        
        OSCILLATION_TYPE_COUNT
    };
    
    TreeOscillator(Entity * owner);
    virtual ~TreeOscillator() {};
    
    virtual uint32 GetType() const = 0;
    
    virtual void Update(float32 timeElapsed) {};
    virtual Vector3 GetOscillationTrunkOffset(const Vector3 & forPosition) const = 0;
    virtual float32 GetOscillationLeafsSpeed(const Vector3 & forPosition) const = 0;
    
    virtual bool HasInfluence(const Vector3 & forPosition);

	Entity * GetOwner() const { return entityOwner; }

protected:
	void SetInfluenceDistance(float32 distance);

	float32 influenceSqDistance;

    //weak link
    Entity * entityOwner;
};
    
class ImpulseTreeOscillator : public TreeOscillator
{
public:
    ImpulseTreeOscillator(Entity * ownder);
    virtual ~ImpulseTreeOscillator() {};
    
    virtual uint32 GetType() const {return OSCILLATION_TYPE_IMPULSE; };
    
    virtual void Update(float32 timeElapsed);
    virtual Vector3 GetOscillationTrunkOffset(const Vector3 & forPosition) const;
    virtual float32 GetOscillationLeafsSpeed(const Vector3 & forPosition) const;
    
	virtual bool HasInfluence(const Vector3 & forPosition);

	void Trigger();

protected:
    float32 time;
	bool triggered;

    ImpulseOscillatorComponent * component;
};
    
class WindTreeOscillator : public TreeOscillator
{
public:
    WindTreeOscillator(Entity * owner);
    virtual ~WindTreeOscillator() {};
    
    virtual uint32 GetType() const {return OSCILLATION_TYPE_WIND; };
    
    virtual void Update(float32 timeElapsed);
    virtual Vector3 GetOscillationTrunkOffset(const Vector3 & forPosition) const;
    virtual float32 GetOscillationLeafsSpeed(const Vector3 & forPosition) const;
    
protected:
    void UpdateWindDirection();

    Vector3 windDirection;
    float32 time;

    WindComponent * windComponent;

    friend class SpeedTreeUpdateSystem;
};
    
class MovingTreeOscillator : public TreeOscillator
{
public:
    MovingTreeOscillator(Entity * entity);
    virtual ~MovingTreeOscillator();
    
    virtual uint32 GetType() const {return OSCILLATION_TYPE_MOVING; };
    
    virtual void Update(float32 timeElapsed);
    virtual Vector3 GetOscillationTrunkOffset(const Vector3 & forPosition) const;
    virtual float32 GetOscillationLeafsSpeed(const Vector3 & forPosition) const;
    
protected:
    float32 currentSpeed;
	Vector3 prevUpdatePosition;

    MovingOscillatorComponent * component;
};
    
}

#endif	/* __DAVAENGINE_SCENE3D_SPEEDTREEUPDATESYSTEM_H__ */

