#include "QtLayer.h"

namespace DAVA 
{
    
QtLayer::QtLayer()
    :   delegate(NULL)
{
    
}
    
void QtLayer::Quit()
{
    if(delegate)
    {
        delegate->Quit();
    }
        
}

void QtLayer::SetDelegate(QtLayerDelegate *delegate)
{
    this->delegate = delegate;
}

void QtLayer::ReleaseAutoreleasePool(void */*pool*/)
{
}
    
};

