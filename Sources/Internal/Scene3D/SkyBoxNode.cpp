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

#include "Scene3D/SkyBoxNode.h"
#include "Render/Material.h"
#include "Scene3D/Components/RenderComponent.h"
#include "Entity/Component.h"
#include "Render/Highlevel/RenderFastNames.h"

//do not create lower cube face
#define SKYBOX_VERTEX_COUNT (5 * 6)

namespace DAVA
{
	REGISTER_CLASS(SkyBoxNode);
	
	SkyBoxNode::SkyBoxNode()	:	renderBatch(NULL),
									skyBoxMaterial(NULL),
									zShift(0.0f),
									boxSize(1.0f, 1.0f, 1.0f),
									rotationAngle(0.0f)
	{
	}
	
	SkyBoxNode::~SkyBoxNode()
	{
		SafeRelease(skyBoxMaterial);
		SafeRelease(renderBatch);
	}
	
	void SkyBoxNode::BuildSkyBox()
	{
		if(NULL == renderBatch)
		{
			RenderDataObject* renderDataObj = new RenderDataObject();
			
			skyBoxMaterial = new Material();
			skyBoxMaterial->SetType(Material::MATERIAL_SKYBOX);
			skyBoxMaterial->SetAlphablend(false);
			skyBoxMaterial->SetName("SkyBox_material");
			skyBoxMaterial->GetRenderState()->SetDepthFunc(CMP_LEQUAL);
			skyBoxMaterial->GetRenderState()->state |= RenderState::STATE_DEPTH_TEST;
			skyBoxMaterial->GetRenderState()->state &= ~RenderState::STATE_DEPTH_WRITE;
			
			renderBatch = new SkyBoxRenderBatch();
			renderBatch->SetRenderDataObject(renderDataObj);
			renderBatch->SetMaterial(skyBoxMaterial);
			SafeRelease(renderDataObj);
			
			RenderObject* renderObj = new RenderObject();
			renderObj->AddRenderBatch(renderBatch);
			
			RenderComponent* renderComponent = new RenderComponent();
			renderComponent->SetRenderObject(renderObj);
			renderComponent->SetEntity(this);
			AddComponent(renderComponent);
			
			SafeRelease(renderObj);
		}
		
		skyBoxMaterial->SetTexture(Material::TEXTURE_DIFFUSE, DAVA::Texture::CreateFromFile(texturePath));
		UpdateSkyBoxSize();
	}
	
	void SkyBoxNode::UpdateSkyBoxSize()
	{
		DVASSERT(renderBatch != NULL);
		DVASSERT(boxSize.Length() > 0);
				 
		AABBox3 box(Vector3(-0.5 * boxSize.x, -0.5 * boxSize.y, -0.5 * boxSize.z),
					Vector3(0.5 * boxSize.x, 0.5 * boxSize.y, 0.5 * boxSize.z));
		
		renderBatch->SetBox(box);
		renderBatch->SetVerticalOffset(zShift);
		renderBatch->SetRotation(rotationAngle);
	}
	
	void SkyBoxNode::SceneDidLoaded()
	{
		BuildSkyBox();
	}
	
	void SkyBoxNode::Save(KeyedArchive * archive, SceneFileV2 * sceneFileV2)
	{
		Entity::Save(archive, sceneFileV2);

		if(archive != NULL)
		{
			archive->SetString("sbn.texture", texturePath.GetRelativePathname());
			archive->SetFloat("sbn.verticalOffset", zShift);
			archive->SetFloat("sbn.rotation", rotationAngle);
		}
	}
	
	void SkyBoxNode::Load(KeyedArchive * archive, SceneFileV2 * sceneFileV2)
	{
		Entity::Load(archive, sceneFileV2);
		
		if(archive != NULL)
		{
			SetTexture(archive->GetString("sbn.texture"));
			SetVerticalOffset(archive->GetFloat("sbn.verticalOffset"));
			SetRotationAngle(archive->GetFloat("sbn.rotation"));
		}
	}
	
	Entity* SkyBoxNode::Clone(Entity *dstNode)
	{
		return SafeRetain(this);
	}
	
	void SkyBoxNode::SetTexture(const FilePath& texPath)
	{
		texturePath = texPath;
		BuildSkyBox();
	}
	
	FilePath SkyBoxNode::GetTexture()
	{
		return texturePath;
	}
	
	void SkyBoxNode::SetVerticalOffset(const float32& verticalOffset)
	{
		zShift = verticalOffset;
		if(renderBatch != NULL)
		{
			renderBatch->SetVerticalOffset(zShift);
		}
	}
	
