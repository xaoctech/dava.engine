#include "TArc/Controls/PropertyPanel/PropertyPanelMimeData.h"
#include "TArc/Controls/PropertyPanel/Private/ReflectedPropertyItem.h"

namespace DAVA
{
void PropertyPanelMimeData::AddItem(ReflectedPropertyItem* item)
{
    storedItems.push_back(item);
}

const Vector<ReflectedPropertyItem*>& PropertyPanelMimeData::GetPropertyItem() const
{
    return storedItems;
}
} //DAVA
