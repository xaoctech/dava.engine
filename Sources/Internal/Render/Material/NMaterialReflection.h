#pragma once

#include "FileSystem/VariantType.h"
#include "Reflection/Reflection.h"
#include "Reflection/ReflectedTypeDB.h"
#include "Render/Shader/ShaderDescriptor.h"

namespace DAVA
{
class NMaterial;
struct NMaterialProperty;
struct MaterialTextureInfo;

class NMaterialValueWrapper : public ValueWrapper
{
public:
    const Type* GetType(const ReflectedObject& object) const override;
    bool IsReadonly(const ReflectedObject& object) const override;
    Any GetValue(const ReflectedObject& object) const override;
    bool SetValue(const ReflectedObject& object, const Any& value) const override;
    bool SetValueWithCast(const ReflectedObject& object, const Any& value) const override;
    ReflectedObject GetValueObject(const ReflectedObject& object) const override;
};

Reflection CreateNMaterialReflection(NMaterial* material);

class NMaterialPropertyValueWrapper : public ValueWrapper
{
private:
    struct PropData
    {
        uint32 size;
        rhi::ShaderProp::Type type;
        const float32* value;
    };

    FastName key;
    void SetProperty(NMaterial* material, const PropData& prop) const;
    Any GetPropertyValue(rhi::ShaderProp::Type type, const float32* data) const;
    rhi::ShaderProp::Type GetDefaultPropertyType(const NMaterial* material) const;
    Any GetDefaultPropertyValue(const NMaterial* material) const;

public:
    NMaterialPropertyValueWrapper(const FastName& key);
    const Type* GetType(const ReflectedObject& object) const override;
    bool IsReadonly(const ReflectedObject& object) const override;
    Any GetValue(const ReflectedObject& object) const override;
    bool SetValue(const ReflectedObject& object, const Any& value) const override;
    bool SetValueWithCast(const ReflectedObject& object, const Any& value) const override;
    ReflectedObject GetValueObject(const ReflectedObject& object) const override;
};

class NMaterialTextureInfoValueWrapper : public ValueWrapper
{
private:
    FastName key;
    void SetTexture(NMaterial* material, const FilePath& texturePath) const;

public:
    NMaterialTextureInfoValueWrapper(const FastName& key);
    const Type* GetType(const ReflectedObject& object) const override;
    bool IsReadonly(const ReflectedObject& object) const override;
    Any GetValue(const ReflectedObject& object) const override;
    bool SetValue(const ReflectedObject& object, const Any& value) const override;
    bool SetValueWithCast(const ReflectedObject& object, const Any& value) const override;
    ReflectedObject GetValueObject(const ReflectedObject& object) const override;
};

class NMaterialFlagValueWrapper : public ValueWrapper
{
private:
    FastName key;
    int32& GetFlagValue(const ReflectedObject& object) const;
    void SetFlag(NMaterial* material, int32 value) const;

public:
    NMaterialFlagValueWrapper(const FastName& key);
    const Type* GetType(const ReflectedObject& object) const override;
    bool IsReadonly(const ReflectedObject& object) const override;
    Any GetValue(const ReflectedObject& object) const override;
    bool SetValue(const ReflectedObject& object, const Any& value) const override;
    bool SetValueWithCast(const ReflectedObject& object, const Any& value) const override;
    ReflectedObject GetValueObject(const ReflectedObject& object) const override;
};

class NMaterialPropertyStructureWrapper : public StructureWrapperDefault
{
private:
    void GetPropertiesRecursive(const ReflectedObject& object, const NMaterial* material, Set<FastName>& names, Vector<Reflection::Field>& fields) const;
    mutable UnorderedMap<FastName, NMaterialPropertyValueWrapper> propertyValueWrappers;
    const NMaterialPropertyValueWrapper& GetOrCreateValueWrapper(const FastName& key) const;

public:
    bool HasFields(const ReflectedObject& object, const ValueWrapper* vw) const override;
    Reflection GetField(const ReflectedObject& object, const ValueWrapper* vw, const Any& key) const override;
    Vector<Reflection::Field> GetFields(const ReflectedObject& object, const ValueWrapper* vw, Reflection::MetaPredicate pred) const override;
    bool AddField(const ReflectedObject& object, const ValueWrapper* vw, const Any& key, const Any& value) const override;
    bool RemoveField(const ReflectedObject& object, const ValueWrapper* vw, const Any& key) const override;
};

class NMaterialTextureInfoStructureWrapper : public StructureWrapperDefault
{
private:
    void GetTextureInfoRecursive(const ReflectedObject& object, const NMaterial* material, Set<FastName>& names, Vector<Reflection::Field>& fields) const;
    mutable UnorderedMap<FastName, NMaterialTextureInfoValueWrapper> textureInfoValueWrappers;
    const NMaterialTextureInfoValueWrapper& GetOrCreateValueWrapper(const FastName& key) const;

public:
    bool HasFields(const ReflectedObject& object, const ValueWrapper* vw) const override;
    Reflection GetField(const ReflectedObject& object, const ValueWrapper* vw, const Any& key) const override;
    Vector<Reflection::Field> GetFields(const ReflectedObject& object, const ValueWrapper* vw, Reflection::MetaPredicate pred) const override;
    bool AddField(const ReflectedObject& object, const ValueWrapper* vw, const Any& key, const Any& value) const override;
    bool RemoveField(const ReflectedObject& object, const ValueWrapper* vw, const Any& key) const override;
};

class NMaterialFlagStructureWrapper : public StructureWrapperDefault
{
private:
    mutable UnorderedMap<FastName, NMaterialFlagValueWrapper> flagValueWrappers;
    const NMaterialFlagValueWrapper& GetOrCreateValueWrapper(const FastName& key) const;
    static Vector<FastName> flags;

public:
    bool HasFields(const ReflectedObject& object, const ValueWrapper* vw) const override;
    Reflection GetField(const ReflectedObject& object, const ValueWrapper* vw, const Any& key) const override;
    Vector<Reflection::Field> GetFields(const ReflectedObject& object, const ValueWrapper* vw, Reflection::MetaPredicate pred) const override;
    bool AddField(const ReflectedObject& object, const ValueWrapper* vw, const Any& key, const Any& value) const override;
    bool RemoveField(const ReflectedObject& object, const ValueWrapper* vw, const Any& key) const override;
};

class NMaterialBaseStructureWrapper : public StructureWrapperDefault
{
private:
    Reflection CreateReflection(const ReflectedObject& object, const ValueWrapper* vm, const String& key) const;

public:
    bool HasFields(const ReflectedObject& object, const ValueWrapper* vw) const override;
    Reflection GetField(const ReflectedObject& object, const ValueWrapper* vw, const Any& key) const override;
    Vector<Reflection::Field> GetFields(const ReflectedObject& object, const ValueWrapper* vw, Reflection::MetaPredicate pred) const override;
};

class NMaterialStructureWrapper : public StructureWrapperDefault
{
private:
    ValueWrapperObject wrapper;

    NMaterialBaseStructureWrapper baseStructureWrapper;
    NMaterialFlagStructureWrapper flagStructureWrapper;
    NMaterialPropertyStructureWrapper propertyStructureWrapper;
    NMaterialTextureInfoStructureWrapper textureInfoStructureWrapper;

    Reflection CreateReflection(const ReflectedObject& object, const String& key) const;

public:
    bool HasFields(const ReflectedObject& object, const ValueWrapper* vw) const override;
    size_t GetFieldsCount(const ReflectedObject& object, const ValueWrapper* vw) const override;
    Reflection GetField(const ReflectedObject& object, const ValueWrapper* vw, const Any& key) const override;
    Vector<Reflection::Field> GetFields(const ReflectedObject& object, const ValueWrapper* vw, Reflection::MetaPredicate pred) const override;
};
}
