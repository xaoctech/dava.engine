#ifdef __DAVAENGINE_BEAST__

#ifndef __BEAST_MESH_INSTANCE__
#define __BEAST_MESH_INSTANCE__

#include "DAVAEngine.h"
#include "BeastTypes.h"
#include "BeastResource.h"
#include "TextureTarget.h"

class BeastMesh;
class BeastMeshInstance : public BeastResource<BeastMeshInstance>
{
public:
    virtual ~BeastMeshInstance();

    void InitWithRenderBatchAndTransform(DAVA::RenderBatch* batch, DAVA::int32 partIndex, const DAVA::Matrix4& transform);
    void InitWithLandscape(DAVA::Landscape* landscapeNode, BeastMesh* beastMesh);

    DAVA_BEAST::ILBInstanceHandle GetILBMeshInstance();
    DAVA_BEAST::int32 GetTextureCoordCount();

    TextureTarget* GetTextureTarget();
    void SetTextureTarget(TextureTarget* val);

    DAVA_BEAST::int32 GetLightmapSize();
    void SetLightmapSize(DAVA_BEAST::int32 size);

    BeastMesh* GetBuddyMesh();

    bool IsLandscape()
    {
        return isLandscape;
    }

    void SetLodLevel(DAVA_BEAST::int32 lodLevel);
    DAVA_BEAST::int32 GetLodLevel();

    void UseLightMap();
    bool IsUseLightmap();

    DAVA::String GetMeshInstancePointerString();

    DAVA::RenderBatch* GetRenderBatch();

private:
    friend class BeastResource<BeastMeshInstance>;
    BeastMeshInstance(const DAVA::String& name, BeastManager* manager);
    void GetCastReceiveShadowOptions(DAVA::RenderBatch* batch);

private:
    DAVA_BEAST::ILBInstanceHandle meshInstanceHandle = nullptr;
    TextureTarget* textureTarget = nullptr;
    BeastMesh* buddyMesh = nullptr;
    DAVA::RenderBatch* renderBatch = nullptr;
    DAVA::int32 partIndex = 0;
    DAVA::int32 lightmapSize = 0;
    DAVA::int32 lodLevel = -1;
    bool useLightmap = false;
    bool isLandscape = false; // TODO: get rid of it, use subclass
};

#endif //__BEAST_MESH_INSTANCE__

#endif //__DAVAENGINE_BEAST__
