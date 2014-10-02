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

    virtual DAVA::YamlNode *CreateYamlNode(DAVA::UIControl *control);

private:
    bool AddControlToNode(DAVA::UIControl *control, DAVA::YamlNode *node, DAVA::YamlNode *prototypeChildren);
    bool AddChildrenToNode(DAVA::UIControl *control, DAVA::YamlNode *childrenNode, DAVA::YamlNode *prototypeChildren);
    bool AddPropertiesToNode(DAVA::UIControl *control, DAVA::YamlNode *node);

//    bool HaveToSaveProperty(DAVA::UIControl *control, DAVA::BaseObject *obj, const DAVA::InspMember *member);

private:
    void SetClonedFromPrototypeProperty(DAVA::UIControl *control, const DAVA::String &path);

};

#endif // __EDITORUICONTROLFACTORY_H__
