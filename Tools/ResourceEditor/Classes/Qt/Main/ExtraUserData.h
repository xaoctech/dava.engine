//
//  ExtraUserData.h
//  ResourceEditorQt
//
//  Created by Yuri Coder on 11/26/12.
//
//

#ifndef __ResourceEditorQt__ExtraUserData__
#define __ResourceEditorQt__ExtraUserData__

#include <QString>

// Extra User Data - base class for all non-scene nodes User Data.
class ExtraUserData
{
public:
    ExtraUserData();
    virtual ~ExtraUserData();
    
    // Get the name of the data attached.
    virtual QString GetName() const {return ""; };
};

#endif /* defined(__ResourceEditorQt__ExtraUserData__) */
