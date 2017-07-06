#include "UI/Preview/ScrollBarAdapter.h"

#include "Modules/DocumentsModule/EditorCanvasData.h"

#include <TArc/Core/ContextAccessor.h>

#include <Reflection/ReflectedTypeDB.h>

DAVA::FastName ScrollBarAdapter::positionPropertyName{ "position" };
DAVA::FastName ScrollBarAdapter::minPosPropertyName{ "minimum position" };
DAVA::FastName ScrollBarAdapter::maxPosPropertyName{ "maximum position" };
DAVA::FastName ScrollBarAdapter::pageStepPropertyName{ "page step" };
DAVA::FastName ScrollBarAdapter::enabledPropertyName{ "enabled" };
DAVA::FastName ScrollBarAdapter::visiblePropertyName{ "visible" };
DAVA::FastName ScrollBarAdapter::orientationPropertyName{ "orientation" };

DAVA_REFLECTION_IMPL(ScrollBarAdapter)
{
    DAVA::ReflectionRegistrator<ScrollBarAdapter>::Begin()
    .Field(positionPropertyName.c_str(), &ScrollBarAdapter::GetPosition, &ScrollBarAdapter::SetPosition)
    .Field(minPosPropertyName.c_str(), &ScrollBarAdapter::GetMinPos, nullptr)
    .Field(maxPosPropertyName.c_str(), &ScrollBarAdapter::GetMaxPos, nullptr)
    .Field(pageStepPropertyName.c_str(), &ScrollBarAdapter::GetPageStep, nullptr)
    .Field(enabledPropertyName.c_str(), &ScrollBarAdapter::IsEnabled, nullptr)
    .Field(visiblePropertyName.c_str(), &ScrollBarAdapter::IsVisible, nullptr)
    .Field(orientationPropertyName.c_str(), &ScrollBarAdapter::GetOrientation, nullptr)
    .End();
}

ScrollBarAdapter::ScrollBarAdapter(DAVA::Vector2::eAxis orientation_, DAVA::TArc::ContextAccessor* accessor_)
    : orientation(orientation_)
    , accessor(accessor_)
{
    editorCanvasDataWrapper = accessor->CreateWrapper(DAVA::ReflectedTypeDB::Get<EditorCanvasData>());
}

int ScrollBarAdapter::GetPosition() const
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

void ScrollBarAdapter::SetPosition(int value)
{
    using namespace DAVA;
    DVASSERT(editorCanvasDataWrapper.HasData());
    Any positionValue = editorCanvasDataWrapper.GetFieldValue(EditorCanvasData::positionPropertyName);
    DVASSERT(positionValue.CanGet<Vector2>());
    Vector2 position = positionValue.Get<Vector2>();
    position[orientation] = static_cast<float32>(value);
    editorCanvasDataWrapper.SetFieldValue(EditorCanvasData::positionPropertyName, position);
}

int ScrollBarAdapter::GetMinPos() const
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

int ScrollBarAdapter::GetMaxPos() const
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

int ScrollBarAdapter::GetPageStep() const
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

bool ScrollBarAdapter::IsEnabled() const
{
    return editorCanvasDataWrapper.HasData();
}

bool ScrollBarAdapter::IsVisible() const
{
    return GetMaxPos() > 0;
}

Qt::Orientation ScrollBarAdapter::GetOrientation() const
{
    return orientation == DAVA::Vector2::AXIS_X ? Qt::Horizontal : Qt::Vertical;
}
