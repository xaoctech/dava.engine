#pragma once

#include <TArc/DataProcessing/DataWrapper.h>

#include <Base/BaseTypes.h>
#include <Reflection/Reflection.h>

namespace DAVA
{
namespace TArc
{
class ContextAccessor;
}
}

class ScaleComboBoxData
{
public:
    static DAVA::FastName scalePropertyName;
    static DAVA::FastName enumeratorPropertyName;
    static DAVA::FastName enabledPropertyName;

    ScaleComboBoxData(DAVA::TArc::ContextAccessor* accessor);

private:
    DAVA::Any GetScale() const;
    void SetScale(const DAVA::Any& scale);

    DAVA::Any GetScales() const;

    DAVA::Any IsEnabled() const;

    DAVA::TArc::DataWrapper editorCanvasDataWrapper;

    DAVA_REFLECTION(EditorCanvasData);
};
