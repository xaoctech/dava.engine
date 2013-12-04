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



#include "Scene3D/Systems/ParticleEffectSystem.h"
#include "Scene3D/Components/ParticleEffectComponent.h"
#include "Particles/ParticleEmitter.h"
#include "Platform/SystemTimer.h"
#include "Debug/Stats.h"
#include "Core/PerformanceSettings.h"

namespace DAVA
{

ParticleEffectSystem::ParticleEffectSystem(Scene * scene) :	SceneSystem(Component::PARTICLE_EFFECT_COMPONENT, scene)	
{
}

void ParticleEffectSystem::Process()
{
    TIME_PROFILE("ParticleEffectSystem::Process");
	float32 timeElapsed = SystemTimer::Instance()->FrameDelta();
	
	/*shortEffectTime*/
	float32 currFps = 1.0f/timeElapsed;
	float32 currPSValue = (currFps - PerformanceSettings::Instance()->GetPsPerformanceMinFPS())/(PerformanceSettings::Instance()->GetPsPerformanceMaxFPS()-PerformanceSettings::Instance()->GetPsPerformanceMinFPS());
	currPSValue = Clamp(currPSValue, 0.0f, 1.0f);
	float32 speedMult = 1.0f+(PerformanceSettings::Instance()->GetPsPerformanceSpeedMult()-1.0f)*(1-currPSValue);
	float32 shortEffectTime = timeElapsed*speedMult;
	
	for(int i=0; i<activeComponents.size(); i++) //we take size in loop as it can actually change
	{
		ParticleEffectComponent * component = activeComponents[i];
		UpdateEffect(component, timeElapsed, shortEffectTime);
		/*finish restart criteria*/
		if ((component->state == ParticleEffectComponent::STATE_STOPPING)&&component->effectData.groups.empty())
		{
			//effect is moved to stopped state and removed form activeEffects/renderSystem
		}
		
	}
}

void ParticleEffectSystem::UpdateEffect(ParticleEffectComponent *effect, float32 time, float32 shortEffectTime)
{
	for (List<ParticleGroup>::iterator it = effect->effectData.groups.begin(), e=effect->effectData.groups.end(); it!=e;)
	{
		ParticleGroup &group = *it;
		float32 dt = group.emitter->IsShortEffect()?shortEffectTime:time;
		group.time+=dt;
		/*imagine here is update code from emitter/layer/particle*/
		/*finally*/
		if (group.finishingGroup&&(!group.head))
			it = effect->effectData.groups.erase(it);
		else
			++it;
	}
}




void ParticleEffectSystem::SetGlobalExtertnalValue(const String& name, float32 value)
{
	globalExternalValues[name] = value;
	for (Vector<Component *>::iterator it = components.begin(), e=components.end(); it!=e; ++it)
		((ParticleEffectComponent *)(*it))->SetExtertnalValue(name, value);
}

float32 ParticleEffectSystem::GetGlobalExternalValue(const String& name)
{
	Map<String, float32>::iterator it = globalExternalValues.find(name);
	if (it!=globalExternalValues.end())
		return (*it).second;
	else
		return 0.0f;
}

Map<String, float32> ParticleEffectSystem::GetGlobalExternals()
{
	return globalExternalValues;
}

}