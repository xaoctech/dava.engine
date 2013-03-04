#ifndef __DAVAENGINE_UI_VECTOR_SPINNER_ADAPTER_H__
#define __DAVAENGINE_UI_VECTOR_SPINNER_ADAPTER_H__

#include "UISpinner.h"

namespace DAVA
{

template<typename T> class VectorSpinnerAdapter : public SpinnerAdapter
{
public:
    VectorSpinnerAdapter(const Vector<T> &aCollection, int32 aSelectedIndex = 0)
        : data(aCollection)
        , selectedIndex(aSelectedIndex)
    {};

    virtual ~VectorSpinnerAdapter() {};

    virtual bool IsSelectedLast() const
    {
        return selectedIndex == data.size() - 1;
    }

    virtual bool IsSelectedFirst() const
    {
        return selectedIndex == 0;
    }

    int32 GetSelected() {return selectedIndex;}

    void SetSelected(int32 aSelectedIndex)
    {
        DVASSERT(0 <= aSelectedIndex && aSelectedIndex < data.size());
        if (selectedIndex != aSelectedIndex)
        {
            selectedIndex = aSelectedIndex;
            NotifyObservers(IsSelectedFirst(), IsSelectedLast(), true);
        }
    }

    /*
     * Call it after you made changes to dataset via 'data' field.
     * You should specify if selected element data was changed with 'isSelectedChanged' argument.
     */
    void NotifyDataSetChange(bool isSelectedChanged)
    {
        NotifyObservers(IsSelectedFirst(), IsSelectedLast(), isSelectedChanged);
    }

    Vector<T> data;

protected:
    virtual bool SelectNext()
    {
        bool isSelectedLast = IsSelectedLast();
        if (!isSelectedLast)
            selectedIndex++;
        return !isSelectedLast;
    }

    virtual bool SelectPrevious()
    {
        bool isSelectedFirst = IsSelectedFirst();
        if (!isSelectedFirst)
            selectedIndex--;
        return !isSelectedFirst;
    }

    int32 selectedIndex;
};

}

#endif //__DAVAENGINE_UI_VECTOR_SPINNER_ADAPTER_H__
