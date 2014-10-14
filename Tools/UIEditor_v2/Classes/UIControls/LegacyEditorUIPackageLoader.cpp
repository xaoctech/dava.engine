#include "LegacyEditorUIPackageLoader.h"

#include "Base/ObjectFactory.h"
#include "FileSystem/FilePath.h"
#include "FileSystem/YamlNode.h"
#include "FileSystem/YamlEmitter.h"
#include "UI/UIControl.h"
#include "UI/UIStaticText.h"
#include "UI/UIControlHelpers.h"
#include "UI/UIPackage.h"

using namespace DAVA;

LegacyEditorUIPackageLoader::LegacyEditorUIPackageLoader(LegacyControlData *data) : legacyData(SafeRetain(data))
{
    
    // for legacy loading
    propertyNamesMap["UIControl"]["visible"] = "recursiveVisible";
    
    // UIButton bg
    propertyNamesMap["UIButton"]["sprite"] = "stateSprite";
    propertyNamesMap["UIButton"]["drawType"] = "stateDrawType";
    propertyNamesMap["UIButton"]["align"] = "stateAlign";
    propertyNamesMap["UIButton"]["frame"] = "stateFrame";
    propertyNamesMap["UIButton"]["spriteModification"] = "stateSpriteModification";
    propertyNamesMap["UIButton"]["colorInherit"] = "stateColorInherit";
    propertyNamesMap["UIButton"]["perPixelAccuracy"] = "statePerPixelAccuracy";
    propertyNamesMap["UIButton"]["color"] = "stateColor";
    propertyNamesMap["UIButton"]["leftRightStretchCap"] = "leftRightStretchCap";
    propertyNamesMap["UIButton"]["font"] = "stateFont";
    propertyNamesMap["UIButton"]["text"] = "stateText";
    propertyNamesMap["UIButton"]["multiline"] = "stateMultiline";
    propertyNamesMap["UIButton"]["textColor"] = "stateTextcolor";
    propertyNamesMap["UIButton"]["shadowcolor"] = "stateShadowcolor";
    propertyNamesMap["UIButton"]["shadowoffset"] = "stateShadowoffset";
    propertyNamesMap["UIButton"]["fitting"] = "stateFittingOption";
    propertyNamesMap["UIButton"]["textalign"] = "stateTextAlign";
    propertyNamesMap["UIButton"]["textcolorInheritType"] = "stateTextColorInheritType";
    
    baseClasses["UIButton"] = "UIControl";
}

LegacyEditorUIPackageLoader::~LegacyEditorUIPackageLoader()
{
    SafeRelease(legacyData);
    for (auto it = importedPackages.begin(); it != importedPackages.end(); ++it)
    {
        it->second->Release();
    }
    importedPackages.clear();
}

UIPackage *LegacyEditorUIPackageLoader::LoadPackage(const FilePath &packagePath)
{
    RefPtr<YamlParser> parser(YamlParser::Create(packagePath));
    
    YamlNode *rootNode = parser.Valid() ? parser->GetRootNode() : NULL;
    if (!rootNode)
        return NULL;
    

    UIPackage *package = new UIPackage(packagePath);
    UIControl *legacyControl = CreateControlByClassName("UIControl");
    const LegacyControlData::Data *data = legacyData ? legacyData->Get(packagePath.GetFrameworkPath()) : NULL;
    if (data)
    {
        legacyControl->SetSize(data->size);
        legacyControl->SetName(data->name);
    }
    else
    {
        legacyControl->SetName("LegacyControl");
    }
    
    LoadControl(legacyControl, rootNode, package);
    
    package->AddControl(legacyControl);
    
    return package;
}

UIControl *LegacyEditorUIPackageLoader::CreateControl(const YamlNode *node, UIPackage *currentPackage)
{
    if (node->GetType() != YamlNode::TYPE_MAP)
        return NULL;
    
    const YamlNode *type = node->Get("type");
    const YamlNode *baseType = node->Get("baseType");
    UIControl *control;
    if (type->AsString() == "UIAggregatorControl")
    {
        const YamlNode *pathNode = node->Get("aggregatorPath");
        auto it = importedPackages.find(pathNode->AsString());
        UIPackage *importedPackage;
        if (it != importedPackages.end())
        {
            importedPackage = it->second;
            currentPackage->AddPackage(importedPackage);
        }
        else
        {
            importedPackage = LoadPackage(pathNode->AsString());
            importedPackages[pathNode->AsString()] = importedPackage;
            currentPackage->AddPackage(importedPackage);
        }
        DVASSERT(importedPackage);
        UIControl *prototype = importedPackage->GetControl(0);
        return CreateControlFromPrototype(prototype);
        
    }
    else if (baseType)
        control = CreateCustomControl(type->AsString(), baseType->AsString());
    else
        control = CreateControlByClassName(type->AsString());
    
    List<UIControl* > subcontrols = control->GetSubcontrols();
    List<UIControl* >::iterator iter = subcontrols.begin();
    
    for ( ; iter!=subcontrols.end(); ++iter)//remove all sub controls
    {
        control->RemoveControl(*iter);
    }
    
    return control;

}

