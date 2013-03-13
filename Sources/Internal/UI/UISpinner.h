#ifndef __DAVAENGINE_UI_SPINNER_H__
#define __DAVAENGINE_UI_SPINNER_H__

#include "Base/BaseTypes.h"
#include "UI/UIControl.h"
#include "UI/UIButton.h"

namespace DAVA
{

class UISpinner;

/*
 * Provider of sequential data for UISpinner. Data set it contains can be bounded, cyclic or infinite.
 * 
 * Methods related to selection or dataset change (Next()/Previous() and those from specific implementations) can be called from any place, 
 * that's why UISpinner should use notification mechanism to stay in sync with adapter.
 */
class SpinnerAdapter : public BaseObject
{

public:
    class SelectionObserver
    {
    public:

        /*
         * If isSelectedChanged == true selected element was actually changed.
         *
         * Call with isSelectedChanged == false is possible for changeable dataset when selected 
         * element becomes first/last/not first/not last as a result of data set change.
         */
        virtual void OnSelectedChanged(bool isSelectedFirst, bool isSelectedLast, bool isSelectedChanged) = 0;

    };

    virtual ~SpinnerAdapter() {};

    /*
     * This method actually displays selected element in a manner specific for particular control (it can be text or image or whatever).
     * To implement it use lookup by name to find controls inside the 'spinner' and fill them with data.
     *
     * Implementation depends on a kind of controls used to display data element.
     */
    virtual void DisplaySelectedData(UISpinner * spinner) = 0;

    /*
     * Select next element. Returns 'true' and calls OnSelectedChanged for all observers if next element selected successfully. Returns 'false' otherwise.
     */
    bool Next();

    /*
     * Select previous element. Returns 'true' and calls OnSelectedChanged for all observers if previous element selected successfully. Returns 'false' otherwise.
     */
    bool Previous();
   
    //For next two implementation depends on a type of data set, not on a kind of controls used to display data element.
    virtual bool IsSelectedLast() const = 0;
    virtual bool IsSelectedFirst() const = 0;

    void AddObserver(SelectionObserver* anObserver);
    void RemoveObserver(SelectionObserver* anObserver);

protected:
    Set<SelectionObserver*> observers;
         
    //For next two implementation depends on a type of data set, not on a kind of controls used to display data element.
    //See description for Next() and Previous()
    virtual bool SelectNext() = 0;
    virtual bool SelectPrevious() = 0;

    void NotifyObservers(bool isSelectedFirst, bool isSelectedLast, bool isSelectedChanged);

};

/*
 * UISpinner has two buttons to select next and previous element from some dataset.
 * To display selected element UISpinner itself (as UIControl) or its children should be used:
 * use SpinnerAdapter with your custom DisplaySelectedData implementation of display logic.
 */
class UISpinner : public UIControl, SpinnerAdapter::SelectionObserver
{
public:
    UISpinner(const Rect &rect = Rect(), bool rectInAbsoluteCoordinates = false);
    virtual ~UISpinner();

    SpinnerAdapter * GetAdater() {return adapter;}
    void SetAdapter(SpinnerAdapter * adapter);

    virtual void LoadFromYamlNode(YamlNode * node, UIYamlLoader * loader);
    virtual void LoadFromYamlNodeCompleted();
    virtual void CopyDataFrom(DAVA::UIControl *srcControl);

    UIButton * GetButtonNext() {return buttonNext;}
    UIButton * GetButtonPrevious() {return buttonPrevious;}

protected:
    SpinnerAdapter * adapter;

    UIButton * buttonNext;
    UIButton * buttonPrevious;

    void OnNextPressed(DAVA::BaseObject * caller, void * param, void *callerData);
    void OnPreviousPressed(DAVA::BaseObject * caller, void * param, void *callerData);

    virtual void OnSelectedChanged(bool isSelectedFirst, bool isSelectedLast, bool isSelectedChanged);

    void InitButtons();
    void ReleaseButtons();
    void FindRequiredControls();
};

}
#endif //__DAVAENGINE_UI_SPINNER_H__