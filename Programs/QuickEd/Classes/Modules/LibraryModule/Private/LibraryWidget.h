#pragma once

#include "Classes/Model/PackageHierarchy/PackageNode.h"

#include <Base/BaseTypes.h>

#include <QWidget>

class LibraryModel;
class QTreeView;

namespace DAVA
{
namespace TArc
{
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

    QTreeView* GetTreeView();

private:
    void InitUI();

private:
    QTreeView* treeView;
    LibraryModel* libraryModel = nullptr;
    DAVA::TArc::ContextAccessor* accessor = nullptr;
};
