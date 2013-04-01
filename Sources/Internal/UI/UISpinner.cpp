#include "UISpinner.h"

namespace DAVA 
{

REGISTER_CLASS(UISpinner);

//use these names for children buttons to define UISpinner in .yaml
static const String BUTTON_NEXT_NAME = "buttonNext";
static const String BUTTON_PREVIOUS_NAME = "buttonPrevious";

void SpinnerAdapter::AddObserver(SelectionObserver* anObserver)
{
    observers.insert(anObserver);
}

void SpinnerAdapter::RemoveObserver(SelectionObserver* anObserver)
{
    observers.erase(anObserver);
}

void SpinnerAdapter::NotifyObservers(bool isSelectedFirst, bool isSelectedLast, bool isSelectedChanged)
{
    Set<SelectionObserver*>::const_iterator end = observers.end();
    for (Set<SelectionObserver*>::iterator it = observers.begin(); it != end; ++it)
    {
        (*it)->OnSelectedChanged(isSelectedFirst, isSelectedLast, isSelectedChanged);
    }
}

bool SpinnerAdapter::Next()
{
    bool completedOk = SelectNext();
    if (completedOk)
        NotifyObservers(false /*as we selected next it can't be first*/, IsSelectedLast(), true);
    return completedOk;
}

bool SpinnerAdapter::Previous()
{
    bool completedOk = SelectPrevious();
    if (completedOk)
        NotifyObservers(IsSelectedFirst(), false /*as we selected previous it can't be last*/, true);
    return completedOk;
}


UISpinner::UISpinner(const Rect &rect, bool rectInAbsoluteCoordinates/* = FALSE*/) 
    : UIControl(rect, rectInAbsoluteCoordinates)
    , buttonNext(new UIButton())
    , buttonPrevious(new UIButton())
    , adapter(NULL)
{
    buttonNext->SetName(BUTTON_NEXT_NAME);
    buttonPrevious->SetName(BUTTON_PREVIOUS_NAME);
    AddControl(buttonNext);
    AddControl(buttonPrevious);
    InitButtons();
}

UISpinner::~UISpinner()
{
    ReleaseButtons();
    if (adapter)
        adapter->RemoveObserver(this);
    SafeRelease(adapter);
}

void UISpinner::InitButtons()
{
    buttonNext->SetDisabled(!adapter || adapter->IsSelectedLast());
    buttonPrevious->SetDisabled(!adapter || adapter->IsSelectedFirst());
    buttonNext->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &UISpinner::OnNextPressed));
    buttonPrevious->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &UISpinner::OnPreviousPressed));
}

void UISpinner::ReleaseButtons()
{
    buttonNext->RemoveEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &UISpinner::OnNextPressed));
    buttonPrevious->RemoveEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &UISpinner::OnPreviousPressed));
    SafeRelease(buttonNext);
    SafeRelease(buttonPrevious);
}

void UISpinner::LoadFromYamlNode(YamlNode * node, UIYamlLoader * loader)
{
    //release default buttons - they have to be loaded from yaml
    RemoveControl(buttonNext);
    RemoveControl(buttonPrevious);
    ReleaseButtons();
    UIControl::LoadFromYamlNode(node, loader);
}

void UISpinner::CopyDataFrom(UIControl *srcControl)
{
	UIControl* buttonPrevClone = buttonPrevious->Clone();
	UIControl* buttonNextClone = buttonNext->Clone();

    RemoveControl(buttonNext);
    RemoveControl(buttonPrevious);
    ReleaseButtons();

    UIControl::CopyDataFrom(srcControl);
	
	// Yuri Coder, 2013/03/28. CopyDataFrom works with real children,
	// so need to copy inner buttons explicitely and replace their pointers.
	this->buttonPrevious = static_cast<UIButton*>(buttonPrevClone);
	AddControl(buttonPrevClone);
	SafeRelease(buttonPrevClone);

	this->buttonNext = static_cast<UIButton*>(buttonNextClone);
	AddControl(buttonNextClone);
	SafeRelease(buttonNextClone);

    FindRequiredControls();
    if (IsPointerToExactClass<UISpinner>(srcControl)) //we can also copy other controls, that's why we check
    {
        UISpinner * srcSpinner = static_cast<UISpinner*>(srcControl);
        buttonNext->RemoveEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(srcSpinner, &UISpinner::OnNextPressed));
        buttonPrevious->RemoveEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(srcSpinner, &UISpinner::OnPreviousPressed));
    }
    InitButtons();
}

