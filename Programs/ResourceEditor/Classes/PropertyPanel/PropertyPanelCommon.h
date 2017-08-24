#pragma once

#include <TArc/Controls/PropertyPanel/PropertyModelExtensions.h>

namespace PropertyPanel
{
enum eREPropertyType
{
    GroupQualityProperty = DAVA::TArc::PropertyNode::DomainSpecificProperty,
    AddComponentProperty,
    SlotName,
    SlotTypeFilters,
    SlotJointAttachment,
    SlotPreviewProperty,
    SlotTemplateName
};
} // namespace PropertyPanel
