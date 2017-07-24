#pragma once

#include "UI/Preview/Data/CanvasDataAdapter.h"

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

class ScaleComboBoxAdapter
{
public:
    static DAVA::FastName scalePropertyName;
    static DAVA::FastName enumeratorPropertyName;
    static DAVA::FastName enabledPropertyName;

    ScaleComboBoxAdapter(DAVA::TArc::ContextAccessor* accessor);

private:
    DAVA::Any GetScale() const;
    void SetScale(const DAVA::Any& scale);

    const DAVA::Vector<DAVA::float32>& GetScales() const;

    bool IsEnabled() const;

    CanvasDataAdapter canvasDataAdapter;
    DAVA::TArc::ContextAccessor* accessor = nullptr;

    DAVA_REFLECTION(CanvasData);
};
