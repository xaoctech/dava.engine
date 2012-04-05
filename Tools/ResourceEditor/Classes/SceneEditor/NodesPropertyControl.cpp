#include "NodesPropertyControl.h"
#include "ControlsFactory.h"

#include "DraggableDialog.h"

#include "EditorSettings.h"


NodesPropertyControl::NodesPropertyControl(const Rect & rect, bool _createNodeProperties)
    :   UIControl(rect)
{
	btnPlus = 0;
	btnMinus = 0;
	propControl = 0;
	workingScene = 0;

    deletionList = NULL;
    listHolder = NULL;
    btnCancel = NULL;
    
    
    nodesDelegate = NULL;
    currentSceneNode = NULL;
    currentDataNode = NULL;
    createNodeProperties = _createNodeProperties;

    
    Rect propertyRect(0, 0, rect.dx, rect.dy);
    
    if(!createNodeProperties)
    {
        propertyRect.dy -= ControlsFactory::BUTTON_HEIGHT;
        
        btnPlus = ControlsFactory::CreateButton(
                                                Rect(0, propertyRect.dy, 
                                                     ControlsFactory::BUTTON_HEIGHT, ControlsFactory::BUTTON_HEIGHT), 
                                                L"+");
        btnPlus->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &NodesPropertyControl::OnPlus));
        AddControl(btnPlus);
        
        btnMinus = ControlsFactory::CreateButton(
                                                 Rect(ControlsFactory::BUTTON_HEIGHT, propertyRect.dy, 
                                                      ControlsFactory::BUTTON_HEIGHT, ControlsFactory::BUTTON_HEIGHT), 
                                                 L"-");
        btnMinus->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &NodesPropertyControl::OnMinus));
        AddControl(btnMinus);
        
        propControl = new CreatePropertyControl(Rect(0, rect.dy - ControlsFactory::BUTTON_HEIGHT*4, 
                                                     rect.dx, ControlsFactory::BUTTON_HEIGHT*3), this);
        
        
        listHolder = new UIControl(propertyRect);
        btnCancel = ControlsFactory::CreateButton(
                                                Rect(0, propertyRect.dy - ControlsFactory::BUTTON_HEIGHT, 
                                                propertyRect.dx, ControlsFactory::BUTTON_HEIGHT), 
                                                LocalizedString(L"dialog.cancel"));
        btnCancel->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &NodesPropertyControl::OnCancel));
        listHolder->AddControl(btnCancel);
    }
    
    propertyList = new PropertyList(propertyRect, this);
    AddControl(propertyList);
    
}
    
NodesPropertyControl::~NodesPropertyControl()
{
    SafeRelease(deletionList);
    SafeRelease(listHolder);
    SafeRelease(btnCancel);

    
    SafeRelease(propControl);
    
    SafeRelease(btnMinus);
    SafeRelease(btnPlus);
    
    
    SafeRelease(propertyList);
}

void NodesPropertyControl::WillAppear()
{
}