void LegacyEditorUIPackageLoader::LoadControl(UIControl *control, const YamlNode *node, UIPackage *currentPackage)
{
    LoadPropertiesFromYamlNode(control, node);
    
    // load children
    const YamlNode * childrenNode = node->Get("children");
    if (childrenNode == NULL)
        childrenNode = node;
    for (uint32 i = 0; i < childrenNode->GetCount(); ++i)
    {
        const YamlNode *childNode = childrenNode->Get(i);
        
        if (!childNode->Get("type"))
            continue;
        
        UIControl *child = CreateControl(childNode, currentPackage);
        if (child)
        {
            child->SetName(childrenNode->GetItemKeyName(i));
            LoadControl(child, childNode, currentPackage);
            control->AddControl(child);
        }
    }

    
    control->LoadFromYamlNodeCompleted();
    control->ApplyAlignSettingsForChildren();
    // yamlLoader->PostLoad(control);
}

UIControl *LegacyEditorUIPackageLoader::CreateControlByClassName(const String &className)
{
    UIControl *control = ObjectFactory::Instance()->New<UIControl>(className);
    if (!control)
        Logger::Warning("[DefaultUIControlFactory::CreateControl] Can't create control with class name \"%s\"", className.c_str());
    
    return control;
}

UIControl *LegacyEditorUIPackageLoader::CreateCustomControl(const String &customClassName, const String &baseClassName)
{
    UIControl *control = CreateControlByClassName(customClassName);
    if (!control)
    {
        control = CreateControlByClassName(baseClassName);
    }
    DVASSERT(control != NULL);
    control->SetCustomControlClassName(customClassName);
    return control;
}


UIControl *LegacyEditorUIPackageLoader::CreateControlFromPrototype(UIControl *control)
{
    return control->Clone();
}

void LegacyEditorUIPackageLoader::LoadPropertiesFromYamlNode(UIControl *control, const YamlNode *node)
{
    LoadControlPropertiesFromYamlNode(control, control->GetTypeInfo(), node);
    LoadBgPropertiesFromYamlNode(control, node);
    LoadInternalControlPropertiesFromYamlNode(control, node);
}

void LegacyEditorUIPackageLoader::LoadControlPropertiesFromYamlNode(UIControl *control, const InspInfo *typeInfo, const YamlNode *node)
{
    const InspInfo *baseInfo = typeInfo->BaseInfo();
    if (baseInfo)
        LoadControlPropertiesFromYamlNode(control, baseInfo, node);
    
    String className = control->GetControlClassName();
//    UIPackageSectionLoader *section = CreateControlSectionLoader(control, typeInfo->Name());
//    for (int i = 0; i < typeInfo->MembersCount(); i++)
//    {
//        const InspMember *member = typeInfo->Member(i);
//        String memberName = member->Name();
//        
//        memberName = GetOldPropertyName(className, memberName);
//        
//        VariantType res;
//        if (memberName == "name")
//            res = VariantType(control->GetName());
//        else if (node)
//            res = ReadVariantTypeFromYamlNode(member, node, -1, memberName);
//        
//        section->SetProperty(member, res);
//        if (res.GetType() != VariantType::TYPE_NONE)
//        {
//            if (String(member->Name()).find("Align") != String::npos)
//            {
//                String enabledProp = String(member->Name()) + "Enabled";
//                const InspMember *m = typeInfo->Member(enabledProp.c_str());
//                if (m)
//                    section->SetProperty(m, VariantType(true));
//            }
//        }
//    }
//    ReleaseSectionLoader(section);
}

void LegacyEditorUIPackageLoader::LoadBgPropertiesFromYamlNode(UIControl *control, const YamlNode *node)
{
//    String className = control->GetControlClassName();
//    
//    for (int i = 0; i < control->GetBackgroundComponentsCount(); i++)
//    {
//        UIPackageSectionLoader *section = CreateBackgroundSectionLoader(control, i);
//        const InspInfo *insp = section->GetBaseObject()->GetTypeInfo();
//        String bgName = control->GetBackgroundComponentName(i);
//        
//        for (int j = 0; j < insp->MembersCount(); j++)
//        {
//            const InspMember *member = insp->Member(j);
//            String memberName = member->Name();
//            int subNodeIndex = -1;
//            
//            memberName = GetOldPropertyName(className, memberName);
//            if (memberName == "stateSprite")
//            {
//                subNodeIndex = 0;
//                memberName = "stateSprite";
//            }
//            else if (memberName == "stateFrame")
//            {
//                subNodeIndex = 1;
//                memberName = "stateSprite";
//            }
//            else if (memberName == "stateSpriteModification")
//            {
//                subNodeIndex = 2;
//                memberName = "stateSprite";
//            }
//            
//            memberName = GetOldBgPrefix(className, bgName) + memberName + GetOldBgPostfix(className, bgName);
//            
//            VariantType res = ReadVariantTypeFromYamlNode(member, node, subNodeIndex, memberName);
//            section->SetProperty(member, res);
//        }
//        ReleaseSectionLoader(section);
//    }
}

