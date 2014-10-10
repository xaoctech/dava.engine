#include "UIPackageLoader.h"

#include "Base/ObjectFactory.h"
#include "FileSystem/FilePath.h"
#include "FileSystem/YamlNode.h"
#include "FileSystem/YamlEmitter.h"
#include "UI/UIYamlLoader.h"
#include "UI/UIControl.h"
#include "UI/UIStaticText.h"
#include "UI/UIControlHelpers.h"
#include "UI/UIPackage.h"
#include "UIPackageSectionLoader.h"


namespace DAVA
{

UIPackageLoader::UIPackageLoader(LegacyControlData *data) : useIntrospectionForLegacyData(false), legacyData(SafeRetain(data))
{
    yamlLoader = new UIYamlLoader();

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

UIPackageLoader::~UIPackageLoader()
{
    SafeRelease(legacyData);
    SafeRelease(yamlLoader);
    for (auto it = importedPackages.begin(); it != importedPackages.end(); ++it)
    {
        it->second->Release();
    }
    importedPackages.clear();
}
    
UIPackage *UIPackageLoader::LoadPackage(const FilePath &packagePath)
{
    if (!loadingQueue.empty())
    {
        DVASSERT(false);
        loadingQueue.clear();
    }

    if (!packagePath.Exists())
        return NULL;
    
    ScopedPtr<YamlParser> parser(YamlParser::Create(packagePath));
    
    YamlNode *rootNode = parser->GetRootNode();
    if (!rootNode)
        return NULL;
    
    bool legacySupport;
    const YamlNode *headerNode = rootNode->Get("Header");
    
    if (headerNode)
    {
        const YamlNode *versionNode = headerNode->Get("version");
        if (versionNode == NULL || versionNode->GetType() != YamlNode::TYPE_STRING)
            headerNode = NULL; // legacy node with children with name Header
    }
    
    if (headerNode)
    {
        UIPackage *package = new UIPackage(packagePath);
        legacySupport = false;
        
        const YamlNode *importedPackagesNode = rootNode->Get("ImportedPackages");
        if (importedPackagesNode)
        {
            for (int32 i = 0; i < (int32) importedPackagesNode->GetCount(); i++)
            {
                const YamlNode *node = importedPackagesNode->Get(i);
                auto it = importedPackages.find(node->AsString());
                UIPackage *importedPackage;
                if (it != importedPackages.end())
                {
                    importedPackage = it->second;
                    package->AddPackage(importedPackage);
                }
                else
                {
                    importedPackage = LoadPackage(node->AsString());
                    importedPackages[node->AsString()] = importedPackage;
                    package->AddPackage(importedPackage);
                }
                DVASSERT(importedPackage);
            }
        }

        const YamlNode *controlsNode = rootNode->Get("Controls");
        if (controlsNode)
        {
            int32 count = (int32) controlsNode->GetCount();
            for (int32 i = 0; i < count; i++)
            {
                const YamlNode *node = controlsNode->Get(i);
                QueueItem item;
                item.name = node->Get("name")->AsString();
                item.node = node;
                item.status = STATUS_WAIT;
                item.control = 0;
                loadingQueue.push_back(item);
            }
            
            for (int32 i = 0; i < count; i++)
            {
                if (loadingQueue[i].status == STATUS_WAIT)
                    LoadRootControl(i, package);
            }
            for (int32 i = 0; i < count; i++)
            {
                DVASSERT(loadingQueue[i].control != NULL);
                DVASSERT(loadingQueue[i].status == STATUS_LOADED);
                package->AddControl(loadingQueue[i].control);
                SafeRelease(loadingQueue[i].control);
            }
            loadingQueue.clear();
        }
        return package;
    }
    else
    {
        legacySupport = true;
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

        LoadControl(legacyControl, rootNode, legacySupport, package);
        
        package->AddControl(legacyControl);
        return package;
    }
    

    return NULL;
}
    
bool UIPackageLoader::SavePackage(UIPackage *package)
{
    ScopedPtr<YamlNode> rootNode(YamlNode::CreateMapNode(false));
    
    YamlNode *headerNode = YamlNode::CreateMapNode();
    headerNode->Set("version", "0");
    rootNode->Add("Header", headerNode);
    
    YamlNode *packagesNode = YamlNode::CreateArrayNode(YamlNode::AR_BLOCK_REPRESENTATION);
    for (int32 i = 0; i < package->GetPackagesCount(); i++)
        packagesNode->Add(package->GetPackage(i)->GetFilePath().GetFrameworkPath());
    rootNode->Add("ImportedPackages", packagesNode);

    YamlNode *controlsNode = YamlNode::CreateArrayNode(YamlNode::AR_BLOCK_REPRESENTATION);
    for (int32 i = 0; i < package->GetControlsCount(); ++i)
        controlsNode->Add(CreateYamlNode(package->GetControl(i)));
    
    rootNode->Add("Controls", controlsNode);

    return YamlEmitter::SaveToYamlFile(package->GetFilePath(), rootNode);
}

void UIPackageLoader::SetUsingIntrospectionForLegacyData(bool useIntrospection)
{
    this->useIntrospectionForLegacyData = useIntrospection;
}
        
void UIPackageLoader::LoadRootControl(int index, UIPackage *currentPackage)
{
    DVASSERT(loadingQueue[index].status == STATUS_WAIT);
    
    const YamlNode *node = loadingQueue[index].node;
    loadingQueue[index].status = STATUS_LOADING;
    loadingQueue[index].control = CreateControl(node, false, currentPackage);
    LoadControl(loadingQueue[index].control, node, false, currentPackage);
    loadingQueue[index].status = STATUS_LOADED;
}

UIControl *UIPackageLoader::GetLoadedControlByName(const String &name, UIPackage *currentPackage)
{
    for (size_t index = 0; index < loadingQueue.size(); index++)
    {
        if (loadingQueue[index].name == name)
        {
            switch (loadingQueue[index].status)
            {
                case STATUS_WAIT:
                    LoadRootControl(index, currentPackage);
                    if (loadingQueue[index].status != STATUS_LOADED)
                    {
                        DVASSERT(false);
                        return NULL;
                    }
                    return loadingQueue[index].control;
                    
                case STATUS_LOADED:
                    return loadingQueue[index].control;

                case STATUS_LOADING:
                    DVASSERT(false);
                    return NULL;
                    
                default:
                    DVASSERT(false);
            }
        }
    }
    return NULL;
}

UIControl *UIPackageLoader::CreateControl(const YamlNode *node, bool legacySupport, UIPackage *currentPackage)
{
    if (legacySupport)
    {
        if (node->GetType() != YamlNode::TYPE_MAP)
            return NULL;
        
        const YamlNode *type = node->Get("type");
        const YamlNode *baseType = node->Get("baseType");
        UIControl *control;
        if (useIntrospectionForLegacyData && type->AsString() == "UIAggregatorControl")
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
            return CreateControlFromPrototype(prototype, importedPackage);
            
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
    else
    {
        const YamlNode *prototypeNode = node->Get("prototype");
        const YamlNode *classNode = node->Get("class");
        if (prototypeNode)
        {
            UIControl *prototype = NULL;
            UIPackage *prototypePackage = NULL;
            String name = prototypeNode->AsString();
            size_t pos = name.find('/');
            if (pos == String::npos)
            {
                prototype = GetLoadedControlByName(name, currentPackage);
            }
            else
            {
                String packageName = name.substr(0, pos);
                String controlName = name.substr(pos + 1, name.length() - pos - 1);
                UIPackage *pack = currentPackage->GetPackage(packageName);
                if (pack)
                {
                    prototype = pack->GetControl(controlName);
                    prototypePackage = pack;
                }
            }
            
            if (!prototype)
            {
                Logger::Error("[UIYamlLoader::CreateControlFromYamlNode] Can't create prototype with name \"%s\"", prototypeNode->AsString().c_str());
                DVASSERT(false); // TODO remove assert
                return NULL;
            }
            
            return CreateControlFromPrototype(prototype, prototypePackage);
        }
        else if (classNode)
        {
            const YamlNode *customClassNode = node->Get("customClass");
            
            UIControl *control = NULL;
            if (customClassNode)
                control = CreateCustomControl(customClassNode->AsString(), classNode->AsString());
            else
                control = CreateControlByClassName(classNode->AsString());
            
            List<UIControl* > subcontrols = control->GetSubcontrols();
            List<UIControl* >::iterator iter = subcontrols.begin();

            for ( ; iter!=subcontrols.end(); ++iter)//remove all sub controls
            {
                control->RemoveControl(*iter);
            }
            return control;
        }
    }
    DVASSERT(false);
    return NULL;
}

void UIPackageLoader::LoadControl(UIControl *control, const YamlNode *node, bool legacySupport, UIPackage *currentPackage)
{
    if (legacySupport && !useIntrospectionForLegacyData)
    {
        DVVERIFY(control->LoadPropertiesFromYamlNode(node, yamlLoader));
    }
    else
    {
        LoadPropertiesFromYamlNode(control, node, legacySupport);
    }
    
    // load children
    const YamlNode * childrenNode = node->Get("children");
    if (legacySupport)
    {
        if (childrenNode == NULL)
            childrenNode = node;
        for (uint32 i = 0; i < childrenNode->GetCount(); ++i)
        {
            const YamlNode *childNode = childrenNode->Get(i);
            
            if (!childNode->Get("type"))
                continue;
            
            UIControl *child = CreateControl(childNode, legacySupport, currentPackage);
            if (child)
            {
                child->SetName(childrenNode->GetItemKeyName(i));
                LoadControl(child, childNode, legacySupport, currentPackage);
                control->AddControl(child);
            }
        }
    }
    else
    {
        if (childrenNode)
        {
            uint32 count = childrenNode->GetCount();
            for (uint32 i = 0; i < count; i++)
            {
                const YamlNode *childNode = childrenNode->Get(i);
                const YamlNode *pathNode = childNode->Get("path");

                if (pathNode)
                {
                    UIControl *child = UIControlHelpers::GetControlByPath(pathNode->AsString(), control);
                    if (child)
                        LoadControl(child, childNode, legacySupport, currentPackage);
                }
                else
                {
                    UIControl *child = CreateControl(childNode, legacySupport, currentPackage);
                    if (child)
                    {
                        LoadControl(child, childNode, legacySupport, currentPackage);
                        control->AddControl(child);
                    }
                }
            }
        }
    }

    control->LoadFromYamlNodeCompleted();
    control->UpdateLayout();
    yamlLoader->PostLoad(control);
}

UIControl *UIPackageLoader::CreateControlByClassName(const String &className)
{
    UIControl *control = ObjectFactory::Instance()->New<UIControl>(className);
    if (!control)
        Logger::Warning("[DefaultUIControlFactory::CreateControl] Can't create control with class name \"%s\"", className.c_str());

    return control;
}

UIControl *UIPackageLoader::CreateCustomControl(const String &customClassName, const String &baseClassName)
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
    
    
UIControl *UIPackageLoader::CreateControlFromPrototype(UIControl *control, UIPackage *prototypePackage)
{
    return control->Clone();
}

YamlNode *UIPackageLoader::CreateYamlNode(UIControl *control)
{
    YamlNode *node = YamlNode::CreateMapNode(false, YamlNode::MR_BLOCK_REPRESENTATION, YamlNode::SR_PLAIN_REPRESENTATION);
    AddPropertiesToNode(control, node);

    List<UIControl*> &children = control->GetRealChildren();
    if(!children.empty())
    {
        YamlNode *childrenNode = YamlNode::CreateArrayNode(YamlNode::AR_BLOCK_REPRESENTATION);

        List<UIControl* >::const_iterator iter = children.begin();
        for (;iter != children.end(); ++iter)
        {
            YamlNode *childNode = CreateYamlNode(*iter);
            if (childNode)
                childrenNode->Add(childNode);
        }
        
        if (childrenNode->GetCount() > 0)
            node->Add("children", childrenNode);
        else
            SafeRelease(childrenNode);

    }
    return SafeRetain<YamlNode>(node);
}

void UIPackageLoader::LoadPropertiesFromYamlNode(UIControl *control, const YamlNode *node, bool legacySupport)
{
    LoadControlPropertiesFromYamlNode(control, control->GetTypeInfo(), node, legacySupport);
    LoadBgPropertiesFromYamlNode(control, node, legacySupport);
    LoadInternalControlPropertiesFromYamlNode(control, node, legacySupport);
}
    
void UIPackageLoader::LoadControlPropertiesFromYamlNode(UIControl *control, const InspInfo *typeInfo, const YamlNode *node, bool legacySupport)
{
    const InspInfo *baseInfo = typeInfo->BaseInfo();
    if (baseInfo)
        LoadControlPropertiesFromYamlNode(control, baseInfo, node, legacySupport);
    
    String className = control->GetControlClassName();
    UIPackageSectionLoader *section = CreateControlSectionLoader(control, typeInfo->Name());
    for (int i = 0; i < typeInfo->MembersCount(); i++)
    {
        const InspMember *member = typeInfo->Member(i);
        String memberName = member->Name();
        
        if (legacySupport)
            memberName = GetOldPropertyName(className, memberName);
        
        VariantType res;
        if (legacySupport && memberName == "name")
            res = VariantType(control->GetName());
        else if (node)
            res = ReadVariantTypeFromYamlNode(member, node, -1, memberName, legacySupport);
        
        section->SetProperty(member, res);
        if (res.GetType() != VariantType::TYPE_NONE)
        {
            if (legacySupport)
            {
                // FIXME: temporary hack: find enabled properties
                if (String(member->Name()).find("Align") != String::npos)
                {
                    String enabledProp = String(member->Name()) + "Enabled";
                    const InspMember *m = typeInfo->Member(enabledProp.c_str());
                    if (m)
                        section->SetProperty(m, VariantType(true));
                }
            }
        }
    }
    ReleaseSectionLoader(section);
}
    
void UIPackageLoader::LoadBgPropertiesFromYamlNode(UIControl *control, const YamlNode *node, bool legacySupport)
{
    String className = control->GetControlClassName();
    const YamlNode *componentsNode = node ? node->Get("components") : NULL;
    
    for (int i = 0; i < control->GetBackgroundComponentsCount(); i++)
    {
        const YamlNode *componentNode = NULL;
        
        if (legacySupport)
            componentNode = node;
        else if (componentsNode)
            componentNode = componentsNode->Get(control->GetBackgroundComponentName(i));
        
        UIPackageSectionLoader *section = CreateBackgroundSectionLoader(control, i);
        const InspInfo *insp = section->GetBaseObject()->GetTypeInfo();
        String bgName = control->GetBackgroundComponentName(i);
        
        for (int j = 0; j < insp->MembersCount(); j++)
        {
            const InspMember *member = insp->Member(j);
            String memberName = member->Name();
            int subNodeIndex = -1;
            
            if (legacySupport)
            {
                memberName = GetOldPropertyName(className, memberName);
                if (memberName == "stateSprite")
                {
                    subNodeIndex = 0;
                    memberName = "stateSprite";
                }
                else if (memberName == "stateFrame")
                {
                    subNodeIndex = 1;
                    memberName = "stateSprite";
                }
                else if (memberName == "stateSpriteModification")
                {
                    subNodeIndex = 2;
                    memberName = "stateSprite";
                }
                
                memberName = GetOldBgPrefix(className, bgName) + memberName + GetOldBgPostfix(className, bgName);
            }
            
            VariantType res;
            if (componentNode)
                res = ReadVariantTypeFromYamlNode(member, componentNode, subNodeIndex, memberName, legacySupport);
            section->SetProperty(member, res);
        }
        ReleaseSectionLoader(section);
    }
}

void UIPackageLoader::LoadInternalControlPropertiesFromYamlNode(UIControl *control, const YamlNode *node, bool legacySupport)
{
    String className = control->GetControlClassName();
    const YamlNode *componentsNode = node ? node->Get("components") : NULL;
    for (int i = 0; i < control->GetInternalControlsCount(); i++)
    {
        const YamlNode *componentNode = NULL;
        
        if (legacySupport)
            componentNode = node;
        else if (componentsNode)
            componentNode = componentsNode->Get(control->GetInternalControlName(i) + control->GetInternalControlDescriptions());
        
        UIPackageSectionLoader *section = CreateInternalControlSectionLoader(control, i);
        
        const InspInfo *insp = section->GetBaseObject()->GetTypeInfo();
        String bgName = control->GetInternalControlName(i);
        
        for (int j = 0; j < insp->MembersCount(); j++)
        {
            const InspMember *member = insp->Member(j);
            String memberName = member->Name();
            
            if (legacySupport)
            {
                memberName = GetOldPropertyName(className, memberName);
                memberName = GetOldBgPrefix(className, bgName) + memberName + GetOldBgPostfix(className, bgName);
            }
            
            VariantType value;
            if (componentNode)
                value = ReadVariantTypeFromYamlNode(member, componentNode, -1, memberName, legacySupport);
            section->SetProperty(member, value);
        }

        ReleaseSectionLoader(section);
    }
}

UIPackageSectionLoader *UIPackageLoader::CreateControlSectionLoader(UIControl *control, const String &name)
{
    return new UIPackageControlSectionLoader(control, name);
}

UIPackageSectionLoader *UIPackageLoader::CreateBackgroundSectionLoader(UIControl *control, int bgNum)
{
    return new UIPackageBackgroundSectionLoader(control, bgNum);
}

UIPackageSectionLoader *UIPackageLoader::CreateInternalControlSectionLoader(UIControl *control, int internalControlNum)
{
    return new UIPackageInternalControlSectionLoader(control, internalControlNum);
}
    
void UIPackageLoader::ReleaseSectionLoader(UIPackageSectionLoader *section)
{
    section->Apply();
    SafeRelease(section);
}

VariantType UIPackageLoader::ReadVariantTypeFromYamlNode(const InspMember *member, const YamlNode *node, int subNodeIndex, const String &propertyName, bool legacySupport)
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
                if (legacySupport && propertyName == "multiline")
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
        if (legacySupport)
        {
            // TODO: FIXME: temporary hack for loading rect as position or size
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
        
    }
    return VariantType();
}
    
bool UIPackageLoader::AddObjectPropertyToYamlNode(BaseObject *obj, const InspMember *member, YamlNode *node)
{
    if (member->Desc().type == InspDesc::T_ENUM)
    {
        VariantType val = member->Value(obj);
        const char *res = member->Desc().enumMap->ToString(val.AsInt32());
        if (res)
        {
            node->Add(member->Name(), res);
        }
        else
        {
            DVASSERT(false);
            return false;
        }
    }
    else if (member->Desc().type == InspDesc::T_FLAGS)
    {
        YamlNode *valueNode = new YamlNode(YamlNode::TYPE_ARRAY);
        int32 val = member->Value(obj).AsInt32();
        
        int r = 0;
        while (val > 0)
        {
            if ((val & 1) != 0)
                valueNode->Add(member->Desc().enumMap->ToString(1 << r));
            
            r++;
            val >>= 1;
        }
        node->Add(member->Name(), valueNode);
    }
    else
    {
        node->Add(member->Name(), member->Value(obj));
        return true;
    }
    
    return true;
}

void UIPackageLoader::AddPropertiesToNode(UIControl *control, YamlNode *node)
{
    String className = control->GetControlClassName();

    node->Set("class", control->GetClassName());
    if (!control->GetCustomControlClassName().empty())
        node->Set("customClass", control->GetCustomControlClassName());

    const InspInfo *insp = control->GetTypeInfo();
    while (insp)
    {
        for (int i = 0; i < insp->MembersCount(); i++)
        {
            const InspMember *member = insp->Member(i);
            if ((member->Flags() & I_SAVE) != 0)
            {
                AddObjectPropertyToYamlNode(control, member, node);
            }
        }
        
        insp = insp->BaseInfo();
    }

    YamlNode *componentsNode = YamlNode::CreateMapNode(false, YamlNode::MR_BLOCK_REPRESENTATION, YamlNode::SR_PLAIN_REPRESENTATION);

    for (int i = 0; i < control->GetBackgroundComponentsCount(); i++)
    {
        UIControlBackground *bg = control->GetBackgroundComponent(i);
        if (bg)
        {
            YamlNode *componentNode = YamlNode::CreateMapNode(false, YamlNode::MR_BLOCK_REPRESENTATION, YamlNode::SR_PLAIN_REPRESENTATION);

            const InspInfo *insp = bg->GetTypeInfo();
            for (int j = 0; j < insp->MembersCount(); j++)
            {
                const InspMember *member = insp->Member(j);
                if ((member->Flags() & I_SAVE) != 0)
                    AddObjectPropertyToYamlNode(bg, member, componentNode);
            }
            
            componentsNode->Add(control->GetBackgroundComponentName(i), componentNode);
        }
    }

    for (int i = 0; i < control->GetInternalControlsCount(); i++)
    {
        UIControl *internalControl = control->GetInternalControl(i);
        if (internalControl)
        {
            YamlNode *componentNode = YamlNode::CreateMapNode(false, YamlNode::MR_BLOCK_REPRESENTATION, YamlNode::SR_PLAIN_REPRESENTATION);

            const InspInfo *insp = internalControl->GetTypeInfo();
            for (int j = 0; j < insp->MembersCount(); j++)
            {
                const InspMember *member = insp->Member(j);
                if ((member->Flags() & I_SAVE) != 0)
                    AddObjectPropertyToYamlNode(internalControl, member, componentNode);
            }
            
            componentsNode->Add(control->GetInternalControlName(i) + control->GetInternalControlDescriptions(), componentNode);
        }
    }

    if (componentsNode->GetCount() > 0)
        node->Add("components", componentsNode);
    else
        SafeRelease(componentsNode);
}

String UIPackageLoader::GetOldPropertyName(const String controlClassName, const String name)
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

String UIPackageLoader::GetOldBgPrefix(const String controlClassName, const String name)
{
    if (controlClassName == "UISlider" && name != "Background")
        return name;
    else
        return "";
}

String UIPackageLoader::GetOldBgPostfix(const String controlClassName, const String name)
{
    if (controlClassName == "UIButton")
        return name;
    else
        return "";
}

}