void NodesPropertyControl::ReadFrom(SceneNode *sceneNode)
{
    currentSceneNode = sceneNode;
    currentDataNode = NULL;
    
    propertyList->ReleaseProperties();
    
    if(!createNodeProperties)
    {
        propertyList->AddSection("property.scenenode.generalc++", GetHeaderState("property.scenenode.generalc++", true));
        propertyList->AddIntProperty("property.scenenode.retaincount", PropertyList::PROPERTY_IS_READ_ONLY);
        propertyList->AddStringProperty("property.scenenode.classname", PropertyList::PROPERTY_IS_READ_ONLY);
        propertyList->AddStringProperty("property.scenenode.c++classname", PropertyList::PROPERTY_IS_READ_ONLY);
        
        propertyList->SetIntPropertyValue("property.scenenode.retaincount", sceneNode->GetRetainCount());
        propertyList->SetStringPropertyValue("property.scenenode.classname", sceneNode->GetClassName());
        propertyList->SetStringPropertyValue("property.scenenode.c++classname", typeid(*sceneNode).name());
    }

    propertyList->AddSection("property.scenenode.scenenode", GetHeaderState("property.scenenode.scenenode", true));
    propertyList->AddStringProperty("property.scenenode.name", PropertyList::PROPERTY_IS_EDITABLE);
    propertyList->SetStringPropertyValue("property.scenenode.name", sceneNode->GetName());

    if(!createNodeProperties)
    {
        propertyList->AddBoolProperty("property.scenenode.isVisible", PropertyList::PROPERTY_IS_EDITABLE);
		propertyList->SetBoolPropertyValue("property.scenenode.isVisible", sceneNode->GetVisible());
		
        propertyList->AddSection("property.scenenode.matrixes", GetHeaderState("property.scenenode.matrixes", false));
        propertyList->AddMatrix4Property("property.scenenode.localmatrix", PropertyList::PROPERTY_IS_EDITABLE);
        propertyList->AddMatrix4Property("property.scenenode.worldmatrix", PropertyList::PROPERTY_IS_READ_ONLY);

        propertyList->SetMatrix4PropertyValue("property.scenenode.localmatrix", sceneNode->GetLocalTransform());
        propertyList->SetMatrix4PropertyValue("property.scenenode.worldmatrix", sceneNode->GetWorldTransform());
    }

    
	{ //static light
		 propertyList->AddSection("Used in static lighting", GetHeaderState("Used in static lighting", true));

		 propertyList->AddBoolProperty("Used in static lighting");
		 propertyList->SetBoolPropertyValue("Used in static lighting", sceneNode->GetCustomProperties()->GetBool("editor.staticlight.used", true));
	}
    
    
    //must be last
    if(!createNodeProperties)
    {
        propertyList->AddSection("property.scenenode.customproperties", GetHeaderState("property.scenenode.customproperties", true));
        
        KeyedArchive *customProperties = sceneNode->GetCustomProperties();
        Map<String, VariantType> propsData = customProperties->GetArchieveData();
        for (Map<String, VariantType>::iterator it = propsData.begin(); it != propsData.end(); ++it)
        {
            String name = it->first;
            VariantType key = it->second;
            switch (key.type) 
            {
                case VariantType::TYPE_BOOLEAN:
                    propertyList->AddBoolProperty(name, PropertyList::PROPERTY_IS_EDITABLE);
                    propertyList->SetBoolPropertyValue(name, key.AsBool());
                    break;
                    
                case VariantType::TYPE_STRING:
                    propertyList->AddStringProperty(name, PropertyList::PROPERTY_IS_EDITABLE);
                    propertyList->SetStringPropertyValue(name, key.AsString());
                    break;

                case VariantType::TYPE_INT32:
                    propertyList->AddIntProperty(name, PropertyList::PROPERTY_IS_EDITABLE);
                    propertyList->SetIntPropertyValue(name, key.AsInt32());
                    break;

                case VariantType::TYPE_FLOAT:
                    propertyList->AddFloatProperty(name, PropertyList::PROPERTY_IS_EDITABLE);
                    propertyList->SetFloatPropertyValue(name, key.AsFloat());
                    break;
                    
                default:
                    break;
            }
        }
    }
    else
    {
        KeyedArchive *customProperties = sceneNode->GetCustomProperties();
        if(customProperties && customProperties->IsKeyExists("editor.isLocked"))
        {
            propertyList->AddSection("property.scenenode.customproperties", GetHeaderState("property.scenenode.customproperties", true));
            propertyList->AddBoolProperty("editor.isLocked", PropertyList::PROPERTY_IS_EDITABLE);
            propertyList->SetBoolPropertyValue("editor.isLocked", customProperties->GetBool("editor.isLocked"));
        }
    }
}

void NodesPropertyControl::ReadFrom(DataNode *dataNode)
{
    currentSceneNode = NULL;
    currentDataNode = dataNode;
    
    propertyList->ReleaseProperties();
    if(!createNodeProperties)
    {
        propertyList->AddSection("property.scenenode.generalc++", GetHeaderState("property.scenenode.generalc++", true));
        propertyList->AddIntProperty("property.scenenode.retaincount", PropertyList::PROPERTY_IS_READ_ONLY);
        propertyList->AddStringProperty("property.scenenode.classname", PropertyList::PROPERTY_IS_READ_ONLY);
        propertyList->AddStringProperty("property.scenenode.c++classname", PropertyList::PROPERTY_IS_READ_ONLY);
        
        propertyList->SetIntPropertyValue("property.scenenode.retaincount", dataNode->GetRetainCount());
        propertyList->SetStringPropertyValue("property.scenenode.classname", dataNode->GetClassName());
        propertyList->SetStringPropertyValue("property.scenenode.c++classname", typeid(*dataNode).name());
    }
}


void NodesPropertyControl::SetDelegate(NodesPropertyDelegate *delegate)
{
    nodesDelegate = delegate;
}

