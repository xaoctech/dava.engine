//
//  PropertyGridWidgetData.h
//  UIEditor
//
//  Created by Yuri Coder on 10/19/12.
//
//

#ifndef __UIEditor__PropertyGridWidgetData__
#define __UIEditor__PropertyGridWidgetData__

#include "PropertyGridWidgetData.h"
#include <QMetaProperty>

namespace DAVA {

class PropertyGridWidgetData
{
public:
    PropertyGridWidgetData(const QMetaProperty& metaProperty,
                           bool needUpdateHierarchyTree,
                           bool stateAware = false);

    const QMetaProperty& getProperty() const;
    bool IsNeedUpdateHierarchyTree() const;
    bool IsStateAware() const;

private:
    QMetaProperty metaProperty;
    
    // Whether the Hierarchy Tree needs to be updated?
    bool needUpdateHierarchyTree;
    
    // Whether this widget is state-aware?
    bool stateAware;
};

};

#endif /* defined(__UIEditor__PropertyGridWidgetData__) */
