#include "UI/Preview/ScrolLBarData.h"

#include "Modules/DocumentsModule/EditorCanvasData.h"

#include <TArc/Core/ContextAccessor.h>

#include <Reflection/ReflectedTypeDB.h>

DAVA::FastName ScrollBarData::positionPropertyName{ "position" };
DAVA::FastName ScrollBarData::minPosPropertyName{ "minimum position" };
DAVA::FastName ScrollBarData::maxPosPropertyName{ "maximum position" };
DAVA::FastName ScrollBarData::pageStepPropertyName{ "page step" };
DAVA::FastName ScrollBarData::enabledPropertyName{ "enabled" };
DAVA::FastName ScrollBarData::visiblePropertyName{ "visible" };
DAVA::FastName ScrollBarData::orientationPropertyName{ "orientation" };

DAVA_REFLECTION_IMPL(ScrollBarData)
{
    DAVA::ReflectionRegistrator<ScrollBarData>::Begin()
    .Field(positionPropertyName.c_str(), &ScrollBarData::GetPosition, &ScrollBarData::SetPosition)
    .Field(minPosPropertyName.c_str(), &ScrollBarData::GetMinPos, nullptr)
    .Field(maxPosPropertyName.c_str(), &ScrollBarData::GetMaxPos, nullptr)
    .Field(pageStepPropertyName.c_str(), &ScrollBarData::GetPageStep, nullptr)
    .Field(enabledPropertyName.c_str(), &ScrollBarData::IsEnabled, nullptr)
    .Field(visiblePropertyName.c_str(), &ScrollBarData::IsVisible, nullptr)
    .Field(orientationPropertyName.c_str(), &ScrollBarData::GetOrientation, nullptr)
    .End();
}

ScrollBarData::ScrollBarData(DAVA::Vector2::eAxis orientation_, DAVA::TArc::ContextAccessor* accessor_)
    : orientation(orientation_)
    , accessor(accessor_)
{
    editorCanvasDataWrapper = accessor->CreateWrapper(DAVA::ReflectedTypeDB::Get<EditorCanvasData>());
}

int ScrollBarData::GetPosition() const
{
    using namespace DAVA;
    if (editorCanvasDataWrapper.HasData())
    {
        Any positionValue = editorCanvasDataWrapper.GetFieldValue(EditorCanvasData::positionPropertyName);
        DVASSERT(positionValue.CanGet<Vector2>());
        Vector2 position = positionValue.Get<Vector2>();
        return static_cast<int>(position[orientation]);
    }
    else
    {
        return 0;
    }
}

void ScrollBarData::SetPosition(int value)
{
    using namespace DAVA;
    DVASSERT(editorCanvasDataWrapper.HasData());
    Any positionValue = editorCanvasDataWrapper.GetFieldValue(EditorCanvasData::positionPropertyName);
    DVASSERT(positionValue.CanGet<Vector2>());
    Vector2 position = positionValue.Get<Vector2>();
    position[orientation] = static_cast<float32>(value);
    editorCanvasDataWrapper.SetFieldValue(EditorCanvasData::positionPropertyName, position);
}

int ScrollBarData::GetMinPos() const
{
    using namespace DAVA;
    if (editorCanvasDataWrapper.HasData())
    {
        Any minPosValue = editorCanvasDataWrapper.GetFieldValue(EditorCanvasData::minimumPositionPropertyName);
        DVASSERT(minPosValue.CanGet<Vector2>());
        Vector2 minPos = minPosValue.Get<Vector2>();
        return static_cast<int>(minPos[orientation]);
    }
    return 0;
}

int ScrollBarData::GetMaxPos() const
{
    using namespace DAVA;
    if (editorCanvasDataWrapper.HasData())
    {
        Any maxPosValue = editorCanvasDataWrapper.GetFieldValue(EditorCanvasData::maximumPositionPropertyName);
        DVASSERT(maxPosValue.CanGet<Vector2>());
        Vector2 maxPos = maxPosValue.Get<Vector2>();
        return static_cast<int>(maxPos[orientation]);
    }
    return 0;
}

int ScrollBarData::GetPageStep() const
{
    using namespace DAVA;
    using namespace DAVA::TArc;

    DataContext* activeContext = accessor->GetActiveContext();
    if (activeContext == nullptr)
    {
        return 0;
    }
    else
    {
        EditorCanvasData* canvasData = activeContext->GetData<EditorCanvasData>();
        return canvasData->GetViewSize()[orientation];
    }
}

bool ScrollBarData::IsEnabled() const
{
    return editorCanvasDataWrapper.HasData();
}

bool ScrollBarData::IsVisible() const
{
    return GetMaxPos() > 0;
}

Qt::Orientation ScrollBarData::GetOrientation() const
{
    return orientation == DAVA::Vector2::AXIS_X ? Qt::Horizontal : Qt::Vertical;
}
