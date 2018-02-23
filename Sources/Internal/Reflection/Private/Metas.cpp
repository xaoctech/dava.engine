#include "Reflection/Private/Metas.h"

#include "Debug/DVAssert.h"
#include "Reflection/ReflectedMeta.h"

namespace DAVA
{
namespace Metas
{
DisplayName::DisplayName(const String& displayName_)
    : displayName(displayName_)
{
}

Range::Range(const Any& minValue_, const Any& maxValue_, const Any& step_)
    : minValue(minValue_)
    , maxValue(maxValue_)
    , step(step_)
{
}

FloatNumberAccuracy::FloatNumberAccuracy(uint32 accuracy_)
    : accuracy(accuracy_)
{
}

MaxLength::MaxLength(uint32 length_)
    : length(length_)
{
}

Validator::Validator(const TValidationFn& fn_)
    : fn(fn_)
{
}

ValidationResult Validator::Validate(const Any& value, const Any& prevValue) const
{
    DVASSERT(fn != nullptr);
    return fn(value, prevValue);
}

File::File(const String& filters_, const String& dlgTitle_)
    : filters(filters_)
    , dlgTitle(dlgTitle_)
{
}

String File::GetDefaultPath() const
{
    return "";
}

String File::GetRootDirectory() const
{
    return "";
}

Group::Group(const char* groupName_)
    : groupName(groupName_)
{
}

ValueDescription::ValueDescription(const TValueDescriptorFn& fn_)
    : fn(fn_)
{
    DVASSERT(fn != nullptr);
}

String ValueDescription::GetDescription(const Any& v) const
{
    return fn(v);
}

Tooltip::Tooltip(const String& tooltipFieldName_)
    : tooltipFieldName(tooltipFieldName_)
{
}

Replicable::Replicable()
    : privacy(Privacy::AS_PARENT)
{
}

Replicable::Replicable(Privacy privacy)
    : privacy(privacy)
{
}

Privacy Replicable::GetMergedPrivacy(Privacy parentPrivacy) const
{
    DVASSERT(parentPrivacy < M::Privacy::AS_PARENT);

    Privacy ret = privacy;
    if (privacy == M::Privacy::AS_PARENT)
    {
        ret = parentPrivacy;
    }

    DVASSERT(ret <= parentPrivacy);
    return ret;
}

//////////////////////////////////////////////////////////////////////////

QuaternionNetPacking::QuaternionNetPacking(uint32 bitsPerComponent)
    : bitsPerComponent(bitsPerComponent)
{
}

FloatNetPacking::FloatNetPacking(float32 rangeFull, float32 precisionFull, float32 rangeDelta, float32 precisionDelta)
    : layoutFull(ComputeLayout(rangeFull, precisionFull))
    , layoutDelta(ComputeLayout(rangeDelta, precisionDelta))
{
}

FloatNetPacking::FloatNetPacking(float32 range, float32 precision)
    : layoutFull(ComputeLayout(range, precision))
    , layoutDelta(ComputeLayout(range, precision))
{
}

FloatNetPacking::Layout FloatNetPacking::ComputeLayout(float32 range, float32 precision)
{
    FloatNetPacking::Layout layout;

    layout.intBits = static_cast<uint32>(std::log2(range + range)) + 1;
    layout.halfRange = std::floor(static_cast<float32>((1 << layout.intBits) - 1) / 2.f);

    layout.fracBits = static_cast<uint32>(std::log2(1.f / precision)) + 1;
    layout.precision = 1.f / static_cast<float32>((1 << layout.fracBits) - 1);
    return layout;
}

SystemProcess::SystemProcess::SystemProcess(Group group_, Type type_, float32 order_)
    : group(group_)
    , type(type_)
    , order(order_)
{
}

} // namespace Metas
} // namespace DAVA
