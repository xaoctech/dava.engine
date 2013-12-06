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
#include "Utils/Random.h"
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
		bool effectEnded = component->stopWhenEmpty?component->effectData.groups.empty():(component->=time>component->effectDuration);
		if (effectEnded)
		{
			component->currRepeatsCont++;
			if ((component->repeatsCount==0)||(component->currRepeatsCont<component->repeatsCount)) //0 means infinite repeats
			{
				//TODO: restart effect
			}
			else
			{
				component->state = ParticleEffectComponent::STATE_STOPPING;
			}
		}
		/*finish restart criteria*/		
		if ((component->state == ParticleEffectComponent::STATE_STOPPING)&&component->effectData.groups.empty())
		{
			//TODO: effect is moved to stopped state and removed form activeEffects/renderSystem			
			continue;
		}
		
		
	}
}

void ParticleEffectSystem::UpdateEffect(ParticleEffectComponent *effect, float32 time, float32 shortEffectTime)
{
	const Matrix4 &effectTransform = effect->GetEntity()->GetWorldTransform();
	Matrix3 rotationMatrix = Matrix3::Identity();
	

	if(worldTransformPtr)
	{
		tempPosition = worldTransformPtr->GetTranslationVector();
		rotationMatrix = Matrix3(*worldTransformPtr);;
	}
	for (List<ParticleGroup>::iterator it = effect->effectData.groups.begin(), e=effect->effectData.groups.end(); it!=e;)
	{
		ParticleGroup &group = *it;
		float32 dt = group.emitter->IsShortEffect()?shortEffectTime:time;
		group.time+=dt;		
		float32 groupEndTime = group.layer->isLooped?group.layer->loopEndTime:group.layer->endTime;
		float32 currLoopTime = group.time;
		if (group.time>groupEndTime)
			group.finishingGroup = true;
		
		if ((!group.finishingGroup)&&group.layer->isLooped)
		{
			currLoopTime -= group.loopStartTime;
			if (currLoopTime>group.loopRestartDuration)
			{
				group.loopStartTime = group.time;
				group.loopDuration = (group.layer->endTime-group.layer->startTime) + group.layer->loopVariation*Random::Instance()->RandFloat();
				group.loopRestartDuration = group.loopDuration + group.layer->deltaTime + group.layer->deltaVariation*Random::Instance()->RandFloat();
				currLoopTime = 0;
			}			
		}
		//prepare forces as they will now actually change in time even fro already generated particles
		Vector<Vector3> currForceValues;
		int32 forcesCount;
		if (group.head)
		{
			forcesCount = group.layer->forces.size();
			if (forcesCount)
			{
				currForceValues.resize(forcesCount);
				for (int32 i=0; i<forcesCount; ++i)
					currForceValues[i]=group.layer->forces[i].force->GetValue(currLoopTime);
			}
		}		
		Particle *current = group.head;
		Particle *prev = 0;		
		while (current)
		{
			current->life += dt;
			if (current->life >= current->lifeTime)
			{
				Particle *next = current->next;
				if (prev == 0)
					group.head = next;
				else prev->next = next;				
				delete current;
				current = next;
				continue;
			}			

			float32 overLifeTime = current->life/current->lifeTime;
			float32 currVelocityOverLife = 1.0f;
			if (group.layer->velocityOverLife)
				currVelocityOverLife = group.layer->velocityOverLife->GetValue(overLifeTime);
			current->position += current->speed * (currVelocityOverLife*dt);			
			float32 currSpinOverLife = 1;
			if (group.layer->spinOverLife)
				currSpinOverLife = group.layer->spinOverLife->GetValue(overLifeTime);
			current->angle += current->spin * currSpinOverLife *dt;	
			if (forcesCount)
			{
				Vector3 acceleration;
				for(int32 i = 0; i < forcesCount; ++i)
				{
					acceleration += (group.layer->forces[i].forceOverLife)?(currForceValues[i] * group.layer->forces[i].forceOverLife->GetValue(overLifeTime)) : currForceValues[i];				
				}
				current->speed+=acceleration*dt;
			}			
			//TODO update inner emitter here - rethink it						
			
			if (group.layer->frameOverLifeEnabled)
			{
				float32 animDelta = group.layer->frameOverLifeFPS;
				if (group.layer->animSpeedOverLife)
					animDelta*=group.layer->animSpeedOverLife->GetValue(overLifeTime);
				current->animTime+=animDelta;

				while (current->animTime>1.0f)
				{
					current->frame ++;					
					//TODO: note - code to compute frame according to sprite frame count was here - later add it to render object
					current->animTime -= 1.0f;
				}
			}			
			current=current->next;			
		}		
		bool allowParticleGeneration = !group.finishingGroup;
		if (group.layer->isLooped)
			allowParticleGeneration&=currLoopTime<group.loopDuration;
		//TODO: add LoD criteria here
		if (allowParticleGeneration)
		{
			if (group.layer->type == ParticleLayer::TYPE_SINGLE_PARTICLE)
			{
				if (!group.head)
					GenerateNewParticle(group, currLoopTime, );
			}else
			{
				float32 newParticles = 0.0f;
				
				if (group.layer->number)
					newParticles = group.layer->number->GetValue(currLoopTime);
				if (group.layer->numberVariation)
					newParticles += group.layer->numberVariation->GetValue(currLoopTime)*Random::Instance()->RandFloat();
				newParticles*=dt;				
				group.particlesToGenerate += newParticles;

				while(group.particlesToGenerate >= 1.0f)
				{
					group.particlesToGenerate -= 1.0f;					
					GenerateNewParticle(group, currLoopTime);					
				}
			}
			
		}

		/*finally*/
		if (group.finishingGroup&&(!group.head))
			it = effect->effectData.groups.erase(it);
		else
			++it;
	}
}


