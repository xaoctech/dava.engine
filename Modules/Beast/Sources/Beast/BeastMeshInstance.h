#pragma once

#include "Beast/BeastTypes.h"
#include "Beast/BeastResource.h"
#include "Beast/TextureTarget.h"

#include <Base/BaseTypes.h>
#include <Math/Matrix4.h>

namespace DAVA
{
class RenderBatch;
class Landscape;
}

class BeastMesh;
class BeastMeshInstance : public BeastResource<BeastMeshInstance>
{
public:
    virtual ~BeastMeshInstance();

    void InitWithRenderBatchAndTransform(DAVA::RenderBatch* batch, DAVA::int32 partIndex, const DAVA::Matrix4& transform);
    void InitWithLandscape(DAVA::Landscape* landscapeNode, BeastMesh* beastMesh);

    ILBInstanceHandle GetILBMeshInstance();
    int32 GetTextureCoordCount();

    TextureTarget* GetTextureTarget();
    void SetTextureTarget(TextureTarget* val);

    int32 GetLightmapSize();
    void SetLightmapSize(int32 size);

    BeastMesh* GetBuddyMesh();

    bool IsLandscape()
    {
        return isLandscape;
    }

    void SetLodLevel(int32 lodLevel);
    int32 GetLodLevel();

    void UseLightMap();
    bool IsUseLightmap();

    DAVA::String GetMeshInstancePointerString();

    DAVA::RenderBatch* GetRenderBatch();

private:
    friend class BeastResource<BeastMeshInstance>;
    BeastMeshInstance(const DAVA::String& name, BeastManager* manager);
    void GetCastReceiveShadowOptions(DAVA::RenderBatch* batch);

private:
    ILBInstanceHandle meshInstanceHandle = nullptr;
    TextureTarget* textureTarget = nullptr;
    BeastMesh* buddyMesh = nullptr;
    DAVA::RenderBatch* renderBatch = nullptr;
    DAVA::int32 partIndex = 0;
    DAVA::int32 lightmapSize = 0;
    DAVA::int32 lodLevel = -1;
    bool useLightmap = false;
    bool isLandscape = false; // TODO: get rid of it, use subclass
};
