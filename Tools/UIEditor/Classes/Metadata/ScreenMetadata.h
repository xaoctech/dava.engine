//
//  ScreenMetadata.h
//  UIEditor
//
//  Created by Yuri Coder on 10/23/12.
//
//

#ifndef __UIEditor__ScreenMetadata__
#define __UIEditor__ScreenMetadata__

#include "BaseMetadata.h"
#include "HierarchyTreeScreenNode.h"

namespace DAVA {
 
// Metadata class for Screen Node.
class ScreenMetadata : public BaseMetadata
{
    Q_OBJECT
    
    // Properties which are specific for Platform Node..
    Q_PROPERTY(QString Name READ GetName WRITE SetName);
    
    // Accessor to the Tree Node.
    HierarchyTreeScreenNode* GetScreenNode() const;
    
    // Getters/setters.
    QString GetName() const;
    void SetName(const QString& name);
};
    
}

#endif /* defined(__UIEditor__ScreenMetadata__) */
