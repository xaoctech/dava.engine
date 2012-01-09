#include "CreatePropertyControl.h"
#include "ControlsFactory.h"

CreatePropertyControl::CreatePropertyControl(const Rect & rect, CreatePropertyControlDelegate *newDelegate)
    :   UIControl(rect)
{
    ControlsFactory::CustomizeDialog(this);
    
    delegate = newDelegate;
    
    Vector<String> types;
    types.push_back("String");
    types.push_back("Integer");
    types.push_back("Float");
    types.push_back("Bool");

    Rect buttonRect(0, rect.dy - ControlsFactory::BUTTON_HEIGHT, rect.dx / 2, ControlsFactory::BUTTON_HEIGHT);
    UIButton *btnCancel = ControlsFactory::CreateButton(buttonRect, L"Cancel");
    btnCancel->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &CreatePropertyControl::OnCancel));
    AddControl(btnCancel);
    SafeRelease(btnCancel);

    buttonRect.x = rect.dx - buttonRect.dx;
    UIButton *btnCreate = ControlsFactory::CreateButton(buttonRect, L"Create");
    btnCreate->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &CreatePropertyControl::OnCreate));
    AddControl(btnCreate);
    SafeRelease(btnCreate);

    
    Rect textRect(0, 0, rect.dx / 3, (rect.dy - buttonRect.dy) / 2);
    Rect controlRect(textRect.dx, 0, rect.dx - textRect.dx, textRect.dy);

    UIStaticText *t = new UIStaticText(textRect);
    t->SetText(L"Type:");
    t->SetFont(ControlsFactory::GetFontLight());
    AddControl(t);
    
    typeCombo = new ComboBox(controlRect, NULL, types);
    AddControl(typeCombo);

    textRect.y = textRect.dy;
    controlRect.y = controlRect.dy;
    
    t = new UIStaticText(textRect);
    t->SetText(L"Name:");
    t->SetFont(ControlsFactory::GetFontLight());
    AddControl(t);
    
    nameField = new UITextField(controlRect);
    ControlsFactory::CustomizeEditablePropertyCell(nameField);
    nameField->SetDelegate(this);
    nameField->SetFont(ControlsFactory::GetFontLight());
    AddControl(nameField);
}
    
CreatePropertyControl::~CreatePropertyControl()
{
    SafeRelease(typeCombo);
    SafeRelease(nameField);
}

void CreatePropertyControl::WillAppear()
{
    nameField->SetText(L"");
    typeCombo->SetSelectedIndex(0, false);
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

const String CreatePropertyControl::GetPropName() const
{
    return WStringToString(nameField->GetText());
}

int32 CreatePropertyControl::GetPropType() const
{
    return typeCombo->GetSelectedIndex();
}

void CreatePropertyControl::OnCancel(BaseObject * object, void * userData, void * callerData)
{
    if(delegate)
    {
        delegate->NodeCreated(false);
    }
}

void CreatePropertyControl::OnCreate(BaseObject * object, void * userData, void * callerData)
{
    if(delegate)
    {
        delegate->NodeCreated(true);
    }
}

