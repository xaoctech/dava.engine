#include "Particles/ParticleLayer3D.h"
#include "Render/RenderDataObject.h"
#include "Render/RenderManager.h"
#include "Render/Material.h"
#include "Math/MathHelpers.h"
#include "Render/Highlevel/Camera.h"

namespace DAVA
{

ParticleLayer3D::ParticleLayer3D()
{
	renderData = new RenderDataObject();

	//TODO: set material from outside
	
	Material * material = new Material();
	material->SetType(Material::MATERIAL_VERTEX_COLOR_ALPHABLENDED);
	material->SetAlphablend(true);
	material->SetBlendSrc(BLEND_SRC_ALPHA);
	material->SetBlendDest(BLEND_ONE);
	material->SetName("ParticleLayer3D_material");

	renderBatch->SetMaterial(material);
	SafeRelease(material);
}

ParticleLayer3D::~ParticleLayer3D()
{
	SafeRelease(renderData);
}

void ParticleLayer3D::Draw(Camera * camera)
{
	if(!sprite)
		return;

    Matrix4 rotationMatrix = Matrix4::IDENTITY;
    switch(RenderManager::Instance()->GetRenderOrientation())
    {
        case Core::SCREEN_ORIENTATION_LANDSCAPE_LEFT:
            //glRotatef(90.0f, 0.0f, 0.0f, 1.0f);
            rotationMatrix.CreateRotation(Vector3(0.0f, 0.0f, 1.0f), DegToRad(90.0f));
            break;
        case Core::SCREEN_ORIENTATION_LANDSCAPE_RIGHT:
            //glRotatef(-90.0f, 0.0f, 0.0f, 1.0f);
            rotationMatrix.CreateRotation(Vector3(0.0f, 0.0f, 1.0f), DegToRad(-90.0f));
            break;
    }

    Matrix4 mv = RenderManager::Instance()->GetMatrix(RenderManager::MATRIX_MODELVIEW)*rotationMatrix;
    
	Vector3 _up(mv._01, mv._11, mv._21);
	Vector3 _left(mv._00, mv._10, mv._20);

	verts.clear();
	textures.clear();
	colors.clear();
	int32 totalCount = 0;

	Particle * current = head;
	if(current)
	{
		renderBatch->GetMaterial()->GetRenderState()->SetTexture(sprite->GetTexture(current->frame));
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

		float32 pivotRight = ((sprite->GetWidth()-pivotPoint.x)*current->size.x*current->sizeOverLife.x)/2.f;
		float32 pivotLeft = (pivotPoint.x*current->size.x*current->sizeOverLife.x)/2.f;
		float32 pivotUp = (pivotPoint.y*current->size.y*current->sizeOverLife.y)/2.f;
		float32 pivotDown = ((sprite->GetHeight()-pivotPoint.y)*current->size.y*current->sizeOverLife.y)/2.f;

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

		// Yuri Coder, 2013/04/03. Need to use drawColor here instead of just colot
		// to take colorOverlife property into account.
		uint32 color = (((uint32)(current->drawColor.a*255.f))<<24) |  (((uint32)(current->drawColor.b*255.f))<<16) | (((uint32)(current->drawColor.g*255.f))<<8) | ((uint32)(current->drawColor.r*255.f));
		for(int32 i = 0; i < 6; ++i)
		{
			colors.push_back(color);
		}

		totalCount++;
		current = TYPE_PARTICLES == type ? current->next : 0;
	}

	renderBatch->SetTotalCount(totalCount);
	if(totalCount > 0)
	{			
		renderData->SetStream(EVF_VERTEX, TYPE_FLOAT, 3, 0, &verts.front());
		renderData->SetStream(EVF_TEXCOORD0, TYPE_FLOAT, 2, 0, &textures.front());
		renderData->SetStream(EVF_COLOR, TYPE_UNSIGNED_BYTE, 4, 0, &colors.front());

		renderBatch->SetRenderDataObject(renderData);
	}
}

void ParticleLayer3D::LoadFromYaml(const String & configPath, YamlNode * node)
{
	ParticleLayer::LoadFromYaml(configPath, node);
	SetAdditive(additive);
}

ParticleLayer * ParticleLayer3D::Clone(ParticleLayer * dstLayer /*= 0*/)
{
	if(!dstLayer)
	{
		dstLayer = new ParticleLayer3D();
	}

	ParticleLayer::Clone(dstLayer);

	return dstLayer;
}

Material * ParticleLayer3D::GetMaterial()
{
	return renderBatch->GetMaterial();
}
	
void ParticleLayer3D::SetAdditive(bool additive)
{
	ParticleLayer::SetAdditive(additive);
	if(additive)
	{
		renderBatch->GetMaterial()->SetBlendSrc(BLEND_SRC_ALPHA);
		renderBatch->GetMaterial()->SetBlendDest(BLEND_ONE);
	}
	else
	{
		renderBatch->GetMaterial()->SetBlendSrc(BLEND_SRC_ALPHA);
		renderBatch->GetMaterial()->SetBlendDest(BLEND_ONE_MINUS_SRC_ALPHA);
	}
}

};
