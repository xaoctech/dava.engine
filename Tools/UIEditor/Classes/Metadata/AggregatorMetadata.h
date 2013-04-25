#ifndef __UIEditor__AggregatorMetadata__
#define __UIEditor__AggregatorMetadata__

#include "BaseMetadata.h"
#include "HierarchyTreeAggregatorNode.h"

namespace DAVA {

// Metadata class for Platform Node.
class AggregatorMetadata : public BaseMetadata
{
    Q_OBJECT
    
    // Properties which are specific for Platform Node..
    Q_PROPERTY(QString Name READ GetName WRITE SetName);
    
    // Width and height.
    Q_PROPERTY(float Width READ GetWidth WRITE SetWidth);
    Q_PROPERTY(float Height READ GetHeight WRITE SetHeight);
    
protected:
    // Accessors to the Tree Node.
    HierarchyTreeAggregatorNode* GetNode() const;
	
    // Getters/setters.
    QString GetName() const;
    void SetName(const QString& name);
    
    float GetHeight() const;
    void SetHeight(float value);
    float GetWidth() const;
    void SetWidth(float value);
};
    
};

#endif /* defined(__UIEditor__AggregatorMetadata__) */
