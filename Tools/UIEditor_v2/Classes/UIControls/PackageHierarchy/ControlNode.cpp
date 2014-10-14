#include "ControlNode.h"

using namespace DAVA;

ControlNode::ControlNode(UIControl *control)
    : PackageBaseNode(NULL)
    , control(SafeRetain(control))
    , propertiesRoot(NULL)
    , prototype(NULL)
    , prototypePackage(NULL)
    , creationType(CREATED_FROM_CLASS)
    , editable(true)
{
    this->propertiesRoot = new PropertiesRoot(control);
}

ControlNode::ControlNode(ControlNode *prototype, UIPackage *prototypePackage, eCreationType creationType)
    : PackageBaseNode(NULL)
    , control(NULL)
    , propertiesRoot(NULL)
    , prototype(NULL)
    , prototypePackage(NULL)
    , creationType(creationType)
    , editable(true)
{
    UIControl *sourceControl = prototype->GetControl();
    control = ObjectFactory::Instance()->New<UIControl>(sourceControl->GetControlClassName());
    control->SetCustomControlClassName(sourceControl->GetCustomControlClassName());
    
    this->prototype = SafeRetain(prototype);
    this->prototypePackage = SafeRetain(prototypePackage);
    
    propertiesRoot = new PropertiesRoot(control, prototype->GetPropertiesRoot());
    
    for (auto it = prototype->nodes.begin(); it != prototype->nodes.end(); ++it)
    {
        ControlNode *childNode = new ControlNode(*it, NULL, CREATED_FROM_PROTOTYPE_CHILD);
        childNode->SetParent(this);
        nodes.push_back(childNode);
        control->AddControl(childNode->GetControl());
    }
}

ControlNode::~ControlNode()
{
    for (auto it = nodes.begin(); it != nodes.end(); ++it)
        (*it)->Release();
    nodes.clear();
    
    SafeRelease(control);
    SafeRelease(propertiesRoot);
    SafeRelease(prototype);
    SafeRelease(prototypePackage);
}

void ControlNode::Add(ControlNode *node)
{
    DVASSERT(node->GetParent() == NULL);
    node->SetParent(this);
    nodes.push_back(SafeRetain(node));
    control->AddControl(node->GetControl());
}

int ControlNode::GetCount() const
{
    return (int) nodes.size();
}

PackageBaseNode *ControlNode::Get(int index) const
{
    return nodes[index];
}

ControlNode *ControlNode::FindByName(const DAVA::String &name) const
{
    for (auto it = nodes.begin(); it != nodes.end(); ++it)
    {
        if ((*it)->GetName() == name)
            return *it;
    }
    return NULL;
}

String ControlNode::GetName() const
{
    return control->GetName();
}

UIControl *ControlNode::GetControl() const
{
    return control;
}

bool ControlNode::IsInstancedFromPrototype() const
{
    return creationType == CREATED_FROM_PROTOTYPE;
}

bool ControlNode::IsCloned() const
{
    return creationType != CREATED_FROM_CLASS;
}

YamlNode *ControlNode::Serialize(YamlNode *prototypeChildren) const
{
    YamlNode *node = YamlNode::CreateMapNode(false);
    
    if (creationType == CREATED_FROM_PROTOTYPE)
    {
        if (prototypePackage)
            node->Add("prototype", prototypePackage->GetName() + "/" + prototype->GetName());
        else
            node->Add("prototype", prototype->GetName());
    }
    else if (creationType == CREATED_FROM_PROTOTYPE_CHILD)
    {
        String path = GetName();
        PackageBaseNode *p = GetParent();
        while (p != NULL && p->GetControl() != NULL && static_cast<ControlNode*>(p)->GetCreationType() != CREATED_FROM_PROTOTYPE)
        {
            path = p->GetName() + "/" + path;
            p = p->GetParent();
        }
        node->Add("path", path);
    }
    else
    {
        node->Add("class", control->GetClassName());
        if (!control->GetCustomControlClassName().empty())
            node->Add("customClass", control->GetCustomControlClassName());
    }
    
    propertiesRoot->AddPropertiesToNode(node);
    if (!nodes.empty())
    {
        YamlNode *children = YamlNode::CreateArrayNode(YamlNode::AR_BLOCK_REPRESENTATION);
        YamlNode *newPrototypeChildren = creationType == CREATED_FROM_PROTOTYPE ? children : prototypeChildren;
        for (auto it = nodes.begin(); it != nodes.end(); ++it)
        {
            ControlNode *child = *it;
            YamlNode *childYamlNode = child->Serialize(newPrototypeChildren);

            switch (child->GetCreationType())
            {
                case CREATED_FROM_CLASS:
                    children->Add(childYamlNode);
                    break;
                case CREATED_FROM_PROTOTYPE:
                    children->Add(childYamlNode);
                    break;
                case CREATED_FROM_PROTOTYPE_CHILD:
                    if (childYamlNode->GetCount() > 1) // for skip empty nodes (one node for path)
                        newPrototypeChildren->Add(childYamlNode);
                    else
                        SafeRelease(childYamlNode);
                    break;
                default:
                    DVASSERT(false);
                    break;
            }
        }
        
        if (children->GetCount() > 0)
            node->Add("children", children);
        else
            SafeRelease(children);
    }
    
    return node;
}
