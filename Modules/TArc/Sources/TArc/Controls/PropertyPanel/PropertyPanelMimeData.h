#pragma once

#include <Base/Vector.h>
#include <Reflection/Reflection.h>

#include <QMimeData>

namespace DAVA
{
class PropertyPanelMimeData : public QMimeData
{
public:
    void AddItem(Reflection::Field item);
    const Vector<Reflection::Field>& GetPropertyItem() const;

private:
    Vector<Reflection::Field> storedItems;
};
} //DAVA
