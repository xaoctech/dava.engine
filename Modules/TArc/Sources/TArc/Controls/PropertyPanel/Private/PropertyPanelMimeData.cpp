#include "TArc/Controls/PropertyPanel/PropertyPanelMimeData.h"

namespace DAVA
{
void PropertyPanelMimeData::AddItem(Reflection::Field item)
{
    storedItems.push_back(item);
}

const Vector<Reflection::Field>& PropertyPanelMimeData::GetPropertyItem() const
{
    return storedItems;
}
} //DAVA
