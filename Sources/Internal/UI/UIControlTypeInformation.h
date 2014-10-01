#ifndef __DAVAENGINE_UI_CONTROL_TYPE_INFORMATION_H__
#define __DAVAENGINE_UI_CONTROL_TYPE_INFORMATION_H__

#include "Base/BaseTypes.h"
#include "FileSystem/VariantType.h"
#include "FileSystem/YamlNode.h"

namespace DAVA {
    
    class UIControlTypeInformation
    {
    public:
        UIControlTypeInformation();
        virtual ~UIControlTypeInformation();
        
        void PutType(String &propertyName, VariantType::eVariantType type);
        VariantType GetValueFromYamlNode(const String &propertyName, const YamlNode *node);
        String GetStringValue(const String &propertyName, const YamlNode *node);
        
    private:
        Map<String, VariantType::eVariantType> types;
    };
}

#endif // __DAVAENGINE_UI_CONTROL_TYPE_INFORMATION_H__
