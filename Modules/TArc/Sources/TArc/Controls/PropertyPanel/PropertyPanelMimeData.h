#pragma once

#include <Base/Vector.h>
#include <QMimeData>

namespace DAVA
{
class ReflectedPropertyItem;
class PropertyPanelMimeData : public QMimeData
{
public:
    void AddItem(ReflectedPropertyItem* item);
    const Vector<ReflectedPropertyItem*>& GetPropertyItem() const;

private:
    Vector<ReflectedPropertyItem*> storedItems;
};
} //DAVA
