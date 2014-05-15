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


#include "WaveSystem.h"
#include "Scene3D/Entity.h"
#include "Scene3D/Components/ComponentHelpers.h"
#include "Scene3D/Components/WaveComponent.h"
#include "Scene3D/Components/TransformComponent.h"
#include "Scene3D/Systems/EventSystem.h"
#include "Scene3D/Scene.h"
#include "Utils/Random.h"
#include "Math/Math2D.h"

namespace DAVA
{

WaveSystem::WaveSystem(Scene * scene) : 
    SceneSystem(scene)
{
    RenderOptions * options = RenderManager::Instance()->GetOptions();
    options->AddObserver(this);
    HandleEvent(options);
}

WaveSystem::~WaveSystem()
{
}

void WaveSystem::AddEntity(Entity * entity)
{
    //WindComponent * wind = GetWindComponent(entity);
    //winds.push_back(new WindInfo(wind));
}

void WaveSystem::RemoveEntity(Entity * entity)
{
    //Vector<WindInfo *>::iterator it = winds.begin();
    //while(it != winds.end())
    //{
    //    WindInfo * info = *it;
    //    if(info->component->entity == entity)
    //    {
    //        SafeDelete(info);
    //        winds.erase(it);
    //        break;
    //    }
    //    ++it;
    //}
}

void WaveSystem::Process(float32 timeElapsed)
{
    if(!isWavesEnabled)
        return;

    //int32 windCount = winds.size();
    //for(int32 i = 0; i < windCount; ++i)
    //{
    //    winds[i]->timeValue += timeElapsed * winds[i]->component->windSpeed;
    //}
}

Vector3 WaveSystem::GetWaveDisturbance(const Vector3 & inPosition) const
{
    Vector3 ret;
    //int32 windCount = winds.size();
    //for(int32 i = 0; i < windCount; ++i)
    //{
    //    WindInfo * info = winds[i];
    //    if(info->component->influenceBbox.IsInside(inPosition))
    //    {
    //        ret += info->component->GetDirection() * info->component->windForce * GetWindValueFromTable(inPosition, info);
    //    }
    //}

    return ret;
}

void WaveSystem::HandleEvent(Observable * observable)
{
    RenderOptions * options = static_cast<RenderOptions *>(observable);
    isWavesEnabled = options->IsOptionEnabled(RenderOptions::WAVE_DISTURBANCE_PROCESS);
}

};