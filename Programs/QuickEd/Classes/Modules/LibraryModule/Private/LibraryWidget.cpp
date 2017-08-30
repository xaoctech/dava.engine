#include "Modules/LibraryModule/Private/LibraryWidget.h"
#include "Modules/LibraryModule/Private/LibraryModel.h"

#include <QTreeView>

#include <Base/Any.h>
#include <Reflection/ReflectedTypeDB.h>
#include <UI/UIControl.h>

LibraryWidget::LibraryWidget(DAVA::TArc::ContextAccessor* accessor, DAVA::TArc::UI* ui, QWidget* parent)
    : QWidget(parent)
    , libraryModel(new LibraryModel(this))
    , accessor(accessor)
{
    InitUI();
    treeView->setModel(libraryModel);
    libraryModel->Setup(ui, accessor);
}

LibraryWidget::~LibraryWidget() = default;

void LibraryWidget::InitUI()
{
    QVBoxLayout* verticalLayout = new QVBoxLayout(this);
    verticalLayout->setSpacing(5);
    verticalLayout->setContentsMargins(0, 0, 0, 0);

    treeView = new QTreeView(this);
    treeView->setDragEnabled(true);
    treeView->setDragDropMode(QAbstractItemView::DragOnly);
    treeView->setDefaultDropAction(Qt::CopyAction);
    treeView->header()->setVisible(false);

    verticalLayout->addWidget(treeView);
}

void LibraryWidget::SetLibraryPackages(const DAVA::Vector<DAVA::RefPtr<PackageNode>>& projectLibraries)
{
    libraryModel->SetLibraryPackages(projectLibraries);
}

void LibraryWidget::SetCurrentPackage(PackageNode* package)
{
    libraryModel->SetPackageNode(package);
    treeView->setEnabled(package != nullptr);
    treeView->expandAll();
    treeView->collapse(libraryModel->GetDefaultControlsModelIndex());
}
