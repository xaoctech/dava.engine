#include "Beast/BeastMaterial.h"
#include "Beast/BeastTexture.h"
#include "Beast/BeastDebug.h"
#include "Beast/SceneParser.h"

BeastMaterial::BeastMaterial(const DAVA::String& name, BeastManager* manager)
    : BeastResource(name, manager)
    ,
    material(0)
{
}

void BeastMaterial::InitWithTextureAndDavaMaterial(BeastTexture* beastTexture, DAVA::NMaterial* davaMaterial)
{
    if (0 == material)
    {
        BEAST_VERIFY(ILBCreateMaterial(manager->GetILBScene(), STRING_TO_BEAST_STRING(resourceName), &material));

        if (beastTexture)
        {
            BEAST_VERIFY(ILBSetMaterialTexture(material, ILB_CC_DIFFUSE, beastTexture->GetILBTexture()));
        }
        else
        {
            BEAST_VERIFY(ILBSetMaterialUseVertexColors(material, ILB_CC_DIFFUSE));
        }

        BEAST_VERIFY(ILBSetChannelUVLayer(material, ILB_CC_DIFFUSE, CONST_STRING_TO_BEAST_STRING("0")));

        if (SceneParser::IsMaterialTemplateContainString(davaMaterial, "Alphatest")
            || SceneParser::IsMaterialTemplateContainString(davaMaterial, "Alphablend")
            || SceneParser::IsMaterialTemplateContainString(davaMaterial, "SpeedTreeLeaf"))
        {
            BEAST_VERIFY(ILBSetAlphaAsTransparency(material, true));
        }
    }
}

void BeastMaterial::AttachNormalMap(BeastTexture* beastTexture)
{
    if (material)
    {
        BEAST_VERIFY(ILBSetMaterialTexture(material, ILB_CC_NORMAL, beastTexture->GetILBTexture()));
        BEAST_VERIFY(ILBSetChannelUVLayer(material, ILB_CC_NORMAL, CONST_STRING_TO_BEAST_STRING("2")));
    }
}
