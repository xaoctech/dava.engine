//
//  ChangePropertyCommand.h
//  UIEditor
//
//  Created by Yuri Coder on 10/22/12.
//
//

#ifndef __UIEditor__ChangePropertyCommand__
#define __UIEditor__ChangePropertyCommand__

#include <QString>

#include "UI/UIControl.h"

#include "CommandsController.h"

#include "BaseCommand.h"
#include "MetadataFactory.h"
#include "HierarchyTreeNode.h"
#include "PropertyGridWidgetData.h"
#include "PropertiesHelper.h"
#include "UIControlStateHelper.h"

using namespace DAVA;

// Templated Command Data to store the list of "Tree Node ID - Tree Node Values"
// for different value types.
template<typename T> class ChangePropertyCommandData
{
public:
    ChangePropertyCommandData(HierarchyTreeNode::HIERARCHYTREENODEID treeNodeID,
                              const T& treeNodePropertyValue)
    {
        this->treeNodeID = treeNodeID;
        this->treeNodePropertyValue = treeNodePropertyValue;
    }

    HierarchyTreeNode::HIERARCHYTREENODEID GetTreeNodeID() const
    {
        return this->treeNodeID;
    }
    
    const T& GetTreeNodePropertyValue() const
    {
        return treeNodePropertyValue;
    }

private:
    HierarchyTreeNode::HIERARCHYTREENODEID treeNodeID;
    T treeNodePropertyValue;
};

// Template ChangePropertyCommand - common for all types.
template <typename Type>
class ChangePropertyCommand : public BaseCommand
{
public:
    ChangePropertyCommand(BaseMetadata* baseMetadata,
                          const PropertyGridWidgetData& propertyGridWidgetData,
                          Type value, bool allStates = false);
	virtual ~ChangePropertyCommand();

    virtual void Execute();
	virtual void Rollback();

	virtual bool IsUndoRedoSupported() {return true;};

protected:
	// Command data.
    typedef Vector<ChangePropertyCommandData<Type> > COMMANDDATAVECT;
    typedef typename COMMANDDATAVECT::iterator COMMANDDATAVECTITER;

    // Build the Command Data based on the type.
    Vector<ChangePropertyCommandData<Type> > BuildCommandData(BaseMetadata* baseMetadata);

    // Get the Metadata with Params attached for the Tree Node passed.
    BaseMetadata* GetMetadataForTreeNode(HierarchyTreeNode::HIERARCHYTREENODEID treeNodeID);

    // Get the property name.
    QString GetPropertyName();

	// Actually apply the property value.
	bool ApplyPropertyValue(const COMMANDDATAVECTITER& iter, Type newValue);

    // Property Grid Widget data.
    PropertyGridWidgetData propertyGridWidgetData;
    
    // Current value.
    Type curValue;
    
    bool setPropertyForAllStates;
    
    // Current state.
	Vector<UIControl::eControlState> curStates;

	// The vector of Command Data with the initial values.
    COMMANDDATAVECT commandData;
};

// Change Property Command Helper class is needed to retain/release the pointers to the data stored when needed.
template <typename Type>
	class ChangePropertyCommandHelper
{
public:
	static void RetainBeforeStore(Type /*value*/) {};
	static void ReleaseBeforeDelete(Type /*value*/) {};
};

// Partial specialization for Font class - it has to be retained/released.
template <>
	class ChangePropertyCommandHelper<Font*>
{
public:
	static void RetainBeforeStore(Font* value) { SafeRetain(value);};
	static void ReleaseBeforeDelete(Font* value) { SafeRelease(value);};
};

template<typename Type>
    ChangePropertyCommand<Type>::ChangePropertyCommand(BaseMetadata* baseMetadata,
                                                       const PropertyGridWidgetData& propertyGridWidgetData,
                                                       Type value, bool allStates) :
    propertyGridWidgetData(propertyGridWidgetData)
{
    this->curValue = value;
    this->setPropertyForAllStates = allStates;
    this->curStates = baseMetadata->GetUIControlStates();
    this->commandData = BuildCommandData(baseMetadata);
}

template<typename Type>
	ChangePropertyCommand<Type>::~ChangePropertyCommand()
{
	// Cleanup the Command Data items previously retained, if any.
	for (COMMANDDATAVECTITER iter = this->commandData.begin(); iter != this->commandData.end(); iter ++)
	{
		ChangePropertyCommandHelper<Type>::ReleaseBeforeDelete((*iter).GetTreeNodePropertyValue());
	}
}