UIControl* UISpinner::Clone()
{
	UISpinner *t = new UISpinner(GetRect());
	t->CopyDataFrom(this);
	return t;
}

void UISpinner::FindRequiredControls()
{
    UIControl * nextButtonControl = FindByName(BUTTON_NEXT_NAME);
    UIControl * previousButtonControl = FindByName(BUTTON_PREVIOUS_NAME);
    DVASSERT(nextButtonControl);
    DVASSERT(previousButtonControl);
    buttonNext = SafeRetain(DynamicTypeCheck<UIButton*>(nextButtonControl));
    buttonPrevious = SafeRetain(DynamicTypeCheck<UIButton*>(previousButtonControl));
    DVASSERT(buttonNext);
    DVASSERT(buttonPrevious);
}

void UISpinner::LoadFromYamlNodeCompleted()
{
    FindRequiredControls();
    InitButtons();
}

YamlNode * UISpinner::SaveToYamlNode(UIYamlLoader * loader)
{
	YamlNode *node = UIControl::SaveToYamlNode(loader);

	//Control Type
	SetPreferredNodeType(node, "UISpinner");
	
	// "Prev/Next" buttons have to be saved too.
	YamlNode* prevButtonNode = buttonPrevious->SaveToYamlNode(loader);
	YamlNode* nextButtonNode = buttonNext->SaveToYamlNode(loader);
	
	node->AddNodeToMap(BUTTON_PREVIOUS_NAME, prevButtonNode);
	node->AddNodeToMap(BUTTON_NEXT_NAME, nextButtonNode);

	return node;
}

List<UIControl* >& UISpinner::GetRealChildren()
{
	List<UIControl* >& realChildren = UIControl::GetRealChildren();
	realChildren.remove(buttonPrevious);
	realChildren.remove(buttonNext);

	return realChildren;
}
	
List<UIControl* > UISpinner::GetSubcontrols()
{
	List<UIControl* > subControls;
	subControls.push_back(buttonPrevious);
	subControls.push_back(buttonNext);
	
	return subControls;
}

void UISpinner::SetAdapter(SpinnerAdapter * anAdapter)
{
    if (adapter)
    {
        adapter->RemoveObserver(this);
        SafeRelease(adapter);
    }

    adapter = SafeRetain(anAdapter);
    if (adapter)
    {
        buttonNext->SetDisabled(adapter->IsSelectedLast());
        buttonPrevious->SetDisabled(adapter->IsSelectedFirst());
        adapter->DisplaySelectedData(this);
        adapter->AddObserver(this);
    }
    else
    {
        buttonNext->SetDisabled(true);
        buttonPrevious->SetDisabled(true);
    }
}

void UISpinner::OnNextPressed(DAVA::BaseObject * caller, void * param, void *callerData)
{
    //buttonNext is disabled if we have no adapter or selected adapter element is last, so we don't need checks here
    adapter->Next();
}    

void UISpinner::OnPreviousPressed(DAVA::BaseObject * caller, void * param, void *callerData)
{
    //buttonPrevious is disabled if we have no adapter or selected adapter element is first, so we don't need checks here
    adapter->Previous();
}

void UISpinner::OnSelectedChanged(bool isSelectedFirst, bool isSelectedLast, bool isSelectedChanged)
{
    buttonNext->SetDisabled(isSelectedLast);
    buttonPrevious->SetDisabled(isSelectedFirst);
    if (isSelectedChanged)
    {
        adapter->DisplaySelectedData(this);
        PerformEvent(UIControl::EVENT_VALUE_CHANGED);
    }
}

}