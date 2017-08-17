#pragma once

#include "Beast/BeastTypes.h"
#include "Beast/BeastResource.h"

namespace DAVA
{
class Texture;
class FilePath;
}

class BeastTexture : public BeastResource<BeastTexture>
{
public:
    void InitWithTexture(DAVA::Texture* davaTexture);
    void InitWithFile(const DAVA::FilePath& filePath);
    ILBTextureHandle GetILBTexture();

private:
    BeastTexture(const DAVA::String& name, BeastManager* manager);
    ILBTextureHandle texture = nullptr;

    friend class BeastResource<BeastTexture>;
};