void LegacyEditorUIPackageLoader::LoadInternalControlPropertiesFromYamlNode(UIControl *control, const YamlNode *node)
{
//    String className = control->GetControlClassName();
//    for (int i = 0; i < control->GetInternalControlsCount(); i++)
//    {
//        UIPackageSectionLoader *section = CreateInternalControlSectionLoader(control, i);
//        
//        const InspInfo *insp = section->GetBaseObject()->GetTypeInfo();
//        String bgName = control->GetInternalControlName(i);
//        
//        for (int j = 0; j < insp->MembersCount(); j++)
//        {
//            const InspMember *member = insp->Member(j);
//            String memberName = member->Name();
//            memberName = GetOldPropertyName(className, memberName);
//            memberName = GetOldBgPrefix(className, bgName) + memberName + GetOldBgPostfix(className, bgName);
//            
//            VariantType value = ReadVariantTypeFromYamlNode(member, node, -1, memberName);
//            section->SetProperty(member, value);
//        }
//        
//        ReleaseSectionLoader(section);
//    }
}

VariantType LegacyEditorUIPackageLoader::ReadVariantTypeFromYamlNode(const InspMember *member, const YamlNode *node, int subNodeIndex, const String &propertyName)
{
    
    const YamlNode *valueNode = node->Get(propertyName);
    if (valueNode)
    {
        if (subNodeIndex != -1)
            valueNode = valueNode->Get(subNodeIndex);
        
        if (member->Desc().type == InspDesc::T_ENUM)
        {
            int val = 0;
            if (member->Desc().enumMap->ToValue(valueNode->AsString().c_str(), val))
            {
                return VariantType(val);
            }
            else
            {
                if (propertyName == "multiline")
                {
                    if (valueNode->AsBool())
                    {
                        const YamlNode *bySymbolNode = node->Get("multilineBySymbol");
                        if (bySymbolNode && bySymbolNode->AsBool())
                            return VariantType(UIStaticText::MULTILINE_ENABLED_BY_SYMBOL);
                        else
                            return VariantType(UIStaticText::MULTILINE_ENABLED);
                    }
                    else
                    {
                        return VariantType(UIStaticText::MULTILINE_DISABLED);
                    }
                }
                else
                {
                    DVASSERT(false);
                }
            }
        }
        else if (member->Desc().type == InspDesc::T_FLAGS)
        {
            int val = 0;
            for (uint32 i = 0; i < valueNode->GetCount(); i++)
            {
                const YamlNode *flagNode = valueNode->Get(i);
                int flag = 0;
                if (member->Desc().enumMap->ToValue(flagNode->AsString().c_str(), flag))
                {
                    val |= flag;
                }
                else
                {
                    DVASSERT(false);
                }
            }
            return VariantType(val);
        }
        else if (member->Type() == MetaInfo::Instance<bool>())
            return VariantType(valueNode->AsBool());
        else if (member->Type() == MetaInfo::Instance<int32>())
            return VariantType(valueNode->AsInt32());
        else if (member->Type() == MetaInfo::Instance<uint32>())
            return VariantType(valueNode->AsUInt32());
        else if (member->Type() == MetaInfo::Instance<String>())
            return VariantType(valueNode->AsString());
        else if (member->Type() == MetaInfo::Instance<WideString>())
            return VariantType(valueNode->AsWString());
        else if (member->Type() == MetaInfo::Instance<float32>())
            return VariantType(valueNode->AsFloat());
        else if (member->Type() == MetaInfo::Instance<Vector2>())
            return VariantType(valueNode->AsVector2());
        else if (member->Type() == MetaInfo::Instance<Color>())
            return VariantType(valueNode->AsColor());
        else if (member->Type() == MetaInfo::Instance<FilePath>())
            return VariantType(FilePath(valueNode->AsString()));
        else
        {
            DVASSERT(false);
            return VariantType();
        }
    }
    else
    {
        String name = member->Name();
        bool isPosition = name == "position";
        if (isPosition || name == "size")
        {
            valueNode = node->Get("rect");
            if (valueNode)
            {
                if (isPosition)
                    return VariantType(valueNode->AsRect().GetPosition());
                else
                    return VariantType(valueNode->AsRect().GetSize());
            }
        }
        
    }
    return VariantType();
}


String LegacyEditorUIPackageLoader::GetOldPropertyName(const String controlClassName, const String name)
{
    auto mapIt = propertyNamesMap.find(controlClassName);
    if (mapIt != propertyNamesMap.end())
    {
        const Map<String, String> &map = mapIt->second;
        auto it = map.find(name);
        if (it != map.end())
            return it->second;
    }
    
    auto baseIt = baseClasses.find(controlClassName);
    if (baseIt != baseClasses.end())
        return GetOldPropertyName(baseIt->second, name);
    
    return name;
}

String LegacyEditorUIPackageLoader::GetOldBgPrefix(const String controlClassName, const String name)
{
    if (controlClassName == "UISlider" && name != "Background")
        return name;
    else
        return "";
}

String LegacyEditorUIPackageLoader::GetOldBgPostfix(const String controlClassName, const String name)
{
    if (controlClassName == "UIButton")
        return name;
    else
        return "";
}
