#pragma once

#include "Base/BaseTypes.h"
#include "Base/BaseObject.h"
#include "Base/FastName.h"
#include "FileSystem/File.h"

namespace DAVA
{
/**
	\ingroup filesystem
	\brief this is a class that should be used for serialization & deserialization of the items
 */

class KeyedArchiveAny
{
protected:
    virtual ~KeyedArchiveAny() = default;

public:
    KeyedArchiveAny() = default;

    /**
        \brief Dumps archive to console
	 */
    void Dump() const;

    /**
		\brief Function to check if key is available in this archive.
		\param[in] key string key
		\returns true if key available
	 */
    bool IsKeyExists(const FastName& key) const;
    /**
		\brief Function to get variable from archive.
		\param[in] key string key
		\param[in] defaultValue this is value that is used if variable with this key do not exists in archive
		\returns value of variable or defaultValue if key isn't available
	 */
    const Any& Get(const FastName& key, Any defaultValue = Any()) const;

    template <class T>
    const T& Get(const FastName& key, const T& defaultValue = T()) const;

    /**
		\brief Function to set variable in archive.
		\param[in] key string key
		\param[in] value we want to set for this key
	 */
    void Set(const FastName& key, const Any& value);
    /**
		\brief Function to set variable in archive.
		\param[in] key string key
		\param[in] value we want to set for this key
	 */
    template <class T>
    void Set(const FastName& key, const T& value);

    /**
     ??? Not sure that required
     \brief Function to set another keyed archive as key for this archive.
     Function is copying archive inside. If you need to work with this archive later use GetArchive().
     \param[in] key string key
     \param[in] value we want to set for this key
	 */
    //void SetArchive(const FastName& key, KeyedArchiveAny* archive);

    /**
		\brief Deletes named key.
		\param[in] key name of the key to delete
	 */
    void DeleteKey(const FastName& key);

    /**
		\brief Deletes all keys, making archive empty.
	 */
    void DeleteAllKeys();

    /**
     \brief Function to get all data of archive.
     \returns map of VariantType class with names
	 */
    using UnderlyingMap = UnorderedMap<FastName, Any>;
    const UnderlyingMap& GetArchieveData() const;

private:
    UnderlyingMap objectsMap;
};

// Implementation
template <class T>
const T& KeyedArchiveAny::Get(const FastName& key, const T& defaultValue) const
{
    const auto it = objectsMap.find(key);
    if (it != objectsMap.end())
    {
        const Any& value = it->second;
        return value.Get<T>();
    }
    return defaultValue;
}

inline void KeyedArchiveAny::Set(const FastName& key, const Any& value)
{
    objectsMap.emplace(key, value);
}

template <class T>
void KeyedArchiveAny::Set(const FastName& key, const T& value)
{
    objectsMap.emplace(key, Any(value));
}

inline void KeyedArchiveAny::DeleteKey(const FastName& key)
{
    objectsMap.erase(key);
}

inline void KeyedArchiveAny::DeleteAllKeys()
{
    objectsMap.clear();
}
};
