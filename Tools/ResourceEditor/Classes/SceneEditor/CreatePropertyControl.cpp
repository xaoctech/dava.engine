#include "CreatePropertyControl.h"
#include "ControlsFactory.h"

#include "EditorConfig.h"

CreatePropertyControl::CreatePropertyControl(const Rect & rect, CreatePropertyControlDelegate *newDelegate)
    :   UIControl(rect)
	, presetText(NULL)
	, presetCombo(NULL)
{
    ControlsFactory::CustomizeDialog(this);
    
    delegate = newDelegate;
    
    Vector<String> types;
    types.push_back("String");
    types.push_back("Integer");
    types.push_back("Float");
    types.push_back("Bool");

    Rect buttonRect(0, rect.dy - ControlsFactory::BUTTON_HEIGHT, rect.dx / 2, ControlsFactory::BUTTON_HEIGHT);
    UIButton *btnCancel = ControlsFactory::CreateButton(buttonRect, LocalizedString(L"dialog.cancel"));
    btnCancel->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &CreatePropertyControl::OnCancel));
    AddControl(btnCancel);
    SafeRelease(btnCancel);

    buttonRect.x = rect.dx - buttonRect.dx;
    UIButton *btnCreate = ControlsFactory::CreateButton(buttonRect, LocalizedString(L"dialog.create"));
    btnCreate->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &CreatePropertyControl::OnCreate));
    AddControl(btnCreate);
    SafeRelease(btnCreate);
    
    Rect textRect(0, 0, rect.dx / 3, (rect.dy - buttonRect.dy) / 3);
    Rect controlRect(textRect.dx, 0, rect.dx - textRect.dx, textRect.dy);
    
	Vector<String> presetNames = EditorConfig::Instance()->GetProjectPropertyNames();
	if(presetNames.empty())
	{
		presetNames.push_back("");
	}

	presetText = new UIStaticText(textRect);
	presetText->SetText(LocalizedString(L"createproperty.preset"));
	presetText->SetFont(ControlsFactory::GetFontLight());

	presetCombo = new ComboBox(controlRect, this, presetNames);

	textRect.y = textRect.dy;
    controlRect.y = controlRect.dy;

    UIStaticText *t = new UIStaticText(textRect);
    t->SetText(LocalizedString(L"createproperty.type"));
    t->SetFont(ControlsFactory::GetFontLight());
    AddControl(t);
    SafeRelease(t);
    
    typeCombo = new ComboBox(controlRect, NULL, types);
    AddControl(typeCombo);

    textRect.y = 2*textRect.dy;
    controlRect.y = 2*controlRect.dy;
    
    t = new UIStaticText(textRect);
    t->SetText(LocalizedString(L"createproperty.name"));
    t->SetFont(ControlsFactory::GetFontLight());
    AddControl(t);
    SafeRelease(t);
    
    nameField = new UITextField(controlRect);
    ControlsFactory::CustomizeEditablePropertyCell(nameField);
    nameField->SetDelegate(this);
    nameField->SetFont(ControlsFactory::GetFontLight());
    AddControl(nameField);
}
    
CreatePropertyControl::~CreatePropertyControl()
{
	SafeRelease(presetText);
	SafeRelease(presetCombo);
    SafeRelease(typeCombo);
    SafeRelease(nameField);
}

void CreatePropertyControl::WillAppear()
{
    nameField->SetText(L"");
    typeCombo->SetSelectedIndex(0, false);

	Vector<String> presetNames = EditorConfig::Instance()->GetProjectPropertyNames();
	if(presetNames.empty())
	{
		presetNames.push_back("");
	}
	else
	{
		AddControl(presetText);
		AddControl(presetCombo);
	}
	presetCombo->SetNewItemsSet(presetNames);
	presetCombo->SetSelectedIndex(0, false);
}

void CreatePropertyControl::TextFieldShouldReturn(UITextField * textField)
{
    nameField->ReleaseFocus();
}

void CreatePropertyControl::TextFieldShouldCancel(UITextField * textField)
{
    nameField->SetText(L"");
    nameField->ReleaseFocus();
}

void CreatePropertyControl::TextFieldLostFocus(UITextField * textField)
{
}

bool CreatePropertyControl::TextFieldKeyPressed(UITextField * textField, int32 replacementLocation, int32 replacementLength, const WideString & replacementString)
{
    return true;
}

void CreatePropertyControl::OnItemSelected(ComboBox *forComboBox, const String &itemKey, int itemIndex)
{
    if(delegate)
    {
		int32 propertyValueType = EditorConfig::Instance()->GetPropertyValueType(itemKey);
        delegate->NodeCreated(true, itemKey, propertyValueType);
    }
}

// const String CreatePropertyControl::GetPropName() const
// {
//     return WStringToString(nameField->GetText());
// }
// 
// int32 CreatePropertyControl::GetPropType() const
// {
//     return typeCombo->GetSelectedIndex();
// }

void CreatePropertyControl::OnCancel(BaseObject * object, void * userData, void * callerData)
{
    if(delegate)
    {
        delegate->NodeCreated(false, "", VariantType::TYPE_NONE);
    }
}

void CreatePropertyControl::OnCreate(BaseObject * object, void * userData, void * callerData)
{
    if(delegate)
    {
        delegate->NodeCreated(true, WStringToString(nameField->GetText()), typeCombo->GetSelectedIndex());
    }
}

