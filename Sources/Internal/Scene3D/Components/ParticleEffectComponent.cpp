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



#include "Scene3D/Components/ParticleEffectComponent.h"
#include "Scene3D/Components/RenderComponent.h"
#include "Scene3D/Entity.h"
#include "Particles/ParticleEmitter.h"
#include "Scene3D/Components/ComponentHelpers.h"

namespace DAVA
{
    
REGISTER_CLASS(ParticleEffectComponent)

ParticleEffectComponent::ParticleEffectComponent()
{
	repeatsCount = -1;
	stopWhenEmpty = true;
	effectDuration = 0.0f;	
}

Component * ParticleEffectComponent::Clone(Entity * toEntity)
{
	DVASSERT(0);
	ParticleEffectComponent * newComponent = new ParticleEffectComponent();
	newComponent->SetEntity(toEntity);
	newComponent->repeatsCount = repeatsCount;
	newComponent->stopWhenEmpty = stopWhenEmpty;	
	newComponent->playbackComplete = playbackComplete;
	newComponent->effectDuration = effectDuration;
	return newComponent;
}

void ParticleEffectComponent::Start()
{		
}

void ParticleEffectComponent::Stop(bool isDeleteAllParticles)
{

}

void ParticleEffectComponent::Pause(bool isPaused /*= true*/)
{	

}

bool ParticleEffectComponent::IsStopped()
{
	return false;
}

bool ParticleEffectComponent::IsPaused()
{
	return false;
}

void ParticleEffectComponent::Step(float32 delta)
{
	
}
	
void ParticleEffectComponent::Restart(bool isDeleteAllParticles)
{
	
}

void ParticleEffectComponent::StopAfterNRepeats(int32 numberOfRepeats)
{
    repeatsCount = numberOfRepeats;
}

void ParticleEffectComponent::StopWhenEmpty(bool value /*= true*/)
{
	stopWhenEmpty = value;
}



void ParticleEffectComponent::SetPlaybackCompleteMessage(const Message & msg)
{
	playbackComplete = msg;
}


float32 ParticleEffectComponent::GetPlaybackSpeed()
{
	return 1.0f;
}

void ParticleEffectComponent::SetPlaybackSpeed(float32 value)
{

}


void ParticleEffectComponent::SetDesiredLodLevel(int32 level)
{
	desiredLodLevel = level;
	for (List<ParticleGroup>::iterator it = effectData.groups.begin(), e=effectData.groups.end(); it!=e;++it)
	{
		ParticleGroup& group = *it;
		if (!group.emitter->shortEffect)
			group.visibleLod = group.layer->IsLodActive(level);
	}
}


void ParticleEffectComponent::SetExtertnalValue(const String& name, float32 value)
{
	externalValues[name] = value;
	for (MultiMap<String, ModifiablePropertyLineBase *>::iterator it = externalModifiables.lower_bound(name), e=externalModifiables.upper_bound(name); it!=e; ++it)
		(*it).second->SetModifier(value);
}

float32 ParticleEffectComponent::GetExternalValue(const String& name)
{
	Map<String, float32>::iterator it = externalValues.find(name);
	if (it!=externalValues.end())
		return (*it).second;
	else
		return 0.0f;
}

Set<String> ParticleEffectComponent::EnumerateVariables()
{
	Set<String> res;
	for (MultiMap<String, ModifiablePropertyLineBase *>::iterator it = externalModifiables.begin(), e=externalModifiables.end(); it!=e; ++it)
		res.insert((*it).first);
	return res;
}

void ParticleEffectComponent::RegisterModifiable(ModifiablePropertyLineBase *propertyLine)
{
	externalModifiables.insert(std::make_pair(propertyLine->GetValueName(), propertyLine));
	Map<String, float32>::iterator it = externalValues.find(propertyLine->GetValueName());
	if (it!=externalValues.end())
		propertyLine->SetModifier((*it).second);
}
void ParticleEffectComponent::UnRegisterModifiable(ModifiablePropertyLineBase *propertyLine)
{
	for (MultiMap<String, ModifiablePropertyLineBase *>::iterator it = externalModifiables.lower_bound(propertyLine->GetValueName()), e=externalModifiables.upper_bound(propertyLine->GetValueName()); it!=e; ++it)
	{
		if ((*it).second == propertyLine) 
		{
			externalModifiables.erase(it);
			return;
		}
	}
}

void ParticleEffectComponent::RebuildEffectModifiables()
{
	externalModifiables.clear();
	List<ModifiablePropertyLineBase *> modifiables;
	
	for (int32 i = 0, sz = emitters.size(); i < sz; i ++)
	{		
		emitters[i]->GetModifableLines(modifiables);					
	}

	for (List<ModifiablePropertyLineBase *>::iterator it = modifiables.begin(), e=modifiables.end(); it!=e; ++it)
	{
		externalModifiables.insert(std::make_pair((*it)->GetValueName(), (*it)));
		Map<String, float32>::iterator itName = externalValues.find((*it)->GetValueName());
		if (itName!=externalValues.end())
			(*it)->SetModifier((*itName).second);
	}	
}

	
int32 ParticleEffectComponent::GetActiveParticlesCount()
{
	int32 totalActiveParticles = 0;
	for (List<ParticleGroup>::iterator it = effectData.groups.begin(), e=effectData.groups.end(); it!=e; ++it)
		totalActiveParticles+=(*it).activeParticleCount;
	

	return totalActiveParticles;
}

void ParticleEffectComponent::Serialize(KeyedArchive *archive, SerializationContext *serializationContext)
{
	Component::Serialize(archive, serializationContext);
}

void ParticleEffectComponent::Deserialize(KeyedArchive *archive, SerializationContext *serializationContext)
{
	Component::Deserialize(archive, serializationContext);
}



}
