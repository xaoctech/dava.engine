#pragma once 

#include "Base/BaseObject.h"
#include "Base/BaseTypes.h"
#include "Base/IntrospectionBase.h"
#include "Math/Vector.h"
#include "Math/AABBox3.h"
#include "Reflection/Reflection.h"
#include "Render/Highlevel/LandscapePageRenderer.h"

namespace DAVA
{
class NMaterial;
class Texture;
struct PageRenderParams;

class LandscapeLayerRenderer : public LandscapePageRenderer
{
public:
    LandscapeLayerRenderer(uint32 lodCount);
    ~LandscapeLayerRenderer();

    LandscapeLayerRenderer* Clone() const;
    bool RenderPage(const PageRenderParams& params) override;

    uint32 GetLODCount() const;

    void SetTerrainLODMaterial(uint32 lod, NMaterial* material);
    NMaterial* GetTerrainLODMaterial(uint32 lod) const;

    void SetDecorationLODMaterial(uint32 lod, NMaterial* material);
    NMaterial* GetDecorationLODMaterial(uint32 lod) const;

protected:
    Vector<NMaterial*> terrainMaterials;
    Vector<NMaterial*> decorationMaterials;

    NMaterial* GetMicroMaterialRefl() const;
    void SetMicroMaterialRefl(NMaterial* material);

    NMaterial* GetMiddleMaterialRefl() const;
    void SetMiddleMaterialRefl(NMaterial* material);

    NMaterial* GetMacroMaterialRefl() const;
    void SetMacroMaterialRefl(NMaterial* material);

    DAVA_VIRTUAL_REFLECTION(LandscapeLayerRenderer, LandscapePageRenderer);
};

inline uint32 LandscapeLayerRenderer::GetLODCount() const
{
    return uint32(terrainMaterials.size());
}

inline void LandscapeLayerRenderer::SetTerrainLODMaterial(uint32 lod, NMaterial* material)
{
    DVASSERT(lod < GetLODCount());

    SafeRelease(terrainMaterials[lod]);
    terrainMaterials[lod] = SafeRetain(material);
}

inline NMaterial* LandscapeLayerRenderer::GetTerrainLODMaterial(uint32 lod) const
{
    DVASSERT(lod < GetLODCount());
    return terrainMaterials[lod];
}

inline void LandscapeLayerRenderer::SetDecorationLODMaterial(uint32 lod, NMaterial* material)
{
    DVASSERT(lod < GetLODCount());

    SafeRelease(decorationMaterials[lod]);
    decorationMaterials[lod] = SafeRetain(material);
}

inline NMaterial* LandscapeLayerRenderer::GetDecorationLODMaterial(uint32 lod) const
{
    DVASSERT(lod < GetLODCount());
    return decorationMaterials[lod];
}
}; //ns DAVA