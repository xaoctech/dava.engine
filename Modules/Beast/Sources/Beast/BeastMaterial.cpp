#ifdef __DAVAENGINE_BEAST__

#include "BeastMaterial.h"
#include "BeastTexture.h"
#include "BeastDebug.h"
#include "SceneParser.h"

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
        BEAST_VERIFY(DAVA_BEAST::ILBCreateMaterial(manager->GetILBScene(), STRING_TO_BEAST_STRING(resourceName), &material));

        if (beastTexture)
        {
            BEAST_VERIFY(DAVA_BEAST::ILBSetMaterialTexture(material, DAVA_BEAST::ILB_CC_DIFFUSE, beastTexture->GetILBTexture()));
        }
        else
        {
            BEAST_VERIFY(DAVA_BEAST::ILBSetMaterialUseVertexColors(material, DAVA_BEAST::ILB_CC_DIFFUSE));
        }

        BEAST_VERIFY(DAVA_BEAST::ILBSetChannelUVLayer(material, DAVA_BEAST::ILB_CC_DIFFUSE, CONST_STRING_TO_BEAST_STRING("0")));

        if (SceneParser::IsMaterialTemplateContainString(davaMaterial, "Alphatest")
            || SceneParser::IsMaterialTemplateContainString(davaMaterial, "Alphablend")
            || SceneParser::IsMaterialTemplateContainString(davaMaterial, "SpeedTreeLeaf"))
        {
            BEAST_VERIFY(DAVA_BEAST::ILBSetAlphaAsTransparency(material, true));
        }
    }
}

void BeastMaterial::AttachNormalMap(BeastTexture* beastTexture)
{
    if (material)
    {
        BEAST_VERIFY(DAVA_BEAST::ILBSetMaterialTexture(material, DAVA_BEAST::ILB_CC_NORMAL, beastTexture->GetILBTexture()));
        BEAST_VERIFY(DAVA_BEAST::ILBSetChannelUVLayer(material, DAVA_BEAST::ILB_CC_NORMAL, CONST_STRING_TO_BEAST_STRING("2")));
    }
}

#endif //__DAVAENGINE_BEAST__
