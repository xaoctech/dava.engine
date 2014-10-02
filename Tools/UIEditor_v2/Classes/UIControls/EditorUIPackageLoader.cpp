#include "EditorUIPackageLoader.h"

#include "UIEditorComponent.h"
#include "BaseProperty.h"
#include "ValueProperty.h"
#include "PropertiesSection.h"
#include "ControlPropertiesSection.h"
#include "BackgroundPropertiesSection.h"
#include "InternalControlPropertiesSection.h"
#include "UI/UIControlHelpers.h"

using namespace DAVA;

class EditorUIPackageControlSection : public UIPackageControlSection
{
public:
    EditorUIPackageControlSection(UIControl *control, const String &name) : UIPackageControlSection(control, name)
    {
        section = new ControlPropertiesSection(name);
    }

    virtual void SetProperty(const InspMember *member, const DAVA::VariantType &value)
    {
        if ((member->Flags() & I_EDIT) != 0)
        {
            UIPackageControlSection::SetProperty(member, value);
            ValueProperty *property = new ValueProperty(GetBaseObject(), member);
            if (value.GetType() != VariantType::TYPE_NONE)
                property->SetValue(value);
            section->AddProperty(property);
        }
    }
    
    virtual void Apply()
    {
        if (section->GetCount() > 0)
        {
            UIEditorComponent *component = dynamic_cast<UIEditorComponent*>(control->GetCustomData());
            DVASSERT(component != NULL);
            component->GetPropertiesRoot()->AddProperty(section);
        }
        else
        {
            SafeRelease(section);
        }
    }
    
private:
    PropertiesSection *section;
};

class EditorUIPackageBackgroundSection : public UIPackageBackgroundSection
{
public:
    EditorUIPackageBackgroundSection(UIControl *control, int num) : UIPackageBackgroundSection(control, num)
    {
        section = new BackgroundPropertiesSection(control, num);

        UIEditorComponent *component = dynamic_cast<UIEditorComponent*>(control->GetCustomData());
        DVASSERT(component != NULL);
        component->GetPropertiesRoot()->AddProperty(section);
    }
    
    virtual void SetProperty(const InspMember *member, const DAVA::VariantType &value)
    {
        if (control->GetName() == "DefaultButton")
        {
            Logger::Debug("!!!! %s, %s, %x", section->GetName().c_str(), member->Name(), dynamic_cast<UIEditorComponent*>(control->GetCustomData())->GetPrototype());
        }
        UIPackageBackgroundSection::SetProperty(member, value);
        ValueProperty *property = new ValueProperty(GetBaseObject(), member);
        if (value.GetType() != VariantType::TYPE_NONE)
            property->SetValue(value);
        section->AddProperty(property);
    }
    
    virtual void Apply()
    {
        UIPackageBackgroundSection::Apply();
        if (bgWasCreated && !bgHasChanges)
            section->HideContent();
    }

private:
    BackgroundPropertiesSection *section;
};

class EditorUIPackageInternalControlSection : public UIPackageInternalControlSection
{
public:
    EditorUIPackageInternalControlSection(UIControl *control, int num) : UIPackageInternalControlSection(control, num)
    {
        section = new InternalControlPropertiesSection(control, num);
        UIEditorComponent *component = dynamic_cast<UIEditorComponent*>(control->GetCustomData());
        DVASSERT(component != NULL);
        component->GetPropertiesRoot()->AddProperty(section);
    }
    
    virtual void SetProperty(const InspMember *member, const DAVA::VariantType &value)
    {
        UIPackageInternalControlSection::SetProperty(member, value);
        ValueProperty *property = new ValueProperty(GetBaseObject(), member);
        if (value.GetType() != VariantType::TYPE_NONE)
            property->SetValue(value);
        section->AddProperty(property);
    }
    
    virtual void Apply()
    {
        UIPackageInternalControlSection::Apply();
        if (internalWasCreated && !internalHasChanges)
            section->HideContent();
    }
    
private:
    InternalControlPropertiesSection *section;
};

EditorUIPackageLoader::EditorUIPackageLoader()
{
    SetUsingIntrospectionForLegacyData(true);
}

EditorUIPackageLoader::~EditorUIPackageLoader()
{
}

UIControl *EditorUIPackageLoader::CreateControlByClassName(const String &className)
{
    UIControl *control = ObjectFactory::Instance()->New<UIControl>(className);
    
    if (control)
    {
        UIEditorComponent *component = new UIEditorComponent();
        control->SetCustomData(component);
        SafeRelease(component);
    }
    else
    {
        Logger::Warning("[DefaultUIControlFactory::CreateControl] Can't create control with class name \"%s\"", className.c_str());
    }
    
    return control;
}

UIControl *EditorUIPackageLoader::CreateCustomControl(const String &customClassName, const String &baseClassName)
{
    UIControl *control = CreateControlByClassName(baseClassName);
    control->SetCustomControlClassName(customClassName);

    UIEditorComponent *component = new UIEditorComponent();
    control->SetCustomData(component);
    SafeRelease(component);

    return control;
}

UIControl *EditorUIPackageLoader::CreateControlFromPrototype(UIControl *prototype)
{
    UIControl *control = prototype->Clone();
    
    UIEditorComponent *component = new UIEditorComponent();
    component->SetPrototype(prototype);
    control->SetCustomData(component);
    SafeRelease(component);
    
    SetClonedFromPrototypeProperty(control, "");

    return control;
}

UIPackageSection *EditorUIPackageLoader::CreateControlSection(DAVA::UIControl *control, const DAVA::String &name)
{
    return new EditorUIPackageControlSection(control, name);
}

UIPackageSection *EditorUIPackageLoader::CreateBackgroundSection(DAVA::UIControl *control, int bgNum)
{
    return new EditorUIPackageBackgroundSection(control, bgNum);
}