	float32 SkyBoxNode::GetVerticalOffset()
	{
		return zShift;
	}
	
	void SkyBoxNode::SetRotationAngle(const float32& rotation)
	{
		rotationAngle = rotation;
		if(renderBatch != NULL)
		{
			renderBatch->SetRotation(rotationAngle);
		}
	}
	
	float32 SkyBoxNode::GetRotationAngle()
	{
		return rotationAngle;
	}


	
	//////////////////////////////////////////////////////////////////////////////////////
	
	SkyBoxNode::SkyBoxRenderBatch::SkyBoxRenderBatch()	:	positionStream(NULL),
															texCoordStream(NULL),
															nonClippingDistance(0.0f),
															zOffset(0.0f),
															rotation(0.0f)
	{
		SetOwnerLayerName(LAYER_AFTER_OPAQUE);
	}
	
	SkyBoxNode::SkyBoxRenderBatch::~SkyBoxRenderBatch()
	{
		//don't delete streams - they are part of renderdataobject and will be free'd up with it
		positionStream = NULL;
		texCoordStream = NULL;
	}
	
	void SkyBoxNode::SkyBoxRenderBatch::SetVerticalOffset(float32 verticalOffset)
	{
		zOffset = verticalOffset;
	}
	
	void SkyBoxNode::SkyBoxRenderBatch::SetRotation(float32 angle)
	{
		rotation = DAVA::DegToRad(angle);
	}
	
