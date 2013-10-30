#ifndef __BASEINTROSPECTION_H__
#define __BASEINTROSPECTION_H__

#include "Base/Introspection.h"

namespace DAVA
{
class BaseIntrospection
{
protected:
    virtual ~BaseIntrospection(){}
public:
    BaseIntrospection(){}

    EMPTY_INTROSPECTION(BaseIntrospection)
};
};
#endif // __BASEINTROSPECTION_H__
