#include "REPlatform/Commands/SplineEditorCommands.h"

#include <Reflection/ReflectionRegistrator.h>
#include <Debug/DVAssert.h>

namespace DAVA
{
SplinePointAddRemoveCommand::SplinePointAddRemoveCommand(Entity* entity_, SplineComponent* spline_, SplineComponent::SplinePoint* point_, size_t pointIndex_, const String& description)
    : RECommand(description)
    , spline(spline_)
    , point(point_)
    , pointIndex(pointIndex_)
{
}

SplinePointAddRemoveCommand::~SplinePointAddRemoveCommand()
{
    if (isPointInserted == false)
    {
        delete point;
    }
}

DAVA::SplineComponent* SplinePointAddRemoveCommand::GetSpline() const
{
    return spline;
}

DAVA::SplineComponent::SplinePoint* SplinePointAddRemoveCommand::GetSplinePoint() const
{
    return point;
}

size_t SplinePointAddRemoveCommand::GetSplinePointIndex() const
{
    return pointIndex;
}

bool SplinePointAddRemoveCommand::IsPointInserted() const
{
    return isPointInserted;
}

void SplinePointAddRemoveCommand::AddSplinePoint()
{
    spline->controlPoints.insert(spline->controlPoints.begin() + pointIndex, point);
    isPointInserted = true;
}

void SplinePointAddRemoveCommand::RemoveSplinePoint()
{
    spline->controlPoints.erase(spline->controlPoints.begin() + pointIndex);
    isPointInserted = false;
}

//////////////////////////////////////////////////////////////////////////

AddSplinePointCommand::AddSplinePointCommand(Entity* entity, SplineComponent* spline, SplineComponent::SplinePoint* point, size_t pointIndex)
    : SplinePointAddRemoveCommand(entity, spline, point, pointIndex, "Add Spline Point")
{
}

void AddSplinePointCommand::Redo()
{
    AddSplinePoint();
}

void AddSplinePointCommand::Undo()
{
    RemoveSplinePoint();
}

DAVA_VIRTUAL_REFLECTION_IMPL(AddSplinePointCommand)
{
    ReflectionRegistrator<AddSplinePointCommand>::Begin()
    .End();
}

//////////////////////////////////////////////////////////////////////////

RemoveSplinePointCommand::RemoveSplinePointCommand(Entity* entity, SplineComponent* spline, SplineComponent::SplinePoint* point, size_t pointIndex)
    : SplinePointAddRemoveCommand(entity, spline, point, pointIndex, "Remove point")
{
    isPointInserted = true;
}

void RemoveSplinePointCommand::Redo()
{
    RemoveSplinePoint();
}

void RemoveSplinePointCommand::Undo()
{
    AddSplinePoint();
}

DAVA_VIRTUAL_REFLECTION_IMPL(RemoveSplinePointCommand)
{
    ReflectionRegistrator<RemoveSplinePointCommand>::Begin()
    .End();
}
} // namespace DAVA
