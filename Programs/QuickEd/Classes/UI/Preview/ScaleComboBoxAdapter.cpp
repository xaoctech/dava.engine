#include "UI/Preview/ScaleComboBoxAdapter.h"

#include "Modules/DocumentsModule/EditorCanvasData.h"

#include <TArc/Core/ContextAccessor.h>

DAVA::FastName ScaleComboBoxAdapter::scalePropertyName{ "scale" };
DAVA::FastName ScaleComboBoxAdapter::enumeratorPropertyName{ "enumerator" };
DAVA::FastName ScaleComboBoxAdapter::enabledPropertyName{ "enabled" };

DAVA_REFLECTION_IMPL(ScaleComboBoxAdapter)
{
    DAVA::ReflectionRegistrator<ScaleComboBoxAdapter>::Begin()
    .Field(scalePropertyName.c_str(), &ScaleComboBoxAdapter::GetScale, &ScaleComboBoxAdapter::SetScale)
    .Field(enumeratorPropertyName.c_str(), &ScaleComboBoxAdapter::GetScales, nullptr)
    .Field(enabledPropertyName.c_str(), &ScaleComboBoxAdapter::IsEnabled, nullptr)
    .End();
}

ScaleComboBoxAdapter::ScaleComboBoxAdapter(DAVA::TArc::ContextAccessor* accessor_)
    : accessor(accessor_)
{
    editorCanvasDataWrapper = accessor->CreateWrapper(DAVA::ReflectedTypeDB::Get<EditorCanvasData>());
}

DAVA::Any ScaleComboBoxAdapter::GetScale() const
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

void ScaleComboBoxAdapter::SetScale(const DAVA::Any& scale)
{
    DVASSERT(editorCanvasDataWrapper.HasData());
    editorCanvasDataWrapper.SetFieldValue(EditorCanvasData::scalePropertyName, scale);
}

const DAVA::Vector<DAVA::float32>& ScaleComboBoxAdapter::GetScales() const
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

bool ScaleComboBoxAdapter::IsEnabled() const
{
    return editorCanvasDataWrapper.HasData();
}
