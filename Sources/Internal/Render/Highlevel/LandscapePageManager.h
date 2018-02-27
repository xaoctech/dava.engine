#pragma once 

#include "Base/BaseTypes.h"
#include "Base/UnordererMap.h"
#include "Math/Vector.h"
#include "Render/VirtualTexture.h"
#include "Render/Highlevel/LandscapeLayerRenderer.h"

namespace DAVA
{
class NMaterial;
class Texture;
class LandscapeSubdivision;
class RenderSystem;
class VTDecalPageRenderer;
class LandscapePageManager
{
public:
    struct PageMapping
    {
        Vector2 uvOffset0;
        Vector2 uvScale0;
        Vector2 uvOffset1;
        Vector2 uvScale1;
    };

    LandscapePageManager(const VirtualTexture::Descriptor& descriptor);
    ~LandscapePageManager();

    bool AddPageRenderer(LandscapePageRenderer* pageRenderer);
    bool RemovePageRenderer(LandscapePageRenderer* pageRenderer);
    void ClearPageRenderers();

    void RequestPage(uint32 level, uint32 x, uint32 y, float32 priority);
    void ProcessRequests(const LandscapeSubdivision* subdivision, uint32 maxPageUpdates, LandscapePageRenderer::eLandscapeComponent component);
    void RejectRequests();

    PageMapping GetSuitablePage(uint32 level, uint32 x, uint32 y);

    bool InvalidatePage(uint32 level, uint32 x, uint32 y);
    void Invalidate();

    VirtualTexture* GetVirtualTexture() const;

    void SetMiddleLODLevel(uint32 level);
    uint32 GetMiddleLODLevel() const;

    void SetMacroLODLevel(uint32 level);
    uint32 GetMacroLODLevel() const;

protected:
    struct ResidentPage
    {
        int32 pageID;
        uint32 updateID;
        float32 priority;
    };

    struct UpdateRequest
    {
        uint64 pageKey;
        float32 priority;

        bool operator<(const UpdateRequest& request) const
        {
            return (priority > request.priority);
        }
    };

    bool TryFreePage(float32 requestedRadiusError);
    Vector4 MapToPage(uint32 level, uint32 x, uint32 y, uint32 pLevel, int32 pageID); //(offset, scale)
    Vector4 GetPageUV(uint64 pagekey); //(uv0.x, uv0.y, uv1.x, uv1.y)
    AABBox3 GetPageBBox(uint64 pageKey, const LandscapeSubdivision* subdivision);

    VirtualTexture* vTexture = nullptr;

    Vector<UpdateRequest> updateRequests;
    UnorderedMap<uint64, ResidentPage> residentPages;

    uint32 middleLODLevel = 0;
    uint32 macroLODLevel = 0;
    float32 pageTexelOffset = 0.f;
    List<LandscapePageRenderer*> pageRenderers;
};

inline VirtualTexture* LandscapePageManager::GetVirtualTexture() const
{
    return vTexture;
}

inline void LandscapePageManager::SetMiddleLODLevel(uint32 level)
{
    middleLODLevel = level;
    macroLODLevel = Min(middleLODLevel, macroLODLevel);
    Invalidate();
}

inline uint32 LandscapePageManager::GetMiddleLODLevel() const
{
    return middleLODLevel;
}

inline void LandscapePageManager::SetMacroLODLevel(uint32 level)
{
    macroLODLevel = level;
    middleLODLevel = Max(middleLODLevel, macroLODLevel);
    Invalidate();
}

inline uint32 LandscapePageManager::GetMacroLODLevel() const
{
    return macroLODLevel;
}

}; //ns DAVA
