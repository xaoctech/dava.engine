#include "Particles/ParticleLayer3D.h"
#include "Render/RenderDataObject.h"
#include "Render/RenderManager.h"

namespace DAVA
{

ParticleLayer3D::ParticleLayer3D()
{
	renderData = new RenderDataObject();

}

ParticleLayer3D::~ParticleLayer3D()
{
	SafeRelease(renderData);
}

void ParticleLayer3D::Draw(Matrix4 * transform)
{
	if(TYPE_PARTICLES == type)
	{
		verts.clear();
		textures.clear();
		int32 totalCount = 0;

		Particle * current = head;
		if(current)
		{
			RenderManager::Instance()->SetTexture(sprite->GetTexture(current->frame));
		}

		while(current != 0)
		{
			float32 halfW = sprite->GetWidth()*current->size.x*current->sizeOverLife/2.f;
			float32 halfH = sprite->GetHeight()*current->size.y*current->sizeOverLife/2.f;

			verts.push_back(current->position.x-halfW);//0
			verts.push_back(current->position.y-halfH);
			verts.push_back(current->position.z);

			verts.push_back(current->position.x+halfW);//1
			verts.push_back(current->position.y-halfH);
			verts.push_back(current->position.z);

			verts.push_back(current->position.x-halfW);//2
			verts.push_back(current->position.y+halfH);
			verts.push_back(current->position.z);

			verts.push_back(current->position.x-halfW);//2
			verts.push_back(current->position.y+halfH);
			verts.push_back(current->position.z);

			verts.push_back(current->position.x+halfW);//1
			verts.push_back(current->position.y-halfH);
			verts.push_back(current->position.z);

			verts.push_back(current->position.x+halfW);//3
			verts.push_back(current->position.y+halfH);
			verts.push_back(current->position.z);

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

			totalCount++;
			current = current->next;
		}

		if(totalCount > 0)
		{
			RenderManager::Instance()->SetMatrix(RenderManager::MATRIX_MODELVIEW, *transform);

			
			renderData->SetStream(EVF_VERTEX, TYPE_FLOAT, 3, 0, &verts.front());
			renderData->SetStream(EVF_TEXCOORD0, TYPE_FLOAT, 2, 0, &textures.front());
			RenderManager::Instance()->SetRenderData(renderData);

			RenderManager::Instance()->FlushState();
			RenderManager::Instance()->DrawArrays(PRIMITIVETYPE_TRIANGLELIST, 0, 4*totalCount);
		}
	}
}

};
