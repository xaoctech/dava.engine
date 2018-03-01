#pragma once

#include <TArc/Controls/PropertyPanel/PropertyModelExtensions.h>

class SplineEditorCreator : public DAVA::EditorComponentExtension
{
public:
    std::unique_ptr<DAVA::BaseComponentValue> GetEditor(const std::shared_ptr<const DAVA::PropertyNode>& node) const override;
};

class SplineComponentChildCreator : public DAVA::ChildCreatorExtension
{
public:
    void ExposeChildren(const std::shared_ptr<DAVA::PropertyNode>& node, DAVA::Vector<std::shared_ptr<DAVA::PropertyNode>>& children) const override;
};

//////////////////////////////////////////////////////////////////////////

class SplinePointEditorCreator : public DAVA::EditorComponentExtension
{
public:
    std::unique_ptr<DAVA::BaseComponentValue> GetEditor(const std::shared_ptr<const DAVA::PropertyNode>& node) const override;
};

class SplinePointChildCreator : public DAVA::ChildCreatorExtension
{
public:
    void ExposeChildren(const std::shared_ptr<DAVA::PropertyNode>& node, DAVA::Vector<std::shared_ptr<DAVA::PropertyNode>>& children) const override;
};
