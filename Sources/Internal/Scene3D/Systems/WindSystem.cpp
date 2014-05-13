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


#include "WindSystem.h"
#include "Scene3D/Entity.h"
#include "Scene3D/Components/ComponentHelpers.h"
#include "Scene3D/Components/WindComponent.h"
#include "Scene3D/Components/TransformComponent.h"
#include "Scene3D/Systems/EventSystem.h"
#include "Scene3D/Scene.h"
#include "Utils/Random.h"
#include "Math/Math2D.h"

namespace DAVA
{

WindSystem::WindInfo::WindInfo(WindComponent * c) :
component(c),
currentWindValue(0.f)
{
    timeValue = (float32)Random::Instance()->RandFloat(1000.f);
}

WindSystem::WindSystem(Scene * scene) : 
    SceneSystem(scene)
{
}

WindSystem::~WindSystem()
{

}

void WindSystem::AddEntity(Entity * entity)
{
    WindComponent * wind = GetWindComponent(entity);
    winds.push_back(new WindInfo(wind));
}

void WindSystem::RemoveEntity(Entity * entity)
{
    Vector<WindInfo *>::iterator it = winds.begin();
    while(it != winds.end())
    {
        WindInfo * info = *it;
        if(info->component->entity == entity)
        {
            SafeDelete(info);
            winds.erase(it);
            break;
        }
        ++it;
    }

}

void WindSystem::Process(float32 timeElapsed)
{
    Vector3 windVector;
    int32 windCount = winds.size();
    for(int32 i = 0; i < windCount; ++i)
    {
        WindInfo * info = winds[i];
        ProcessWind(info, timeElapsed);
        windVector += info->component->GetWindDirection() * info->currentWindValue;
    }
    float32 globalForce = 0.f;
    if(!windVector.IsZero())
        globalForce = windVector.Normalize();
    globalWind = Vector4(windVector);
    globalWind.w = globalForce;
}

Vector4 WindSystem::GetWind(const Vector3 & inPosition, uint32 typeMask /* = WIND_TYPE_MASK_ALL */)
{
    Vector4 ret;
    if((typeMask & WindComponent::WIND_TYPE_GLOBAL) != 0)
        ret += globalWind;
    return ret;
}

void WindSystem::WindTriggered(WindComponent * wind)
{

}

void WindSystem::ProcessWind(WindInfo * wind, float32 timeElapsed)
{
    wind->timeValue += timeElapsed;
    if(wind->component->type == WindComponent::WIND_TYPE_GLOBAL)
    {
        wind->currentWindValue = wind->component->force * (2.f + sinf(wind->timeValue) * 0.8f + cosf(wind->timeValue * 10) * 0.2f);
    }
}

};