void ParticleEffectSystem::GenerateNewParticle(ParticleGroup& group, float32 currLoopTime, Matrix4 *worldTransform)
{
	Particle *particle = new Particle();		
	particle->life = 0.0f;	

	particle->color = Color();
	if (group.layer->colorRandom)
	{
		particle->color = group.layer->colorRandom->GetValue(Random::Instance()->RandFloat());
	}
	particle->lifeTime = 0.0f;
	if (group.layer->life)
		particle->lifeTime += group.layer->life->GetValue(currLoopTime);
	if (group.layer->lifeVariation)
		particle->lifeTime += (group.layer->lifeVariation->GetValue(currLoopTime) * Random::Instance()->RandFloat());

	// size 
	particle->size = Vector2(1.0f, 1.0f); 
	if (group.layer->size)
		particle->size = group.layer->size->GetValue(currLoopTime);
	if (group.layer->sizeVariation)
		particle->size +=(group.layer->sizeVariation->GetValue(currLoopTime) * Random::Instance()->RandFloat());

	//TODO: superemitter stuff - rethink later
	/*if (emitter->parentParticle){
		particle->size.x*=emitter->parentParticle->size.x;
		particle->size.y*=emitter->parentParticle->size.y;
	}*/

	float32 vel = 0.0f;
	if (group.layer->velocity)
		vel += group.layer->velocity->GetValue(currLoopTime);
	if (group.layer->velocityVariation)
		vel += (group.layer->velocityVariation->GetValue(currLoopTime) * Random::Instance()->RandFloat());

	particle->angle = 0.0f;
	particle->spin = 0.0f;
	if (group.layer->angle)
		particle->angle = DegToRad(group.layer->angle->GetValue(currLoopTime));
	if (group.layer->angleVariation)
		particle->angle += DegToRad(group.layer->angleVariation->GetValue(currLoopTime) * Random::Instance()->RandFloat());		
	if (group.layer->spin)
		particle->spin = DegToRad(group.layer->spin->GetValue(currLoopTime));
	if (group.layer->spinVariation)
		particle->spin += DegToRad(group.layer->spinVariation->GetValue(currLoopTime) * Random::Instance()->RandFloat());
	if (group.layer->randomSpinDirection)
	{
		int32 dir = Rand()&1;
		particle->spin *= (dir)*2-1;
	}
	particle->frame = 0;
	particle->animTime = 0;
	if (group.layer->randomFrameOnStart&&group.layer->sprite)
	{
		particle->frame =  (int32)(Random::Instance()->RandFloat() * (float32)(group.layer->sprite->GetFrameCount()));
	}	
	
	
	//particle->position = emitter->GetPosition();	
	// parameters should be prepared inside prepare emitter parameters
	emitter->PrepareEmitterParameters(particle, vel, emitIndex);
	if (this->emitter&&!inheritPosition) //just generate at correct position
	{		
		particle->position += emitter->GetPosition()-emitter->GetInitialTranslationVector();
	}		

		
	particle->next = group.head;
	group.head = particle;	
}



void ParticleEffectSystem::SetGlobalExtertnalValue(const String& name, float32 value)
{
	globalExternalValues[name] = value;
	for (Vector<ParticleEffectComponent *>::iterator it = activeComponents.begin(), e=activeComponents.end(); it!=e; ++it)
		(*it)->SetExtertnalValue(name, value);
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