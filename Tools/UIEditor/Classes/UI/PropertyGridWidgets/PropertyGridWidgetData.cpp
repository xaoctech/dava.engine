//
//  PropertyGridWidgetData.cpp
//  UIEditor
//
//  Created by Yuri Coder on 10/19/12.
//
//

#include "PropertyGridWidgetData.h"

using namespace DAVA;

PropertyGridWidgetData::PropertyGridWidgetData(const QMetaProperty& metaProperty,
                                               bool needUpdateHierarchyTree,
                                               bool stateAware) :
    metaProperty(metaProperty),
    needUpdateHierarchyTree(needUpdateHierarchyTree),
    stateAware(stateAware)
{
}

const QMetaProperty& PropertyGridWidgetData::getProperty() const
{
    return metaProperty;
}

bool PropertyGridWidgetData::IsNeedUpdateHierarchyTree() const
{
    return needUpdateHierarchyTree;
}

bool PropertyGridWidgetData::IsStateAware() const
{
    return stateAware;
}