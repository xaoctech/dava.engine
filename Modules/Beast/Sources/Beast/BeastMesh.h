#ifdef __DAVAENGINE_BEAST__

#ifndef __BEAST_MESH__
#define __BEAST_MESH__

#include "DAVAEngine.h"
#include "BeastTypes.h"
#include "BeastResource.h"
#include "BeastManager.h"

class LandscapeGeometry;
class BeastMesh : public BeastResource<BeastMesh>
{
public:
    void InitWithMeshInstancePart(DAVA::RenderBatch* batch, DAVA::int32 partIndex);
    void InitWithSpeedTreeLeaf(DAVA::RenderBatch* batch, DAVA::int32 partIndex, const DAVA::Matrix4& leafTransform);

    void InitWithLandscape(LandscapeGeometry* geometry, BeastMaterial* material);

    DAVA_BEAST::ILBMeshHandle GetILBMesh() const;
    DAVA::int32 GetTextureCoordCount() const;

private:
    friend class BeastResource<BeastMesh>;
    BeastMesh(const DAVA::String& name, BeastManager* manager);

    bool CheckVertexFormat(DAVA::PolygonGroup* polygonGroup);
    bool HasTangents(DAVA::PolygonGroup* polygonGroup);

    void AddVertices(DAVA::PolygonGroup* polygonGroup);
    void AddTranfromedVertices(DAVA::PolygonGroup* polygonGroup, const DAVA::Matrix4&);
    void AddIndices(DAVA::PolygonGroup* polygonGroup, DAVA::NMaterial* material);
    void AddUV(DAVA::PolygonGroup* polygonGroup, DAVA::int32 textureCoordIndex);
    void AddVertices(LandscapeGeometry* geometry);
    void AddTangents(DAVA::PolygonGroup* polygonGroup);
    void AddIndices(LandscapeGeometry* geometry, BeastMaterial* material);
    void AddUV(LandscapeGeometry* geometry);

private:
    DAVA_BEAST::ILBMeshHandle mesh = nullptr;
    DAVA::int32 indecesCount = 0;
    DAVA::int32 textureCoordCount = 0;
};

#endif //__BEAST_MESH__

#endif //__DAVAENGINE_BEAST__
