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

#define MAKE_BLEND_KEY(SRC, DST) (SRC | DST << 16)

namespace DAVA
{


static const uint32 BLEND_KEYS[] =
{
	MAKE_BLEND_KEY(BLEND_SRC_ALPHA, BLEND_ONE),
	MAKE_BLEND_KEY(BLEND_SRC_ALPHA, BLEND_ONE_MINUS_SRC_ALPHA)
};

static const FastName BLEND_MATERIAL_NAMES[] =
{
	FastName("Global.Textured.VertexColor.Particles0"),
	FastName("Global.Textured.VertexColor.Particles1")
};

static const FastName FRAMEBLEND_MATERIAL_NAMES[] =
{
	FastName("Global.Textured.VertexColor.ParticlesFrameBlend0"),
	FastName("Global.Textured.VertexColor.ParticlesFrameBlend1")
};


const FastName& FindParticleMaterial(eBlendMode src, eBlendMode dst, bool frameBlendEnabled)
{
	uint32 blendKey = MAKE_BLEND_KEY(src, dst);
	for(uint32 i = 0; i < COUNT_OF(BLEND_KEYS); ++i)
	{
		if(BLEND_KEYS[i] == blendKey)
		{
			return (frameBlendEnabled) ? FRAMEBLEND_MATERIAL_NAMES[i] : BLEND_MATERIAL_NAMES[i];
		}
	}

	//VI: fallback to default material
	return (frameBlendEnabled) ? FRAMEBLEND_MATERIAL_NAMES[0] : BLEND_MATERIAL_NAMES[0];
}


ParticleEffectSystem::ParticleEffectSystem(Scene * scene) :	SceneSystem(scene)	
{
}

void ParticleEffectSystem::Process()
{    
	if(!RenderManager::Instance()->GetOptions()->IsOptionEnabled(RenderOptions::UPDATE_PARTICLE_EMMITERS)) 
		return;
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
		bool effectEnded = component->stopWhenEmpty?component->effectData.groups.empty():(component->time>component->effectDuration);
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
	const Matrix4 &worldTransform = effect->GetEntity()->GetWorldTransform();
	

	AABBox3 bbox;
	for (List<ParticleGroup>::iterator it = effect->effectData.groups.begin(), e=effect->effectData.groups.end(); it!=e;++it)
	{
		ParticleGroup &group = *it;
		group.activeParticleCount = 0;
		float32 dt = group.emitter->shortEffect?shortEffectTime:time;
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
		//prepare forces as they will now actually change in time even for already generated particles
		Vector<Vector3> currForceValues;
		int32 forcesCount;
		if (group.head)
		{
			forcesCount = group.layer->forces.size();
			if (forcesCount)
			{
				currForceValues.resize(forcesCount);
				for (int32 i=0; i<forcesCount; ++i)
					currForceValues[i]=group.layer->forces[i]->force->GetValue(currLoopTime);
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
			group.activeParticleCount++;

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
					acceleration += (group.layer->forces[i]->forceOverLife)?(currForceValues[i] * group.layer->forces[i]->forceOverLife->GetValue(overLifeTime)) : currForceValues[i];				
				}
				current->speed+=acceleration*dt;
			}		
			
			if (group.layer->sizeOverLifeXY)
			{
				current->currSize=current->baseSize*group.layer->sizeOverLifeXY->GetValue(current->life);
				Vector2 pivotSize = current->currSize*group.layer->layerPivotSizeOffsets;
				current->currRadius = pivotSize.Length();
			}
			AddParticleToBBox(current, bbox);
			
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
		allowParticleGeneration&=group.visibleLod;
		if (allowParticleGeneration)
		{
			if (group.layer->type == ParticleLayer::TYPE_SINGLE_PARTICLE)
			{
				if (!group.head)
				{
					current = GenerateNewParticle(group, currLoopTime, worldTransform);																
					AddParticleToBBox(current, bbox);
				}
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
					current = GenerateNewParticle(group, currLoopTime, worldTransform);																
					AddParticleToBBox(current, bbox);
				}
			}
			
		}

		/*finally*/
		if (group.finishingGroup&&(!group.head))
			it = effect->effectData.groups.erase(it);
		else
			++it;
	}
	if (bbox.IsEmpty())
	{
		Vector3 pos = worldTransform.GetTranslationVector();
		bbox = AABBox3(pos, pos);
	}
	effect->effectRenderObject->SetAABBox(bbox);
}

void ParticleEffectSystem::AddParticleToBBox(Particle *particle, AABBox3& bbox)
{	
	Vector3 sz = Vector3(particle->currRadius, particle->currRadius, particle->currRadius);			
	bbox.AddPoint(particle->position-sz);
	bbox.AddPoint(particle->position+sz);
}