void NodesPropertyControl::OnStringPropertyChanged(PropertyList *forList, const String &forKey, const String &newValue)
{
    if(forKey == "property.scenenode.name") //SceneNode
    {
        if(currentSceneNode)
        {
            currentSceneNode->SetName(newValue);
        }
        else if(currentDataNode)
        {
            currentDataNode->SetName(newValue);
        }
    }
    else if(!createNodeProperties)
    {
        if(currentSceneNode)
        {
            KeyedArchive *customProperties = currentSceneNode->GetCustomProperties();
            if(customProperties->IsKeyExists(forKey))
            {
                customProperties->SetString(forKey, newValue);
            }
        }
    }
    
    if(nodesDelegate)
    {
        nodesDelegate->NodesPropertyChanged();
    }
}
void NodesPropertyControl::OnFloatPropertyChanged(PropertyList *forList, const String &forKey, float newValue)
{
    if(!createNodeProperties)
    {
        if(currentSceneNode)
        {
            KeyedArchive *customProperties = currentSceneNode->GetCustomProperties();
            if(customProperties->IsKeyExists(forKey))
            {
                customProperties->SetFloat(forKey, newValue);
            }
        }
    }

    
    if(nodesDelegate)
    {
        nodesDelegate->NodesPropertyChanged();
    }
}
void NodesPropertyControl::OnIntPropertyChanged(PropertyList *forList, const String &forKey, int newValue)
{
    if(!createNodeProperties)
    {
        if(currentSceneNode)
        {
            KeyedArchive *customProperties = currentSceneNode->GetCustomProperties();
            if(customProperties->IsKeyExists(forKey))
            {
                customProperties->SetInt32(forKey, newValue);
            }
        }
    }

    
    if(nodesDelegate)
    {
        nodesDelegate->NodesPropertyChanged();
    }
}
void NodesPropertyControl::OnBoolPropertyChanged(PropertyList *forList, const String &forKey, bool newValue)
{
    if(currentSceneNode)
    {
        KeyedArchive *customProperties = currentSceneNode->GetCustomProperties();
        
        if("Used in static lighting" == forKey)
        {
            customProperties->SetBool("editor.staticlight.used", newValue);
        }
        
        if(!createNodeProperties)
        {
            KeyedArchive *customProperties = currentSceneNode->GetCustomProperties();
            if (forKey == "property.scenenode.isVisible")
            {
                currentSceneNode->SetVisible(newValue);
            }
            
            if(customProperties->IsKeyExists(forKey))
            {
                customProperties->SetBool(forKey, newValue);
            }
        }
        else
        {
            KeyedArchive *customProperties = currentSceneNode->GetCustomProperties();
            if(customProperties->IsKeyExists(forKey))
            {
                customProperties->SetBool(forKey, newValue);
            }
        }
    }

    
    if(nodesDelegate)
    {
        nodesDelegate->NodesPropertyChanged();
    }
}
void NodesPropertyControl::OnFilepathPropertyChanged(PropertyList *forList, const String &forKey, const String &newValue)
{
    if(nodesDelegate)
    {
        nodesDelegate->NodesPropertyChanged();
    }
}
void NodesPropertyControl::OnComboIndexChanged(PropertyList *forList, const String &forKey, 
                                               int32 newItemIndex, const String &newItemKey)
{
    if(nodesDelegate)
    {
        nodesDelegate->NodesPropertyChanged();
    }
}

void NodesPropertyControl::OnMatrix4Changed(PropertyList *forList, const String &forKey, const Matrix4 & matrix4)
{
    if(forKey == "property.scenenode.localmatrix")
    {
        if(currentSceneNode)
        {
            currentSceneNode->SetLocalTransform(matrix4);
        }
    }
    
    
    if(nodesDelegate)
    {
        nodesDelegate->NodesPropertyChanged();
    }
}

void NodesPropertyControl::OnSectionExpanded(PropertyList *forList, const String &forKey, bool isExpanded)
{
    SetHeaderState(forKey, isExpanded);
}


