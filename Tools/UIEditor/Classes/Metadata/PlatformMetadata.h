//
//  PlatformMetadata.h
//  UIEditor
//
//  Created by Yuri Coder on 10/23/12.
//
//

#ifndef __UIEditor__PlatformMetadata__
#define __UIEditor__PlatformMetadata__

#include "BaseMetadata.h"
#include "HierarchyTreePlatformNode.h"
#include "HierarchyTreeScreenNode.h"

namespace DAVA {

// Metadata class for Platform Node.
class PlatformMetadata : public BaseMetadata
{
    Q_OBJECT
    
    // Properties which are specific for Platform Node..
    Q_PROPERTY(QString Name READ GetName WRITE SetName);
    
    // Width and height.
    Q_PROPERTY(float Width READ GetWidth WRITE SetWidth);
    Q_PROPERTY(float Height READ GetHeight WRITE SetHeight);
    
protected:
    // Accessors to the Tree Node.
    HierarchyTreePlatformNode* GetPlatformNode() const;

    // Getters/setters.
    QString GetName() const;
    void SetName(const QString& name);
    
    float GetHeight() const;
    void SetHeight(float value);
    float GetWidth() const;
    void SetWidth(float value);
};
    
};

#endif /* defined(__UIEditor__PlatformMetadata__) */
