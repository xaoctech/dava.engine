#pragma once

#include "Beast/BeastTypes.h"
#include "Beast/BeastResource.h"
#include "Beast/BeastManager.h"

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
