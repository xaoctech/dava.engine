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
class KeyedArchiveAnySerializer
{
public:
    /**
     \brief Function loads data from given file.
     \param[in] pathName relative pathname in application documents folder
     */
    bool Load(const FilePath& pathName);
    /**
     \brief Function saves data to given file.
     \param[in] pathName relative pathname in application documents folder
     */
    bool Save(const FilePath& pathName) const;

    /**
     \brief Function loads data from given file.
     \param[in] file to load from
     */
    bool Load(File* file);
    /**
     \brief Function saves data to given file.
     \param[in] file to save
     */
    bool Save(File* file) const;

    /**
     \brief Function to save archieve to byte array.
     \param[in] data byte arrat for archieve data, if data is null function returns only requested size of data for serialization
     \param[in] size size of byte array, if size is 0 function returns only requested size of data for serialization
     \returns size of really serialized data
     */
    uint32 Save(uint8* data, uint32 size) const;

    /**
     \brief Function to load archieve from byte array.
     \param[in] data byte arrat with archieve data
     \param[in] size size of byte array
     \returns result of loading
     */
    bool Load(const uint8* data, uint32 size);

    /**
     \brief Function loads data from given yaml file.
     \param[in] pathName relative pathname in application documents folder
     */
    bool LoadFromYamlFile(const FilePath& pathName);

    /**
     \brief Function saves data to given yaml file.
     \param[in] file to save
     */
    bool SaveToYamlFile(const FilePath& pathName) const;
};

// Implementation
};