void NodesPropertyControl::OnPlus(BaseObject * object, void * userData, void * callerData)
{
    if(propControl->GetParent() || listHolder->GetParent())
    {
        return;
    }

    AddControl(propControl);
}
void NodesPropertyControl::OnMinus(BaseObject * object, void * userData, void * callerData)
{
    if(propControl->GetParent() || listHolder->GetParent())
    {
        return;
    }
    
    if(currentSceneNode)
    {
        KeyedArchive *customProperties = currentSceneNode->GetCustomProperties();
        Map<String, VariantType> propsData = customProperties->GetArchieveData();
        
        int32 size = propsData.size();
        if(size)
        {
            Rect r = listHolder->GetRect();
            r.dy = Min(r.dy, (float32)size * CellHeight(NULL, 0));
            r.y = listHolder->GetRect().dy - r.dy - ControlsFactory::BUTTON_HEIGHT;
            
            deletionList = new UIList(r, UIList::ORIENTATION_VERTICAL);
            ControlsFactory::CustomizePropertyCell(deletionList, false);
            
            deletionList->SetDelegate(this);
            
            listHolder->AddControl(deletionList);
            deletionList->Refresh();
            AddControl(listHolder);
        }
    }
}

void NodesPropertyControl::NodeCreated(bool success)
{
    RemoveControl(propControl);
    if(success && currentSceneNode)
    {
        KeyedArchive *currentProperties = currentSceneNode->GetCustomProperties();
        
        String name = propControl->GetPropName();
        switch (propControl->GetPropType()) 
        {
            case CreatePropertyControl::EPT_STRING:
                currentProperties->SetString(name, "");
                break;

            case CreatePropertyControl::EPT_INT:
                currentProperties->SetInt32(name, 0);

                break;
            case CreatePropertyControl::EPT_FLOAT:
                currentProperties->SetFloat(name, 0.f);

                break;
            case CreatePropertyControl::EPT_BOOL:
                currentProperties->SetBool(name, false);

                break;

            default:
                break;
        }
        
        UpdateFieldsForCurrentNode();
    }
}


int32 NodesPropertyControl::ElementsCount(UIList * list)
{
    if(currentSceneNode)
    {
        KeyedArchive *customProperties = currentSceneNode->GetCustomProperties();
        Map<String, VariantType> propsData = customProperties->GetArchieveData();
        
        return propsData.size();
    }
    
    return 0;
}

UIListCell *NodesPropertyControl::CellAtIndex(UIList *list, int32 index)
{
    UIListCell *c = (UIListCell *)list->GetReusableCell("Deletion list");
    if (!c) 
    {
        c = new UIListCell(Rect(0, 0, 200, 20), "Deletion list");
    }
    
    if(currentSceneNode)
    {
        KeyedArchive *customProperties = currentSceneNode->GetCustomProperties();
        Map<String, VariantType> propsData = customProperties->GetArchieveData();
        int32 i = 0; 
        for (Map<String, VariantType>::iterator it = propsData.begin(); it != propsData.end(); ++it, ++i)
        {
            if(i == index)
            {
                String name = it->first;
                
                ControlsFactory::CustomizeListCell(c, StringToWString(name));
                break;
            }
        }
    }
    
    return c;
}

int32 NodesPropertyControl::CellHeight(UIList * list, int32 index)
{
    return CELL_HEIGHT;
}

void NodesPropertyControl::OnCellSelected(UIList *forList, UIListCell *selectedCell)
{
    if(currentSceneNode)
    {
        int32 index = selectedCell->GetIndex();
        KeyedArchive *customProperties = currentSceneNode->GetCustomProperties();
        Map<String, VariantType> propsData = customProperties->GetArchieveData();
        int32 i = 0; 
        for (Map<String, VariantType>::iterator it = propsData.begin(); it != propsData.end(); ++it, ++i)
        {
            if(i == index)
            {
                customProperties->DeleteKey(it->first);
                
                OnCancel(NULL, NULL, NULL);
                ReadFrom(currentSceneNode);
                break;
            }
        }
    }
}

void NodesPropertyControl::OnCancel(BaseObject * object, void * userData, void * callerData)
{
    listHolder->RemoveControl(deletionList);
    SafeRelease(deletionList);
    RemoveControl(listHolder);
}


void NodesPropertyControl::SetWorkingScene(DAVA::Scene *scene)
{
    workingScene = scene;
}


void NodesPropertyControl::UpdateFieldsForCurrentNode()
{
    if(currentSceneNode)
    {
        ReadFrom(currentSceneNode);
    }
}

bool NodesPropertyControl::GetHeaderState(const String & headerName, bool defaultValue)
{
    KeyedArchive *settings = EditorSettings::Instance()->GetSettings();
    return settings->GetBool("NodesProperety." + headerName, defaultValue);
}

void NodesPropertyControl::SetHeaderState(const String & headerName, bool newState)
{
    KeyedArchive *settings = EditorSettings::Instance()->GetSettings();
    settings->SetBool("NodesProperety." + headerName, newState);
    EditorSettings::Instance()->Save();
}

