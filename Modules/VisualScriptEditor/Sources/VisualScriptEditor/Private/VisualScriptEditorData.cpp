#include "VisualScriptEditor/Private/VisualScriptEditorData.h"

#include <VisualScript/VisualScript.h>
#include <VisualScript/VisualScriptNode.h>

#include <Base/Vector.h>

namespace DAVA
{
const char* VisualScriptEditorData::scriptDescriptorFieldName = "scriptDescriptorFieldName";
const char* VisualScriptEditorData::scriptNodesFieldName = "scriptNodesFieldName";
const char* VisualScriptEditorData::reflectionHolderProperty = "reflectionHolderProperty";

DAVA_VIRTUAL_REFLECTION_IMPL(VisualScriptEditorData)
{
    DAVA::ReflectionRegistrator<VisualScriptEditorData>::Begin()
    .Field(scriptDescriptorFieldName, [](const VisualScriptEditorData* pThis) { return pThis->activeDescriptor; }, nullptr)
    .Field(scriptNodesFieldName, [](const VisualScriptEditorData* pThis) {
        if (pThis->activeDescriptor != nullptr)
        {
            return pThis->activeDescriptor->script->GetNodes();
        }
        return Vector<VisualScriptNode*>();
    },
           nullptr)
    .Field(reflectionHolderProperty, &VisualScriptEditorData::GetReflection, nullptr)
    .End();
}

template <>
bool AnyCompare<VisualScriptEditorReflectionHolder>::IsEqual(const Any& v1, const Any& v2)
{
    const VisualScriptEditorReflectionHolder rh1 = v1.Get<VisualScriptEditorReflectionHolder>();
    const VisualScriptEditorReflectionHolder rh2 = v2.Get<VisualScriptEditorReflectionHolder>();
    return AnyCompare<Vector<Reflection>>::IsEqual(rh1.reflectedModels, rh2.reflectedModels);
}

} //DAVA
