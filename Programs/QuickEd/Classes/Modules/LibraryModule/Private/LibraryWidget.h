#pragma once

#include <Base/BaseTypes.h>
#include <Model/PackageHierarchy/PackageNode.h>

class LibraryModel;
class QTreeView;

namespace DAVA
{
namespace TArc
{
class FieldBinder;
class ContextAccessor;
class UI;
}
}

class LibraryWidget : public QWidget
{
    Q_OBJECT

public:
    LibraryWidget(DAVA::TArc::ContextAccessor* accessor, DAVA::TArc::UI* ui, QWidget* parent = nullptr);
    ~LibraryWidget() override;

    void SetLibraryPackages(const DAVA::Vector<DAVA::RefPtr<PackageNode>>& projectLibrary);
    void SetCurrentPackage(PackageNode* package);

private:
    void InitUI();

private:
    QTreeView* treeView;
    LibraryModel* libraryModel = nullptr;
    std::unique_ptr<DAVA::TArc::FieldBinder> fieldBinder;
    DAVA::TArc::ContextAccessor* accessor = nullptr;
};
