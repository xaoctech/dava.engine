#ifndef __DAVAENGINE_UI_PACKAGE_H__
#define __DAVAENGINE_UI_PACKAGE_H__

#include "Base/BaseTypes.h"
#include "Base/BaseObject.h"

namespace DAVA
{
class UIControl;
class YamlNode;

class UIPackage: public BaseObject
{
public:
    UIPackage(const FilePath &packagePath);
protected:
    ~UIPackage();
public:
    String GetName() const;
    const FilePath &getFilePath() const {
        return packagePath;
    }

    uint32 GetControlsCount() const;
    UIControl *GetControl(uint32 index) const;

    UIControl *GetControl(const String &name) const;

    void AddControl(UIControl *control);
    void RemoveControl(UIControl *control);
    
protected:
    void SaveControls(YamlNode *rootNode) const;

protected:
    FilePath packagePath;
    Vector<UIControl *> controls;
};

};
#endif // __DAVAENGINE_UI_PACKAGE_H__
