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

ScaleComboBoxData::ScaleComboBoxData(DAVA::TArc::ContextAccessor* accessor_)
    : accessor(accessor_)
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

const DAVA::Vector<DAVA::float32>& ScaleComboBoxData::GetScales() const
{
    DAVA::TArc::DataContext* activeContext = accessor->GetActiveContext();
    if (activeContext == nullptr)
    {
        static DAVA::Vector<DAVA::float32> emptyData;
        return emptyData;
    }
    else
    {
        return activeContext->GetData<EditorCanvasData>()->GetPredefinedScales();
    }
}

bool ScaleComboBoxData::IsEnabled() const
{
    return editorCanvasDataWrapper.HasData();
}
