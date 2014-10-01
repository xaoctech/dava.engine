//
//  PackageTreeWidget.cpp
//  UIEditor
//
//  Created by Dmitry Belsky on 10.9.14.
//
//

#include "PackageDockWidget.h"
#include "ui_PackageDockWidget.h"
#include "UIPackageModel.h"
#include "DAVAEngine.h"
#include "UI/PackageView/UIFilteredPackageModel.h"
#include "UI/PackageDocument.h"

#include "Project.h"

using namespace DAVA;

PackageDockWidget::PackageDockWidget(QWidget *parent)
    : QDockWidget(parent)
    , ui(new Ui::PackageDockWidget())
    , document(NULL)
{
    ui->setupUi(this);
    ui->treeView->header()->setResizeMode(QHeaderView::ResizeToContents);
    ui->treeView->setDefaultDropAction(Qt::MoveAction);

    connect(ui->filterLine, SIGNAL(textChanged(const QString &)), this, SLOT(filterTextChanged(const QString &)));

    //setTitleBarWidget(ui->filterLine);
    //ui->filterLine->setEnabled(false);
    //ui->treeView->setEnabled(false);
}

PackageDockWidget::~PackageDockWidget()
{
    disconnect(ui->filterLine, SIGNAL(textChanged(const QString &)), this, SLOT(filterTextChanged(const QString &)));
    ui->treeView->setModel(NULL);
    delete ui;
    ui = NULL;
}

void PackageDockWidget::SetDocument(PackageDocument *newDocument)
{
    if (document)
    {
        disconnect(ui->treeView->selectionModel(), SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)), this, SLOT(OnSelectionChanged(const QItemSelection &, const QItemSelection &)));
        ui->treeView->setModel(NULL);
        //ui->filterLine->setEnabled(false);
        //ui->treeView->setEnabled(false);
    }
    
    document = newDocument;
    
    if (document)
    {
        ui->treeView->setModel(document->GetTreeContext()->proxyModel);
        ui->filterLine->setText(document->GetTreeContext()->filterString);
        connect(ui->treeView->selectionModel(), SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)), this, SLOT(OnSelectionChanged(const QItemSelection &, const QItemSelection &)));
        //ui->filterLine->setEnabled(true);
        //ui->treeView->setEnabled(true);
    }
}

void PackageDockWidget::OnSelectionChanged(const QItemSelection &proxySelected, const QItemSelection &proxyDeselected)
{
    QList<UIControl *> selectedRootControl;
    QList<UIControl *> deselectedRootControl;
    
    QList<UIControl *> selectedControl;
    QList<UIControl *> deselectedControl;
    
    QItemSelection selected = document->GetTreeContext()->proxyModel->mapSelectionToSource(proxySelected);
    QItemSelection deselected = document->GetTreeContext()->proxyModel->mapSelectionToSource(proxyDeselected);
    
    QModelIndexList selectedIndexList = selected.indexes();
    if (!selectedIndexList.empty())
    {
        foreach(QModelIndex index, selectedIndexList)
        {
            UIControl *control = static_cast<UIControl*>(index.internalPointer());
            selectedControl.push_back(control);
            
            while(index.parent().isValid())
            {
                index = index.parent();
            }
            control = static_cast<UIControl*>(index.internalPointer());
            if (selectedRootControl.indexOf(control) < 0)
            {
                selectedRootControl.push_back(control);
            }
        }
    }
    
    QModelIndexList deselectedIndexList = deselected.indexes();
    if (!selectedIndexList.empty())
    {
        foreach(QModelIndex index, deselectedIndexList)
        {
            UIControl *control = static_cast<UIControl*>(index.internalPointer());
            deselectedControl.push_back(control);
            
            while(index.parent().isValid())
            {
                index = index.parent();
            }
            control = static_cast<UIControl*>(index.internalPointer());
            if (deselectedRootControl.indexOf(control) < 0)
            {
                deselectedRootControl.push_back(control);
            }
        }
    }

    emit SelectionRootControlChanged(selectedRootControl, deselectedRootControl);
    
    emit SelectionControlChanged(selectedControl, deselectedControl);
}

void PackageDockWidget::filterTextChanged(const QString &filterText)
{
    if (document)
    {
        document->GetTreeContext()->proxyModel->setFilterFixedString(filterText);
        ui->treeView->expandAll();
    }
}
