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

#ifndef __DAVAENGINE_ACTION_COMPONENT_H__
#define __DAVAENGINE_ACTION_COMPONENT_H__

#include "Entity/Component.h"

namespace DAVA
{
	class Entity;
	class ActionComponent : public Component
	{
	public:
		
		struct Action
		{
			enum eType
			{
				TYPE_NONE = 0,
				TYPE_PARTICLE_EFFECT,
				TYPE_SOUND
			};
			
			eType type;
			float32 delay;
			String entityName;
			
			Action() : type(TYPE_NONE), delay(0.0f)
			{
			}
			
			const Action& operator=(const Action& action)
			{
				type = action.type;
				delay = action.delay;
				entityName = action.entityName;
				
				return *this;
			}
		};
		
	public:
		
		ActionComponent();
		virtual ~ActionComponent();
		
		void Start();
		bool IsStarted();
		void Stop();
		
		void Add(ActionComponent::Action action);
		void Remove(const ActionComponent::Action& action);
		void Remove(const ActionComponent::Action::eType type, const String& entityName);
		uint32 GetCount();
		const ActionComponent::Action& Get(uint32 index);
		
		void Update(float32 timeElapsed);
		
		virtual Component * Clone(Entity * toEntity);
		virtual void Serialize(KeyedArchive *archive, SceneFileV2 *sceneFile);
		virtual void Deserialize(KeyedArchive *archive, SceneFileV2 *sceneFile);
		
		static ActionComponent::Action MakeAction(ActionComponent::Action::eType type, String targetName, float32 delay);
		
		IMPLEMENT_COMPONENT_TYPE(ACTION_COMPONENT);
		
	private:
		
		void EvaluateAction(const Action& action);
		
		void OnActionParticleEffect(const Action& action);
		void OnActionSound(const Action& action);
		
		Entity* GetTargetEntity(const String& name, Entity* parent);
		
	private:
		
		struct ActionContainer
		{
			Action action;
			float32 timer;
			bool active;
			
			ActionContainer() : timer(0.0f), active(false)
			{
			}
			
			ActionContainer(const Action& srcAction) : timer(0.0f), active(false)
			{
				action = srcAction;
			}
		};
		
		Vector<ActionComponent::ActionContainer> actions;
		bool started;
		bool allActionsActive; //skip processing when all actions are active
	};
};

#endif /* defined(__DAVAENGINE_ACTION_COMPONENT_H__) */
