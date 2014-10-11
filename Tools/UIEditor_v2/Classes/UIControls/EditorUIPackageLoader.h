#ifndef __EDITORUICONTROLFACTORY_H__
#define __EDITORUICONTROLFACTORY_H__

#include "DAVAEngine.h"
/*
class EditorUIPackageLoader: public DAVA::UIPackageLoader
{
public:
    EditorUIPackageLoader();
    virtual ~EditorUIPackageLoader();

    bool SavePackage(DAVA::UIPackage *package);

protected:
    virtual DAVA::UIControl *CreateControlByClassName(const DAVA::String &className) override;
    virtual DAVA::UIControl *CreateCustomControl(const DAVA::String &customClassName, const DAVA::String &baseClassName) override;
    virtual DAVA::UIControl *CreateControlFromPrototype(DAVA::UIControl *prototype, DAVA::UIPackage *prototypePackage) override;

    virtual DAVA::UIPackageSectionLoader *CreateControlSectionLoader(DAVA::UIControl *control, const DAVA::String &name) override;
    virtual DAVA::UIPackageSectionLoader *CreateBackgroundSectionLoader(DAVA::UIControl *control, int bgNum) override;
    virtual DAVA::UIPackageSectionLoader *CreateInternalControlSectionLoader(DAVA::UIControl *control, int internalControlNum) override;

    DAVA::YamlNode *CreateYamlNode(DAVA::UIControl *control);

private:
    bool AddControlToNode(DAVA::UIControl *control, DAVA::YamlNode *node, DAVA::YamlNode *prototypeChildren);
    bool AddChildrenToNode(DAVA::UIControl *control, DAVA::YamlNode *childrenNode, DAVA::YamlNode *prototypeChildren);
    bool AddPropertiesToNode(DAVA::UIControl *control, DAVA::YamlNode *node);

//    bool HaveToSaveProperty(DAVA::UIControl *control, DAVA::BaseObject *obj, const DAVA::InspMember *member);

private:
    void SetClonedFromPrototypeProperty(DAVA::UIControl *control, const DAVA::String &path);

};
*/
#endif // __EDITORUICONTROLFACTORY_H__
