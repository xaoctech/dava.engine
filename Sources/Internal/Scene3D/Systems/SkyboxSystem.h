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

#ifndef __DAVAENGINE_SKYBOX_SYSTEM_H__
#define __DAVAENGINE_SKYBOX_SYSTEM_H__

#include "Base/BaseTypes.h"
#include "Math/MathConstants.h"
#include "Math/Matrix4.h"
#include "Base/Singleton.h"
#include "Scene3D/Systems/BaseProcessSystem.h"
#include "Render/Highlevel/RenderBatch.h"


namespace DAVA
{
	class Material;
	class Entity;
	class Scene;
	class SkyboxSystem : public BaseProcessSystem
	{
	private:
		
		enum SkyboxState
		{
			SYSTEM_NONE = 0,
			SYSTEM_INITIAL_STATE,
			SYSTEM_DIRTY,
			SYSTEM_READY
		};
		
		class SkyBoxRenderBatch : public RenderBatch
		{
		private:
			
			RenderDataStream* positionStream;
			RenderDataStream* texCoordStream;
			float32 nonClippingDistance;
			float32 zOffset;
			float32 rotation;
			
		public:
			
			SkyBoxRenderBatch();
			~SkyBoxRenderBatch();
			
			void SetBox(const AABBox3& box);
			void SetVerticalOffset(float32 verticalOffset);
			void SetRotation(float32 angle);
			
			virtual void Draw(DAVA::Camera * camera);
		};
		
	private:
		
		SkyboxState state;
		Entity* skyboxEntity;
		SkyBoxRenderBatch* skyboxRenderBatch;
		Material* skyboxMaterial;
		
	private:
		
		void CreateSkybox();
		void UpdateSkybox();
		
	public:
		SkyboxSystem(Scene* scene);
		~SkyboxSystem();
		virtual void Process();
		
		virtual void AddEntity(Entity * entity);
		virtual void RemoveEntity(Entity * entity);
		
		void SetSystemStateDirty();
	};
};

#endif //__DAVAENGINE_SKYBOX_SYSTEM_H__
