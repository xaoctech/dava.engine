#pragma once

#include "Base/BaseTypes.h"
#include "Reflection/Reflection.h"
#include "Reflection/ReflectionRegistrator.h"
#include "Entity/Component.h"

namespace DAVA
{
class Entity;
class KeyedArchive;
class SerializationContext;
class StaticLightingSystem;
class LightmapSingleComponent;
class LightmapComponent : public Component
{
public:
    class LightmapParam : public ReflectionBase
    {
    public:
        uint32 GetLightmapSize() const;
        void SetLightmapSize(uint32 size);

        bool IsCastShadow() const;
        void SetCastShadow(bool cast);

        bool IsReceiveShadow() const;
        void SetReceiveShadow(bool receive);

        const float32* GetLightmapSizePtr() const; //is used for dynamic property binding

        bool operator==(const LightmapParam& other) const
        {
            return GetLightmapSize() == other.GetLightmapSize() &&
            castShadow == other.castShadow &&
            receiveShadow == other.receiveShadow;
        }

    protected:
        LightmapComponent* component = nullptr;
        float32 lightmapSize;
        bool castShadow = true;
        bool receiveShadow = true;

        LightmapSingleComponent* GetSingletonComponent() const;

        DAVA_VIRTUAL_REFLECTION_IN_PLACE(LightmapParam, ReflectionBase)
        {
            ReflectionRegistrator<LightmapParam>::Begin()
            .Field("lightmapSize", &LightmapParam::GetLightmapSize, &LightmapParam::SetLightmapSize)
            .Field("castShadow", &LightmapParam::IsCastShadow, &LightmapParam::SetCastShadow)
            .Field("receiveShadow", &LightmapParam::IsReceiveShadow, &LightmapParam::SetReceiveShadow)
            .End();
        }

        friend class LightmapComponent;
    };

    LightmapComponent() = default;
    ~LightmapComponent() = default;

    Component* Clone(Entity* toEntity) override;
    void Serialize(KeyedArchive* archive, SerializationContext* serializationContext) override;
    void Deserialize(KeyedArchive* archive, SerializationContext* serializationContext) override;

    uint32 GetParamsCount() const;
    void SetParamsCount(uint32 count);

    LightmapParam& GetLightmapParam(uint32 index);

protected:
    Vector<LightmapParam> params;

    DAVA_VIRTUAL_REFLECTION(LightmapComponent, Component);
};

template <>
struct AnyCompare<LightmapComponent::LightmapParam>
{
    static bool IsEqual(const Any& v1, const Any& v2)
    {
        return v1.Get<LightmapComponent::LightmapParam>() == v2.Get<LightmapComponent::LightmapParam>();
    }
};
}
