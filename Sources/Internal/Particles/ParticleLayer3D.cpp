#include "Particles/ParticleLayer3D.h"
#include "Render/RenderDataObject.h"
#include "Render/RenderManager.h"
#include "Render/Material.h"
#include "Math/MathHelpers.h"
#include "Render/Highlevel/Camera.h"
#include "ParticleEmitter3D.h"

namespace DAVA
{

ParticleLayer3D::ParticleLayer3D(ParticleEmitter* parent)
{
	isLong = false;
	renderData = new RenderDataObject();
	this->parent = parent;

	//TODO: set material from outside
	
//	Material * material = new Material();
//	material->SetType(Material::MATERIAL_VERTEX_COLOR_ALPHABLENDED);
//	material->SetAlphablend(true);
//	material->SetBlendSrc(BLEND_SRC_ALPHA);
//	material->SetBlendDest(BLEND_ONE);
//	material->SetName("ParticleLayer3D_material");

    NMaterial * material = new NMaterial();
    NMaterialInstance * materialInstance = new NMaterialInstance();
    
    renderBatch->SetMaterial(material);
    renderBatch->SetMaterialInstance(materialInstance);

	SafeRelease(material);
    SafeRelease(materialInstance);
}

ParticleLayer3D::~ParticleLayer3D()
{
	SafeRelease(renderData);
}

void ParticleLayer3D::Draw(Camera * camera)
{
	DrawLayer(camera);
}

void ParticleLayer3D::DrawLayer(Camera* camera)
{
	if (!sprite)
	{
		return;
	}

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
    
	_up = Vector3(mv._01, mv._11, mv._21);
	_left = Vector3(mv._00, mv._10, mv._20);
	direction = camera->GetDirection();

	verts.clear();
	textures.clear();
	colors.clear();
	int32 totalCount = 0;

	Particle * current = head;
	if(current)
	{
		renderBatch->GetMaterialInstance()->GetRenderState()->SetTexture(sprite->GetTexture(current->frame));
	}

	while(current != 0)
	{
		Vector3 topRight;
		Vector3 topLeft;
		Vector3 botRight;
		Vector3 botLeft;

		if (IsLong())
		{
			CalcLong(current, topLeft, topRight, botLeft, botRight);
		}
		else
		{
			CalcNonLong(current, topLeft, topRight, botLeft, botRight);
		}

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
		uint32 color = (((uint32)(current->drawColor.a*255.f))<<24) |  (((uint32)(current->drawColor.b*255.f))<<16) |
			(((uint32)(current->drawColor.g*255.f))<<8) | ((uint32)(current->drawColor.r*255.f));
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

		if (IsLong())
		{
			RenderManager::Instance()->SetRenderData(renderData);
			renderBatch->GetMaterial()->PrepareRenderState();
		}
		renderBatch->SetRenderDataObject(renderData);
	}
}

void ParticleLayer3D::CalcNonLong(Particle* current,
								  Vector3& topLeft,
								  Vector3& topRight,
								  Vector3& botLeft,
								  Vector3& botRight)
{
	Vector3 dx(_left);
	Vector3 dy(_up);

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

	topLeft = current->position+(dxs+dyc)*pivotLeft + (dxc-dys)*pivotDown;
	topRight = current->position+(-dxc+dys)*pivotUp + (dxs+dyc)*pivotLeft;
	botLeft = current->position+(dxc-dys)*pivotDown + (-dxs-dyc)*pivotRight;
	botRight = current->position+(-dxs-dyc)*pivotRight + (-dxc+dys)*pivotUp;
}

void ParticleLayer3D::CalcLong(Particle* current,
							   Vector3& topLeft,
							   Vector3& topRight,
							   Vector3& botLeft,
							   Vector3& botRight)
{
	Vector3 vecShort = current->direction.CrossProduct(direction);
	vecShort /= 2.f;
		
	Vector3 vecLong = -current->direction;

	float32 widthDiv2 = sprite->GetWidth()*current->size.x*current->sizeOverLife.x;
	float32 heightDiv2 = sprite->GetHeight()*current->size.y*current->sizeOverLife.y;

	topRight = current->position + widthDiv2*vecShort;
	topLeft = current->position - widthDiv2*vecShort;
	botRight = topRight + heightDiv2*vecLong;
	botLeft = topLeft + heightDiv2*vecLong;
}


void ParticleLayer3D::LoadFromYaml(const FilePath & configPath, YamlNode * node)
{
	ParticleLayer::LoadFromYaml(configPath, node);
	SetAdditive(additive);
}

ParticleLayer * ParticleLayer3D::Clone(ParticleLayer * dstLayer /*= 0*/)
{
	if(!dstLayer)
	{
		ParticleEmitter* parentFor3DLayer = NULL;
		if (dynamic_cast<ParticleLayer3D*>(dstLayer))
		{
			parentFor3DLayer = (dynamic_cast<ParticleLayer3D*>(dstLayer))->GetParent();
		}

		dstLayer = new ParticleLayer3D(parentFor3DLayer);
	}

	ParticleLayer::Clone(dstLayer);

	return dstLayer;
}

NMaterial * ParticleLayer3D::GetMaterial()
{
	return renderBatch->GetMaterial();
}
	
void ParticleLayer3D::SetAdditive(bool additive)
{
	ParticleLayer::SetAdditive(additive);
	if(additive)
	{
		renderBatch->GetMaterialInstance()->GetRenderState()->SetBlendSrc(BLEND_SRC_ALPHA);
		renderBatch->GetMaterialInstance()->GetRenderState()->SetBlendDest(BLEND_ONE);
	}
	else
	{
		renderBatch->GetMaterialInstance()->GetRenderState()->SetBlendSrc(BLEND_SRC_ALPHA);
		renderBatch->GetMaterialInstance()->GetRenderState()->SetBlendDest(BLEND_ONE_MINUS_SRC_ALPHA);
	}
}

bool ParticleLayer3D::IsLong()
{
	return isLong;
}

void ParticleLayer3D::SetLong(bool value)
{
	isLong = value;
	renderBatch->GetMaterial()->SetTwoSided(isLong);
}

};
