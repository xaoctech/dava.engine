#ifdef __DAVAENGINE_BEAST__

#ifndef __BEAST_MATERIAL__
#define __BEAST_MATERIAL__

#include "DAVAEngine.h"
#include "BeastTypes.h"
#include "BeastResource.h"
#include "BeastManager.h"

class BeastTexture;
class BeastMaterial : public BeastResource<BeastMaterial>
{
public:
    void InitWithTextureAndDavaMaterial(BeastTexture* beastTexture, DAVA::NMaterial* davaMaterial);
    void AttachNormalMap(BeastTexture* beastTexture);

private:
    BeastMaterial(const DAVA::String& name, BeastManager* manager);
    ILBMaterialHandle material;

    friend class BeastResource<BeastMaterial>;
};

#endif //__BEAST_MATERIAL__

#endif //__DAVAENGINE_BEAST__
