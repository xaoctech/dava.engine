#ifndef __EDITORUICONTROLFACTORY_H__
#define __EDITORUICONTROLFACTORY_H__

#include "DAVAEngine.h"

class EditorUIPackageLoader: public DAVA::UIPackageLoader
{
public:
    EditorUIPackageLoader();
    virtual ~EditorUIPackageLoader();

protected:
    virtual DAVA::UIControl *CreateControlByClassName(const DAVA::String &className);
    virtual DAVA::UIControl *CreateCustomControl(const DAVA::String &customClassName, const DAVA::String &baseClassName);
    virtual DAVA::UIControl *CreateControlFromPrototype(DAVA::UIControl *prototype);

    virtual DAVA::UIPackageSection *CreateControlSection(DAVA::UIControl *control, const DAVA::String &name);
    virtual DAVA::UIPackageSection *CreateBackgroundSection(DAVA::UIControl *control, int bgNum);
    virtual DAVA::UIPackageSection *CreateInternalControlSection(DAVA::UIControl *control, int internalControlNum);

    virtual void AddClassPropertiesToYamlNode(DAVA::UIControl *control, DAVA::YamlNode *node);
    virtual void AddPropertiesToNode(DAVA::UIControl *control, DAVA::YamlNode *node);

//    bool HaveToSaveProperty(DAVA::UIControl *control, DAVA::BaseObject *obj, const DAVA::InspMember *member);

private:
    void SetClonedFromPrototypeProperty(DAVA::UIControl *control);

};

#endif // __EDITORUICONTROLFACTORY_H__
