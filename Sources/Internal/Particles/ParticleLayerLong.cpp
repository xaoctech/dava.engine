#include "Particles/ParticleLayerLong.h"
#include "Render/RenderDataObject.h"
#include "Render/RenderManager.h"
#include "Render/Material.h"
#include "Render/Highlevel/Camera.h"

namespace DAVA
{

ParticleLayerLong::ParticleLayerLong()
{
	renderBatch->GetMaterial()->SetTwoSided(true);
}

void ParticleLayerLong::Draw(Camera * camera)
{
	verts.clear();
	textures.clear();
	colors.clear();
	int32 totalCount = 0;

	Particle * current = head;
	if(current)
	{
		renderBatch->GetMaterial()->GetRenderState()->SetTexture(sprite->GetTexture(current->frame));
	}

	Vector3 direction = camera->GetDirection();
	while(current != 0)
	{
		Vector3 vecShort = current->direction.CrossProduct(direction);
		vecShort /= 2.f;

		Vector3 vecLong = -current->direction;

		float32 widthDiv2 = sprite->GetWidth()*current->size.x*current->sizeOverLife;
		float32 heightDiv2 = sprite->GetHeight()*current->size.y*current->sizeOverLife;

		Vector3 topRight = current->position + widthDiv2*vecShort;
		Vector3 topLeft = current->position - widthDiv2*vecShort;
		Vector3 botRight = topRight + heightDiv2*vecLong;
		Vector3 botLeft = topLeft + heightDiv2*vecLong;

		verts.push_back(topLeft.x);//0
		verts.push_back(topLeft.y);
		verts.push_back(topLeft.z);

		verts.push_back(topRight.x);//1
		verts.push_back(topRight.y);
		verts.push_back(topRight.z);

		verts.push_back(botLeft.x);//2
		verts.push_back(botLeft.y);
		verts.push_back(botLeft.z);

		verts.push_back(botLeft.x);//2
		verts.push_back(botLeft.y);
		verts.push_back(botLeft.z);

		verts.push_back(topRight.x);//1
		verts.push_back(topRight.y);
		verts.push_back(topRight.z);

		verts.push_back(botRight.x);//3
		verts.push_back(botRight.y);
		verts.push_back(botRight.z);

		float32 *pT = sprite->GetTextureVerts(current->frame);

		textures.push_back(pT[0]);
		textures.push_back(pT[1]);

		textures.push_back(pT[2]);
		textures.push_back(pT[3]);

		textures.push_back(pT[4]);
		textures.push_back(pT[5]);

		textures.push_back(pT[4]);
		textures.push_back(pT[5]);

		textures.push_back(pT[2]);
		textures.push_back(pT[3]);

		textures.push_back(pT[6]);
		textures.push_back(pT[7]);

		uint32 color = (((uint32)(current->color.a*255.f))<<24) |  (((uint32)(current->color.b*255.f))<<16) | (((uint32)(current->color.g*255.f))<<8) | ((uint32)(current->color.r*255.f));
		for(int32 i = 0; i < 6; ++i)
		{
			colors.push_back(color);
		}

		totalCount++;
		current = TYPE_PARTICLES == type ? current->next : 0;
	}

	if(totalCount > 0)
	{			
		renderData->SetStream(EVF_VERTEX, TYPE_FLOAT, 3, 0, &verts.front());
		renderData->SetStream(EVF_TEXCOORD0, TYPE_FLOAT, 2, 0, &textures.front());
		renderData->SetStream(EVF_COLOR, TYPE_UNSIGNED_BYTE, 4, 0, &colors.front());
		
		RenderManager::Instance()->SetRenderData(renderData);
		renderBatch->GetMaterial()->PrepareRenderState();

		RenderManager::Instance()->HWDrawArrays(PRIMITIVETYPE_TRIANGLELIST, 0, 6*totalCount);
	}
}



};
