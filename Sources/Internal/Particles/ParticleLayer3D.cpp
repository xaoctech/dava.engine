#include "Particles/ParticleLayer3D.h"
#include "Render/RenderDataObject.h"
#include "Render/RenderManager.h"
#include "Render/Material.h"

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

void ParticleLayer3D::Draw(const Vector3 & _up, const Vector3 & _left, const Vector3 & _direction)
{
	//if(TYPE_PARTICLES == type)
	{
		verts.clear();
		textures.clear();
		colors.clear();
		int32 totalCount = 0;

		//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

		Particle * current = head;
		if(current)
		{
			RenderManager::Instance()->SetTexture(sprite->GetTexture(current->frame));
		}

		while(current != 0)
		{
			//Vector3 up, left;
			//if(current->angle != 0)
			//{
			//	Matrix3 rotation;
			//	rotation.CreateRotation(_direction, current->angle);
			//	up = _up*rotation;
			//	left = _left*rotation;
			//}
			//else
			//{
			//	up = _up;
			//	left = _left;
			//}

			//float32 halfW = sprite->GetWidth()*current->size.x*current->sizeOverLife/2.f;
			//float32 halfH = sprite->GetHeight()*current->size.y*current->sizeOverLife/2.f;

			//up = up*halfH;
			//left = left*halfW;

			//Vector3 topLeft = current->position+up+left;
			//Vector3 topRight = current->position+up-left;
			//Vector3 botLeft = current->position-up+left;
			//Vector3 botRight = current->position-up-left;

			Vector3 dx(_left);
			Vector3 dy(_up);

			dx *= sqrt(2.f);
			dy *= sqrt(2.f);

			float32 sine = sinf(current->angle);
			float32 cosine = cosf(current->angle);

			float32 pivotRight = ((sprite->GetWidth()-pivotPoint.x)*current->size.x*current->sizeOverLife)/2.f;
			float32 pivotLeft = (pivotPoint.x*current->size.x*current->sizeOverLife)/2.f;
			float32 pivotUp = (pivotPoint.y*current->size.y*current->sizeOverLife)/2.f;
			float32 pivotDown = ((sprite->GetHeight()-pivotPoint.y)*current->size.y*current->sizeOverLife)/2.f;

			Vector3 dxc = dx*cosine;
			Vector3 dxs = dx*sine;
			Vector3 dyc = dy*cosine;
			Vector3 dys = dy*sine;

			// v[0].xyz = p.position - dxc + dys
			// v[1].xyz = p.position - dxs - dyc
			// v[2].xyz = p.position + dxc - dys
			// v[3].xyz = p.position + dxs + dyc

			//Vector3 topLeft = current->position-dxc+dys;
			//Vector3 topRight = current->position-dxs-dyc;
			//Vector3 botLeft = current->position+dxs+dyc;
			//Vector3 botRight = current->position+dxc-dys;

			//Vector3 topLeft = current->position+(-dxc+dys);
			//Vector3 topRight = current->position+(-dxs-dyc);
			//Vector3 botLeft = current->position+(dxs+dyc);
			//Vector3 botRight = current->position+(dxc-dys);
			
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
			current = 0;//current->next;
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

		//glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
}

};
