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

#ifndef __DAVAENGINE_SKYBOXNODE_H__
#define __DAVAENGINE_SKYBOXNODE_H__

#include "Base/BaseObject.h"
#include "Base/BaseTypes.h"
#include "Scene3D/Entity.h"
#include "Render/Highlevel/RenderBatch.h"

namespace DAVA
{
	class Material;
	class RenderDataStream;
	class SkyBoxNode : public Entity
	{
	private:
		
		class SkyBoxRenderBatch : public RenderBatch
		{
		private:
			
			RenderDataStream* positionStream;
			RenderDataStream* texCoordStream;
			float32 nonClippingDistance;
			float32 zOffset;
			
		public:
			
			SkyBoxRenderBatch();
			~SkyBoxRenderBatch();
			
			void SetBox(const AABBox3& box);
			void SetVerticalOffset(float32 verticalOffset);
			
			virtual void Draw(DAVA::Camera * camera);
		};
		
	private:
		
		FilePath texturePath;
		Vector3 boxSize;
		SkyBoxRenderBatch* renderBatch;
		Material* skyBoxMaterial;
		float32 zShift;
		
	private:
		
		void UpdateSkyBoxSize();
		void BuildSkyBox();
				
	public:
		
		SkyBoxNode();
		virtual ~SkyBoxNode();
			
		virtual void SceneDidLoaded(); //initialization happens here
		
		virtual void Save(KeyedArchive * archive, SceneFileV2 * sceneFileV2);
		virtual void Load(KeyedArchive * archive, SceneFileV2 * sceneFileV2);
		virtual Entity* Clone(Entity *dstNode);
		
		void SetTexture(const FilePath& texPath);
		FilePath GetTexture();
		void SetVerticalOffset(const float32& verticalOffset);
		float32 GetVerticalOffset();

		
		INTROSPECTION_EXTEND(SkyBoxNode, Entity,
							 PROPERTY("texture", "Texture Path", GetTexture, SetTexture, I_SAVE | I_VIEW | I_EDIT)
							 PROPERTY("verticalOffset", "Vertical Offset", GetVerticalOffset, SetVerticalOffset, I_SAVE | I_VIEW | I_EDIT)
							 );

	};
};

#endif /* defined(__DAVAENGINE_SKYBOXNODE_H__) */
