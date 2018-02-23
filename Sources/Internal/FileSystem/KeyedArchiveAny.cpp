#include "FileSystem/KeyedArchiveAny.h"

namespace DAVA
{
bool KeyedArchiveAny::IsKeyExists(const FastName& key) const
{
    return objectsMap.find(key) != objectsMap.end();
}

const Any& KeyedArchiveAny::Get(const FastName& key, Any defaultValue) const
{
    const auto it = objectsMap.find(key);
    return (it != objectsMap.end()) ? (it->second) : defaultValue;
}
};
