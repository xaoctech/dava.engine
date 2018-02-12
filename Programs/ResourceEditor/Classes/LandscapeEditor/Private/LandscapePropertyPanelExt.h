#pragma once

#include <TArc/Controls/PropertyPanel/PropertyModelExtensions.h>

class LandscapeEditorCreator : public DAVA::EditorComponentExtension
{
public:
    std::unique_ptr<DAVA::BaseComponentValue> GetEditor(const std::shared_ptr<const DAVA::PropertyNode>& node) const;
};

class LandscapeEditorChildCreator : public DAVA::ChildCreatorExtension
{
public:
    void ExposeChildren(const std::shared_ptr<DAVA::PropertyNode>& node,
                        DAVA::Vector<std::shared_ptr<DAVA::PropertyNode>>& children) const;
};