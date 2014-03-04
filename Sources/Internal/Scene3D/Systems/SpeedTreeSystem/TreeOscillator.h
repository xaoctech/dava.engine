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

class TreeOscillator: public BaseObject
{
public:
    enum eType
    {
        OSCILLATION_TYPE_WIND = 0,
        OSCILLATION_TYPE_IMPULSE,
        OSCILLATION_TYPE_MOVING,
        
        OSCILLATION_TYPE_COUNT
    };
    
    TreeOscillator(float32 influenceDistance, Entity * owner);
    virtual ~TreeOscillator() {};
    
    virtual uint32 GetType() const = 0;
    
    virtual void Update(float32 timeElapsed) {};
    virtual Vector3 GetOsscilationTrunkOffset(const Vector3 & forPosition) const = 0;
    virtual float32 GetOsscilationLeafsSpeed(const Vector3 & forPosition) const = 0;
    virtual bool IsActive() const = 0;
    
    inline const float32 & GetInfluenceSqDistance() const;
    inline const Vector3 & GetCurrentPosition() const;
    
    bool HasInfluence(const Vector3 & forPosition, float32 * outSqDistance = 0) const;
    
protected:
    float32 influenceSqDistance;
    
    //weak link
    Entity * entityOwner;
};
    
class ImpulseTreeOscillator : public TreeOscillator
{
public:
    ImpulseTreeOscillator(float32 influenceDistance, Entity * ownder, float32 forceValue);
    virtual ~ImpulseTreeOscillator() {};
    
    virtual uint32 GetType() const {return OSCILLATION_TYPE_IMPULSE; };
    
    virtual void Update(float32 timeElapsed);
    virtual Vector3 GetOsscilationTrunkOffset(const Vector3 & forPosition) const;
    virtual float32 GetOsscilationLeafsSpeed(const Vector3 & forPosition) const;
    virtual bool IsActive() const;
    
protected:
    float32 time;
    float32 forceValue;
};
    
class WindTreeOscillator : public TreeOscillator
{
public:
    WindTreeOscillator(Entity * owner);
    virtual ~WindTreeOscillator() {};
    
    virtual uint32 GetType() const {return OSCILLATION_TYPE_WIND; };
    
    virtual void Update(float32 timeElapsed);
    virtual Vector3 GetOsscilationTrunkOffset(const Vector3 & forPosition) const;
    virtual float32 GetOsscilationLeafsSpeed(const Vector3 & forPosition) const;
    virtual bool IsActive() const;
    
protected:
    float32 time;
};
    
class MovingTreeOscillator : public TreeOscillator
{
public:
    MovingTreeOscillator(float32 distance, Entity * entity);
    virtual ~MovingTreeOscillator();
    
    virtual uint32 GetType() const {return OSCILLATION_TYPE_MOVING; };
    
    virtual void Update(float32 timeElapsed);
    virtual Vector3 GetOsscilationTrunkOffset(const Vector3 & forPosition) const;
    virtual float32 GetOsscilationLeafsSpeed(const Vector3 & forPosition) const;
    virtual bool IsActive() const;
    
protected:
    float32 currentSpeed;
    Vector3 prevUpdatePosition;
};
    
}

#endif	/* __DAVAENGINE_SCENE3D_SPEEDTREEUPDATESYSTEM_H__ */

