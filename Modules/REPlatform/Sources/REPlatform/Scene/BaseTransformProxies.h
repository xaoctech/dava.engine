#pragma once

#include "REPlatform/DataNodes/Selectable.h"

namespace DAVA
{
class EntityTransformProxy : public Selectable::TransformProxy
{
public:
    const Matrix4& GetWorldTransform(const Any& object) override;
    const Matrix4& GetLocalTransform(const Any& object) override;
    void SetLocalTransform(Any& object, const Matrix4& matrix) override;
    bool SupportsTransformType(const Any& object, Selectable::TransformType) const override;
    bool TransformDependsFromObject(const Any& dependant, const Any& dependsOn) const override;
};

class EmitterTransformProxy : public Selectable::TransformProxy
{
public:
    const Matrix4& GetWorldTransform(const Any& object) override;
    const Matrix4& GetLocalTransform(const Any& object) override;
    void SetLocalTransform(Any& object, const Matrix4& matrix) override;
    bool SupportsTransformType(const Any& object, Selectable::TransformType) const override;
    bool TransformDependsFromObject(const Any& dependant, const Any& dependsOn) const override;
};
} // namespace DAVA
