#include "LandscapePageManager.h"
#include "Render/Material/NMaterial.h"
#include "Render/Material/NMaterialNames.h"
#include "Engine/Engine.h"
#include "Render/DynamicBufferAllocator.h"
#include "Render/Highlevel/RenderPassNames.h"
#include "Render/Highlevel/RenderSystem.h"
#include "Render/Highlevel/LandscapeSubdivision.h"
#include "Render/Highlevel/VTDecalPageRenderer.h"

namespace DAVA
{

#define PAGE_KEY(level, x, y) (uint64(level) << 56 | uint64(x & 0xFFFFFFF) << 28 | uint64(y & 0xFFFFFFF)) //uint64: 0xLLXXXXXXXYYYYYYY
#define PAGE_LEVEL(key) uint32(key >> 56)
#define PAGE_X(key) uint32(((key) >> 28) & 0xFFFFFFF)
#define PAGE_Y(key) uint32((key)&0xFFFFFFF)

LandscapePageManager::LandscapePageManager(const VirtualTexture::Descriptor& descriptor)
{
    vTexture = new VirtualTexture(descriptor);
    pageTexelOffset = 0.5f * (1 << descriptor.mipLevelCount);
}

LandscapePageManager::~LandscapePageManager()
{
    SafeDelete(vTexture);
}

bool LandscapePageManager::AddPageRenderer(LandscapePageRenderer* pageRenderer)
{
    auto it = std::find(pageRenderers.cbegin(), pageRenderers.cend(), pageRenderer);
    if (it == pageRenderers.cend())
    {
        pageRenderers.push_back(pageRenderer);
        return true;
    }
    return false;
}

bool LandscapePageManager::RemovePageRenderer(LandscapePageRenderer* pageRenderer)
{
    auto it = std::find(pageRenderers.cbegin(), pageRenderers.cend(), pageRenderer);
    if (it != pageRenderers.cend())
    {
        pageRenderers.erase(it);
        return true;
    }
    return false;
}

void LandscapePageManager::ClearPageRenderers()
{
    pageRenderers.clear();
}

void LandscapePageManager::RequestPage(uint32 level, uint32 x, uint32 y, float32 priority)
{
    uint64 pageKey = PAGE_KEY(level, x, y);

    //check already resident pages
    auto found = residentPages.find(pageKey);
    if (found != residentPages.cend())
    {
        found->second.updateID = Engine::Instance()->GetGlobalFrameIndex();
        return;
    }

    DVASSERT((std::find_if(updateRequests.cbegin(), updateRequests.cend(), [&pageKey](const UpdateRequest& r) { return (r.pageKey == pageKey); }) == updateRequests.cend()) && "page was alread requested");

    //add request for update
    updateRequests.push_back({ pageKey, priority });
}

LandscapePageManager::PageMapping LandscapePageManager::GetSuitablePage(uint32 level, uint32 x, uint32 y)
{
    uint32 pLevel = level;
    uint32 px = x;
    uint32 py = y;

    uint32 frameIndex = Engine::Instance()->GetGlobalFrameIndex();

    PageMapping pageMapping;
    bool page0found = false;
    while (pLevel != uint32(-1))
    {
        auto found = residentPages.find(PAGE_KEY(pLevel, px, py));

        if ((found != residentPages.cend()) && (found->second.updateID == frameIndex))
        {
            Vector4 mapping = MapToPage(level, x, y, pLevel, found->second.pageID);
            if (!page0found)
            {
                pageMapping.uvOffset0 = Vector2(mapping.x, mapping.y);
                pageMapping.uvScale0 = Vector2(mapping.z, mapping.w);
                page0found = true;
            }

            if (pLevel < level || level == 0)
            {
                pageMapping.uvOffset1 = Vector2(mapping.x, mapping.y);
                pageMapping.uvScale1 = Vector2(mapping.z, mapping.w);
                break;
            }
        }

        --pLevel;
        px >>= 1;
        py >>= 1;
    }

    return pageMapping;
}

void LandscapePageManager::ProcessRequests(const LandscapeSubdivision* subdivision, uint32 maxPageUpdates, LandscapePageRenderer::eLandscapeComponent component)
{
    LandscapePageRenderer::PageRenderParams pageRenderParams;
    pageRenderParams.pageSrc.resize(vTexture->GetIntermediateBufffersLayersCount());
    pageRenderParams.pageDst.resize(vTexture->GetIntermediateBufffersLayersCount());
    pageRenderParams.pageSize = vTexture->GetPageSize();
    pageRenderParams.component = component;

    DVASSERT(pageRenderers.size() > 0);

    std::sort(updateRequests.begin(), updateRequests.end());

    uint32 updatesCount = Min(maxPageUpdates, uint32(updateRequests.size()));
    for (uint32 i = 0; i < updatesCount; ++i)
    {
        const UpdateRequest& request = updateRequests[i];

        if (vTexture->GetFreePagesCount() == 0 && !TryFreePage(request.priority))
            break;

        int32 pageID = vTexture->AcquireFreePage();
        DVASSERT(pageID != -1);
        DVASSERT(residentPages.count(request.pageKey) == 0);

        Vector4 uv = GetPageUV(request.pageKey); //(uv0.x, uv0.y, uv1.x, uv1.y)
        pageRenderParams.relativeCoord0 = Vector2(uv.x, uv.y);
        pageRenderParams.relativeCoord1 = Vector2(uv.z, uv.w);
        pageRenderParams.pageBBox = GetPageBBox(request.pageKey, subdivision);

        uint64 pageLevel = PAGE_LEVEL(request.pageKey);
        pageRenderParams.lod = (pageLevel < macroLODLevel) ? 2u : (pageLevel < middleLODLevel) ? 1u : 0u;

        for (LandscapePageRenderer* renderer : pageRenderers)
        {
            pageRenderParams.pageSrc = vTexture->GetIntermediateSourceBuffers();
            pageRenderParams.pageDst = vTexture->GetIntermediateDestinationBuffers();

            bool renderHappens = renderer->RenderPage(pageRenderParams);
            //GFX_COMPLETE solovey lets rethink this stuff - updating pageRenderParams after each swap looks not cool
            if (renderHappens)
                vTexture->SwapIntermediateBuffers();
        }

        vTexture->BlitIntermediateBuffer(pageID);
        uint32 frameIndex = Engine::Instance()->GetGlobalFrameIndex();
        residentPages[request.pageKey] = { pageID, frameIndex, request.priority };
    }

    updateRequests.clear();
}

void LandscapePageManager::RejectRequests()
{
    updateRequests.clear();
}

bool LandscapePageManager::InvalidatePage(uint32 level, uint32 x, uint32 y)
{
    uint64 pageKey = PAGE_KEY(level, x, y);

    auto uFound = std::find_if(updateRequests.begin(), updateRequests.end(), [&pageKey](const UpdateRequest& r) { return (r.pageKey == pageKey); });
    if (uFound != updateRequests.end())
    {
        updateRequests.erase(uFound);
    }

    auto found = residentPages.find(pageKey);
    if (found != residentPages.cend())
    {
        vTexture->FreePage(found->second.pageID);
        residentPages.erase(found);

        return true;
    }

    return false;
}

void LandscapePageManager::Invalidate()
{
    residentPages.clear();
    updateRequests.clear();
    vTexture->FreePages();
}

bool LandscapePageManager::TryFreePage(float32 pagePriority)
{
    //search one visible and one invisible page with smallest radius-error
    auto it = residentPages.cbegin();
    auto end = residentPages.cend();
    auto invisiblePage = residentPages.cend();
    auto visiblePage = residentPages.cend();
    float32 lowestInvisiblePriority = std::numeric_limits<float32>::max();
    float32 lowestVisiblePriority = pagePriority;
    bool pageFreed = false;

    //GFX_COMPLETE try optimize page searching?
    uint32 frameIndex = Engine::Instance()->GetGlobalFrameIndex();
    for (; it != end; ++it)
    {
        if (it->second.updateID != frameIndex && lowestInvisiblePriority > it->second.priority)
        {
            lowestInvisiblePriority = it->second.priority;
            invisiblePage = it;
        }

        if (it->second.updateID == frameIndex && lowestVisiblePriority > it->second.priority)
        {
            lowestVisiblePriority = it->second.priority;
            visiblePage = it;
        }
    }

    if (invisiblePage != residentPages.cend())
    {
        //try free invisible page
        vTexture->FreePage(invisiblePage->second.pageID);
        residentPages.erase(invisiblePage);

        pageFreed = true;
    }
    else if (visiblePage != residentPages.cend())
    {
        //try free visible page
        vTexture->FreePage(visiblePage->second.pageID);
        residentPages.erase(visiblePage);

        pageFreed = true;
    }

    return pageFreed;
}

Vector4 LandscapePageManager::MapToPage(uint32 level, uint32 x, uint32 y, uint32 pLevel, int32 pageID)
{
    const VirtualTexture::PageInfo& page = vTexture->GetPageInfo(pageID);
    DVASSERT(!page.isFree);

    float32 pageSize = vTexture->GetPageSize() - 2.f * pageTexelOffset;
    Vector2 pageOffset = { page.offsetX + pageTexelOffset, page.offsetY + pageTexelOffset };

    uint32 levelDelta = level - pLevel;
    uint32 originX = (x >> levelDelta) << levelDelta;
    uint32 originY = (y >> levelDelta) << levelDelta;
    float32 sizeDelta = float32(1 << levelDelta);

    Vector2 patchCoord(float32(x - originX), float32(y - originY));
    Vector2 patchOffset = patchCoord / sizeDelta * pageSize;
    float32 patchSize = pageSize / sizeDelta;

    Vector4 mapping;
    mapping.x = (pageOffset.x + patchOffset.x) / vTexture->GetWidth();
    mapping.y = (pageOffset.y + patchOffset.y) / vTexture->GetHeight();
    mapping.z = patchSize / vTexture->GetWidth();
    mapping.w = patchSize / vTexture->GetHeight();

    return mapping;
}

Vector4 LandscapePageManager::GetPageUV(uint64 pageKey)
{
    uint32 level = PAGE_LEVEL(pageKey);
    uint32 x = PAGE_X(pageKey);
    uint32 y = PAGE_Y(pageKey);

    Vector4 uv;
    float32 levelSize = float32(LandscapeSubdivision::GetLevelSize(level));
    float32 texelOffset = pageTexelOffset / (vTexture->GetPageSize() - 2 * pageTexelOffset);

    uv.x = (x - texelOffset) / levelSize;
    uv.y = (y - texelOffset) / levelSize;
    uv.z = ((x + 1) + texelOffset) / levelSize;
    uv.w = ((y + 1) + texelOffset) / levelSize;

    return uv;
}

AABBox3 LandscapePageManager::GetPageBBox(uint64 pageKey, const LandscapeSubdivision* subdivision)
{
    float32 pageBoxScale = vTexture->GetPageSize() / (vTexture->GetPageSize() - 2 * pageTexelOffset);

    AABBox3 patchBBox = subdivision->GetPatchAABBox(PAGE_LEVEL(pageKey), PAGE_X(pageKey), PAGE_Y(pageKey));
    Vector3 bboxCenter = patchBBox.GetCenter();
    Vector3 pageBoxHalfSize = patchBBox.GetSize() * 0.5f * pageBoxScale;

    return AABBox3(bboxCenter - pageBoxHalfSize, bboxCenter + pageBoxHalfSize);
}

#undef PAGE_KEY
#undef PAGE_LEVEL
#undef PAGE_X
#undef PAGE_Y

}; //ns DAVA
