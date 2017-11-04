#include "ModelTemplatePath.h"

#include "FileSystem/FilePath.h"

const DAVA::FilePath& ModelTemplate::GetArenaDef()
{
    static DAVA::FilePath arenaDef("~res:/modelTemplate\\arena_def.xml");
    return arenaDef;
}
