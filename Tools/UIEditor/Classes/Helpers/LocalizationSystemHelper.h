//
//  LocalizationSystemHelper.h
//  UIEditor
//
//  Created by Yuri Coder on 10/31/12.
//
//

#ifndef __UIEditor__LocalizationSystemHelper__
#define __UIEditor__LocalizationSystemHelper__

#include "Base/BaseTypes.h"

namespace DAVA {
    
class LocalizationSystemHelper
{
public:
    // Helper to work with Localization System.
    static int GetSupportedLanguagesCount();
    static String GetSupportedLanguageID(int index);
    static String GetSupportedLanguageDesc(int index);
    static String GetLanguageDescByLanguageID(String languageID);
    static String GetLanguageIDByLanguageDesc(String languageDesc);
    
protected:
    // Validate the language index.
    static bool ValidateLanguageIndex(int index);

    struct LocalizationSystemHelperData
    {
        String languageID;
        String languageDescription;
    };

    static const LocalizationSystemHelperData helperData[];
};

}
#endif /* defined(__UIEditor__LocalizationSystemHelper__) */
