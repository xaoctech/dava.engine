#include "UI/Preview/ScaleComboBoxData.h"

#include "Modules/DocumentsModule/EditorCanvasData.h"

#include <TArc/Core/ContextAccessor.h>

DAVA::FastName ScaleComboBoxData::scalePropertyName{ "scale" };
DAVA::FastName ScaleComboBoxData::enumeratorPropertyName{ "enumerator" };
DAVA::FastName ScaleComboBoxData::enabledPropertyName{ "enabled" };

DAVA_REFLECTION_IMPL(ScaleComboBoxData)
{
    DAVA::ReflectionRegistrator<ScaleComboBoxData>::Begin()
    .Field(scalePropertyName.c_str(), &ScaleComboBoxData::GetScale, &ScaleComboBoxData::SetScale)
    .Field(enumeratorPropertyName.c_str(), &ScaleComboBoxData::GetScales, nullptr)
    .Field(enabledPropertyName.c_str(), &ScaleComboBoxData::IsEnabled, nullptr)
    .End();
}

ScaleComboBoxData::ScaleComboBoxData(DAVA::TArc::ContextAccessor* accessor)
{
    editorCanvasDataWrapper = accessor->CreateWrapper(DAVA::ReflectedTypeDB::Get<EditorCanvasData>());
}

DAVA::Any ScaleComboBoxData::GetScale() const
{
    if (editorCanvasDataWrapper.HasData())
    {
        return editorCanvasDataWrapper.GetFieldValue(EditorCanvasData::scalePropertyName);
    }
    else
    {
        return DAVA::Any();
    }
}

void ScaleComboBoxData::SetScale(const DAVA::Any& scale)
{
    DVASSERT(editorCanvasDataWrapper.HasData());
    editorCanvasDataWrapper.SetFieldValue(EditorCanvasData::scalePropertyName, scale);
}

DAVA::Any ScaleComboBoxData::GetScales() const
{
    if (editorCanvasDataWrapper.HasData())
    {
        return editorCanvasDataWrapper.GetFieldValue(EditorCanvasData::predefinedScalesPropertyName);
    }
    else
    {
        return DAVA::Any();
    }
}

DAVA::Any ScaleComboBoxData::IsEnabled() const
{
    return editorCanvasDataWrapper.HasData();
}
