#include "UIControlTypeInformation.h"

namespace DAVA
{
    UIControlTypeInformation::UIControlTypeInformation()
    {
        
    }
    
    UIControlTypeInformation::~UIControlTypeInformation()
    {
        
    }

    void UIControlTypeInformation::PutType(String &propertyName, VariantType::eVariantType type)
    {
        types[propertyName] = type;
    }

    VariantType UIControlTypeInformation::GetValueFromYamlNode(const String &propertyName, const YamlNode *node)
    {
        auto it = types.find(propertyName);
        if (it != types.end())
        {
            switch (it->second)
            {
                case VariantType::TYPE_BOOLEAN:
                    return VariantType(node->AsBool());
                    
                case VariantType::TYPE_INT32:
                    return VariantType(node->AsInt32());

                case VariantType::TYPE_FLOAT:
                    return VariantType(node->AsFloat());

                case VariantType::TYPE_STRING:
                    return VariantType(node->AsString());
                    
                case VariantType::TYPE_WIDE_STRING:
                    return VariantType(node->AsWString());
                    
                case VariantType::TYPE_UINT32:
                    return VariantType(node->AsUInt32());
                    
                case VariantType::TYPE_INT64:
                    return VariantType(node->AsInt64());
                    
                case VariantType::TYPE_UINT64:
                    return VariantType(node->AsUInt64());
                    
                case VariantType::TYPE_VECTOR2:
                    return VariantType(node->AsVector2());
                    
                case VariantType::TYPE_VECTOR3:
                    return VariantType(node->AsVector3());
                    
                case VariantType::TYPE_VECTOR4:
                    return VariantType(node->AsVector4());
                    
                case VariantType::TYPE_COLOR:
                    return VariantType(node->AsColor());
                    
                case VariantType::TYPE_MATRIX3:
                case VariantType::TYPE_MATRIX4:
                case VariantType::TYPE_MATRIX2:
                case VariantType::TYPE_AABBOX3:
                case VariantType::TYPE_FILEPATH:
                case VariantType::TYPE_KEYED_ARCHIVE:
                case VariantType::TYPE_BYTE_ARRAY:
                case VariantType::TYPE_FASTNAME:
                default:
                    break;
            }
        }
        DVASSERT(false);
        return VariantType();
    }
    
}