UIPackageSection *EditorUIPackageLoader::CreateInternalControlSection(DAVA::UIControl *control, int internalControlNum)
{
    return new EditorUIPackageInternalControlSection(control, internalControlNum);
}

YamlNode *EditorUIPackageLoader::CreateYamlNode(UIControl *control)
{
    YamlNode *node = YamlNode::CreateMapNode(false, YamlNode::MR_BLOCK_REPRESENTATION, YamlNode::SR_PLAIN_REPRESENTATION);
    AddControlToNode(control, node, NULL);
    return SafeRetain<YamlNode>(node);
}

bool EditorUIPackageLoader::AddControlToNode(DAVA::UIControl *control, DAVA::YamlNode *node, YamlNode *prototypeChildren)
{
    bool propertiesAdded = AddPropertiesToNode(control, node);

    UIEditorComponent *component = dynamic_cast<UIEditorComponent*>(control->GetCustomData());
    UIControl *prototype = NULL;
    if (component)
        prototype = component->GetPrototype();

    YamlNode *childrenNode = YamlNode::CreateArrayNode(YamlNode::AR_BLOCK_REPRESENTATION);
    bool childrensAdded = AddChildrenToNode(control, childrenNode, prototype ? childrenNode : prototypeChildren);
    
    if (childrenNode->GetCount() > 0)
        node->Add("children", childrenNode);
    else
        SafeRelease(childrenNode);
    
    return propertiesAdded || childrensAdded;
}

bool EditorUIPackageLoader::AddChildrenToNode(UIControl *control, YamlNode *childrenNode, YamlNode *prototypeChildren)
{
    bool atLeastOneChildWasAdded = false;
    const List<UIControl*> &children = control->GetRealChildren();
    for (auto iter = children.begin();iter != children.end(); ++iter)
    {
        YamlNode *childNode = YamlNode::CreateMapNode(false, YamlNode::MR_BLOCK_REPRESENTATION, YamlNode::SR_PLAIN_REPRESENTATION);
        if (AddControlToNode(*iter, childNode, prototypeChildren))
        {
            UIEditorComponent *component = dynamic_cast<UIEditorComponent*>(control->GetCustomData());
            if (component && component->IsClonedFromPrototype())
                prototypeChildren->Add(childNode);
            else
            {
                childrenNode->Add(childNode);
                atLeastOneChildWasAdded = true;
            }
        }
        else
        {
            SafeRelease(childNode);
        }
    }
    return atLeastOneChildWasAdded;
}

bool EditorUIPackageLoader::AddPropertiesToNode(UIControl *control, YamlNode *node)
{
    bool hasChanges = false;
    UIEditorComponent *component = dynamic_cast<UIEditorComponent*>(control->GetCustomData());
    UIControl *prototype = NULL;
    if (component)
        prototype = component->GetPrototype();
    
    if (component && component->IsClonedFromPrototype())
    {
        if (prototype)
        {
            hasChanges = true;
            node->Set("prototype", prototype->GetName());
        }
        else
        {
            node->Set("path", component->GetPathFromPrototype());
        }
    }
    else
    {
        hasChanges = true;
        node->Set("class", control->GetClassName());
        if (!control->GetCustomControlClassName().empty())
            node->Set("customClass", control->GetCustomControlClassName());
    }

    
    String className = control->GetControlClassName();

    YamlNode *componentsNode = YamlNode::CreateMapNode(false, YamlNode::MR_BLOCK_REPRESENTATION, YamlNode::SR_PLAIN_REPRESENTATION);
    BaseProperty *root = component->GetPropertiesRoot();
    for (int i = 0; i < root->GetCount(); i++)
    {
        BaseProperty *section = root->GetProperty(i);
        bool isControl = dynamic_cast<ControlPropertiesSection*>(section) != NULL;
        
        YamlNode *sectionNode = isControl ? node : NULL;

        for (int j = 0; j < section->GetCount(); j++)
        {
            ValueProperty *valueProperty = dynamic_cast<ValueProperty*>(section->GetProperty(j));
            if (valueProperty && valueProperty->IsReplaced())
            {
                if (sectionNode == NULL)
                {
                    sectionNode = YamlNode::CreateMapNode(false, YamlNode::MR_BLOCK_REPRESENTATION, YamlNode::SR_PLAIN_REPRESENTATION);
                    componentsNode->AddNodeToMap(section->GetName(), sectionNode);
                }
                AddObjectPropertyToYamlNode(valueProperty->GetBaseObject(), valueProperty->GetMember(), sectionNode);
                hasChanges = true;
            }
        }
    }
    
    if (componentsNode->GetCount() > 0)
    {
        hasChanges = true;
        node->Add("components", componentsNode);
    }
    else
        SafeRelease(componentsNode);

//
//    if (String(member->Name()) == "name" && component->IsClonedFromPrototype())
//        return false;
//    
//    return true; //component->IsMemberReplaced(obj, member);
    return hasChanges;
   
}

void EditorUIPackageLoader::SetClonedFromPrototypeProperty(UIControl *control, const DAVA::String &path)
{
    if (!control->GetCustomData())
    {
        UIEditorComponent *component = new UIEditorComponent();
        component->SetClonedFromPrototype(path);
        control->SetCustomData(component);
        SafeRelease(component);

        LoadPropertiesFromYamlNode(control, NULL, false);
    }
    
    const DAVA::List<UIControl*> &children = control->GetChildren();
    for (auto it = children.begin(); it != children.end(); ++it)
    {
        String p = path;
        if (!p.empty())
            p += "/";
        p += (*it)->GetName();
        SetClonedFromPrototypeProperty(*it, p);
    }
    
}

