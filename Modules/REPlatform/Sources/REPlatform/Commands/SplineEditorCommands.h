#pragma once

#include "REPlatform/Commands/RECommand.h"

#include <Base/BaseTypes.h>
#include <Reflection/Reflection.h>
#include <Scene3D/Components/SplineComponent.h>
#include <Scene3D/Scene.h>

namespace DAVA
{
class SplinePointAddRemoveCommand : public RECommand
{
public:
    SplinePointAddRemoveCommand(Entity* entity, SplineComponent* spline, SplineComponent::SplinePoint* point, size_t pointIndex, const String& description);
    ~SplinePointAddRemoveCommand() override;

    SplineComponent* GetSpline() const;
    SplineComponent::SplinePoint* GetSplinePoint() const;
    size_t GetSplinePointIndex() const;
    bool IsPointInserted() const;

protected:
    void AddSplinePoint();
    void RemoveSplinePoint();

    SplineComponent* spline = nullptr;
    SplineComponent::SplinePoint* point = nullptr;
    size_t pointIndex = 0;
    bool isPointInserted = false;
};

class AddSplinePointCommand : public SplinePointAddRemoveCommand
{
public:
    AddSplinePointCommand(Entity* entity, SplineComponent* spline, SplineComponent::SplinePoint* point, size_t pointIndex);

    void Redo() override;
    void Undo() override;
    DAVA_VIRTUAL_REFLECTION(AddSplinePointCommand, RECommand);
};

class RemoveSplinePointCommand : public SplinePointAddRemoveCommand
{
public:
    RemoveSplinePointCommand(Entity* entity, SplineComponent* spline, SplineComponent::SplinePoint* point, size_t pointIndex);

    void Redo() override;
    void Undo() override;
    DAVA_VIRTUAL_REFLECTION(RemoveSplinePointCommand, RECommand);
};
} // namespace DAVA
