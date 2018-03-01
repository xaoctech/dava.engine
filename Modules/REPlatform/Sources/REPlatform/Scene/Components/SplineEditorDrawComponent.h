#pragma once

#include <Entity/Component.h>
#include <Reflection/Reflection.h>

namespace DAVA
{
class SplineEditorDrawComponent : public Component
{
public:
    Component* Clone(Entity* toEntity) override;

    void Serialize(KeyedArchive* archive, SerializationContext* serializationContext) override;
    void Deserialize(KeyedArchive* archive, SerializationContext* serializationContext) override;

    bool IsSplineVisible() const;

    bool IsSelected() const;
    void SetSelected(bool);

    bool IsSnapped() const;
    void SetSnapped(bool);

    bool operator==(const SplineEditorDrawComponent& other) const;

private:
    bool isSelected = false;
    bool isSnapped = false;
    DAVA_VIRTUAL_REFLECTION(SplineEditorDrawComponent, Component);
};

//////////////////////////////////////////////////////////////////////////

inline bool SplineEditorDrawComponent::IsSplineVisible() const
{
    return isSelected || isSnapped;
}

inline bool SplineEditorDrawComponent::IsSelected() const
{
    return isSelected;
}

inline void SplineEditorDrawComponent::SetSelected(bool value)
{
    isSelected = value;
}

inline bool SplineEditorDrawComponent::IsSnapped() const
{
    return isSnapped;
}

inline void SplineEditorDrawComponent::SetSnapped(bool value)
{
    isSnapped = value;
}

inline DAVA::Component* SplineEditorDrawComponent::Clone(DAVA::Entity* toEntity)
{
    return nullptr;
}

inline void SplineEditorDrawComponent::Serialize(DAVA::KeyedArchive* archive, DAVA::SerializationContext* serializationContext)
{
}

inline void SplineEditorDrawComponent::Deserialize(DAVA::KeyedArchive* archive, DAVA::SerializationContext* serializationContext)
{
}

//////////////////////////////////////////////////////////////////////////

template <>
bool AnyCompare<SplineEditorDrawComponent>::IsEqual(const Any& v1, const Any& v2);
extern template struct AnyCompare<SplineEditorDrawComponent>;

} // ns DAVA