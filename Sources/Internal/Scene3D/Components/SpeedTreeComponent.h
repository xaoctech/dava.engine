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



#ifndef __DAVAENGINE_SCENE3D_SPEEDTREE_COMPONENT_H__
#define __DAVAENGINE_SCENE3D_SPEEDTREE_COMPONENT_H__

#include "Base/BaseTypes.h"
#include "Entity/Component.h"
#include "Scene3D/Entity.h"
#include "Scene3D/SceneFile/SerializationContext.h"

namespace DAVA 
{

class SpeedTreeUpdateSystem;
class SpeedTreeComponent : public Component
{
protected:
	virtual ~SpeedTreeComponent();
public:
    struct OscillationParams
    {
        OscillationParams() :
        trunkOscillationAmplitude(0.15f),
        leafsOscillationAmplitude(0.03f),
        leafsOscillationSpeed(3.f),
        movingOscillationLeafsSpeed(1.f)
        {}
        
        float32 trunkOscillationAmplitude;
        float32 leafsOscillationAmplitude;
        float32 leafsOscillationSpeed;
        float32 movingOscillationLeafsSpeed;
        
        INTROSPECTION(OscillationParams,
                      MEMBER(trunkOscillationAmplitude, "trunkOscillationAmplitude", I_VIEW | I_EDIT | I_SAVE)
                      MEMBER(leafsOscillationAmplitude, "leafsOscillationAmplitude", I_VIEW | I_EDIT | I_SAVE)
                      MEMBER(leafsOscillationSpeed, "leafsOscillationSpeed", I_VIEW | I_EDIT | I_SAVE)
                      MEMBER(movingOscillationLeafsSpeed, "movingOscillationLeafsSpeed", I_VIEW | I_EDIT | I_SAVE)
                      );
    };
    
	SpeedTreeComponent();

	IMPLEMENT_COMPONENT_TYPE(SPEEDTREE_COMPONENT);

	virtual Component * Clone(Entity * toEntity);
	virtual void Serialize(KeyedArchive *archive, SerializationContext *serializationContext);
	virtual void Deserialize(KeyedArchive *archive, SerializationContext *serializationContext);

    OscillationParams & GetOcciliationParameters();
    
protected:
    OscillationParams params;

public:
	INTROSPECTION_EXTEND(SpeedTreeComponent, Component,
                         MEMBER(params, "params", I_VIEW | I_EDIT | I_SAVE)
                         );
    
	friend class SpeedTreeUpdateSystem;
};

};

#endif