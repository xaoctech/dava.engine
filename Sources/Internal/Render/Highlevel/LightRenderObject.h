#pragma once

#include "Render/Highlevel/RenderObject.h"

namespace DAVA
{
class LightRenderObject : public RenderObject
{
public:
    LightRenderObject();

    void SetLight(Light* light);
    void InvalidateMaterial();
    void PrepareToRender(Camera* camera) override;

private:
    RefPtr<RenderBatch> renderBatch;
    ScopedPtr<NMaterial> backgroundMaterial;
    ScopedPtr<NMaterial> sunDiskMaterial;
    ScopedPtr<PolygonGroup> backgroundQuadPolygon;
    Matrix4 worldTransform = Matrix4::IDENTITY;
    Vector4 lightDirection = Vector4(0.0f, -1.0f, 0.0f, 0.0f);
    Light* sourceLight = nullptr;
};
}
