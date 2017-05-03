#pragma once

#include "Entity/Component.h"

#include "Reflection/Reflection.h"

#include "FileSystem/FilePath.h"
#include "Base/Array.h"
#include "Base/FastName.h"
#include "Base/IntrospectionBase.h"

namespace DAVA
{
class Entity;
class SlotComponent : public Component
{
public:
    enum
    {
        MAX_FILTERS_COUNT = 8
    };

    IMPLEMENT_COMPONENT_TYPE(SLOT_COMPONENT)

    Component* Clone(Entity* toEntity) override;
    void Serialize(KeyedArchive* archive, SerializationContext* serializationContext) override;
    void Deserialize(KeyedArchive* archive, SerializationContext* serializationContext) override;

    FastName GetSlotName() const;
    void SetSlotName(FastName name);

    const Matrix4& GetAttachmentTransform() const;
    void SetAttachmentTransform(const Matrix4& transform);

    const FilePath& GetConfigFilePath() const;
    void SetConfigFilePath(const FilePath& path);

    uint32 GetTypeFiltersCount() const;
    FastName GetTypeFilter(uint32 index) const;
    void AddTypeFilter(FastName filter);
    void RemoveTypeFilter(uint32 index);
    void RemoveTypeFilter(FastName filter);

    FastName GetLoadedItemName() const;

    INTROSPECTION_EXTEND(SlotComponent, Component, nullptr);

    static const FastName SlotNameFieldName;
    static const FastName ConfigPathFieldName;

private:
    friend class SlotSystem;
    FastName slotName;

    Matrix4 attachmentTransform;
    FilePath configFilePath;
    Array<FastName, MAX_FILTERS_COUNT> typeFilters;
    uint32 actualFiltersCount = 0;

    FastName loadedItemName;

    DAVA_VIRTUAL_REFLECTION(SlotComponent, Component);
};

} // namespace DAVA
