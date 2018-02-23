#pragma once
#include "Base/Any.h"
#include "Base/Type.h"
#include "Base/FastName.h"
#include "Reflection/Reflection.h"
#include "Base/AnyHash.h"
#include "FileSystem/FilePath.h"

namespace DAVA
{
class ReflectionSerializatorContext
{
public:
    using PointerID = uint32;

    struct PointerRef
    {
    public:
        PointerRef(PointerID pointer_)
            : pointer(pointer_)
        {
        }

        PointerID pointer;
    };

    PointerID RegisterPointer(const Any& pointer);
    PointerID FindPointer(const Any& pointer);

    UnorderedMap<Any, PointerID> sharedPointers;
    UnorderedMap<PointerID, Any> reverseSharedPointers;

    UnorderedMap<Any, Any> objects;
    PointerID nextUniquePointerID = 1;
};

class ReflectionSerializatorImpl
{
public:
    void Save(const ReflectionSerializatorContext& context);
    void Load(ReflectionSerializatorContext& context);
};

class ReflectionSerializator
{
public:
    void Save(const Reflection& reflection, const FilePath& filepath);
    void Load(const Reflection& reflection, const FilePath& filepath);

    // void SaveClass(const Reflection& reflection);
    // void LoadClass(const Reflection& reflection);

private:
    void DumpHierarchy(const UnorderedMap<Any, Any>& objects, uint32 level = 1);
    void SaveRecursive(const Any& key, const Reflection& reflection, ReflectionSerializatorContext& context, UnorderedMap<Any, Any>& objects);
    void LoadRecursive(const Any& key, const Reflection& reflection, ReflectionSerializatorContext& context, const UnorderedMap<Any, Any>& objects);
};

} // namespace DAVA
