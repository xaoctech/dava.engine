#include "Render/Highlevel/RenderObject.h"
#include "Base/ObjectFactory.h"
#include "Debug/DVAssert.h"
#include "Utils/Utils.h"
#include "Utils/StringFormat.h"
#include "Render/Renderer.h"
#include "Render/3D/PolygonGroup.h"
#include "LightRenderObject.h"
#include "Render/TextureDescriptor.h"
#include "Render/Highlevel/Light.h"

namespace DAVA
{
static const FastName NMaterialParamName_LIGHT_DIRECTION = FastName("lightDirection");
static const FastName NMaterialFlagName_FLAG_CUBEMAP_ENVIRONMENT_TEXTURE = FastName("CUBEMAP_ENVIRONMENT_TEXTURE");
static const FastName NMaterialFlagName_FLAG_EQUIRECTANGULAR_ENVIRONMENT_TEXTURE = FastName("EQUIRECTANGULAR_ENVIRONMENT_TEXTURE");
static const FastName NMaterialFlagName_FLAG_ATMOSPHERE = FastName("ATMOSPHERE");
static const FastName NMaterialFlagName_FLAG_RGBM = FastName("RGBM_INPUT");

LightRenderObject::LightRenderObject()
    : renderBatch(new RenderBatch())
    , backgroundMaterial(new NMaterial())
    , sunDiskMaterial(new NMaterial())
    , backgroundQuadPolygon(new PolygonGroup())
{
    type = eType::TYPE_DISTANT_LIGHT;
    AddFlag(CUSTOM_PREPARE_TO_RENDER);

    sunDiskMaterial->SetFXName(FastName("~res:/Materials2/SunDisk.material"));
    backgroundMaterial->SetFXName(FastName("~res:/Materials2/EnvironmentBackground.material"));

    backgroundMaterial->AddFlag(NMaterialFlagName_FLAG_RGBM, 0);
    backgroundMaterial->AddFlag(NMaterialFlagName_FLAG_ATMOSPHERE, 0);
    backgroundMaterial->AddFlag(NMaterialFlagName_FLAG_CUBEMAP_ENVIRONMENT_TEXTURE, 0);
    backgroundMaterial->AddFlag(NMaterialFlagName_FLAG_EQUIRECTANGULAR_ENVIRONMENT_TEXTURE, 0);
    backgroundMaterial->AddProperty(NMaterialParamName::ENVIRONMENT_COLOR_PROPERTY, Color::White.color, rhi::ShaderProp::Type::TYPE_FLOAT4);
    backgroundMaterial->AddProperty(NMaterialParamName_LIGHT_DIRECTION, lightDirection.data, rhi::ShaderProp::Type::TYPE_FLOAT4);
    backgroundMaterial->AddTexture(NMaterialTextureName::TEXTURE_ENVIRONMENT_MAP, Texture::CreatePink());

    sunDiskMaterial->AddProperty(NMaterialParamName_LIGHT_DIRECTION, lightDirection.data, rhi::ShaderProp::Type::TYPE_FLOAT4);
    sunDiskMaterial->AddProperty(NMaterialParamName::ENVIRONMENT_COLOR_PROPERTY, Color::White.color, rhi::ShaderProp::Type::TYPE_FLOAT4);

    backgroundQuadPolygon->AllocateData(EVF_VERTEX, 4, 6);
    backgroundQuadPolygon->SetCoord(0, Vector3(-1.0f, -1.0f, 1.0f));
    backgroundQuadPolygon->SetCoord(1, Vector3(1.0f, -1.0f, 1.0f));
    backgroundQuadPolygon->SetCoord(2, Vector3(-1.0f, 1.0f, 1.0f));
    backgroundQuadPolygon->SetCoord(3, Vector3(1.0f, 1.0f, 1.0f));
    backgroundQuadPolygon->SetIndex(0, 0);
    backgroundQuadPolygon->SetIndex(1, 1);
    backgroundQuadPolygon->SetIndex(2, 2);
    backgroundQuadPolygon->SetIndex(3, 1);
    backgroundQuadPolygon->SetIndex(4, 3);
    backgroundQuadPolygon->SetIndex(5, 2);
    backgroundQuadPolygon->BuildBuffers();

    renderBatch->SetRenderObject(this);
    renderBatch->SetPolygonGroup(backgroundQuadPolygon);

    SetWorldTransformPtr(&worldTransform);
    AddRenderBatch(renderBatch.Get());
    AddFlag(ALWAYS_CLIPPING_VISIBLE);
}

void LightRenderObject::SetLight(Light* light)
{
    RenderObject::SetLight(0, light);

    SafeRelease(sourceLight);
    sourceLight = SafeRetain(light);

    if (sourceLight == nullptr)
        return;

    NMaterial* material = (sourceLight->GetLightType() == Light::eType::TYPE_SUN) ? sunDiskMaterial : backgroundMaterial;
    material->SetPropertyValue(NMaterialParamName::ENVIRONMENT_COLOR_PROPERTY, sourceLight->GetColor().color);

    if (sourceLight->GetLightType() == Light::eType::TYPE_SUN)
    {
    }
    else if (sourceLight->GetLightType() == Light::eType::TYPE_ENVIRONMENT_IMAGE)
    {
        ScopedPtr<Texture> texture(Texture::CreateFromFile(sourceLight->GetEnvironmentMap()));
        int32 isCubeMap = texture->GetDescriptor()->IsCubeMap() ? 1 : 0;

        material->SetFlag(NMaterialFlagName_FLAG_ATMOSPHERE, 0);
        material->SetFlag(NMaterialFlagName_FLAG_RGBM, (texture->GetFormat() == PixelFormat::FORMAT_RGBM) ? 1 : 0);
        material->SetFlag(NMaterialFlagName_FLAG_CUBEMAP_ENVIRONMENT_TEXTURE, isCubeMap);
        material->SetFlag(NMaterialFlagName_FLAG_EQUIRECTANGULAR_ENVIRONMENT_TEXTURE, 1 - isCubeMap);
        material->SetTexture(NMaterialTextureName::TEXTURE_ENVIRONMENT_MAP, texture);
    }
    else if (sourceLight->GetLightType() == Light::eType::TYPE_ATMOSPHERE)
    {
        material->SetFlag(NMaterialFlagName_FLAG_CUBEMAP_ENVIRONMENT_TEXTURE, 0);
        material->SetFlag(NMaterialFlagName_FLAG_EQUIRECTANGULAR_ENVIRONMENT_TEXTURE, 0);
        material->SetFlag(NMaterialFlagName_FLAG_ATMOSPHERE, 1);
    }
    else if (sourceLight->GetLightType() == Light::eType::TYPE_UNIFORM_COLOR)
    {
        material->SetFlag(NMaterialFlagName_FLAG_CUBEMAP_ENVIRONMENT_TEXTURE, 0);
        material->SetFlag(NMaterialFlagName_FLAG_EQUIRECTANGULAR_ENVIRONMENT_TEXTURE, 0);
        material->SetFlag(NMaterialFlagName_FLAG_ATMOSPHERE, 0);
    }
}

void LightRenderObject::InvalidateMaterial()
{
    sunDiskMaterial->InvalidateRenderVariants();
    backgroundMaterial->InvalidateRenderVariants();
}

void LightRenderObject::PrepareToRender(Camera* camera)
{
    if (sourceLight == nullptr)
        return;

    lightDirection = Vector4(-sourceLight->GetDirection(), 0.0f);

    NMaterial* material = (sourceLight->GetLightType() == Light::eType::TYPE_SUN) ? sunDiskMaterial : backgroundMaterial;
    material->SetPropertyValue(NMaterialParamName_LIGHT_DIRECTION, lightDirection.data);
    material->SetPropertyValue(NMaterialParamName::ENVIRONMENT_COLOR_PROPERTY, sourceLight->GetColor().color);
    renderBatch->SetMaterial(material);
}
};
