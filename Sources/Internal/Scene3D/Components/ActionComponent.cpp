/*==================================================================================
 Copyright (c) 2008, DAVA, INC
 All rights reserved.
 
 Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
 * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
 * Neither the name of the DAVA, INC nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.
 
 THIS SOFTWARE IS PROVIDED BY THE DAVA, INC AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL DAVA, INC BE LIABLE FOR ANY
 DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 =====================================================================================*/

#include "Scene3D/Entity.h"
#include "Particles/ParticleEmitter.h"
#include "Scene3D/Components/ActionComponent.h"
#include "Scene3D/Components/ParticleEffectComponent.h"
#include "Scene3D/Components/SoundComponent.h"
#include "Sound/SoundEvent.h"

namespace DAVA
{

	ActionComponent::ActionComponent() : started(false), allActionsActive(false)
	{
		
	}
	
	ActionComponent::~ActionComponent()
	{
	}
	
	ActionComponent::Action ActionComponent::MakeAction(ActionComponent::Action::eType type, String targetName, float32 delay)
	{
		Action action;
		
		action.type = type;
		action.entityName = targetName;
		action.delay = delay;
		
		return action;
	}
	
	void ActionComponent::Start()
	{
		Stop();
		
		started = true;
		allActionsActive = false;
	}
	
	bool ActionComponent::IsStarted()
	{
		return started;
	}
	
	void ActionComponent::Stop()
	{
		if(started)
		{
			started = false;
			allActionsActive = false;
			
			uint32 count = actions.size();
			for(uint32 i = 0; i < count; ++i)
			{
				actions[i].active = false;
				actions[i].timer = 0.0f;
			}
		}
	}
	
	void ActionComponent::Add(ActionComponent::Action action)
	{
		actions.push_back(ActionContainer(action));
		allActionsActive = false;
	}
	
	void ActionComponent::Remove(const ActionComponent::Action::eType type, const String& entityName)
	{
		Vector<ActionComponent::ActionContainer>::iterator i = actions.begin();
		for(; i < actions.end(); ++i)
		{
			const Action& innerAction = (*i).action;
			if(innerAction.type == type &&
			   innerAction.entityName == entityName)
			{
				actions.erase(i);
				break;
			}
		}
		
		uint32 activeActionCount = 0;
		uint32 count = actions.size();
		for(uint32 i = 0; i < count; ++i)
		{
			if(actions[i].active)
			{
				activeActionCount++;
			}
		}
		
		allActionsActive = (activeActionCount == count);

	}
	
	void ActionComponent::Remove(const ActionComponent::Action& action)
	{
		Remove(action.type, action.entityName);
	}
	
	uint32 ActionComponent::GetCount()
	{
		return actions.size();
	}
	
	const ActionComponent::Action& ActionComponent::Get(uint32 index)
	{
		DVASSERT(index >= 0 && index < actions.size());
		return actions[index].action;
	}
	
	void ActionComponent::Update(float32 timeElapsed)
	{
		//do not evaluate time if all actions started
		if(started && !allActionsActive)
		{
			uint32 activeActionCount = 0;
			uint32 count = actions.size();
			for(uint32 i = 0; i < count; ++i)
			{
				ActionContainer& container = actions[i];
				if(!container.active)
				{
					container.timer += timeElapsed;
					
					if(container.timer >= container.action.delay)
					{
						container.active = true;
						EvaluateAction(container.action);
					}
				}
				
				if(container.active)
				{
					activeActionCount++;
				}
			}
			
			allActionsActive = (activeActionCount == count);
		}
	}
	
	Component * ActionComponent::Clone(Entity * toEntity)
	{
		ActionComponent* actionComponent = new ActionComponent();
		actionComponent->SetEntity(toEntity);
		
		uint32 count = actions.size();
		for(uint32 i = 0; i < count; ++i)
		{
			actionComponent->actions[i] = actions[i];
		}
		
		return actionComponent;
	}
	
	void ActionComponent::Serialize(KeyedArchive *archive, SceneFileV2 *sceneFile)
	{
		Component::Serialize(archive, sceneFile);
		
		if(NULL != archive)
		{
			uint32 count = actions.size();
			archive->SetUInt32("ac.actionCount", count);
			
			for(uint32 i = 0; i < count; ++i)
			{
				KeyedArchive* actionArchive = new KeyedArchive();
				
				actionArchive->SetFloat("act.delay", actions[i].action.delay);
				actionArchive->SetUInt32("act.type", actions[i].action.type);
				actionArchive->SetString("act.entityName", actions[i].action.entityName);
			
				archive->SetArchive(KeyedArchive::GenKeyFromIndex(i), actionArchive);
				SafeRelease(actionArchive);
			}
		}
	}
	
	void ActionComponent::Deserialize(KeyedArchive *archive, SceneFileV2 *sceneFile)
	{
		actions.clear();
		
		if(NULL != archive)
		{
			uint32 count = archive->GetUInt32("ac.actionCount");
			for(uint32 i = 0; i < count; ++i)
			{
				KeyedArchive* actionArchive = archive->GetArchive(KeyedArchive::GenKeyFromIndex(i));
				
				Action action;
				action.type = (Action::eType)actionArchive->GetUInt32("act.type");
				action.delay = actionArchive->GetFloat("act.delay");
				action.entityName = actionArchive->GetString("act.entityName");
				
				Add(action);
			}
		}
		
		Component::Deserialize(archive, sceneFile);
	}

	
	void ActionComponent::EvaluateAction(const Action& action)
	{
		if(Action::TYPE_PARTICLE_EFFECT == action.type)
		{
			OnActionParticleEffect(action);
		}
		else if(Action::TYPE_SOUND == action.type)
		{
			OnActionSound(action);
		}
	}
	
	void ActionComponent::OnActionParticleEffect(const Action& action)
	{
		Entity* target = GetTargetEntity(action.entityName, entity);
		
		if(target != NULL)
		{
			ParticleEffectComponent* component = static_cast<ParticleEffectComponent*>(target->GetComponent(Component::PARTICLE_EFFECT_COMPONENT));
			if(component)
			{
				component->Start();
			}
		}
	}
	
	void ActionComponent::OnActionSound(const Action& action)
	{
		Entity* target = GetTargetEntity(action.entityName, entity);
		
		if(target != NULL)
		{
			SoundComponent* component = static_cast<SoundComponent*>(target->GetComponent(Component::SOUND_COMPONENT));
			if(component)
			{
				SoundEvent* soundEvent = component->GetSoundEvent();
				if(soundEvent)
				{
					soundEvent->Play();
				}
			}

		}
	}
	
	Entity* ActionComponent::GetTargetEntity(const String& name, Entity* parent)
	{
		if(parent->GetName() == name)
		{
			return parent;
		}
		
		return parent->FindByName(name);		
	}

};