Particle* ParticleEffectSystem::GenerateNewParticle(ParticleGroup& group, float32 currLoopTime, const Matrix4 &worldTransform)
{
	Particle *particle = new Particle();		
	particle->life = 0.0f;	

	particle->color = Color();
	if (group.layer->colorRandom)
	{
		particle->color = group.layer->colorRandom->GetValue(Random::Instance()->RandFloat());
	}
	if (group.emitter->colorOverLife)
	{
		particle->color*=group.emitter->colorOverLife->GetValue(group.time);
	}

	particle->lifeTime = 0.0f;
	if (group.layer->life)
		particle->lifeTime += group.layer->life->GetValue(currLoopTime);
	if (group.layer->lifeVariation)
		particle->lifeTime += (group.layer->lifeVariation->GetValue(currLoopTime) * Random::Instance()->RandFloat());

	// size 
	particle->baseSize = Vector2(1.0f, 1.0f); 
	if (group.layer->size)
		particle->baseSize = group.layer->size->GetValue(currLoopTime);
	if (group.layer->sizeVariation)
		particle->baseSize +=(group.layer->sizeVariation->GetValue(currLoopTime) * Random::Instance()->RandFloat());
	particle->currSize = particle->baseSize;
	if (group.layer->sizeOverLifeXY)
		particle->currSize*=group.layer->sizeOverLifeXY->GetValue(0);
	Vector2 pivotSize = particle->currSize*group.layer->layerPivotSizeOffsets;
	particle->currRadius = pivotSize.Length();


	//TODO: superemitter stuff - rethink later
	/*if (emitter->parentParticle){
		particle->size.x*=emitter->parentParticle->size.x;
		particle->size.y*=emitter->parentParticle->size.y;
	}*/	

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
	
	
	
	PrepareEmitterParameters(particle, group, worldTransform);
	
	float32 vel = 0.0f;
	if (group.layer->velocity)
		vel += group.layer->velocity->GetValue(currLoopTime);
	if (group.layer->velocityVariation)
		vel += (group.layer->velocityVariation->GetValue(currLoopTime) * Random::Instance()->RandFloat());
	particle->speed*=vel;
	
	//TODO: superemitter stuff
	/*if (this->emitter&&!inheritPosition) //just generate at correct position
	{		
		particle->position += emitter->GetPosition()-emitter->GetInitialTranslationVector();
	}*/		
		
	particle->next = group.head;
	group.head = particle;	
	group.activeParticleCount++;
	return particle;
}

void ParticleEffectSystem::PrepareEmitterParameters(Particle * particle, ParticleGroup &group, const Matrix4 &worldTransform)
{	
	//calculate position new particle position in emitter space (for point leave it V3(0,0,0))
	if (group.emitter->emitterType == ParticleEmitter::EMITTER_RECT)
	{        
		if (group.emitter->size)
		{
			Vector3 currSize = group.emitter->size->GetValue(group.time);
			particle->position = Vector3(currSize.x*(Random::Instance()->RandFloat() - 0.5f), currSize.y*(Random::Instance()->RandFloat() - 0.5f), currSize.z*(Random::Instance()->RandFloat() - 0.5f));
		}				
	}
	else if ((group.emitter->emitterType == ParticleEmitter::EMITTER_ONCIRCLE_VOLUME) || (group.emitter->emitterType == ParticleEmitter::EMITTER_ONCIRCLE_EDGES)||(group.emitter->emitterType == ParticleEmitter::EMITTER_SHOCKWAVE))
	{
		float32 curRadius = 1.0f;
		if (group.emitter->radius)		
			curRadius = group.emitter->radius->GetValue(group.time);		
		float32 curAngle = PI_2 * (float32)Random::Instance()->RandFloat();
		if (group.emitter->emitterType == ParticleEmitter::EMITTER_ONCIRCLE_VOLUME)		
			curRadius *= (float32)Random::Instance()->RandFloat();		
		float sinAngle = 0.0f;
		float cosAngle = 0.0f;
		SinCosFast(curAngle, sinAngle, cosAngle);
		particle->position = Vector3(curRadius * cosAngle, curRadius * sinAngle, 0.0f);
	}
	particle->position += group.emitter->position;
	//current emission vector and it's length
	Vector3 currEmissionVector(0,0,1);
	if (group.emitter->emissionVector)
		currEmissionVector = group.emitter->emissionVector->GetValue(group.time);
	float32 currEmissionPower = currEmissionVector.Length();
	//calculate speed in emitter space not transformed by emission vector yet
	if (group.emitter->emitterType == ParticleEmitter::EMITTER_SHOCKWAVE)
	{
		particle->speed = particle->position;
		float32 spl = particle->speed.SquareLength();
		if (spl>EPSILON)
		{
			particle->speed*=currEmissionPower/sqrtf(spl);
		}
	}
	else
	{
		if (group.emitter->emissionRange)
		{
			float32 theta = Random::Instance()->RandFloat()*group.emitter->emissionRange->GetValue(group.time);
			float phi = Random::Instance()->RandFloat() * PI_2;
			particle->speed = Vector3(currEmissionPower* cos(phi) * sin(theta), currEmissionPower * sin (phi) * sin(theta), currEmissionPower * cos(theta));			
		}
		else
		{
			particle->speed = Vector3(0, 0, currEmissionPower);
		}
	}

	//now transform position and speed by emissionVector and worldTransfrom rotations - preserving length	
	Matrix3 newTransform(worldTransform);
	if ((fabs(currEmissionVector.x)<EPSILON)&&(fabs(currEmissionVector.z)<EPSILON))
	{
		if (currEmissionVector.z<0)
		{
			Matrix3 rotation;
			rotation.CreateRotation(Vector3(1,0,0), PI);
			newTransform = rotation*newTransform;
		}
	}
	else
	{
		Vector3 axis(currEmissionVector.y, -currEmissionVector.x, 0);
		axis.Normalize();
		float32 angle = acosf(currEmissionVector.z/currEmissionPower);
		Matrix3 rotation;
		rotation.CreateRotation(axis, angle);
		newTransform = rotation*newTransform;
	}
	TransformPerserveLength(particle->speed, newTransform);
	TransformPerserveLength(particle->position, newTransform); //note - from now emitter position is not effected by scale anymore (artist request)
	particle->position+=worldTransform.GetTranslationVector();		
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