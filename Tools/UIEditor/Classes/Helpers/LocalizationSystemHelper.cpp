//
//  LocalizationSystemHelper.cpp
//  UIEditor
//
//  Created by Yuri Coder on 10/31/12.
//
//

#include "LocalizationSystemHelper.h"
#include "FileSystem/Logger.h"

using namespace DAVA;

const LocalizationSystemHelper::LocalizationSystemHelperData LocalizationSystemHelper::helperData[] =
{
    {"en", "English"},
	{"ru", "Russian"},
    {"fr", "French"},
    {"de", "German"},
    {"it", "Italian"},
    {"es", "Spanish"},
    {"nl", "Dutch"},
    {"sv", "Swedish"}
};

int LocalizationSystemHelper::GetSupportedLanguagesCount()
{
    return sizeof(helperData)/sizeof(*helperData);
}

String LocalizationSystemHelper::GetSupportedLanguageID(int index)
{
    if (ValidateLanguageIndex(index) == false)
    {
        return  helperData[0].languageID;
    }
    
    return helperData[index].languageID;
}

String LocalizationSystemHelper::GetSupportedLanguageDesc(int index)
{
    if (ValidateLanguageIndex(index) == false)
    {
        return helperData[0].languageDescription;
    }
    
    return helperData[index].languageDescription;
}

String LocalizationSystemHelper::GetLanguageDescByLanguageID(String languageID)
{
    for (int i = 0; i < GetSupportedLanguagesCount(); ++i)
    {
        if (languageID.compare(helperData[i].languageID) == 0) {
            return helperData[i].languageDescription;
        }
    }
    return helperData[0].languageDescription;
}

String LocalizationSystemHelper::GetLanguageIDByLanguageDesc(String languageDesc)
{
    for (int i = 0; i < GetSupportedLanguagesCount(); ++i)
    {
        if ( languageDesc.compare(helperData[i].languageDescription) == 0) {
            return helperData[i].languageID;
        }
    }
    return helperData[0].languageID;
}

bool LocalizationSystemHelper::ValidateLanguageIndex(int index)
{
    if (index < 0 || index >= GetSupportedLanguagesCount())
    {
        Logger::Error("Language index %i is out of bounds!", index);
        return false;
    }
    
    return true;
}