template<typename Type>
    inline Vector<ChangePropertyCommandData<Type> > ChangePropertyCommand<Type>::BuildCommandData(BaseMetadata* baseMetadata)
{
    // Take the params and use them for building Command Data.
    const METADATAPARAMSVECT& paramsVect = baseMetadata->GetParams();
    Vector< ChangePropertyCommandData<Type> > commandData;

    int paramsCount = baseMetadata->GetParamsCount();
    for (BaseMetadataParams::METADATAPARAMID i = 0; i < paramsCount; i ++)
    {
        const BaseMetadataParams& params = paramsVect[i] ;
        HierarchyTreeNode::HIERARCHYTREENODEID nodeID = params.GetTreeNodeID();

        Type nodeValue = PropertiesHelper::GetPropertyValue<Type>(baseMetadata, GetPropertyName(), i);
        ChangePropertyCommandHelper<Type>::RetainBeforeStore(nodeValue);

        commandData.push_back(ChangePropertyCommandData<Type>(nodeID, nodeValue));
    }
    
    return commandData;
}

template<typename Type>
    BaseMetadata* ChangePropertyCommand<Type>::GetMetadataForTreeNode(HierarchyTreeNode::HIERARCHYTREENODEID treeNodeID)
{
    const HierarchyTreeNode* treeNode = HierarchyTreeController::Instance()->GetTree().GetNode(treeNodeID);
    if (treeNode == NULL)
    {
        Logger::Error("Tree Node is NULL for Tree Node ID %i", treeNodeID);
        return NULL;
    }
    
    UIControl* uiControl = NULL;
    const HierarchyTreeControlNode* controlNode = dynamic_cast<const HierarchyTreeControlNode*>(treeNode);
    if (controlNode)
    {
        uiControl = controlNode->GetUIObject();
    }
    
    BaseMetadata* baseMetadata = MetadataFactory::Instance()->GetMetadataForTreeNode(treeNode);
    if (baseMetadata == NULL)
    {
        Logger::Error("Unable to found Hierarchy Tree Node Metadata for node %i while executing Command!",
                      treeNodeID);
        return NULL;
    }
    
    METADATAPARAMSVECT params;
    params.push_back(BaseMetadataParams(treeNodeID, uiControl));
    baseMetadata->SetupParams(params);
    
    // Restore the Metadata UI Control state.
    baseMetadata->SetUIControlStates(curStates);
    
    return baseMetadata;
}

template<typename Type>
    void ChangePropertyCommand<Type>::Execute()
{
	QString propertyName = GetPropertyName();
    for (COMMANDDATAVECTITER iter = this->commandData.begin(); iter != commandData.end(); iter ++)
    {
		bool propertySetOK = ApplyPropertyValue(iter, curValue);
        if (propertySetOK)
        {
            CommandsController::Instance()->EmitChangePropertySucceeded(propertyName);
        }
        else
        {
            CommandsController::Instance()->EmitChangePropertyFailed(propertyName);
        }
    }
}

template<typename Type>
	void ChangePropertyCommand<Type>::Rollback()
{
	// The previous values are stored in Command Data.
	QString propertyName = GetPropertyName();
    for (COMMANDDATAVECTITER iter = this->commandData.begin(); iter != commandData.end(); iter ++)
    {
		Type previousValue = (*iter).GetTreeNodePropertyValue();
		bool propertySetOK = ApplyPropertyValue(iter, previousValue);

        if (propertySetOK)
        {
            CommandsController::Instance()->EmitChangePropertySucceeded(propertyName);
        }
        else
        {
            CommandsController::Instance()->EmitChangePropertyFailed(propertyName);
        }
    }
}

template<typename Type>
	bool ChangePropertyCommand<Type>::ApplyPropertyValue(const COMMANDDATAVECTITER& iter, Type newValue)
{
	QString propertyName = GetPropertyName();
	BaseMetadata* baseMetadata = GetMetadataForTreeNode((*iter).GetTreeNodeID());

	if (setPropertyForAllStates)
	{
		Vector<UIControl::eControlState> states;

		int statesCount = UIControlStateHelper::GetUIControlStatesCount();
		for (int stateID = 0; stateID < statesCount; stateID ++)
		{
			states.push_back(UIControlStateHelper::GetUIControlState(stateID));
		}

		baseMetadata->SetUIControlStates(states);
		PropertiesHelper::SetAllPropertyValues<Type>(baseMetadata, propertyName, newValue);
		baseMetadata->SetUIControlStates(this->curStates);
	}
	else
	{
		baseMetadata->SetUIControlStates(this->curStates);
		PropertiesHelper::SetAllPropertyValues<Type>(baseMetadata, propertyName, newValue);
	}
	
	// Verify whether the properties were indeed changed.
	bool isPropertyValueDiffers = false;
	Type realValue = PropertiesHelper::GetAllPropertyValues<Type>(baseMetadata, propertyName,
																  isPropertyValueDiffers);
	
	bool propertySetOK = (realValue == curValue);
	SAFE_DELETE(baseMetadata);

	return propertySetOK;
}

template<typename Type>
    QString ChangePropertyCommand<Type>::GetPropertyName()
{
    return this->propertyGridWidgetData.getProperty().name();
}

#endif /* defined(__UIEditor__ChangePropertyCommand__) */
