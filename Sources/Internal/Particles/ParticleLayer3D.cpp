#include "Particles/ParticleLayer3D.h"
#include "Render/RenderDataObject.h"
#include "Render/RenderManager.h"
#include "Render/Material.h"
#include "Math/MathHelpers.h"

namespace DAVA
{

ParticleLayer3D::ParticleLayer3D()
{
	renderData = new RenderDataObject();

	//TODO: set material from outside
	material = new Material();
	material->SetType(Material::MATERIAL_VERTEX_COLOR_ALPHABLENDED);
	material->SetAlphablend(true);
	material->blendSrc = BLEND_SRC_ALPHA;
	material->blendDst = BLEND_ONE;
}

ParticleLayer3D::~ParticleLayer3D()
{
	SafeRelease(material);
	SafeRelease(renderData);
}

void ParticleLayer3D::Draw(const Vector3 & _up, const Vector3 & _left)
{
	verts.clear();
	textures.clear();
	colors.clear();
	int32 totalCount = 0;

	Particle * current = head;
	if(current)
	{
		RenderManager::Instance()->SetTexture(sprite->GetTexture(current->frame));
	}

	while(current != 0)
	{
		Vector3 dx(_left);
		Vector3 dy(_up);

		//dx *= sqrt(2.f);
		//dy *= sqrt(2.f);

		float32 sine;
		float32 cosine;
		SinCosFast(current->angle, sine, cosine);

		float32 pivotRight = ((sprite->GetWidth()-pivotPoint.x)*current->size.x*current->sizeOverLife)/2.f;
		float32 pivotLeft = (pivotPoint.x*current->size.x*current->sizeOverLife)/2.f;
		float32 pivotUp = (pivotPoint.y*current->size.y*current->sizeOverLife)/2.f;
		float32 pivotDown = ((sprite->GetHeight()-pivotPoint.y)*current->size.y*current->sizeOverLife)/2.f;

		Vector3 dxc = dx*cosine;
		Vector3 dxs = dx*sine;
		Vector3 dyc = dy*cosine;
		Vector3 dys = dy*sine;
			
		Vector3 topLeft = current->position+(-dxc+dys)*pivotUp + (dxs+dyc)*pivotLeft;
		Vector3 topRight = current->position+(-dxs-dyc)*pivotRight + (-dxc+dys)*pivotUp;
		Vector3 botLeft = current->position+(dxs+dyc)*pivotLeft + (dxc-dys)*pivotDown;
		Vector3 botRight = current->position+(dxc-dys)*pivotDown + (-dxs-dyc)*pivotRight;

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

 		material->PrepareRenderState();

		RenderManager::Instance()->HWDrawArrays(PRIMITIVETYPE_TRIANGLELIST, 0, 6*totalCount);
	}
}

};