	void SkyBoxNode::SkyBoxRenderBatch::SetBox(const AABBox3& box)
	{
		RenderDataObject* renderDataObj = GetRenderDataObject();
		
		if(NULL == positionStream)
		{
			positionStream = renderDataObj->SetStream(EVF_VERTEX, TYPE_FLOAT, 3, 0, NULL);
		}
		
		if(NULL == texCoordStream)
		{
			texCoordStream = renderDataObj->SetStream(EVF_TEXCOORD0, TYPE_FLOAT, 3, 0, NULL);
		}
		
		static Vector3 cubeVertices[SKYBOX_VERTEX_COUNT];
		static Vector3 cubeTexCoords[SKYBOX_VERTEX_COUNT] = {
			Vector3(-1, -1, -1), Vector3(-1, -1, 1), Vector3(1, -1, -1), Vector3(1, -1, -1), Vector3(-1, -1, 1), Vector3(1, -1, 1),
			Vector3(1, -1, -1), Vector3(1, -1, 1), Vector3(1, 1, -1), Vector3(1, 1, -1), Vector3(1, -1, 1), Vector3(1, 1, 1),
			Vector3(1, 1, -1), Vector3(1, 1, 1), Vector3(-1, 1, -1), Vector3(-1, 1, -1), Vector3(1, 1, 1), Vector3(-1, 1, 1),
			Vector3(-1, 1, -1), Vector3(-1, 1, 1), Vector3(-1, -1, -1), Vector3(-1, -1, -1), Vector3(-1, 1, 1), Vector3(-1, -1, 1),
			Vector3(-1, -1, 1), Vector3(-1, 1, 1), Vector3(1, -1, 1), Vector3(1, -1, 1), Vector3(-1, 1, 1), Vector3(1, 1, 1)
		};
				
		//face 0
		
		cubeVertices[0].x = box.min.x;
		cubeVertices[0].y = box.min.y;
		cubeVertices[0].z = box.min.z;

		cubeVertices[1].x = box.min.x;
		cubeVertices[1].y = box.min.y;
		cubeVertices[1].z = box.max.z;

		cubeVertices[2].x = box.max.x;
		cubeVertices[2].y = box.min.y;
		cubeVertices[2].z = box.min.z;

		cubeVertices[3].x = box.max.x;
		cubeVertices[3].y = box.min.y;
		cubeVertices[3].z = box.min.z;

		cubeVertices[4].x = box.min.x;
		cubeVertices[4].y = box.min.y;
		cubeVertices[4].z = box.max.z;

		cubeVertices[5].x = box.max.x;
		cubeVertices[5].y = box.min.y;
		cubeVertices[5].z = box.max.z;

		//face 1
		
		cubeVertices[6].x = box.max.x;
		cubeVertices[6].y = box.min.y;
		cubeVertices[6].z = box.min.z;

		cubeVertices[7].x = box.max.x;
		cubeVertices[7].y = box.min.y;
		cubeVertices[7].z = box.max.z;

		cubeVertices[8].x = box.max.x;
		cubeVertices[8].y = box.max.y;
		cubeVertices[8].z = box.min.z;

		cubeVertices[9].x = box.max.x;
		cubeVertices[9].y = box.max.y;
		cubeVertices[9].z = box.min.z;

		cubeVertices[10].x = box.max.x;
		cubeVertices[10].y = box.min.y;
		cubeVertices[10].z = box.max.z;
		
		cubeVertices[11].x = box.max.x;
		cubeVertices[11].y = box.max.y;
		cubeVertices[11].z = box.max.z;


		//face 2
		
		cubeVertices[12].x = box.max.x;
		cubeVertices[12].y = box.max.y;
		cubeVertices[12].z = box.min.z;

		cubeVertices[13].x = box.max.x;
		cubeVertices[13].y = box.max.y;
		cubeVertices[13].z = box.max.z;

		cubeVertices[14].x = box.min.x;
		cubeVertices[14].y = box.max.y;
		cubeVertices[14].z = box.min.z;

		cubeVertices[15].x = box.min.x;
		cubeVertices[15].y = box.max.y;
		cubeVertices[15].z = box.min.z;

		cubeVertices[16].x = box.max.x;
		cubeVertices[16].y = box.max.y;
		cubeVertices[16].z = box.max.z;
		
		cubeVertices[17].x = box.min.x;
		cubeVertices[17].y = box.max.y;
		cubeVertices[17].z = box.max.z;

		//face 3
		
		cubeVertices[18].x = box.min.x;
		cubeVertices[18].y = box.max.y;
		cubeVertices[18].z = box.min.z;
		
		cubeVertices[19].x = box.min.x;
		cubeVertices[19].y = box.max.y;
		cubeVertices[19].z = box.max.z;

		cubeVertices[20].x = box.min.x;
		cubeVertices[20].y = box.min.y;
		cubeVertices[20].z = box.min.z;

		cubeVertices[21].x = box.min.x;
		cubeVertices[21].y = box.min.y;
		cubeVertices[21].z = box.min.z;
		
		cubeVertices[22].x = box.min.x;
		cubeVertices[22].y = box.max.y;
		cubeVertices[22].z = box.max.z;

		cubeVertices[23].x = box.min.x;
		cubeVertices[23].y = box.min.y;
		cubeVertices[23].z = box.max.z;

		//face 4 (top)
		
		cubeVertices[24].x = box.min.x;
		cubeVertices[24].y = box.min.y;
		cubeVertices[24].z = box.max.z;

		cubeVertices[25].x = box.min.x;
		cubeVertices[25].y = box.max.y;
		cubeVertices[25].z = box.max.z;
		
		cubeVertices[26].x = box.max.x;
		cubeVertices[26].y = box.min.y;
		cubeVertices[26].z = box.max.z;

		cubeVertices[27].x = box.max.x;
		cubeVertices[27].y = box.min.y;
		cubeVertices[27].z = box.max.z;

		cubeVertices[28].x = box.min.x;
		cubeVertices[28].y = box.max.y;
		cubeVertices[28].z = box.max.z;

		cubeVertices[29].x = box.max.x;
		cubeVertices[29].y = box.max.y;
		cubeVertices[29].z = box.max.z;
		
		//could be any pair of opposite diagonal vertices
		Vector3 maxDistanceBetweenVertices = cubeVertices[29] - cubeVertices[24];
		nonClippingDistance = 0.5f * maxDistanceBetweenVertices.Length();
		
		positionStream->Set(TYPE_FLOAT, 3, 0, cubeVertices);
		texCoordStream->Set(TYPE_FLOAT, 3, 0, cubeTexCoords);
	}
	
	void SkyBoxNode::SkyBoxRenderBatch::Draw(DAVA::Camera * camera)
	{
		if(!renderObject || !renderDataObject) return;
						
		//scale cube so it's not get clipped by zNear plane
		float32 zNear = camera->GetZNear();
		float32 scale = (nonClippingDistance + zNear) / nonClippingDistance;
		
		Vector3 camPos = camera->GetPosition();
		
		camPos.z += zOffset;
		
		Matrix4 finalMatrix = Matrix4::MakeScale(Vector3(scale, scale, scale)) *
							  Matrix4::MakeTranslation(camPos) *
							  Matrix4::MakeRotation(Vector3(0.0f, 0.0f, 1.0f), rotation) *
							  camera->GetMatrix();
		RenderManager::Instance()->SetMatrix(RenderManager::MATRIX_MODELVIEW, finalMatrix);
		
		RenderManager::Instance()->SetRenderData(renderDataObject);
		material->PrepareRenderState();
		
		RenderManager::Instance()->HWDrawArrays(PRIMITIVETYPE_TRIANGLELIST, 0, SKYBOX_VERTEX_COUNT);		
	}
};