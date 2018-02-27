#pragma once

#include <REPlatform/DataNodes/Selectable.h>

#include <TArc/Core/ContextAccessor.h>

class SplinePointTransformProxy : public DAVA::Selectable::TransformProxy
{
public:
    SplinePointTransformProxy(DAVA::ContextAccessor* accessor);

private:
    const DAVA::Matrix4& GetWorldTransform(const DAVA::Any& object) override;
    DAVA::Matrix4 GetLocalTransform(const DAVA::Any& object) override;
    void SetLocalTransform(DAVA::Any& object, const DAVA::Matrix4& matrix) override;
    bool SupportsTransformType(const DAVA::Any& object, DAVA::Selectable::TransformType) const override;
    bool TransformDependsFromObject(const DAVA::Any& dependant, const DAVA::Any& dependsOn) const override;

    DAVA::ContextAccessor* accessor = nullptr;
};